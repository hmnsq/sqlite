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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>

#ifndef SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#define TCLVAR_NAME_EQ      'e'
#define TCLVAR_NAME_MATCH   'm'
#define TCLVAR_VALUE_GLOB   'g'
#define TCLVAR_VALUE_REGEXP 'r'
#define TCLVAR_VALUE_LIKE   'l'

typedef struct tclvar_vtab tclvar_vtab;
typedef struct tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
struct tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
struct tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_Size i1;              /* Current item in pList1 */
  Tcl_Size i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
static số_nguyên tclvarConnect(
  sqlite3 *db,
  void *pAux,
  số_nguyên argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  tclvar_vtab *pVtab;
  static const char zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /* Value */
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( sizeof(*pVtab) );
  nếu( pVtab==0 ) trả_về SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  trả_về SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

static số_nguyên tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  trả_về SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
static số_nguyên tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(sizeof(tclvar_cursor));
  *ppCursor = &pCur->base;
  trả_về SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
static số_nguyên tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  nếu( pCur->pList1 ){
    Tcl_DecrRefCount(pCur->pList1);
  }
  nếu( pCur->pList2 ){
    Tcl_DecrRefCount(pCur->pList2);
  }
  sqlite3_free(pCur);
  trả_về SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
static số_nguyên next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  nếu( pObj ){
    nếu( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_IncrRefCount(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_DecrRefCount(p);
      pCur->pList2 = Tcl_GetObjResult(interp);
      Tcl_IncrRefCount(pCur->pList2);
      assert( pCur->i2==0 );
    }không_thì{
      Tcl_Size n = 0;
      pCur->i2++;
      Tcl_ListObjLength(0, pCur->pList2, &n);
      nếu( pCur->i2>=n ){
        Tcl_DecrRefCount(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        trả_về 0;
      }
    }
  }

  trả_về 1;
}

static số_nguyên tclvarNext(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_Size n = 0;
  số_nguyên ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_ListObjLength(0, pCur->pList1, &n);
  trong_khi( !ok && pCur->i1<n ){
    Tcl_ListObjIndex(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    nếu( !ok ){
      pCur->i1++;
    }
  }

  trả_về 0;
}

static số_nguyên tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  số_nguyên idxNum, const char *idxStr,
  số_nguyên argc, sqlite3_value **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  const char *zEq = "";
  const char *zMatch = "";
  const char *zGlob = "";
  const char *zRegexp = "";
  const char *zLike = "";
  số_nguyên i;

  cho(i=0; idxStr[i]; i++){
    switch( idxStr[i] ){
      case TCLVAR_NAME_EQ:
        zEq = (const char*)sqlite3_value_text(argv[i]);
        break;
      case TCLVAR_NAME_MATCH:
        zMatch = (const char*)sqlite3_value_text(argv[i]);
        break;
      case TCLVAR_VALUE_GLOB:
        zGlob = (const char*)sqlite3_value_text(argv[i]);
        break;
      case TCLVAR_VALUE_REGEXP:
        zRegexp = (const char*)sqlite3_value_text(argv[i]);
        break;
      case TCLVAR_VALUE_LIKE:
        zLike = (const char*)sqlite3_value_text(argv[i]);
        break;
      default:
        assert( 0 );
    }
  }

  Tcl_IncrRefCount(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  nếu( pCur->pList1 ){
    Tcl_DecrRefCount(pCur->pList1);
  }
  nếu( pCur->pList2 ){
    Tcl_DecrRefCount(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_GetObjResult(interp);
  Tcl_IncrRefCount(pCur->pList1);

  Tcl_DecrRefCount(p);
  trả_về tclvarNext(pVtabCursor);
}

static số_nguyên tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, số_nguyên i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  const char *z1; 
  const char *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_ListObjIndex(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_ListObjIndex(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  nếu( p2 ){
    z2 = Tcl_GetString(p2);
  }
  switch (i) {
    case 0: {
      sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      break;
    }
    case 1: {
      sqlite3_result_text(ctx, z2, -1, SQLITE_TRANSIENT);
      break;
    }
    case 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_result_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      break;
    }
    case 3: {
      char *z3;
      nếu( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_result_text(ctx, z3, -1, sqlite3_free);
      }không_thì{
        sqlite3_result_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      break;
    }
  }
  trả_về SQLITE_OK;
}

static số_nguyên tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  trả_về SQLITE_OK;
}

static số_nguyên tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  trả_về (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
static số_nguyên tclvarAddToIdxstr(char *zStr, char x){
  số_nguyên i;
  cho(i=0; zStr[i]; i++){
    nếu( zStr[i]==x ) trả_về 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  trả_về 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
static số_nguyên tclvarSetOmit(Tcl_Interp *interp){
  số_nguyên rc;
  số_nguyên res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  nếu( rc==TCL_OK ){
    pRes = Tcl_GetObjResult(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  trả_về (rc==TCL_OK && res);
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
static số_nguyên tclvarBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  số_nguyên ii;
  char *zStr = sqlite3_malloc(32);
  số_nguyên iStr = 0;

  nếu( zStr==0 ) trả_về SQLITE_NOMEM;
  zStr[0] = '\0';

  cho(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *pCons = &pIdxInfo->aConstraint[ii];
    struct sqlite3_index_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    nếu( pCons->usable ){
      /* name = ? */
      nếu( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && pCons->iColumn==0 ){
        nếu( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      nếu( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        nếu( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      nếu( pCons->op==SQLITE_INDEX_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        nếu( 0==tclvarAddToIdxstr(zStr, TCLVAR_VALUE_GLOB) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      nếu( pCons->op==SQLITE_INDEX_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        nếu( 0==tclvarAddToIdxstr(zStr, TCLVAR_VALUE_REGEXP) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      nếu( pCons->op==SQLITE_INDEX_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        nếu( 0==tclvarAddToIdxstr(zStr, TCLVAR_VALUE_LIKE) ){
          pUsage->argvIndex = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  trả_về SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
static số_nguyên tclvarUpdate(
  sqlite3_vtab *tab,
  số_nguyên argc,
  sqlite3_value **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  nếu( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    const char *zVar = (const char*)sqlite3_value_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    trả_về SQLITE_OK;
  }
  nếu( sqlite3_value_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    const char *zValue = (const char*)sqlite3_value_text(argv[4]);
    const char *zName;
    nếu( sqlite3_value_type(argv[5])!=SQLITE_TEXT ){
      tab->zErrMsg = sqlite3_mprintf("the 'fullname' column must be TEXT");
      trả_về SQLITE_ERROR;
    }
    zName = (const char*)sqlite3_value_text(argv[5]);
    nếu( zValue ){
      Tcl_SetVar(pTab->interp, zName, zValue, TCL_GLOBAL_ONLY);
    }không_thì{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    trả_về SQLITE_OK;
  }
  nếu( sqlite3_value_type(argv[0])==SQLITE_TEXT
   && sqlite3_value_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    const char *zOldName = (const char*)sqlite3_value_text(argv[0]);
    const char *zNewName = (const char*)sqlite3_value_text(argv[1]);
    const char *zValue = (const char*)sqlite3_value_text(argv[4]);

    nếu( strcmp(zOldName, zNewName)!=0 || zValue==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    nếu( zValue!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, zValue, TCL_GLOBAL_ONLY);
    }
    trả_về SQLITE_OK;
  }
  tab->zErrMsg = sqlite3_mprintf("prohibited TCL variable change");
  trả_về SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
static sqlite3_module tclvarModule = {
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
extern số_nguyên getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
static số_nguyên SQLITE_TCLAPI register_tclvar_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  số_nguyên objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  số_nguyên rc = TCL_OK;
  sqlite3 *db;
  nếu( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    trả_về TCL_ERROR;
  }
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) trả_về TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (void*)interp);
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
  trả_về rc;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
số_nguyên Sqlitetesttclvar_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  số_nguyên i;
  cho(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  trả_về TCL_OK;
}
