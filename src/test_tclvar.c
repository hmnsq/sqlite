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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು TCLVAR_NAME_EQ      'e'
#ವ್ಯಾಖ್ಯಾನಿಸು TCLVAR_NAME_ಹೊಂದಿಕೆ   'm'
#ವ್ಯಾಖ್ಯಾನಿಸು TCLVAR_ಮೌಲ್ಯ_GLOB   'g'
#ವ್ಯಾಖ್ಯಾನಿಸು TCLVAR_ಮೌಲ್ಯ_REGEXP 'r'
#ವ್ಯಾಖ್ಯಾನಿಸು TCLVAR_ಮೌಲ್ಯ_LIKE   'l'

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ tclvar_vtab tclvar_vtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ರಚನೆ tclvar_vtab {
  sqlite3_vtab ತಳ;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ರಚನೆ tclvar_cursor {
  sqlite3_vtab_cursor ತಳ;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_ಗಾತ್ರ i1;              /* Current item in pList1 */
  Tcl_ಗಾತ್ರ i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  tclvar_vtab *pVtab;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*ಮೌಲ್ಯ*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( ಗಾತ್ರ(*pVtab) );
  ಆದರೆ( pVtab==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  *ppVtab = &pVtab->ತಳ;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(ಗಾತ್ರ(tclvar_cursor));
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  ಆದರೆ( pCur->pList1 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList1);
  }
  ಆದರೆ( pCur->pList2 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList2);
  }
  sqlite3_free(pCur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  ಆದರೆ( pObj ){
    ಆದರೆ( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_ಎಣಿಕೆ(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_ಎಣಿಕೆ(p);
      pCur->pList2 = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCur->pList2);
      ಪ್ರತಿಜ್ಞಾ( pCur->i2==0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_ಗಾತ್ರ n = 0;
      pCur->i2++;
      Tcl_List_Obj_ಉದ್ದಳತೆ(0, pCur->pList2, &n);
      ಆದರೆ( pCur->i2>=n ){
        Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        ಹಿಂತಿರುಗಿ 0;
      }
    }
  }

  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvar_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_ಗಾತ್ರ n = 0;
  ಪೂರ್ಣಾಂಕ ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_ಉದ್ದಳತೆ(0, pCur->pList1, &n);
  ಆಗ( !ok && pCur->i1<n ){
    Tcl_List_Obj_ಸೂಚ್ಯಂಕ(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    ಆದರೆ( !ok ){
      pCur->i1++;
    }
  }

  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zEq = "";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಹೊಂದಿಕೆ = "";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zGlob = "";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRegexp = "";
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLike = "";
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; idxStr[i]; i++){
    ಆಯ್ಕೆ( idxStr[i] ){
      ಸ್ಥಿತಿ TCLVAR_NAME_EQ:
        zEq = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TCLVAR_NAME_ಹೊಂದಿಕೆ:
        z_ಹೊಂದಿಕೆ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TCLVAR_ಮೌಲ್ಯ_GLOB:
        zGlob = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TCLVAR_ಮೌಲ್ಯ_REGEXP:
        zRegexp = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TCLVAR_ಮೌಲ್ಯ_LIKE:
        zLike = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        ಪ್ರತಿಜ್ಞಾ( 0 );
    }
  }

  Tcl_Incr_Ref_ಎಣಿಕೆ(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(z_ಹೊಂದಿಕೆ, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  ಆದರೆ( pCur->pList1 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList1);
  }
  ಆದರೆ( pCur->pList2 ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pCur->pList1);

  Tcl_Decr_Ref_ಎಣಿಕೆ(p);
  ಹಿಂತಿರುಗಿ tclvar_ಮುಂದಿನ(pVtabCursor);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvar_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z1; 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_ಸೂಚ್ಯಂಕ(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_ಸೂಚ್ಯಂಕ(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  ಆದರೆ( p2 ){
    z2 = Tcl_GetString(p2);
  }
  ಆಯ್ಕೆ (i) {
    ಸ್ಥಿತಿ 0: {
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, z1, -1, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 1: {
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, z2, -1, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ 3: {
      ಅಕ್ಷರ *z3;
      ಆದರೆ( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, z3, -1, sqlite3_free);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  ಹಿಂತಿರುಗಿ (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvar_ಕೂಡಿಸು_ಗಮ್ಯ_Idxstr(ಅಕ್ಷರ *zStr, ಅಕ್ಷರ x){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; zStr[i]; i++){
    ಆದರೆ( zStr[i]==x ) ಹಿಂತಿರುಗಿ 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvar_ಗಣ_Omit(Tcl_Interp *interp){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  ಆದರೆ( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  ಹಿಂತಿರುಗಿ (rc==TCL_OK && res);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvar_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ಪೂರ್ಣಾಂಕ ii;
  ಅಕ್ಷರ *zStr = sqlite3_malloc(32);
  ಪೂರ್ಣಾಂಕ iStr = 0;

  ಆದರೆ( zStr==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  zStr[0] = '\0';

  ಗಾಗಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint ಸ್ಥಿರಾಂಕ *pCons = &pIdxInfo->aConstraint[ii];
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    ಆದರೆ( pCons->usable ){
      /* name = ? */
      ಆದರೆ( pCons->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ && pCons->i_ಸ್ತಂಭ==0 ){
        ಆದರೆ( 0==tclvar_ಕೂಡಿಸು_ಗಮ್ಯ_Idxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_ಸೂಚ್ಯಂಕ = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      ಆದರೆ( pCons->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಹೊಂದಿಕೆ && pCons->i_ಸ್ತಂಭ==0 ){
        ಆದರೆ( 0==tclvar_ಕೂಡಿಸು_ಗಮ್ಯ_Idxstr(zStr, TCLVAR_NAME_ಹೊಂದಿಕೆ) ){
          pUsage->argv_ಸೂಚ್ಯಂಕ = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      ಆದರೆ( pCons->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GLOB && pCons->i_ಸ್ತಂಭ==2 ){
        ಆದರೆ( 0==tclvar_ಕೂಡಿಸು_ಗಮ್ಯ_Idxstr(zStr, TCLVAR_ಮೌಲ್ಯ_GLOB) ){
          pUsage->argv_ಸೂಚ್ಯಂಕ = ++iStr;
          pUsage->omit = tclvar_ಗಣ_Omit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      ಆದರೆ( pCons->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_REGEXP && pCons->i_ಸ್ತಂಭ==2 ){
        ಆದರೆ( 0==tclvar_ಕೂಡಿಸು_ಗಮ್ಯ_Idxstr(zStr, TCLVAR_ಮೌಲ್ಯ_REGEXP) ){
          pUsage->argv_ಸೂಚ್ಯಂಕ = ++iStr;
          pUsage->omit = tclvar_ಗಣ_Omit(pTab->interp);
        }
      }

      /* value LIKE ? */
      ಆದರೆ( pCons->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LIKE && pCons->i_ಸ್ತಂಭ==2 ){
        ಆದರೆ( 0==tclvar_ಕೂಡಿಸು_ಗಮ್ಯ_Idxstr(zStr, TCLVAR_ಮೌಲ್ಯ_LIKE) ){
          pUsage->argv_ಸೂಚ್ಯಂಕ = ++iStr;
          pUsage->omit = tclvar_ಗಣ_Omit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->need_ಗಮ್ಯ_Free_Idx_Str = 1;

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tclvarUpdate(
  sqlite3_vtab *tab,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ಆದರೆ( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVar = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮೌಲ್ಯ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[4]);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[5])!=SQLITE_ಪಠ್ಯ ){
      tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("the 'fullname' column must be TEXT");
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
    zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[5]);
    ಆದರೆ( z_ಮೌಲ್ಯ ){
      Tcl_ಗಣ_Var(pTab->interp, zName, z_ಮೌಲ್ಯ, TCL_GLOBAL_ONLY);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_ಪಠ್ಯ
   && sqlite3_ಮೌಲ್ಯ_type(argv[1])==SQLITE_ಪಠ್ಯ
  ){
    /* An UPDATE operation */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOldName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zNewName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[1]);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮೌಲ್ಯ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[4]);

    ಆದರೆ( strcmp(zOldName, zNewName)!=0 || z_ಮೌಲ್ಯ==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    ಆದರೆ( z_ಮೌಲ್ಯ!=0 ){
      Tcl_ಗಣ_Var(pTab->interp, zNewName, z_ಮೌಲ್ಯ, TCL_GLOBAL_ONLY);
    }
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  tab->z_Err_ಸಂದೇಶ = sqlite3_mprintf("prohibited TCL variable change");
  ಹಿಂತಿರುಗಿ SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ಸ್ಥಿರ sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_ಮುಂದಿನ,                  /* xNext - advance a cursor */
  tclvarEof,                   /* xEof - check for end of scan */
  tclvar_ಸ್ತಂಭ,                /* xColumn - read data */
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_tclvar_module(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ಪೂರ್ಣಾಂಕ rc = TCL_OK;
  sqlite3 *db;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
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
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "register_tclvar_module",   ರಿಜಿಸ್ಟರ್_tclvar_module, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}
