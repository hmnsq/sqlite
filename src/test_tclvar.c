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

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_مقدار_GLOB   'g'
#define TCLVAR_مقدار_REGEXP 'r'
#define TCLVAR_مقدار_LIKE   'l'

تعریفنوع ساختار tclvar_vtab tclvar_vtab;
تعریفنوع ساختار tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ساختار tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ساختار tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ایستا صحیح tclvarConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  tclvar_vtab *pVtab;
  ایستا ثابت نویسه zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*مقدار*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( اندازه(*pVtab) );
  اگر( pVtab==0 ) بازگشت SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  بازگشت SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ایستا صحیح tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ایستا صحیح tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(اندازه(tclvar_cursor));
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ایستا صحیح tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  اگر( pCur->pList1 ){
    Tcl_Decr_Ref_شمار(pCur->pList1);
  }
  اگر( pCur->pList2 ){
    Tcl_Decr_Ref_شمار(pCur->pList2);
  }
  sqlite3_free(pCur);
  بازگشت SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ایستا صحیح next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  اگر( pObj ){
    اگر( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_شمار(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_شمار(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_شمار(pCur->pList2);
      أكد( pCur->i2==0 );
    }وگرنه{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      اگر( pCur->i2>=n ){
        Tcl_Decr_Ref_شمار(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        بازگشت 0;
      }
    }
  }

  بازگشت 1;
}

ایستا صحیح tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  صحیح ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  وقتی( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    اگر( !ok ){
      pCur->i1++;
    }
  }

  بازگشت 0;
}

ایستا صحیح tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ثابت نویسه *zEq = "";
  ثابت نویسه *zMatch = "";
  ثابت نویسه *zGlob = "";
  ثابت نویسه *zRegexp = "";
  ثابت نویسه *zLike = "";
  صحیح i;

  برای(i=0; idxStr[i]; i++){
    انتخاب( idxStr[i] ){
      حالت TCLVAR_NAME_EQ:
        zEq = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
        بشکن;
      حالت TCLVAR_NAME_MATCH:
        zMatch = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
        بشکن;
      حالت TCLVAR_مقدار_GLOB:
        zGlob = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
        بشکن;
      حالت TCLVAR_مقدار_REGEXP:
        zRegexp = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
        بشکن;
      حالت TCLVAR_مقدار_LIKE:
        zLike = (ثابت نویسه*)sqlite3_مقدار_text(argv[i]);
        بشکن;
      پیشفرض:
        أكد( 0 );
    }
  }

  Tcl_Incr_Ref_شمار(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  اگر( pCur->pList1 ){
    Tcl_Decr_Ref_شمار(pCur->pList1);
  }
  اگر( pCur->pList2 ){
    Tcl_Decr_Ref_شمار(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_شمار(pCur->pList1);

  Tcl_Decr_Ref_شمار(p);
  بازگشت tclvarNext(pVtabCursor);
}

ایستا صحیح tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ثابت نویسه *z1; 
  ثابت نویسه *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  اگر( p2 ){
    z2 = Tcl_GetString(p2);
  }
  انتخاب (i) {
    حالت 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      بشکن;
    }
    حالت 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      بشکن;
    }
    حالت 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      بشکن;
    }
    حالت 3: {
      نویسه *z3;
      اگر( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }وگرنه{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      بشکن;
    }
  }
  بازگشت SQLITE_OK;
}

ایستا صحیح tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  بازگشت SQLITE_OK;
}

ایستا صحیح tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  بازگشت (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ایستا صحیح tclvarAddToIdxstr(نویسه *zStr, نویسه x){
  صحیح i;
  برای(i=0; zStr[i]; i++){
    اگر( zStr[i]==x ) بازگشت 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  بازگشت 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ایستا صحیح tclvarSetOmit(Tcl_Interp *interp){
  صحیح rc;
  صحیح res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  اگر( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  بازگشت (rc==TCL_OK && res);
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
ایستا صحیح tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  صحیح ii;
  نویسه *zStr = sqlite3_malloc(32);
  صحیح iStr = 0;

  اگر( zStr==0 ) بازگشت SQLITE_NOMEM;
  zStr[0] = '\0';

  برای(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساختار sqlite3_index_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    ساختار sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    اگر( pCons->usable ){
      /* name = ? */
      اگر( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        اگر( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      اگر( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        اگر( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      اگر( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        اگر( 0==tclvarAddToIdxstr(zStr, TCLVAR_مقدار_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      اگر( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        اگر( 0==tclvarAddToIdxstr(zStr, TCLVAR_مقدار_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      اگر( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        اگر( 0==tclvarAddToIdxstr(zStr, TCLVAR_مقدار_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  بازگشت SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ایستا صحیح tclvarUpdate(
  sqlite3_vtab *tab,
  صحیح argc,
  sqlite3_مقدار **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  اگر( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ثابت نویسه *zVar = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    بازگشت SQLITE_OK;
  }
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ثابت نویسه *z_مقدار = (ثابت نویسه*)sqlite3_مقدار_text(argv[4]);
    ثابت نویسه *zName;
    اگر( sqlite3_مقدار_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_پیام = sqlite3_mprintf("the 'fullname' column must be TEXT");
      بازگشت SQLITE_ERROR;
    }
    zName = (ثابت نویسه*)sqlite3_مقدار_text(argv[5]);
    اگر( z_مقدار ){
      Tcl_SetVar(pTab->interp, zName, z_مقدار, TCL_GLOBAL_ONLY);
    }وگرنه{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    بازگشت SQLITE_OK;
  }
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_TEXT
   && sqlite3_مقدار_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    ثابت نویسه *zOldName = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    ثابت نویسه *zNewName = (ثابت نویسه*)sqlite3_مقدار_text(argv[1]);
    ثابت نویسه *z_مقدار = (ثابت نویسه*)sqlite3_مقدار_text(argv[4]);

    اگر( strcmp(zOldName, zNewName)!=0 || z_مقدار==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    اگر( z_مقدار!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_مقدار, TCL_GLOBAL_ONLY);
    }
    بازگشت SQLITE_OK;
  }
  tab->z_Err_پیام = sqlite3_mprintf("prohibited TCL variable change");
  بازگشت SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ایستا sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvarBestIndex,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvarNext,                  /* xNext - advance a cursor */
  tclvarEof,                   /* xEof - check for end of scan */
  tclvarColumn,                /* xColumn - read data */
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
بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ایستا صحیح SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  صحیح rc = TCL_OK;
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (تهی*)interp);
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
#endif
  بازگشت rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
     تهی *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  بازگشت TCL_OK;
}
