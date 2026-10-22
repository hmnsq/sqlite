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

प्रकारनिर्देश संरचना tclvar_vtab tclvar_vtab;
प्रकारनिर्देश संरचना tclvar_cursor tclvar_cursor;

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
स्थिर पूर्णाङ्क tclvarConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  tclvar_vtab *pVtab;
  स्थिर ध्रुव वर्ण zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*मूल्य*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( परिमाण(*pVtab) );
  यदि( pVtab==0 ) प्रत्यागम SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  प्रत्यागम SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

स्थिर पूर्णाङ्क tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
स्थिर पूर्णाङ्क tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(परिमाण(tclvar_cursor));
  *ppCursor = &pCur->base;
  प्रत्यागम SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
स्थिर पूर्णाङ्क tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  यदि( pCur->pList1 ){
    Tcl_Decr_Ref_गिनती(pCur->pList1);
  }
  यदि( pCur->pList2 ){
    Tcl_Decr_Ref_गिनती(pCur->pList2);
  }
  sqlite3_free(pCur);
  प्रत्यागम SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
स्थिर पूर्णाङ्क next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  यदि( pObj ){
    यदि( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_गिनती(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_गिनती(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_गिनती(pCur->pList2);
      प्रतिज्ञा( pCur->i2==0 );
    }अन्यथा{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      यदि( pCur->i2>=n ){
        Tcl_Decr_Ref_गिनती(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        प्रत्यागम 0;
      }
    }
  }

  प्रत्यागम 1;
}

स्थिर पूर्णाङ्क tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  पूर्णाङ्क ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  यावत्( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    यदि( !ok ){
      pCur->i1++;
    }
  }

  प्रत्यागम 0;
}

स्थिर पूर्णाङ्क tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ध्रुव वर्ण *zEq = "";
  ध्रुव वर्ण *zMatch = "";
  ध्रुव वर्ण *zGlob = "";
  ध्रुव वर्ण *zRegexp = "";
  ध्रुव वर्ण *zLike = "";
  पूर्णाङ्क i;

  प्रति(i=0; idxStr[i]; i++){
    विकल्प( idxStr[i] ){
      पक्ष TCLVAR_NAME_EQ:
        zEq = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[i]);
        भङ्ग;
      पक्ष TCLVAR_NAME_MATCH:
        zMatch = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[i]);
        भङ्ग;
      पक्ष TCLVAR_मूल्य_GLOB:
        zGlob = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[i]);
        भङ्ग;
      पक्ष TCLVAR_मूल्य_REGEXP:
        zRegexp = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[i]);
        भङ्ग;
      पक्ष TCLVAR_मूल्य_LIKE:
        zLike = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[i]);
        भङ्ग;
      स्वभाव:
        प्रतिज्ञा( 0 );
    }
  }

  Tcl_Incr_Ref_गिनती(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  यदि( pCur->pList1 ){
    Tcl_Decr_Ref_गिनती(pCur->pList1);
  }
  यदि( pCur->pList2 ){
    Tcl_Decr_Ref_गिनती(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_गिनती(pCur->pList1);

  Tcl_Decr_Ref_गिनती(p);
  प्रत्यागम tclvarNext(pVtabCursor);
}

स्थिर पूर्णाङ्क tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ध्रुव वर्ण *z1; 
  ध्रुव वर्ण *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  यदि( p2 ){
    z2 = Tcl_GetString(p2);
  }
  विकल्प (i) {
    पक्ष 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      भङ्ग;
    }
    पक्ष 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      भङ्ग;
    }
    पक्ष 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      भङ्ग;
    }
    पक्ष 3: {
      वर्ण *z3;
      यदि( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }अन्यथा{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      भङ्ग;
    }
  }
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  प्रत्यागम (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
स्थिर पूर्णाङ्क tclvarAddToIdxstr(वर्ण *zStr, वर्ण x){
  पूर्णाङ्क i;
  प्रति(i=0; zStr[i]; i++){
    यदि( zStr[i]==x ) प्रत्यागम 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  प्रत्यागम 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
स्थिर पूर्णाङ्क tclvarSetOmit(Tcl_Interp *interp){
  पूर्णाङ्क rc;
  पूर्णाङ्क res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  यदि( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  प्रत्यागम (rc==TCL_OK && res);
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
स्थिर पूर्णाङ्क tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  पूर्णाङ्क ii;
  वर्ण *zStr = sqlite3_malloc(32);
  पूर्णाङ्क iStr = 0;

  यदि( zStr==0 ) प्रत्यागम SQLITE_NOMEM;
  zStr[0] = '\0';

  प्रति(ii=0; ii<pIdxInfo->nConstraint; ii++){
    संरचना sqlite3_index_constraint ध्रुव *pCons = &pIdxInfo->aConstraint[ii];
    संरचना sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    यदि( pCons->usable ){
      /* name = ? */
      यदि( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        यदि( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      यदि( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        यदि( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      यदि( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        यदि( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      यदि( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        यदि( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      यदि( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        यदि( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  प्रत्यागम SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
स्थिर पूर्णाङ्क tclvarUpdate(
  sqlite3_vtab *tab,
  पूर्णाङ्क argc,
  sqlite3_मूल्य **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  यदि( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ध्रुव वर्ण *zVar = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    प्रत्यागम SQLITE_OK;
  }
  यदि( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ध्रुव वर्ण *z_मूल्य = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[4]);
    ध्रुव वर्ण *zName;
    यदि( sqlite3_मूल्य_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_संदेश = sqlite3_mprintf("the 'fullname' column must be TEXT");
      प्रत्यागम SQLITE_ERROR;
    }
    zName = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[5]);
    यदि( z_मूल्य ){
      Tcl_SetVar(pTab->interp, zName, z_मूल्य, TCL_GLOBAL_ONLY);
    }अन्यथा{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    प्रत्यागम SQLITE_OK;
  }
  यदि( sqlite3_मूल्य_type(argv[0])==SQLITE_TEXT
   && sqlite3_मूल्य_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    ध्रुव वर्ण *zOldName = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[0]);
    ध्रुव वर्ण *zNewName = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[1]);
    ध्रुव वर्ण *z_मूल्य = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[4]);

    यदि( म_भेद(zOldName, zNewName)!=0 || z_मूल्य==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    यदि( z_मूल्य!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_मूल्य, TCL_GLOBAL_ONLY);
    }
    प्रत्यागम SQLITE_OK;
  }
  tab->z_Err_संदेश = sqlite3_mprintf("prohibited TCL variable change");
  प्रत्यागम SQLITE_ERROR;
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
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णाङ्क rc = TCL_OK;
  sqlite3 *db;
  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
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
  प्रत्यागम rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  प्रत्यागम TCL_OK;
}
