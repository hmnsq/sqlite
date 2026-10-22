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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_ମୂଲ୍ୟ_GLOB   'g'
#define TCLVAR_ମୂଲ୍ୟ_REGEXP 'r'
#define TCLVAR_ମୂଲ୍ୟ_LIKE   'l'

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tclvar_vtab tclvar_vtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
ସଂରଚନା tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
ସଂରଚନା tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  tclvar_vtab *pVtab;
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*ମୂଲ୍ୟ*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( ଆକାର(*pVtab) );
  ଯଦି( pVtab==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  ଫେରନ୍ତୁ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(ଆକାର(tclvar_cursor));
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  ଯଦି( pCur->pList1 ){
    Tcl_Decr_Ref_ଗଣନା(pCur->pList1);
  }
  ଯଦି( pCur->pList2 ){
    Tcl_Decr_Ref_ଗଣନା(pCur->pList2);
  }
  sqlite3_free(pCur);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  ଯଦି( pObj ){
    ଯଦି( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_ଗଣନା(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_ଗଣନା(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_ଗଣନା(pCur->pList2);
      ପ୍ରତିଜ୍ଞା( pCur->i2==0 );
    }ଅନ୍ୟଥା{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      ଯଦି( pCur->i2>=n ){
        Tcl_Decr_Ref_ଗଣନା(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        ଫେରନ୍ତୁ 0;
      }
    }
  }

  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  ଯେତେବେଳେ( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    ଯଦି( !ok ){
      pCur->i1++;
    }
  }

  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  ଅଚଳ ଅକ୍ଷର *zEq = "";
  ଅଚଳ ଅକ୍ଷର *zMatch = "";
  ଅଚଳ ଅକ୍ଷର *zGlob = "";
  ଅଚଳ ଅକ୍ଷର *zRegexp = "";
  ଅଚଳ ଅକ୍ଷର *zLike = "";
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; idxStr[i]; i++){
    ବାଛ( idxStr[i] ){
      ସ୍ଥିତି TCLVAR_NAME_EQ:
        zEq = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି TCLVAR_NAME_MATCH:
        zMatch = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି TCLVAR_ମୂଲ୍ୟ_GLOB:
        zGlob = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି TCLVAR_ମୂଲ୍ୟ_REGEXP:
        zRegexp = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି TCLVAR_ମୂଲ୍ୟ_LIKE:
        zLike = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ପ୍ରତିଜ୍ଞା( 0 );
    }
  }

  Tcl_Incr_Ref_ଗଣନା(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  ଯଦି( pCur->pList1 ){
    Tcl_Decr_Ref_ଗଣନା(pCur->pList1);
  }
  ଯଦି( pCur->pList2 ){
    Tcl_Decr_Ref_ଗଣନା(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_ଗଣନା(pCur->pList1);

  Tcl_Decr_Ref_ଗଣନା(p);
  ଫେରନ୍ତୁ tclvarNext(pVtabCursor);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  ଅଚଳ ଅକ୍ଷର *z1; 
  ଅଚଳ ଅକ୍ଷର *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  ଯଦି( p2 ){
    z2 = Tcl_GetString(p2);
  }
  ବାଛ (i) {
    ସ୍ଥିତି 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି 3: {
      ଅକ୍ଷର *z3;
      ଯଦି( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }ଅନ୍ୟଥା{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  ଫେରନ୍ତୁ (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarAddToIdxstr(ଅକ୍ଷର *zStr, ଅକ୍ଷର x){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; zStr[i]; i++){
    ଯଦି( zStr[i]==x ) ଫେରନ୍ତୁ 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  ଫେରନ୍ତୁ 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarSetOmit(Tcl_Interp *interp){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  ଯଦି( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  ଫେରନ୍ତୁ (rc==TCL_OK && res);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ଅକ୍ଷର *zStr = sqlite3_malloc(32);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStr = 0;

  ଯଦି( zStr==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  zStr[0] = '\0';

  ପାଇଁ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ସଂରଚନା sqlite3_index_constraint ଅଚଳ *pCons = &pIdxInfo->aConstraint[ii];
    ସଂରଚନା sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    ଯଦି( pCons->usable ){
      /* name = ? */
      ଯଦି( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        ଯଦି( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      ଯଦି( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        ଯଦି( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      ଯଦି( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        ଯଦି( 0==tclvarAddToIdxstr(zStr, TCLVAR_ମୂଲ୍ୟ_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      ଯଦି( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        ଯଦି( 0==tclvarAddToIdxstr(zStr, TCLVAR_ମୂଲ୍ୟ_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      ଯଦି( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        ଯଦି( 0==tclvarAddToIdxstr(zStr, TCLVAR_ମୂଲ୍ୟ_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tclvarUpdate(
  sqlite3_vtab *tab,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  ଯଦି( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    ଅଚଳ ଅକ୍ଷର *zVar = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    ଫେରନ୍ତୁ SQLITE_OK;
  }
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    ଅଚଳ ଅକ୍ଷର *z_ମୂଲ୍ୟ = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[4]);
    ଅଚଳ ଅକ୍ଷର *zName;
    ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("the 'fullname' column must be TEXT");
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
    zName = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[5]);
    ଯଦି( z_ମୂଲ୍ୟ ){
      Tcl_SetVar(pTab->interp, zName, z_ମୂଲ୍ୟ, TCL_GLOBAL_ONLY);
    }ଅନ୍ୟଥା{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    ଫେରନ୍ତୁ SQLITE_OK;
  }
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_TEXT
   && sqlite3_ମୂଲ୍ୟ_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    ଅଚଳ ଅକ୍ଷର *zOldName = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    ଅଚଳ ଅକ୍ଷର *zNewName = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[1]);
    ଅଚଳ ଅକ୍ଷର *z_ମୂଲ୍ୟ = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[4]);

    ଯଦି( strcmp(zOldName, zNewName)!=0 || z_ମୂଲ୍ୟ==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    ଯଦି( z_ମୂଲ୍ୟ!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_ମୂଲ୍ୟ, TCL_GLOBAL_ONLY);
    }
    ଫେରନ୍ତୁ SQLITE_OK;
  }
  tab->z_Err_ସନ୍ଦେଶ = sqlite3_mprintf("prohibited TCL variable change");
  ଫେରନ୍ତୁ SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ସ୍ଥିର sqlite3_module tclvarModule = {
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
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = TCL_OK;
  sqlite3 *db;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (ଶୂନ୍ୟ*)interp);
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
  ଫେରନ୍ତୁ rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ଶୂନ୍ୟ *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}
