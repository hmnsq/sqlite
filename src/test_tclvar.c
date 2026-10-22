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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#تعریف_کرو TCLVAR_NAME_EQ      'e'
#تعریف_کرو TCLVAR_NAME_مطابقت   'm'
#تعریف_کرو TCLVAR_قدر_GLOB   'g'
#تعریف_کرو TCLVAR_قدر_REGEXP 'r'
#تعریف_کرو TCLVAR_قدر_LIKE   'l'

قسمتعریف ساخت tclvar_vtab tclvar_vtab;
قسمتعریف ساخت tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ساخت tclvar_vtab {
  sqlite3_vtab بنیاد;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ساخت tclvar_cursor {
  sqlite3_vtab_cursor بنیاد;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_سائز i1;              /* Current item in pList1 */
  Tcl_سائز i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ساکن صحیح tclvarConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  tclvar_vtab *pVtab;
  ساکن ثابت حرف zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*قدر*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( حجم(*pVtab) );
  اگر( pVtab==0 ) واپس SQLITE_NOMEM;
  *ppVtab = &pVtab->بنیاد;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  واپس SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ساکن صحیح tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ساکن صحیح tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(حجم(tclvar_cursor));
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ساکن صحیح tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  اگر( pCur->pList1 ){
    Tcl_Decr_Ref_گنتی(pCur->pList1);
  }
  اگر( pCur->pList2 ){
    Tcl_Decr_Ref_گنتی(pCur->pList2);
  }
  sqlite3_free(pCur);
  واپس SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ساکن صحیح next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  اگر( pObj ){
    اگر( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_گنتی(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_گنتی(p);
      pCur->pList2 = Tcl_Get_Obj_نتیجہ(interp);
      Tcl_Incr_Ref_گنتی(pCur->pList2);
      أكد( pCur->i2==0 );
    }ورنہ{
      Tcl_سائز n = 0;
      pCur->i2++;
      Tcl_List_Obj_لمبائی(0, pCur->pList2, &n);
      اگر( pCur->i2>=n ){
        Tcl_Decr_Ref_گنتی(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        واپس 0;
      }
    }
  }

  واپس 1;
}

ساکن صحیح tclvar_اگلا(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_سائز n = 0;
  صحیح ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_لمبائی(0, pCur->pList1, &n);
  جب( !ok && pCur->i1<n ){
    Tcl_List_Obj_اشاریہ(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    اگر( !ok ){
      pCur->i1++;
    }
  }

  واپس 0;
}

ساکن صحیح tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ثابت حرف *zEq = "";
  ثابت حرف *z_مطابقت = "";
  ثابت حرف *zGlob = "";
  ثابت حرف *zRegexp = "";
  ثابت حرف *zLike = "";
  صحیح i;

  برائے(i=0; idxStr[i]; i++){
    انتخاب( idxStr[i] ){
      حالت TCLVAR_NAME_EQ:
        zEq = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
        توڑو;
      حالت TCLVAR_NAME_مطابقت:
        z_مطابقت = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
        توڑو;
      حالت TCLVAR_قدر_GLOB:
        zGlob = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
        توڑو;
      حالت TCLVAR_قدر_REGEXP:
        zRegexp = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
        توڑو;
      حالت TCLVAR_قدر_LIKE:
        zLike = (ثابت حرف*)sqlite3_قدر_متن(argv[i]);
        توڑو;
      طےشدہ:
        أكد( 0 );
    }
  }

  Tcl_Incr_Ref_گنتی(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(z_مطابقت, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  اگر( pCur->pList1 ){
    Tcl_Decr_Ref_گنتی(pCur->pList1);
  }
  اگر( pCur->pList2 ){
    Tcl_Decr_Ref_گنتی(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_نتیجہ(interp);
  Tcl_Incr_Ref_گنتی(pCur->pList1);

  Tcl_Decr_Ref_گنتی(p);
  واپس tclvar_اگلا(pVtabCursor);
}

ساکن صحیح tclvar_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ثابت حرف *z1; 
  ثابت حرف *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_اشاریہ(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_اشاریہ(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  اگر( p2 ){
    z2 = Tcl_GetString(p2);
  }
  انتخاب (i) {
    حالت 0: {
      sqlite3_نتیجہ_متن(ctx, z1, -1, SQLITE_TRANSIENT);
      توڑو;
    }
    حالت 1: {
      sqlite3_نتیجہ_متن(ctx, z2, -1, SQLITE_TRANSIENT);
      توڑو;
    }
    حالت 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_نتیجہ_متن(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      توڑو;
    }
    حالت 3: {
      حرف *z3;
      اگر( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_نتیجہ_متن(ctx, z3, -1, sqlite3_free);
      }ورنہ{
        sqlite3_نتیجہ_متن(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      توڑو;
    }
  }
  واپس SQLITE_OK;
}

ساکن صحیح tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  واپس SQLITE_OK;
}

ساکن صحیح tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  واپس (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ساکن صحیح tclvar_جوڑو_مقصد_Idxstr(حرف *zStr, حرف x){
  صحیح i;
  برائے(i=0; zStr[i]; i++){
    اگر( zStr[i]==x ) واپس 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  واپس 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ساکن صحیح tclvar_سیٹ_Omit(Tcl_Interp *interp){
  صحیح rc;
  صحیح res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  اگر( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_نتیجہ(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  واپس (rc==TCL_OK && res);
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
ساکن صحیح tclvar_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  صحیح ii;
  حرف *zStr = sqlite3_malloc(32);
  صحیح iStr = 0;

  اگر( zStr==0 ) واپس SQLITE_NOMEM;
  zStr[0] = '\0';

  برائے(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساخت sqlite3_اشاریہ_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    ساخت sqlite3_اشاریہ_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    اگر( pCons->usable ){
      /* name = ? */
      اگر( pCons->op==SQLITE_اشاریہ_CONSTRAINT_EQ && pCons->i_عمود==0 ){
        اگر( 0==tclvar_جوڑو_مقصد_Idxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_اشاریہ = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      اگر( pCons->op==SQLITE_اشاریہ_CONSTRAINT_مطابقت && pCons->i_عمود==0 ){
        اگر( 0==tclvar_جوڑو_مقصد_Idxstr(zStr, TCLVAR_NAME_مطابقت) ){
          pUsage->argv_اشاریہ = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      اگر( pCons->op==SQLITE_اشاریہ_CONSTRAINT_GLOB && pCons->i_عمود==2 ){
        اگر( 0==tclvar_جوڑو_مقصد_Idxstr(zStr, TCLVAR_قدر_GLOB) ){
          pUsage->argv_اشاریہ = ++iStr;
          pUsage->omit = tclvar_سیٹ_Omit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      اگر( pCons->op==SQLITE_اشاریہ_CONSTRAINT_REGEXP && pCons->i_عمود==2 ){
        اگر( 0==tclvar_جوڑو_مقصد_Idxstr(zStr, TCLVAR_قدر_REGEXP) ){
          pUsage->argv_اشاریہ = ++iStr;
          pUsage->omit = tclvar_سیٹ_Omit(pTab->interp);
        }
      }

      /* value LIKE ? */
      اگر( pCons->op==SQLITE_اشاریہ_CONSTRAINT_LIKE && pCons->i_عمود==2 ){
        اگر( 0==tclvar_جوڑو_مقصد_Idxstr(zStr, TCLVAR_قدر_LIKE) ){
          pUsage->argv_اشاریہ = ++iStr;
          pUsage->omit = tclvar_سیٹ_Omit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->need_مقصد_Free_Idx_Str = 1;

  واپس SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ساکن صحیح tclvarUpdate(
  sqlite3_vtab *tab,
  صحیح argc,
  sqlite3_قدر **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  اگر( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ثابت حرف *zVar = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    واپس SQLITE_OK;
  }
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ثابت حرف *z_قدر = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);
    ثابت حرف *zName;
    اگر( sqlite3_قدر_type(argv[5])!=SQLITE_متن ){
      tab->z_Err_پیغام = sqlite3_mprintf("the 'fullname' column must be TEXT");
      واپس SQLITE_ERROR;
    }
    zName = (ثابت حرف*)sqlite3_قدر_متن(argv[5]);
    اگر( z_قدر ){
      Tcl_سیٹ_Var(pTab->interp, zName, z_قدر, TCL_GLOBAL_ONLY);
    }ورنہ{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    واپس SQLITE_OK;
  }
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_متن
   && sqlite3_قدر_type(argv[1])==SQLITE_متن
  ){
    /* An UPDATE operation */
    ثابت حرف *zOldName = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    ثابت حرف *zNewName = (ثابت حرف*)sqlite3_قدر_متن(argv[1]);
    ثابت حرف *z_قدر = (ثابت حرف*)sqlite3_قدر_متن(argv[4]);

    اگر( strcmp(zOldName, zNewName)!=0 || z_قدر==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    اگر( z_قدر!=0 ){
      Tcl_سیٹ_Var(pTab->interp, zNewName, z_قدر, TCL_GLOBAL_ONLY);
    }
    واپس SQLITE_OK;
  }
  tab->z_Err_پیغام = sqlite3_mprintf("prohibited TCL variable change");
  واپس SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ساکن sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_بہترین_اشاریہ,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_اگلا,                  /* xNext - advance a cursor */
  tclvarEof,                   /* xEof - check for end of scan */
  tclvar_عمود,                /* xColumn - read data */
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
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ساکن صحیح SQLITE_TCLAPI رجسٹر_tclvar_module(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحیح rc = TCL_OK;
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (خالی*)interp);
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
#اگر_ختم
  واپس rc;
}

#اگر_ختم


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetesttclvar_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "register_tclvar_module",   رجسٹر_tclvar_module, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
#اگر_ختم
  واپس TCL_OK;
}
