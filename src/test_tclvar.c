/*
** 2006 June 13
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the virtual table interfaces.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
**
** The emphasis of this file is a virtual table that provides
** access to TCL variables.
**
** The TCLVAR eponymous virtual table has a schema like this:
**
**    CREATE TABLE tclvar(
**       name TEXT,       -- base name of the variable:  "x" in "$x(y)"
**       arrayname TEXT,  -- array index name: "y" in "$x(y)"
**       value TEXT,      -- the value of the variable 
**       fullname TEXT,   -- the full name of the variable
**       PRIMARY KEY(fullname)
**    ) WITHOUT ROWID;
**
** DELETE, INSERT, and UPDATE operations use the "fullname" field to
** determine the variable to be modified.  Changing "value" to NULL
** deletes the variable.
**
** For SELECT operations, the "name" and "arrayname" fields will always
** match the "fullname" field.  For DELETE, INSERT, and UPDATE, the
** "name" and "arrayname" fields are ignored and the variable is modified
** according to "fullname" and "value" only.
*/
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>

#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#تعریف_کن TCLVAR_NAME_EQ      'e'
#تعریف_کن TCLVAR_NAME_تطابق   'm'
#تعریف_کن TCLVAR_مقدار_GLOB   'g'
#تعریف_کن TCLVAR_مقدار_REGEXP 'r'
#تعریف_کن TCLVAR_مقدار_LIKE   'l'

