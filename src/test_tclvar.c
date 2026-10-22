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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_કિંમત_GLOB   'g'
#define TCLVAR_કિંમત_REGEXP 'r'
#define TCLVAR_કિંમત_LIKE   'l'

પ્રકારવ્યાખ્યા સંરચના tclvar_vtab tclvar_vtab;
પ્રકારવ્યાખ્યા સંરચના tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
સંરચના tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
સંરચના tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
સ્થિર પૂર્ણાંક tclvarConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  tclvar_vtab *pVtab;
  સ્થિર અચલ અક્ષર zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*કિંમત*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( કદ(*pVtab) );
  જો( pVtab==0 ) પાછા SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  પાછા SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

સ્થિર પૂર્ણાંક tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
સ્થિર પૂર્ણાંક tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(કદ(tclvar_cursor));
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
સ્થિર પૂર્ણાંક tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  જો( pCur->pList1 ){
    Tcl_Decr_Ref_ગણતરી(pCur->pList1);
  }
  જો( pCur->pList2 ){
    Tcl_Decr_Ref_ગણતરી(pCur->pList2);
  }
  sqlite3_free(pCur);
  પાછા SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
સ્થિર પૂર્ણાંક next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  જો( pObj ){
    જો( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_ગણતરી(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_ગણતરી(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_Incr_Ref_ગણતરી(pCur->pList2);
      પ્રતિજ્ઞા( pCur->i2==0 );
    }નહીંતર{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      જો( pCur->i2>=n ){
        Tcl_Decr_Ref_ગણતરી(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        પાછા 0;
      }
    }
  }

  પાછા 1;
}

સ્થિર પૂર્ણાંક tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  પૂર્ણાંક ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  જ્યારે( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    જો( !ok ){
      pCur->i1++;
    }
  }

  પાછા 0;
}

સ્થિર પૂર્ણાંક tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  અચલ અક્ષર *zEq = "";
  અચલ અક્ષર *zMatch = "";
  અચલ અક્ષર *zGlob = "";
  અચલ અક્ષર *zRegexp = "";
  અચલ અક્ષર *zLike = "";
  પૂર્ણાંક i;

  માટે(i=0; idxStr[i]; i++){
    પસંદગી( idxStr[i] ){
      સ્થિતિ TCLVAR_NAME_EQ:
        zEq = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
        તોડો;
      સ્થિતિ TCLVAR_NAME_MATCH:
        zMatch = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
        તોડો;
      સ્થિતિ TCLVAR_કિંમત_GLOB:
        zGlob = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
        તોડો;
      સ્થિતિ TCLVAR_કિંમત_REGEXP:
        zRegexp = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
        તોડો;
      સ્થિતિ TCLVAR_કિંમત_LIKE:
        zLike = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[i]);
        તોડો;
      મૂળભૂત:
        પ્રતિજ્ઞા( 0 );
    }
  }

  Tcl_Incr_Ref_ગણતરી(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  જો( pCur->pList1 ){
    Tcl_Decr_Ref_ગણતરી(pCur->pList1);
  }
  જો( pCur->pList2 ){
    Tcl_Decr_Ref_ગણતરી(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_Incr_Ref_ગણતરી(pCur->pList1);

  Tcl_Decr_Ref_ગણતરી(p);
  પાછા tclvarNext(pVtabCursor);
}

સ્થિર પૂર્ણાંક tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  અચલ અક્ષર *z1; 
  અચલ અક્ષર *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  જો( p2 ){
    z2 = Tcl_GetString(p2);
  }
  પસંદગી (i) {
    સ્થિતિ 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      તોડો;
    }
    સ્થિતિ 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      તોડો;
    }
    સ્થિતિ 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      તોડો;
    }
    સ્થિતિ 3: {
      અક્ષર *z3;
      જો( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }નહીંતર{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      તોડો;
    }
  }
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  પાછા (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
સ્થિર પૂર્ણાંક tclvarAddToIdxstr(અક્ષર *zStr, અક્ષર x){
  પૂર્ણાંક i;
  માટે(i=0; zStr[i]; i++){
    જો( zStr[i]==x ) પાછા 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  પાછા 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
સ્થિર પૂર્ણાંક tclvarSetOmit(Tcl_Interp *interp){
  પૂર્ણાંક rc;
  પૂર્ણાંક res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  જો( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  પાછા (rc==TCL_OK && res);
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
સ્થિર પૂર્ણાંક tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  પૂર્ણાંક ii;
  અક્ષર *zStr = sqlite3_malloc(32);
  પૂર્ણાંક iStr = 0;

  જો( zStr==0 ) પાછા SQLITE_NOMEM;
  zStr[0] = '\0';

  માટે(ii=0; ii<pIdxInfo->nConstraint; ii++){
    સંરચના sqlite3_index_constraint અચલ *pCons = &pIdxInfo->aConstraint[ii];
    સંરચના sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    જો( pCons->usable ){
      /* name = ? */
      જો( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        જો( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      જો( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        જો( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      જો( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        જો( 0==tclvarAddToIdxstr(zStr, TCLVAR_કિંમત_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      જો( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        જો( 0==tclvarAddToIdxstr(zStr, TCLVAR_કિંમત_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      જો( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        જો( 0==tclvarAddToIdxstr(zStr, TCLVAR_કિંમત_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  પાછા SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
સ્થિર પૂર્ણાંક tclvarUpdate(
  sqlite3_vtab *tab,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  જો( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    અચલ અક્ષર *zVar = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    પાછા SQLITE_OK;
  }
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    અચલ અક્ષર *z_કિંમત = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[4]);
    અચલ અક્ષર *zName;
    જો( sqlite3_કિંમત_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_સંદેશ = sqlite3_mprintf("the 'fullname' column must be TEXT");
      પાછા SQLITE_ERROR;
    }
    zName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[5]);
    જો( z_કિંમત ){
      Tcl_SetVar(pTab->interp, zName, z_કિંમત, TCL_GLOBAL_ONLY);
    }નહીંતર{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    પાછા SQLITE_OK;
  }
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_TEXT
   && sqlite3_કિંમત_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    અચલ અક્ષર *zOldName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    અચલ અક્ષર *zNewName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[1]);
    અચલ અક્ષર *z_કિંમત = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[4]);

    જો( strcmp(zOldName, zNewName)!=0 || z_કિંમત==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    જો( z_કિંમત!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_કિંમત, TCL_GLOBAL_ONLY);
    }
    પાછા SQLITE_OK;
  }
  tab->z_Err_સંદેશ = sqlite3_mprintf("prohibited TCL variable change");
  પાછા SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
સ્થિર sqlite3_module tclvarModule = {
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
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  પૂર્ણાંક rc = TCL_OK;
  sqlite3 *db;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (શૂન્ય*)interp);
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
  પાછા rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  પાછા TCL_OK;
}
