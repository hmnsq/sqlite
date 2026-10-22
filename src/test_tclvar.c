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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_ಮೌಲ್ಯ_GLOB   'g'
#define TCLVAR_ಮೌಲ್ಯ_REGEXP 'r'
#define TCLVAR_ಮೌಲ್ಯ_LIKE   'l'

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ tclvar_vtab tclvar_vtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ಸಂರಚನಾ tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ಸಂರಚನಾ tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  tclvar_vtab *pVtab;
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*ಮೌಲ್ಯ*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( ಪರಿಮಾಣ(*pVtab) );
  ಯದಿ( pVtab==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(ಪರಿಮಾಣ(tclvar_cursor));
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  ಯದಿ( pCur->pList1 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList1);
  }
  ಯದಿ( pCur->pList2 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList2);
  }
  sqlite3_free(pCur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  ಯದಿ( pObj ){
    ಯದಿ( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_ಎಣಿಕೆ(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_ಎಣಿಕೆ(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCur->pList2);
      ಪ್ರತಿಜ್ಞಾ( pCur->i2==0 );
    }ಅನ್ಯಥಾ{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      ಯದಿ( pCur->i2>=n ){
        Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        ಪ್ರತ್ಯಾಗಮ 0;
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  ಪೂರ್ಣಾಙ್ಕ ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  ಯಾವತ್( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    ಯದಿ( !ok ){
      pCur->i1++;
    }
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ಧ್ರುವ ವರ್ಣ *zEq = "";
  ಧ್ರುವ ವರ್ಣ *zMatch = "";
  ಧ್ರುವ ವರ್ಣ *zGlob = "";
  ಧ್ರುವ ವರ್ಣ *zRegexp = "";
  ಧ್ರುವ ವರ್ಣ *zLike = "";
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; idxStr[i]; i++){
    ವಿಕಲ್ಪ( idxStr[i] ){
      ಪಕ್ಷ TCLVAR_NAME_EQ:
        zEq = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
        ಭಙ್ಗ;
      ಪಕ್ಷ TCLVAR_NAME_MATCH:
        zMatch = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
        ಭಙ್ಗ;
      ಪಕ್ಷ TCLVAR_ಮೌಲ್ಯ_GLOB:
        zGlob = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
        ಭಙ್ಗ;
      ಪಕ್ಷ TCLVAR_ಮೌಲ್ಯ_REGEXP:
        zRegexp = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
        ಭಙ್ಗ;
      ಪಕ್ಷ TCLVAR_ಮೌಲ್ಯ_LIKE:
        zLike = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[i]);
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        ಪ್ರತಿಜ್ಞಾ( 0 );
    }
  }

  Tcl_Incr_Ref_ಎಣಿಕೆ(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  ಯದಿ( pCur->pList1 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList1);
  }
  ಯದಿ( pCur->pList2 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pCur->pList1);

  Tcl_Decr_Ref_ಎಣಿಕೆ(p);
  ಪ್ರತ್ಯಾಗಮ tclvarNext(pVtabCursor);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ಧ್ರುವ ವರ್ಣ *z1; 
  ಧ್ರುವ ವರ್ಣ *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  ಯದಿ( p2 ){
    z2 = Tcl_GetString(p2);
  }
  ವಿಕಲ್ಪ (i) {
    ಪಕ್ಷ 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ 3: {
      ವರ್ಣ *z3;
      ಯದಿ( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }ಅನ್ಯಥಾ{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  ಪ್ರತ್ಯಾಗಮ (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarAddToIdxstr(ವರ್ಣ *zStr, ವರ್ಣ x){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; zStr[i]; i++){
    ಯದಿ( zStr[i]==x ) ಪ್ರತ್ಯಾಗಮ 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarSetOmit(Tcl_Interp *interp){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  ಯದಿ( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  ಪ್ರತ್ಯಾಗಮ (rc==TCL_OK && res);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ವರ್ಣ *zStr = sqlite3_malloc(32);
  ಪೂರ್ಣಾಙ್ಕ iStr = 0;

  ಯದಿ( zStr==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  zStr[0] = '\0';

  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಸಂರಚನಾ sqlite3_index_constraint ಧ್ರುವ *pCons = &pIdxInfo->aConstraint[ii];
    ಸಂರಚನಾ sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    ಯದಿ( pCons->usable ){
      /* name = ? */
      ಯದಿ( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        ಯದಿ( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      ಯದಿ( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        ಯದಿ( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      ಯದಿ( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        ಯದಿ( 0==tclvarAddToIdxstr(zStr, TCLVAR_ಮೌಲ್ಯ_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      ಯದಿ( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        ಯದಿ( 0==tclvarAddToIdxstr(zStr, TCLVAR_ಮೌಲ್ಯ_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      ಯದಿ( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        ಯದಿ( 0==tclvarAddToIdxstr(zStr, TCLVAR_ಮೌಲ್ಯ_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tclvarUpdate(
  sqlite3_vtab *tab,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ಯದಿ( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ಧ್ರುವ ವರ್ಣ *zVar = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ಧ್ರುವ ವರ್ಣ *z_ಮೌಲ್ಯ = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[4]);
    ಧ್ರುವ ವರ್ಣ *zName;
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("the 'fullname' column must be TEXT");
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
    zName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[5]);
    ಯದಿ( z_ಮೌಲ್ಯ ){
      Tcl_SetVar(pTab->interp, zName, z_ಮೌಲ್ಯ, TCL_GLOBAL_ONLY);
    }ಅನ್ಯಥಾ{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_TEXT
   && sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    ಧ್ರುವ ವರ್ಣ *zOldName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    ಧ್ರುವ ವರ್ಣ *zNewName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[1]);
    ಧ್ರುವ ವರ್ಣ *z_ಮೌಲ್ಯ = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[4]);

    ಯದಿ( strcmp(zOldName, zNewName)!=0 || z_ಮೌಲ್ಯ==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    ಯದಿ( z_ಮೌಲ್ಯ!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_ಮೌಲ್ಯ, TCL_GLOBAL_ONLY);
    }
    ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }
  tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("prohibited TCL variable change");
  ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ಸ್ಥಿರ sqlite3_module tclvarModule = {
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಪೂರ್ಣಾಙ್ಕ rc = TCL_OK;
  sqlite3 *db;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (ಶೂನ್ಯ*)interp);
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
  ಪ್ರತ್ಯಾಗಮ rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
