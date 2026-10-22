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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_மதிப்பு_GLOB   'g'
#define TCLVAR_மதிப்பு_REGEXP 'r'
#define TCLVAR_மதிப்பு_LIKE   'l'

வகைவரை கட்டமைப்பு tclvar_vtab tclvar_vtab;
வகைவரை கட்டமைப்பு tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
கட்டமைப்பு tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
கட்டமைப்பு tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
நிலையான முழு tclvarConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  tclvar_vtab *pVtab;
  நிலையான மாறிலி எழுத்து zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*மதிப்பு*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( அளவு(*pVtab) );
  எனில்( pVtab==0 ) திரும்பு SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  திரும்பு SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

நிலையான முழு tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
நிலையான முழு tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(அளவு(tclvar_cursor));
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
நிலையான முழு tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  எனில்( pCur->pList1 ){
    Tcl_Decr_Ref_மொத்தம்(pCur->pList1);
  }
  எனில்( pCur->pList2 ){
    Tcl_Decr_Ref_மொத்தம்(pCur->pList2);
  }
  sqlite3_free(pCur);
  திரும்பு SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
நிலையான முழு next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  எனில்( pObj ){
    எனில்( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_மொத்தம்(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_மொத்தம்(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_மொத்தம்(pCur->pList2);
      ப்ரதிஜ்ஞா( pCur->i2==0 );
    }இல்லையெனில்{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      எனில்( pCur->i2>=n ){
        Tcl_Decr_Ref_மொத்தம்(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        திரும்பு 0;
      }
    }
  }

  திரும்பு 1;
}

நிலையான முழு tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  முழு ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  போது( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    எனில்( !ok ){
      pCur->i1++;
    }
  }

  திரும்பு 0;
}

நிலையான முழு tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  மாறிலி எழுத்து *zEq = "";
  மாறிலி எழுத்து *zMatch = "";
  மாறிலி எழுத்து *zGlob = "";
  மாறிலி எழுத்து *zRegexp = "";
  மாறிலி எழுத்து *zLike = "";
  முழு i;

  க்கு(i=0; idxStr[i]; i++){
    தேர்வு( idxStr[i] ){
      நிலை TCLVAR_NAME_EQ:
        zEq = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
        நிறுத்து;
      நிலை TCLVAR_NAME_MATCH:
        zMatch = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
        நிறுத்து;
      நிலை TCLVAR_மதிப்பு_GLOB:
        zGlob = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
        நிறுத்து;
      நிலை TCLVAR_மதிப்பு_REGEXP:
        zRegexp = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
        நிறுத்து;
      நிலை TCLVAR_மதிப்பு_LIKE:
        zLike = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[i]);
        நிறுத்து;
      இயல்பு:
        ப்ரதிஜ்ஞா( 0 );
    }
  }

  Tcl_Incr_Ref_மொத்தம்(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  எனில்( pCur->pList1 ){
    Tcl_Decr_Ref_மொத்தம்(pCur->pList1);
  }
  எனில்( pCur->pList2 ){
    Tcl_Decr_Ref_மொத்தம்(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_மொத்தம்(pCur->pList1);

  Tcl_Decr_Ref_மொத்தம்(p);
  திரும்பு tclvarNext(pVtabCursor);
}

நிலையான முழு tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, முழு i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  மாறிலி எழுத்து *z1; 
  மாறிலி எழுத்து *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  எனில்( p2 ){
    z2 = Tcl_GetString(p2);
  }
  தேர்வு (i) {
    நிலை 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      நிறுத்து;
    }
    நிலை 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      நிறுத்து;
    }
    நிலை 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      நிறுத்து;
    }
    நிலை 3: {
      எழுத்து *z3;
      எனில்( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }இல்லையெனில்{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      நிறுத்து;
    }
  }
  திரும்பு SQLITE_OK;
}

நிலையான முழு tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  திரும்பு SQLITE_OK;
}

நிலையான முழு tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  திரும்பு (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
நிலையான முழு tclvarAddToIdxstr(எழுத்து *zStr, எழுத்து x){
  முழு i;
  க்கு(i=0; zStr[i]; i++){
    எனில்( zStr[i]==x ) திரும்பு 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  திரும்பு 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
நிலையான முழு tclvarSetOmit(Tcl_Interp *interp){
  முழு rc;
  முழு res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  எனில்( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  திரும்பு (rc==TCL_OK && res);
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
நிலையான முழு tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  முழு ii;
  எழுத்து *zStr = sqlite3_malloc(32);
  முழு iStr = 0;

  எனில்( zStr==0 ) திரும்பு SQLITE_NOMEM;
  zStr[0] = '\0';

  க்கு(ii=0; ii<pIdxInfo->nConstraint; ii++){
    கட்டமைப்பு sqlite3_index_constraint மாறிலி *pCons = &pIdxInfo->aConstraint[ii];
    கட்டமைப்பு sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    எனில்( pCons->usable ){
      /* name = ? */
      எனில்( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        எனில்( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      எனில்( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        எனில்( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      எனில்( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        எனில்( 0==tclvarAddToIdxstr(zStr, TCLVAR_மதிப்பு_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      எனில்( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        எனில்( 0==tclvarAddToIdxstr(zStr, TCLVAR_மதிப்பு_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      எனில்( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        எனில்( 0==tclvarAddToIdxstr(zStr, TCLVAR_மதிப்பு_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  திரும்பு SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
நிலையான முழு tclvarUpdate(
  sqlite3_vtab *tab,
  முழு argc,
  sqlite3_மதிப்பு **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  எனில்( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    மாறிலி எழுத்து *zVar = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    திரும்பு SQLITE_OK;
  }
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    மாறிலி எழுத்து *z_மதிப்பு = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[4]);
    மாறிலி எழுத்து *zName;
    எனில்( sqlite3_மதிப்பு_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_செய்தி = sqlite3_mprintf("the 'fullname' column must be TEXT");
      திரும்பு SQLITE_ERROR;
    }
    zName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[5]);
    எனில்( z_மதிப்பு ){
      Tcl_SetVar(pTab->interp, zName, z_மதிப்பு, TCL_GLOBAL_ONLY);
    }இல்லையெனில்{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    திரும்பு SQLITE_OK;
  }
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_TEXT
   && sqlite3_மதிப்பு_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    மாறிலி எழுத்து *zOldName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    மாறிலி எழுத்து *zNewName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[1]);
    மாறிலி எழுத்து *z_மதிப்பு = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[4]);

    எனில்( strcmp(zOldName, zNewName)!=0 || z_மதிப்பு==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    எனில்( z_மதிப்பு!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_மதிப்பு, TCL_GLOBAL_ONLY);
    }
    திரும்பு SQLITE_OK;
  }
  tab->z_Err_செய்தி = sqlite3_mprintf("prohibited TCL variable change");
  திரும்பு SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
நிலையான sqlite3_module tclvarModule = {
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
வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
நிலையான முழு SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  முழு rc = TCL_OK;
  sqlite3 *db;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (வெறுமை*)interp);
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
  திரும்பு rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     வெறுமை *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  திரும்பு TCL_OK;
}
