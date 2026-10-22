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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_मूल्य_GLOB   'g'
#define TCLVAR_मूल्य_REGEXP 'r'
#define TCLVAR_मूल्य_LIKE   'l'

प्रकारव्याख्या संरचना tclvar_vtab tclvar_vtab;
प्रकारव्याख्या संरचना tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
संरचना tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
संरचना tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
स्थिर पूर्णांक tclvarConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णांक argc, स्थिरांक अक्षर *स्थिरांक*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  tclvar_vtab *pVtab;
  स्थिर स्थिरांक अक्षर zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*मूल्य*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( आकार(*pVtab) );
  जर( pVtab==0 ) परत SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  परत SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

स्थिर पूर्णांक tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  परत SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
स्थिर पूर्णांक tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(आकार(tclvar_cursor));
  *ppCursor = &pCur->base;
  परत SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
स्थिर पूर्णांक tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  जर( pCur->pList1 ){
    Tcl_Decr_Ref_मोजणी(pCur->pList1);
  }
  जर( pCur->pList2 ){
    Tcl_Decr_Ref_मोजणी(pCur->pList2);
  }
  sqlite3_free(pCur);
  परत SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
स्थिर पूर्णांक next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  जर( pObj ){
    जर( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_मोजणी(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_मोजणी(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_मोजणी(pCur->pList2);
      प्रतिज्ञा( pCur->i2==0 );
    }अन्यथा{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      जर( pCur->i2>=n ){
        Tcl_Decr_Ref_मोजणी(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        परत 0;
      }
    }
  }

  परत 1;
}

स्थिर पूर्णांक tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  पूर्णांक ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  पर्यंत( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    जर( !ok ){
      pCur->i1++;
    }
  }

  परत 0;
}

स्थिर पूर्णांक tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिरांक अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  स्थिरांक अक्षर *zEq = "";
  स्थिरांक अक्षर *zMatch = "";
  स्थिरांक अक्षर *zGlob = "";
  स्थिरांक अक्षर *zRegexp = "";
  स्थिरांक अक्षर *zLike = "";
  पूर्णांक i;

  साठी(i=0; idxStr[i]; i++){
    निवड( idxStr[i] ){
      स्थिती TCLVAR_NAME_EQ:
        zEq = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
        थांबा;
      स्थिती TCLVAR_NAME_MATCH:
        zMatch = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
        थांबा;
      स्थिती TCLVAR_मूल्य_GLOB:
        zGlob = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
        थांबा;
      स्थिती TCLVAR_मूल्य_REGEXP:
        zRegexp = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
        थांबा;
      स्थिती TCLVAR_मूल्य_LIKE:
        zLike = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[i]);
        थांबा;
      पूर्वनिर्धारित:
        प्रतिज्ञा( 0 );
    }
  }

  Tcl_Incr_Ref_मोजणी(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  जर( pCur->pList1 ){
    Tcl_Decr_Ref_मोजणी(pCur->pList1);
  }
  जर( pCur->pList2 ){
    Tcl_Decr_Ref_मोजणी(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_मोजणी(pCur->pList1);

  Tcl_Decr_Ref_मोजणी(p);
  परत tclvarNext(pVtabCursor);
}

स्थिर पूर्णांक tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  स्थिरांक अक्षर *z1; 
  स्थिरांक अक्षर *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  जर( p2 ){
    z2 = Tcl_GetString(p2);
  }
  निवड (i) {
    स्थिती 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      थांबा;
    }
    स्थिती 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      थांबा;
    }
    स्थिती 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      थांबा;
    }
    स्थिती 3: {
      अक्षर *z3;
      जर( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }अन्यथा{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      थांबा;
    }
  }
  परत SQLITE_OK;
}

स्थिर पूर्णांक tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  परत SQLITE_OK;
}

स्थिर पूर्णांक tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  परत (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
स्थिर पूर्णांक tclvarAddToIdxstr(अक्षर *zStr, अक्षर x){
  पूर्णांक i;
  साठी(i=0; zStr[i]; i++){
    जर( zStr[i]==x ) परत 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  परत 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
स्थिर पूर्णांक tclvarSetOmit(Tcl_Interp *interp){
  पूर्णांक rc;
  पूर्णांक res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  जर( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  परत (rc==TCL_OK && res);
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
स्थिर पूर्णांक tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  पूर्णांक ii;
  अक्षर *zStr = sqlite3_malloc(32);
  पूर्णांक iStr = 0;

  जर( zStr==0 ) परत SQLITE_NOMEM;
  zStr[0] = '\0';

  साठी(ii=0; ii<pIdxInfo->nConstraint; ii++){
    संरचना sqlite3_index_constraint स्थिरांक *pCons = &pIdxInfo->aConstraint[ii];
    संरचना sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    जर( pCons->usable ){
      /* name = ? */
      जर( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        जर( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      जर( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        जर( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      जर( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        जर( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      जर( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        जर( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      जर( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        जर( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  परत SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
स्थिर पूर्णांक tclvarUpdate(
  sqlite3_vtab *tab,
  पूर्णांक argc,
  sqlite3_मूल्य **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  जर( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    स्थिरांक अक्षर *zVar = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    परत SQLITE_OK;
  }
  जर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    स्थिरांक अक्षर *z_मूल्य = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[4]);
    स्थिरांक अक्षर *zName;
    जर( sqlite3_मूल्य_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_संदेश = sqlite3_mprintf("the 'fullname' column must be TEXT");
      परत SQLITE_ERROR;
    }
    zName = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[5]);
    जर( z_मूल्य ){
      Tcl_SetVar(pTab->interp, zName, z_मूल्य, TCL_GLOBAL_ONLY);
    }अन्यथा{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    परत SQLITE_OK;
  }
  जर( sqlite3_मूल्य_type(argv[0])==SQLITE_TEXT
   && sqlite3_मूल्य_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    स्थिरांक अक्षर *zOldName = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[0]);
    स्थिरांक अक्षर *zNewName = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[1]);
    स्थिरांक अक्षर *z_मूल्य = (स्थिरांक अक्षर*)sqlite3_मूल्य_text(argv[4]);

    जर( म_भेद(zOldName, zNewName)!=0 || z_मूल्य==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    जर( z_मूल्य!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_मूल्य, TCL_GLOBAL_ONLY);
    }
    परत SQLITE_OK;
  }
  tab->z_Err_संदेश = sqlite3_mprintf("prohibited TCL variable change");
  परत SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
स्थिर sqlite3_module tclvarModule = {
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
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिरांक अक्षर *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
स्थिर पूर्णांक SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णांक rc = TCL_OK;
  sqlite3 *db;
  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    परत TCL_ERROR;
  }
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) परत TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (शून्य*)interp);
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
  परत rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  स्थिर संरचना {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  पूर्णांक i;
  साठी(i=0; i<आकार(aObjCmd)/आकार(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  परत TCL_OK;
}