تعريفنوع بنية tclvar_vtab tclvar_vtab;
تعريفنوع بنية tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
بنية tclvar_vtab {
  sqlite3_vtab پایه;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
بنية tclvar_cursor {
  sqlite3_vtab_cursor پایه;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_اندازه i1;              /* Current item in pList1 */
  Tcl_اندازه i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ساكن صحيح tclvarConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  tclvar_vtab *pVtab;
  ساكن ثابت حرف zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*مقدار*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( حجم(*pVtab) );
  إذا( pVtab==0 ) أرجع SQLITE_NOMEM;
  *ppVtab = &pVtab->پایه;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  أرجع SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ساكن صحيح tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ساكن صحيح tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(حجم(tclvar_cursor));
  *ppCursor = &pCur->پایه;
  أرجع SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ساكن صحيح tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  إذا( pCur->pList1 ){
    Tcl_Decr_Ref_شمار(pCur->pList1);
  }
  إذا( pCur->pList2 ){
    Tcl_Decr_Ref_شمار(pCur->pList2);
  }
  sqlite3_free(pCur);
  أرجع SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ساكن صحيح next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  إذا( pObj ){
    إذا( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_شمار(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_شمار(p);
      pCur->pList2 = Tcl_Get_Obj_نتیجه(interp);
      Tcl_Incr_Ref_شمار(pCur->pList2);
      أكد( pCur->i2==0 );
    }وإلا{
      Tcl_اندازه n = 0;
      pCur->i2++;
      Tcl_List_Obj_درازی(0, pCur->pList2, &n);
      إذا( pCur->i2>=n ){
        Tcl_Decr_Ref_شمار(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        أرجع 0;
      }
    }
  }

  أرجع 1;
}

ساكن صحيح tclvar_بعدی(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_اندازه n = 0;
  صحيح ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_درازی(0, pCur->pList1, &n);
  طالما( !ok && pCur->i1<n ){
    Tcl_List_Obj_شاخص(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    إذا( !ok ){
      pCur->i1++;
    }
  }

  أرجع 0;
}

ساكن صحيح tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_رقم, ثابت حرف *idxStr,
  صحيح argc, sqlite3_مقدار **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ثابت حرف *zEq = "";
  ثابت حرف *z_تطابق = "";
  ثابت حرف *zGlob = "";
  ثابت حرف *zRegexp = "";
  ثابت حرف *zLike = "";
  صحيح i;

  لكل(i=0; idxStr[i]; i++){
    بدل( idxStr[i] ){
      حالة TCLVAR_NAME_EQ:
        zEq = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]);
        اكسر;
      حالة TCLVAR_NAME_تطابق:
        z_تطابق = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]);
        اكسر;
      حالة TCLVAR_مقدار_GLOB:
        zGlob = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]);
        اكسر;
      حالة TCLVAR_مقدار_REGEXP:
        zRegexp = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]);
        اكسر;
      حالة TCLVAR_مقدار_LIKE:
        zLike = (ثابت حرف*)sqlite3_مقدار_متن(argv[i]);
        اكسر;
      افتراضي:
        أكد( 0 );
    }
  }

  Tcl_Incr_Ref_شمار(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(z_تطابق, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  إذا( pCur->pList1 ){
    Tcl_Decr_Ref_شمار(pCur->pList1);
  }
  إذا( pCur->pList2 ){
    Tcl_Decr_Ref_شمار(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_نتیجه(interp);
  Tcl_Incr_Ref_شمار(pCur->pList1);

  Tcl_Decr_Ref_شمار(p);
  أرجع tclvar_بعدی(pVtabCursor);
}

ساكن صحيح tclvar_ستونی(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ثابت حرف *z1; 
  ثابت حرف *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_شاخص(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_شاخص(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  إذا( p2 ){
    z2 = Tcl_GetString(p2);
  }
  بدل (i) {
    حالة 0: {
      sqlite3_نتیجه_متن(ctx, z1, -1, SQLITE_TRANSIENT);
      اكسر;
    }
    حالة 1: {
      sqlite3_نتیجه_متن(ctx, z2, -1, SQLITE_TRANSIENT);
      اكسر;
    }
    حالة 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_نتیجه_متن(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      اكسر;
    }
    حالة 3: {
      حرف *z3;
      إذا( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_نتیجه_متن(ctx, z3, -1, sqlite3_free);
      }وإلا{
        sqlite3_نتیجه_متن(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      اكسر;
    }
  }
  أرجع SQLITE_OK;
}

ساكن صحيح tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  أرجع SQLITE_OK;
}

ساكن صحيح tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  أرجع (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ساكن صحيح tclvar_جمع_کن_مقصد_Idxstr(حرف *zStr, حرف x){
  صحيح i;
  لكل(i=0; zStr[i]; i++){
    إذا( zStr[i]==x ) أرجع 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  أرجع 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ساكن صحيح tclvar_مجموعه_Omit(Tcl_Interp *interp){
  صحيح rc;
  صحيح res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  إذا( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_نتیجه(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  أرجع (rc==TCL_OK && res);
}

/*
** The xBestIndex() method. This virtual table supports the following
** operators:
**
**     name = ?                    (omit flag clear)
**     name MATCH ?                (omit flag set)
**     value GLOB ?                (omit flag set iff $::tclvar_set_omit)
**     value REGEXP ?              (omit flag set iff $::tclvar_set_omit)
**     value LIKE ?                (omit flag set iff $::tclvar_set_omit)
**
** For each constraint present, the corresponding TCLVAR_XXX character is
** appended to the idxStr value. 
*/
ساكن صحيح tclvar_بهترین_شاخص(sqlite3_vtab *tab, sqlite3_شاخص_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  صحيح ii;
  حرف *zStr = sqlite3_malloc(32);
  صحيح iStr = 0;

  إذا( zStr==0 ) أرجع SQLITE_NOMEM;
  zStr[0] = '\0';

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_شاخص_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    بنية sqlite3_شاخص_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    إذا( pCons->usable ){
      /* name = ? */
      إذا( pCons->op==SQLITE_شاخص_CONSTRAINT_EQ && pCons->i_ستونی==0 ){
        إذا( 0==tclvar_جمع_کن_مقصد_Idxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_شاخص = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      إذا( pCons->op==SQLITE_شاخص_CONSTRAINT_تطابق && pCons->i_ستونی==0 ){
        إذا( 0==tclvar_جمع_کن_مقصد_Idxstr(zStr, TCLVAR_NAME_تطابق) ){
          pUsage->argv_شاخص = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      إذا( pCons->op==SQLITE_شاخص_CONSTRAINT_GLOB && pCons->i_ستونی==2 ){
        إذا( 0==tclvar_جمع_کن_مقصد_Idxstr(zStr, TCLVAR_مقدار_GLOB) ){
          pUsage->argv_شاخص = ++iStr;
          pUsage->omit = tclvar_مجموعه_Omit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      إذا( pCons->op==SQLITE_شاخص_CONSTRAINT_REGEXP && pCons->i_ستونی==2 ){
        إذا( 0==tclvar_جمع_کن_مقصد_Idxstr(zStr, TCLVAR_مقدار_REGEXP) ){
          pUsage->argv_شاخص = ++iStr;
          pUsage->omit = tclvar_مجموعه_Omit(pTab->interp);
        }
      }

      /* value LIKE ? */
      إذا( pCons->op==SQLITE_شاخص_CONSTRAINT_LIKE && pCons->i_ستونی==2 ){
        إذا( 0==tclvar_جمع_کن_مقصد_Idxstr(zStr, TCLVAR_مقدار_LIKE) ){
          pUsage->argv_شاخص = ++iStr;
          pUsage->omit = tclvar_مجموعه_Omit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->need_مقصد_Free_Idx_Str = 1;

  أرجع SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ساكن صحيح tclvarUpdate(
  sqlite3_vtab *tab,
  صحيح argc,
  sqlite3_مقدار **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  إذا( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ثابت حرف *zVar = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    أرجع SQLITE_OK;
  }
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ثابت حرف *z_مقدار = (ثابت حرف*)sqlite3_مقدار_متن(argv[4]);
    ثابت حرف *zName;
    إذا( sqlite3_مقدار_type(argv[5])!=SQLITE_متن ){
      tab->z_Err_پیام = sqlite3_mprintf("the 'fullname' column must be TEXT");
      أرجع SQLITE_ERROR;
    }
    zName = (ثابت حرف*)sqlite3_مقدار_متن(argv[5]);
    إذا( z_مقدار ){
      Tcl_مجموعه_Var(pTab->interp, zName, z_مقدار, TCL_GLOBAL_ONLY);
    }وإلا{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    أرجع SQLITE_OK;
  }
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_متن
   && sqlite3_مقدار_type(argv[1])==SQLITE_متن
  ){
    /* An UPDATE operation */
    ثابت حرف *zOldName = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
    ثابت حرف *zNewName = (ثابت حرف*)sqlite3_مقدار_متن(argv[1]);
    ثابت حرف *z_مقدار = (ثابت حرف*)sqlite3_مقدار_متن(argv[4]);

    إذا( strcmp(zOldName, zNewName)!=0 || z_مقدار==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    إذا( z_مقدار!=0 ){
      Tcl_مجموعه_Var(pTab->interp, zNewName, z_مقدار, TCL_GLOBAL_ONLY);
    }
    أرجع SQLITE_OK;
  }
  tab->z_Err_پیام = sqlite3_mprintf("prohibited TCL variable change");
  أرجع SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ساكن sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_بهترین_شاخص,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_بعدی,                  /* xNext - advance a cursor */
  tclvarEof,                   /* xEof - check for end of scan */
  tclvar_ستونی,                /* xColumn - read data */
  tclvarRowid,                 /* xRowid - read data */
  tclvarUpdate,                /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  0,                           /* xFindMethod */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};

/*
** Decode a pointer to an sqlite3 object.
*/
خارجي صحيح get_Db_اشاره_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI ثبات_tclvar_module(
  Client_داده client_داده, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحيح rc = TCL_OK;
  sqlite3 *db;
  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (فراغ*)interp);
  rc = Tcl_Eval(interp, 
      "proc like {pattern str} {\n"
      "  set p [string map {% * _ ?} $pattern]\n"
      "  string match $p $str\n"
      "}\n"
      "proc tclvar_filter_cmd {eq match glob regexp like} {\n"
      "  set res {}\n"
      "  set pattern $eq\n"
      "  if {$pattern=={}} { set pattern $match }\n"
      "  if {$pattern=={}} { set pattern * }\n"
      "  foreach v [uplevel #0 info vars $pattern] {\n"
      "    if {($glob=={} || [string match $glob [uplevel #0 set $v]])\n"
      "     && ($like=={} || [like $like [uplevel #0 set $v]])\n"
      "     && ($regexp=={} || [regexp $regexp [uplevel #0 set $v]])\n"
      "    } {\n"
      "      lappend res $v\n"
      "    }\n"
      "  }\n"
      "  set res\n"
      "}\n"
  );
#پایان_اگر
  أرجع rc;
}

#پایان_اگر


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetesttclvar_Init(Tcl_Interp *interp){
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *client_داده;
  } aObjCmd[] = {
     { "register_tclvar_module",   ثبات_tclvar_module, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_داده, 0);
  }
#پایان_اگر
  أرجع TCL_OK;
}
