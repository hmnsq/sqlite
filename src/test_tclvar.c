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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_মান_GLOB   'g'
#define TCLVAR_মান_REGEXP 'r'
#define TCLVAR_মান_LIKE   'l'

প্রকারলেখ কাঠামো tclvar_vtab tclvar_vtab;
প্রকারলেখ কাঠামো tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
কাঠামো tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
কাঠামো tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_আকার i1;              /* Current item in pList1 */
  Tcl_আকার i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
স্থির পূর্ণসংখ্যা tclvarConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  tclvar_vtab *pVtab;
  স্থির ধ্রুবক অক্ষর zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*মান*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( আকার(*pVtab) );
  যদি( pVtab==0 ) ফেরত SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  ফেরত SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

স্থির পূর্ণসংখ্যা tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
স্থির পূর্ণসংখ্যা tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(আকার(tclvar_cursor));
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
স্থির পূর্ণসংখ্যা tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  যদি( pCur->pList1 ){
    Tcl_Decr_Ref_গুনতি(pCur->pList1);
  }
  যদি( pCur->pList2 ){
    Tcl_Decr_Ref_গুনতি(pCur->pList2);
  }
  sqlite3_free(pCur);
  ফেরত SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
স্থির পূর্ণসংখ্যা next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  যদি( pObj ){
    যদি( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_গুনতি(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_গুনতি(p);
      pCur->pList2 = Tcl_Get_Obj_ফলাফল(interp);
      Tcl_Incr_Ref_গুনতি(pCur->pList2);
      প্রতিজ্ঞা( pCur->i2==0 );
    }অন্যথা{
      Tcl_আকার n = 0;
      pCur->i2++;
      Tcl_List_Obj_দৈর্ঘ্য(0, pCur->pList2, &n);
      যদি( pCur->i2>=n ){
        Tcl_Decr_Ref_গুনতি(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        ফেরত 0;
      }
    }
  }

  ফেরত 1;
}

স্থির পূর্ণসংখ্যা tclvar_পরবর্তী(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_আকার n = 0;
  পূর্ণসংখ্যা ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_দৈর্ঘ্য(0, pCur->pList1, &n);
  যখন( !ok && pCur->i1<n ){
    Tcl_List_Obj_সূচক(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    যদি( !ok ){
      pCur->i1++;
    }
  }

  ফেরত 0;
}

স্থির পূর্ণসংখ্যা tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ধ্রুবক অক্ষর *zEq = "";
  ধ্রুবক অক্ষর *zMatch = "";
  ধ্রুবক অক্ষর *zGlob = "";
  ধ্রুবক অক্ষর *zRegexp = "";
  ধ্রুবক অক্ষর *zLike = "";
  পূর্ণসংখ্যা i;

  জন্য(i=0; idxStr[i]; i++){
    নির্বাচন( idxStr[i] ){
      ক্ষেত্র TCLVAR_NAME_EQ:
        zEq = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
        বিরতি;
      ক্ষেত্র TCLVAR_NAME_MATCH:
        zMatch = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
        বিরতি;
      ক্ষেত্র TCLVAR_মান_GLOB:
        zGlob = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
        বিরতি;
      ক্ষেত্র TCLVAR_মান_REGEXP:
        zRegexp = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
        বিরতি;
      ক্ষেত্র TCLVAR_মান_LIKE:
        zLike = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[i]);
        বিরতি;
      পূর্বনির্ধারিত:
        প্রতিজ্ঞা( 0 );
    }
  }

  Tcl_Incr_Ref_গুনতি(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  যদি( pCur->pList1 ){
    Tcl_Decr_Ref_গুনতি(pCur->pList1);
  }
  যদি( pCur->pList2 ){
    Tcl_Decr_Ref_গুনতি(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_ফলাফল(interp);
  Tcl_Incr_Ref_গুনতি(pCur->pList1);

  Tcl_Decr_Ref_গুনতি(p);
  ফেরত tclvar_পরবর্তী(pVtabCursor);
}

স্থির পূর্ণসংখ্যা tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ধ্রুবক অক্ষর *z1; 
  ধ্রুবক অক্ষর *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_সূচক(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_সূচক(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  যদি( p2 ){
    z2 = Tcl_GetString(p2);
  }
  নির্বাচন (i) {
    ক্ষেত্র 0: {
      sqlite3_ফলাফল_text(ctx, z1, -1, SQLITE_TRANSIENT);
      বিরতি;
    }
    ক্ষেত্র 1: {
      sqlite3_ফলাফল_text(ctx, z2, -1, SQLITE_TRANSIENT);
      বিরতি;
    }
    ক্ষেত্র 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_ফলাফল_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      বিরতি;
    }
    ক্ষেত্র 3: {
      অক্ষর *z3;
      যদি( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_ফলাফল_text(ctx, z3, -1, sqlite3_free);
      }অন্যথা{
        sqlite3_ফলাফল_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      বিরতি;
    }
  }
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  ফেরত (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
স্থির পূর্ণসংখ্যা tclvarAddToIdxstr(অক্ষর *zStr, অক্ষর x){
  পূর্ণসংখ্যা i;
  জন্য(i=0; zStr[i]; i++){
    যদি( zStr[i]==x ) ফেরত 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  ফেরত 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
স্থির পূর্ণসংখ্যা tclvarSetOmit(Tcl_Interp *interp){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  যদি( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_ফলাফল(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  ফেরত (rc==TCL_OK && res);
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
স্থির পূর্ণসংখ্যা tclvar_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  পূর্ণসংখ্যা ii;
  অক্ষর *zStr = sqlite3_malloc(32);
  পূর্ণসংখ্যা iStr = 0;

  যদি( zStr==0 ) ফেরত SQLITE_NOMEM;
  zStr[0] = '\0';

  জন্য(ii=0; ii<pIdxInfo->nConstraint; ii++){
    কাঠামো sqlite3_সূচক_constraint ধ্রুবক *pCons = &pIdxInfo->aConstraint[ii];
    কাঠামো sqlite3_সূচক_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    যদি( pCons->usable ){
      /* name = ? */
      যদি( pCons->op==SQLITE_সূচক_CONSTRAINT_EQ && pCons->iColumn==0 ){
        যদি( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_সূচক = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      যদি( pCons->op==SQLITE_সূচক_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        যদি( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argv_সূচক = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      যদি( pCons->op==SQLITE_সূচক_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        যদি( 0==tclvarAddToIdxstr(zStr, TCLVAR_মান_GLOB) ){
          pUsage->argv_সূচক = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      যদি( pCons->op==SQLITE_সূচক_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        যদি( 0==tclvarAddToIdxstr(zStr, TCLVAR_মান_REGEXP) ){
          pUsage->argv_সূচক = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      যদি( pCons->op==SQLITE_সূচক_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        যদি( 0==tclvarAddToIdxstr(zStr, TCLVAR_মান_LIKE) ){
          pUsage->argv_সূচক = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  ফেরত SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
স্থির পূর্ণসংখ্যা tclvarUpdate(
  sqlite3_vtab *tab,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  যদি( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ধ্রুবক অক্ষর *zVar = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    ফেরত SQLITE_OK;
  }
  যদি( sqlite3_মান_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ধ্রুবক অক্ষর *z_মান = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[4]);
    ধ্রুবক অক্ষর *zName;
    যদি( sqlite3_মান_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_বার্তা = sqlite3_mprintf("the 'fullname' column must be TEXT");
      ফেরত SQLITE_ERROR;
    }
    zName = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[5]);
    যদি( z_মান ){
      Tcl_SetVar(pTab->interp, zName, z_মান, TCL_GLOBAL_ONLY);
    }অন্যথা{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    ফেরত SQLITE_OK;
  }
  যদি( sqlite3_মান_type(argv[0])==SQLITE_TEXT
   && sqlite3_মান_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    ধ্রুবক অক্ষর *zOldName = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
    ধ্রুবক অক্ষর *zNewName = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[1]);
    ধ্রুবক অক্ষর *z_মান = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[4]);

    যদি( strcmp(zOldName, zNewName)!=0 || z_মান==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    যদি( z_মান!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_মান, TCL_GLOBAL_ONLY);
    }
    ফেরত SQLITE_OK;
  }
  tab->z_Err_বার্তা = sqlite3_mprintf("prohibited TCL variable change");
  ফেরত SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
স্থির sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_Best_সূচক,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_পরবর্তী,                  /* xNext - advance a cursor */
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
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_tclvar_module(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  পূর্ণসংখ্যা rc = TCL_OK;
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (শূন্য*)interp);
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
  ফেরত rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
#endif
  ফেরত TCL_OK;
}
