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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#నిర్వచించు TCLVAR_NAME_EQ      'e'
#నిర్వచించు TCLVAR_NAME_సరిపోలిక   'm'
#నిర్వచించు TCLVAR_విలువ_GLOB   'g'
#నిర్వచించు TCLVAR_విలువ_REGEXP 'r'
#నిర్వచించు TCLVAR_విలువ_LIKE   'l'

రకనిర్వచనం నిర్మాణం tclvar_vtab tclvar_vtab;
రకనిర్వచనం నిర్మాణం tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
నిర్మాణం tclvar_vtab {
  sqlite3_vtab పునాది;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
నిర్మాణం tclvar_cursor {
  sqlite3_vtab_cursor పునాది;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_కొలత i1;              /* Current item in pList1 */
  Tcl_కొలత i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
స్థిర పూర్ణాంకం tclvarConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  tclvar_vtab *pVtab;
  స్థిర స్థిరరాశి అక్షరం zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*విలువ*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( పరిమాణం(*pVtab) );
  ఒకవేళ( pVtab==0 ) తిరిగి SQLITE_NOMEM;
  *ppVtab = &pVtab->పునాది;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  తిరిగి SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

స్థిర పూర్ణాంకం tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
స్థిర పూర్ణాంకం tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(పరిమాణం(tclvar_cursor));
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
స్థిర పూర్ణాంకం tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  ఒకవేళ( pCur->pList1 ){
    Tcl_Decr_Ref_లెక్క(pCur->pList1);
  }
  ఒకవేళ( pCur->pList2 ){
    Tcl_Decr_Ref_లెక్క(pCur->pList2);
  }
  sqlite3_free(pCur);
  తిరిగి SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
స్థిర పూర్ణాంకం next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  ఒకవేళ( pObj ){
    ఒకవేళ( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_లెక్క(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_లెక్క(p);
      pCur->pList2 = Tcl_Get_Obj_ఫలితం(interp);
      Tcl_Incr_Ref_లెక్క(pCur->pList2);
      ప్రతిజ్ఞా( pCur->i2==0 );
    }లేకపోతే{
      Tcl_కొలత n = 0;
      pCur->i2++;
      Tcl_List_Obj_నిడివి(0, pCur->pList2, &n);
      ఒకవేళ( pCur->i2>=n ){
        Tcl_Decr_Ref_లెక్క(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        తిరిగి 0;
      }
    }
  }

  తిరిగి 1;
}

స్థిర పూర్ణాంకం tclvar_తదుపరి(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_కొలత n = 0;
  పూర్ణాంకం ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_నిడివి(0, pCur->pList1, &n);
  అయితే( !ok && pCur->i1<n ){
    Tcl_List_Obj_సూచీ(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    ఒకవేళ( !ok ){
      pCur->i1++;
    }
  }

  తిరిగి 0;
}

స్థిర పూర్ణాంకం tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  స్థిరరాశి అక్షరం *zEq = "";
  స్థిరరాశి అక్షరం *z_సరిపోలిక = "";
  స్థిరరాశి అక్షరం *zGlob = "";
  స్థిరరాశి అక్షరం *zRegexp = "";
  స్థిరరాశి అక్షరం *zLike = "";
  పూర్ణాంకం i;

  కొరకు(i=0; idxStr[i]; i++){
    ఎంపిక( idxStr[i] ){
      స్థితి TCLVAR_NAME_EQ:
        zEq = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
        ఆపు;
      స్థితి TCLVAR_NAME_సరిపోలిక:
        z_సరిపోలిక = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
        ఆపు;
      స్థితి TCLVAR_విలువ_GLOB:
        zGlob = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
        ఆపు;
      స్థితి TCLVAR_విలువ_REGEXP:
        zRegexp = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
        ఆపు;
      స్థితి TCLVAR_విలువ_LIKE:
        zLike = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
        ఆపు;
      అప్రమేయ:
        ప్రతిజ్ఞా( 0 );
    }
  }

  Tcl_Incr_Ref_లెక్క(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(z_సరిపోలిక, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  ఒకవేళ( pCur->pList1 ){
    Tcl_Decr_Ref_లెక్క(pCur->pList1);
  }
  ఒకవేళ( pCur->pList2 ){
    Tcl_Decr_Ref_లెక్క(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_ఫలితం(interp);
  Tcl_Incr_Ref_లెక్క(pCur->pList1);

  Tcl_Decr_Ref_లెక్క(p);
  తిరిగి tclvar_తదుపరి(pVtabCursor);
}

స్థిర పూర్ణాంకం tclvar_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  స్థిరరాశి అక్షరం *z1; 
  స్థిరరాశి అక్షరం *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_సూచీ(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_సూచీ(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  ఒకవేళ( p2 ){
    z2 = Tcl_GetString(p2);
  }
  ఎంపిక (i) {
    స్థితి 0: {
      sqlite3_ఫలితం_పాఠ్యం(ctx, z1, -1, SQLITE_TRANSIENT);
      ఆపు;
    }
    స్థితి 1: {
      sqlite3_ఫలితం_పాఠ్యం(ctx, z2, -1, SQLITE_TRANSIENT);
      ఆపు;
    }
    స్థితి 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_ఫలితం_పాఠ్యం(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      ఆపు;
    }
    స్థితి 3: {
      అక్షరం *z3;
      ఒకవేళ( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_ఫలితం_పాఠ్యం(ctx, z3, -1, sqlite3_free);
      }లేకపోతే{
        sqlite3_ఫలితం_పాఠ్యం(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      ఆపు;
    }
  }
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  తిరిగి (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
స్థిర పూర్ణాంకం tclvar_కూడు_గమ్యం_Idxstr(అక్షరం *zStr, అక్షరం x){
  పూర్ణాంకం i;
  కొరకు(i=0; zStr[i]; i++){
    ఒకవేళ( zStr[i]==x ) తిరిగి 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  తిరిగి 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
స్థిర పూర్ణాంకం tclvar_సమితి_Omit(Tcl_Interp *interp){
  పూర్ణాంకం rc;
  పూర్ణాంకం res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  ఒకవేళ( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_ఫలితం(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  తిరిగి (rc==TCL_OK && res);
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
స్థిర పూర్ణాంకం tclvar_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  పూర్ణాంకం ii;
  అక్షరం *zStr = sqlite3_malloc(32);
  పూర్ణాంకం iStr = 0;

  ఒకవేళ( zStr==0 ) తిరిగి SQLITE_NOMEM;
  zStr[0] = '\0';

  కొరకు(ii=0; ii<pIdxInfo->nConstraint; ii++){
    నిర్మాణం sqlite3_సూచీ_constraint స్థిరరాశి *pCons = &pIdxInfo->aConstraint[ii];
    నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    ఒకవేళ( pCons->usable ){
      /* name = ? */
      ఒకవేళ( pCons->op==SQLITE_సూచీ_CONSTRAINT_EQ && pCons->i_స్తంభం==0 ){
        ఒకవేళ( 0==tclvar_కూడు_గమ్యం_Idxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_సూచీ = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      ఒకవేళ( pCons->op==SQLITE_సూచీ_CONSTRAINT_సరిపోలిక && pCons->i_స్తంభం==0 ){
        ఒకవేళ( 0==tclvar_కూడు_గమ్యం_Idxstr(zStr, TCLVAR_NAME_సరిపోలిక) ){
          pUsage->argv_సూచీ = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      ఒకవేళ( pCons->op==SQLITE_సూచీ_CONSTRAINT_GLOB && pCons->i_స్తంభం==2 ){
        ఒకవేళ( 0==tclvar_కూడు_గమ్యం_Idxstr(zStr, TCLVAR_విలువ_GLOB) ){
          pUsage->argv_సూచీ = ++iStr;
          pUsage->omit = tclvar_సమితి_Omit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      ఒకవేళ( pCons->op==SQLITE_సూచీ_CONSTRAINT_REGEXP && pCons->i_స్తంభం==2 ){
        ఒకవేళ( 0==tclvar_కూడు_గమ్యం_Idxstr(zStr, TCLVAR_విలువ_REGEXP) ){
          pUsage->argv_సూచీ = ++iStr;
          pUsage->omit = tclvar_సమితి_Omit(pTab->interp);
        }
      }

      /* value LIKE ? */
      ఒకవేళ( pCons->op==SQLITE_సూచీ_CONSTRAINT_LIKE && pCons->i_స్తంభం==2 ){
        ఒకవేళ( 0==tclvar_కూడు_గమ్యం_Idxstr(zStr, TCLVAR_విలువ_LIKE) ){
          pUsage->argv_సూచీ = ++iStr;
          pUsage->omit = tclvar_సమితి_Omit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->need_గమ్యం_Free_Idx_Str = 1;

  తిరిగి SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
స్థిర పూర్ణాంకం tclvarUpdate(
  sqlite3_vtab *tab,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ఒకవేళ( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    స్థిరరాశి అక్షరం *zVar = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    తిరిగి SQLITE_OK;
  }
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    స్థిరరాశి అక్షరం *z_విలువ = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[4]);
    స్థిరరాశి అక్షరం *zName;
    ఒకవేళ( sqlite3_విలువ_type(argv[5])!=SQLITE_పాఠ్యం ){
      tab->z_Err_సందేశం = sqlite3_mprintf("the 'fullname' column must be TEXT");
      తిరిగి SQLITE_ERROR;
    }
    zName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[5]);
    ఒకవేళ( z_విలువ ){
      Tcl_సమితి_Var(pTab->interp, zName, z_విలువ, TCL_GLOBAL_ONLY);
    }లేకపోతే{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    తిరిగి SQLITE_OK;
  }
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_పాఠ్యం
   && sqlite3_విలువ_type(argv[1])==SQLITE_పాఠ్యం
  ){
    /* An UPDATE operation */
    స్థిరరాశి అక్షరం *zOldName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    స్థిరరాశి అక్షరం *zNewName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[1]);
    స్థిరరాశి అక్షరం *z_విలువ = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[4]);

    ఒకవేళ( strcmp(zOldName, zNewName)!=0 || z_విలువ==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    ఒకవేళ( z_విలువ!=0 ){
      Tcl_సమితి_Var(pTab->interp, zNewName, z_విలువ, TCL_GLOBAL_ONLY);
    }
    తిరిగి SQLITE_OK;
  }
  tab->z_Err_సందేశం = sqlite3_mprintf("prohibited TCL variable change");
  తిరిగి SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
స్థిర sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_ఉత్తమ_సూచీ,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_తదుపరి,                  /* xNext - advance a cursor */
  tclvarEof,                   /* xEof - check for end of scan */
  tclvar_స్తంభం,                /* xColumn - read data */
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
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_tclvar_module(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  పూర్ణాంకం rc = TCL_OK;
  sqlite3 *db;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (శూన్య*)interp);
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
#అయితే_ముగింపు
  తిరిగి rc;
}

#అయితే_ముగింపు


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetesttclvar_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "register_tclvar_module",   రిజిస్టరు_tclvar_module, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}
