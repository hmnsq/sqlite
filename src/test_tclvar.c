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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_മൂല്യം_GLOB   'g'
#define TCLVAR_മൂല്യം_REGEXP 'r'
#define TCLVAR_മൂല്യം_LIKE   'l'

തരനിർവചനം ഘടന tclvar_vtab tclvar_vtab;
തരനിർവചനം ഘടന tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ഘടന tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ഘടന tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  tclvar_vtab *pVtab;
  സ്ഥിതം സ്ഥിരം അക്ഷരം zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*മൂല്യം*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( വലുപ്പം(*pVtab) );
  എങ്കിൽ( pVtab==0 ) തിരികെ SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  തിരികെ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(വലുപ്പം(tclvar_cursor));
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  എങ്കിൽ( pCur->pList1 ){
    Tcl_Decr_Ref_എണ്ണം(pCur->pList1);
  }
  എങ്കിൽ( pCur->pList2 ){
    Tcl_Decr_Ref_എണ്ണം(pCur->pList2);
  }
  sqlite3_free(pCur);
  തിരികെ SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  എങ്കിൽ( pObj ){
    എങ്കിൽ( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_എണ്ണം(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_എണ്ണം(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_എണ്ണം(pCur->pList2);
      പ്രതിജ്ഞാ( pCur->i2==0 );
    }അല്ലെങ്കിൽ{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      എങ്കിൽ( pCur->i2>=n ){
        Tcl_Decr_Ref_എണ്ണം(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        തിരികെ 0;
      }
    }
  }

  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  പൂർണ്ണസംഖ്യ ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  ആകുമ്പോൾ( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    എങ്കിൽ( !ok ){
      pCur->i1++;
    }
  }

  തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  സ്ഥിരം അക്ഷരം *zEq = "";
  സ്ഥിരം അക്ഷരം *zMatch = "";
  സ്ഥിരം അക്ഷരം *zGlob = "";
  സ്ഥിരം അക്ഷരം *zRegexp = "";
  സ്ഥിരം അക്ഷരം *zLike = "";
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; idxStr[i]; i++){
    തിരഞ്ഞെടുപ്പ്( idxStr[i] ){
      അവസ്ഥ TCLVAR_NAME_EQ:
        zEq = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
        നിർത്തുക;
      അവസ്ഥ TCLVAR_NAME_MATCH:
        zMatch = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
        നിർത്തുക;
      അവസ്ഥ TCLVAR_മൂല്യം_GLOB:
        zGlob = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
        നിർത്തുക;
      അവസ്ഥ TCLVAR_മൂല്യം_REGEXP:
        zRegexp = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
        നിർത്തുക;
      അവസ്ഥ TCLVAR_മൂല്യം_LIKE:
        zLike = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        പ്രതിജ്ഞാ( 0 );
    }
  }

  Tcl_Incr_Ref_എണ്ണം(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  എങ്കിൽ( pCur->pList1 ){
    Tcl_Decr_Ref_എണ്ണം(pCur->pList1);
  }
  എങ്കിൽ( pCur->pList2 ){
    Tcl_Decr_Ref_എണ്ണം(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_എണ്ണം(pCur->pList1);

  Tcl_Decr_Ref_എണ്ണം(p);
  തിരികെ tclvarNext(pVtabCursor);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  സ്ഥിരം അക്ഷരം *z1; 
  സ്ഥിരം അക്ഷരം *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  എങ്കിൽ( p2 ){
    z2 = Tcl_GetString(p2);
  }
  തിരഞ്ഞെടുപ്പ് (i) {
    അവസ്ഥ 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      നിർത്തുക;
    }
    അവസ്ഥ 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      നിർത്തുക;
    }
    അവസ്ഥ 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      നിർത്തുക;
    }
    അവസ്ഥ 3: {
      അക്ഷരം *z3;
      എങ്കിൽ( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }അല്ലെങ്കിൽ{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      നിർത്തുക;
    }
  }
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  തിരികെ (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarAddToIdxstr(അക്ഷരം *zStr, അക്ഷരം x){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; zStr[i]; i++){
    എങ്കിൽ( zStr[i]==x ) തിരികെ 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  തിരികെ 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarSetOmit(Tcl_Interp *interp){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  എങ്കിൽ( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  തിരികെ (rc==TCL_OK && res);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  പൂർണ്ണസംഖ്യ ii;
  അക്ഷരം *zStr = sqlite3_malloc(32);
  പൂർണ്ണസംഖ്യ iStr = 0;

  എങ്കിൽ( zStr==0 ) തിരികെ SQLITE_NOMEM;
  zStr[0] = '\0';

  വേണ്ടി(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ഘടന sqlite3_index_constraint സ്ഥിരം *pCons = &pIdxInfo->aConstraint[ii];
    ഘടന sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    എങ്കിൽ( pCons->usable ){
      /* name = ? */
      എങ്കിൽ( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        എങ്കിൽ( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      എങ്കിൽ( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        എങ്കിൽ( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      എങ്കിൽ( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        എങ്കിൽ( 0==tclvarAddToIdxstr(zStr, TCLVAR_മൂല്യം_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      എങ്കിൽ( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        എങ്കിൽ( 0==tclvarAddToIdxstr(zStr, TCLVAR_മൂല്യം_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      എങ്കിൽ( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        എങ്കിൽ( 0==tclvarAddToIdxstr(zStr, TCLVAR_മൂല്യം_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  തിരികെ SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tclvarUpdate(
  sqlite3_vtab *tab,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  എങ്കിൽ( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    സ്ഥിരം അക്ഷരം *zVar = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    തിരികെ SQLITE_OK;
  }
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    സ്ഥിരം അക്ഷരം *z_മൂല്യം = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[4]);
    സ്ഥിരം അക്ഷരം *zName;
    എങ്കിൽ( sqlite3_മൂല്യം_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_സന്ദേശം = sqlite3_mprintf("the 'fullname' column must be TEXT");
      തിരികെ SQLITE_ERROR;
    }
    zName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[5]);
    എങ്കിൽ( z_മൂല്യം ){
      Tcl_SetVar(pTab->interp, zName, z_മൂല്യം, TCL_GLOBAL_ONLY);
    }അല്ലെങ്കിൽ{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    തിരികെ SQLITE_OK;
  }
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_TEXT
   && sqlite3_മൂല്യം_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    സ്ഥിരം അക്ഷരം *zOldName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    സ്ഥിരം അക്ഷരം *zNewName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[1]);
    സ്ഥിരം അക്ഷരം *z_മൂല്യം = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[4]);

    എങ്കിൽ( strcmp(zOldName, zNewName)!=0 || z_മൂല്യം==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    എങ്കിൽ( z_മൂല്യം!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_മൂല്യം, TCL_GLOBAL_ONLY);
    }
    തിരികെ SQLITE_OK;
  }
  tab->z_Err_സന്ദേശം = sqlite3_mprintf("prohibited TCL variable change");
  തിരികെ SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
സ്ഥിതം sqlite3_module tclvarModule = {
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
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  പൂർണ്ണസംഖ്യ rc = TCL_OK;
  sqlite3 *db;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (ശൂന്യം*)interp);
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
  തിരികെ rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  തിരികെ TCL_OK;
}
