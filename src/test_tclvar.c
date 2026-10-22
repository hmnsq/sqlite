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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCLVAR_NAME_EQ      'e'
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCLVAR_NAME_ਮੇਲ   'm'
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCLVAR_ਮੁੱਲ_GLOB   'g'
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCLVAR_ਮੁੱਲ_REGEXP 'r'
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCLVAR_ਮੁੱਲ_LIKE   'l'

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tclvar_vtab tclvar_vtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ਬਣਤਰ tclvar_vtab {
  sqlite3_vtab ਅਧਾਰ;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ਬਣਤਰ tclvar_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_ਮਾਪ i1;              /* Current item in pList1 */
  Tcl_ਮਾਪ i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ਸਥਿਰ ਪੂਰਨ tclvarConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  tclvar_vtab *pVtab;
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*ਮੁੱਲ*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( ਆਕਾਰ(*pVtab) );
  ਜੇ( pVtab==0 ) ਵਾਪਸ SQLITE_NOMEM;
  *ppVtab = &pVtab->ਅਧਾਰ;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  ਵਾਪਸ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ਸਥਿਰ ਪੂਰਨ tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ਸਥਿਰ ਪੂਰਨ tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(ਆਕਾਰ(tclvar_cursor));
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ਸਥਿਰ ਪੂਰਨ tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  ਜੇ( pCur->pList1 ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCur->pList1);
  }
  ਜੇ( pCur->pList2 ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCur->pList2);
  }
  sqlite3_free(pCur);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ਸਥਿਰ ਪੂਰਨ next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  ਜੇ( pObj ){
    ਜੇ( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_ਗਿਣਤੀ(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_ਗਿਣਤੀ(p);
      pCur->pList2 = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCur->pList2);
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i2==0 );
    }ਨਹੀਂਤਾਂ{
      Tcl_ਮਾਪ n = 0;
      pCur->i2++;
      Tcl_List_Obj_ਲੰਮਾਈ(0, pCur->pList2, &n);
      ਜੇ( pCur->i2>=n ){
        Tcl_Decr_Ref_ਗਿਣਤੀ(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        ਵਾਪਸ 0;
      }
    }
  }

  ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ tclvar_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_ਮਾਪ n = 0;
  ਪੂਰਨ ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_ਲੰਮਾਈ(0, pCur->pList1, &n);
  ਜਦੋਂ( !ok && pCur->i1<n ){
    Tcl_List_Obj_ਸੂਚਕਾਂਕ(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    ਜੇ( !ok ){
      pCur->i1++;
    }
  }

  ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ਅਚਲ ਅੱਖਰ *zEq = "";
  ਅਚਲ ਅੱਖਰ *z_ਮੇਲ = "";
  ਅਚਲ ਅੱਖਰ *zGlob = "";
  ਅਚਲ ਅੱਖਰ *zRegexp = "";
  ਅਚਲ ਅੱਖਰ *zLike = "";
  ਪੂਰਨ i;

  ਲਈ(i=0; idxStr[i]; i++){
    ਚੋਣ( idxStr[i] ){
      ਹਾਲਤ TCLVAR_NAME_EQ:
        zEq = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
        ਰੋਕੋ;
      ਹਾਲਤ TCLVAR_NAME_ਮੇਲ:
        z_ਮੇਲ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
        ਰੋਕੋ;
      ਹਾਲਤ TCLVAR_ਮੁੱਲ_GLOB:
        zGlob = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
        ਰੋਕੋ;
      ਹਾਲਤ TCLVAR_ਮੁੱਲ_REGEXP:
        zRegexp = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
        ਰੋਕੋ;
      ਹਾਲਤ TCLVAR_ਮੁੱਲ_LIKE:
        zLike = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
        ਰੋਕੋ;
      ਮੂਲ:
        ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
    }
  }

  Tcl_Incr_Ref_ਗਿਣਤੀ(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(z_ਮੇਲ, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  ਜੇ( pCur->pList1 ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCur->pList1);
  }
  ਜੇ( pCur->pList2 ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pCur->pList1);

  Tcl_Decr_Ref_ਗਿਣਤੀ(p);
  ਵਾਪਸ tclvar_ਅਗਲਾ(pVtabCursor);
}

ਸਥਿਰ ਪੂਰਨ tclvar_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ਅਚਲ ਅੱਖਰ *z1; 
  ਅਚਲ ਅੱਖਰ *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_ਸੂਚਕਾਂਕ(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_ਸੂਚਕਾਂਕ(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  ਜੇ( p2 ){
    z2 = Tcl_GetString(p2);
  }
  ਚੋਣ (i) {
    ਹਾਲਤ 0: {
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, z1, -1, SQLITE_TRANSIENT);
      ਰੋਕੋ;
    }
    ਹਾਲਤ 1: {
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, z2, -1, SQLITE_TRANSIENT);
      ਰੋਕੋ;
    }
    ਹਾਲਤ 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      ਰੋਕੋ;
    }
    ਹਾਲਤ 3: {
      ਅੱਖਰ *z3;
      ਜੇ( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, z3, -1, sqlite3_free);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  ਵਾਪਸ (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ਸਥਿਰ ਪੂਰਨ tclvar_ਜੋੜੋ_ਮੰਜ਼ਿਲ_Idxstr(ਅੱਖਰ *zStr, ਅੱਖਰ x){
  ਪੂਰਨ i;
  ਲਈ(i=0; zStr[i]; i++){
    ਜੇ( zStr[i]==x ) ਵਾਪਸ 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  ਵਾਪਸ 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ਸਥਿਰ ਪੂਰਨ tclvar_ਸੈੱਟ_Omit(Tcl_Interp *interp){
  ਪੂਰਨ rc;
  ਪੂਰਨ res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  ਜੇ( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_ਨਤੀਜਾ(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  ਵਾਪਸ (rc==TCL_OK && res);
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
ਸਥਿਰ ਪੂਰਨ tclvar_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ਪੂਰਨ ii;
  ਅੱਖਰ *zStr = sqlite3_malloc(32);
  ਪੂਰਨ iStr = 0;

  ਜੇ( zStr==0 ) ਵਾਪਸ SQLITE_NOMEM;
  zStr[0] = '\0';

  ਲਈ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint ਅਚਲ *pCons = &pIdxInfo->aConstraint[ii];
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    ਜੇ( pCons->usable ){
      /* name = ? */
      ਜੇ( pCons->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ && pCons->i_ਕਾਲਮ==0 ){
        ਜੇ( 0==tclvar_ਜੋੜੋ_ਮੰਜ਼ਿਲ_Idxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_ਸੂਚਕਾਂਕ = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      ਜੇ( pCons->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਮੇਲ && pCons->i_ਕਾਲਮ==0 ){
        ਜੇ( 0==tclvar_ਜੋੜੋ_ਮੰਜ਼ਿਲ_Idxstr(zStr, TCLVAR_NAME_ਮੇਲ) ){
          pUsage->argv_ਸੂਚਕਾਂਕ = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      ਜੇ( pCons->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GLOB && pCons->i_ਕਾਲਮ==2 ){
        ਜੇ( 0==tclvar_ਜੋੜੋ_ਮੰਜ਼ਿਲ_Idxstr(zStr, TCLVAR_ਮੁੱਲ_GLOB) ){
          pUsage->argv_ਸੂਚਕਾਂਕ = ++iStr;
          pUsage->omit = tclvar_ਸੈੱਟ_Omit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      ਜੇ( pCons->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_REGEXP && pCons->i_ਕਾਲਮ==2 ){
        ਜੇ( 0==tclvar_ਜੋੜੋ_ਮੰਜ਼ਿਲ_Idxstr(zStr, TCLVAR_ਮੁੱਲ_REGEXP) ){
          pUsage->argv_ਸੂਚਕਾਂਕ = ++iStr;
          pUsage->omit = tclvar_ਸੈੱਟ_Omit(pTab->interp);
        }
      }

      /* value LIKE ? */
      ਜੇ( pCons->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE && pCons->i_ਕਾਲਮ==2 ){
        ਜੇ( 0==tclvar_ਜੋੜੋ_ਮੰਜ਼ਿਲ_Idxstr(zStr, TCLVAR_ਮੁੱਲ_LIKE) ){
          pUsage->argv_ਸੂਚਕਾਂਕ = ++iStr;
          pUsage->omit = tclvar_ਸੈੱਟ_Omit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->need_ਮੰਜ਼ਿਲ_Free_Idx_Str = 1;

  ਵਾਪਸ SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ਸਥਿਰ ਪੂਰਨ tclvarUpdate(
  sqlite3_vtab *tab,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ਜੇ( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ਅਚਲ ਅੱਖਰ *zVar = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    ਵਾਪਸ SQLITE_OK;
  }
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ਅਚਲ ਅੱਖਰ *z_ਮੁੱਲ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[4]);
    ਅਚਲ ਅੱਖਰ *zName;
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[5])!=SQLITE_ਪਾਠ ){
      tab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("the 'fullname' column must be TEXT");
      ਵਾਪਸ SQLITE_ERROR;
    }
    zName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[5]);
    ਜੇ( z_ਮੁੱਲ ){
      Tcl_ਸੈੱਟ_Var(pTab->interp, zName, z_ਮੁੱਲ, TCL_GLOBAL_ONLY);
    }ਨਹੀਂਤਾਂ{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    ਵਾਪਸ SQLITE_OK;
  }
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_ਪਾਠ
   && sqlite3_ਮੁੱਲ_type(argv[1])==SQLITE_ਪਾਠ
  ){
    /* An UPDATE operation */
    ਅਚਲ ਅੱਖਰ *zOldName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    ਅਚਲ ਅੱਖਰ *zNewName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[1]);
    ਅਚਲ ਅੱਖਰ *z_ਮੁੱਲ = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[4]);

    ਜੇ( strcmp(zOldName, zNewName)!=0 || z_ਮੁੱਲ==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    ਜੇ( z_ਮੁੱਲ!=0 ){
      Tcl_ਸੈੱਟ_Var(pTab->interp, zNewName, z_ਮੁੱਲ, TCL_GLOBAL_ONLY);
    }
    ਵਾਪਸ SQLITE_OK;
  }
  tab->z_Err_ਸੁਨੇਹਾ = sqlite3_mprintf("prohibited TCL variable change");
  ਵਾਪਸ SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ਸਥਿਰ sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_ਅਗਲਾ,                  /* xNext - advance a cursor */
  tclvarEof,                   /* xEof - check for end of scan */
  tclvar_ਕਾਲਮ,                /* xColumn - read data */
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
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_tclvar_module(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ਪੂਰਨ rc = TCL_OK;
  sqlite3 *db;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (ਖਾਲੀ*)interp);
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
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "register_tclvar_module",   ਰਜਿਸਟਰ_tclvar_module, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}
