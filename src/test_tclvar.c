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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE

/*
** Characters that make up the idxStr created by xBestIndex for xFilter.
*/
#घोषणा TCLVAR_NAME_EQ      'e'
#घोषणा TCLVAR_NAME_MATCH   'm'
#घोषणा TCLVAR_मूल्य_GLOB   'g'
#घोषणा TCLVAR_मूल्य_REGEXP 'r'
#घोषणा TCLVAR_मूल्य_LIKE   'l'

प्रकार काष्ठा tclvar_vtab tclvar_vtab;
प्रकार काष्ठा tclvar_cursor tclvar_cursor;

/* 
** A tclvar virtual-table object 
*/
काष्ठा tclvar_vtab {
  sqlite3_vtab base;
  Tcl_Interp *interp;
};

/* A tclvar cursor object */
काष्ठा tclvar_cursor {
  sqlite3_vtab_cursor base;

  Tcl_Obj *pList1;     /* Result of [info vars ?pattern?] */
  Tcl_Obj *pList2;     /* Result of [array names [lindex $pList1 $i1]] */
  Tcl_आकार i1;              /* Current item in pList1 */
  Tcl_आकार i2;              /* Current item (if any) in pList2 */
};

/* Methods for the tclvar module */
अटल पूर्णांक tclvarConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  tclvar_vtab *pVtab;
  अटल स्थिर अक्षर zSchema[] = 
     "CREATE TABLE x("
     "  name TEXT,"                       /* Base name */
     "  arrayname TEXT,"                  /* Array index */
     "  value TEXT,"                      /*मूल्य*/
     "  fullname TEXT PRIMARY KEY"        /* base(index) name */
     ") WITHOUT ROWID";
  pVtab = sqlite3MallocZero( माप(*pVtab) );
  अगर( pVtab==0 ) वापस SQLITE_NOMEM;
  *ppVtab = &pVtab->base;
  pVtab->interp = (Tcl_Interp *)pAux;
  sqlite3_declare_vtab(db, zSchema);
  वापस SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

अटल पूर्णांक tclvarDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new tclvar cursor.
*/
अटल पूर्णांक tclvarOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  tclvar_cursor *pCur;
  pCur = sqlite3MallocZero(माप(tclvar_cursor));
  *ppCursor = &pCur->base;
  वापस SQLITE_OK;
}

/*
** Close a tclvar cursor.
*/
अटल पूर्णांक tclvarClose(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  अगर( pCur->pList1 ){
    Tcl_Decr_Ref_गिनती(pCur->pList1);
  }
  अगर( pCur->pList2 ){
    Tcl_Decr_Ref_गिनती(pCur->pList2);
  }
  sqlite3_free(pCur);
  वापस SQLITE_OK;
}

/*
** Returns 1 if data is ready, or 0 if not.
*/
अटल पूर्णांक next2(Tcl_Interp *interp, tclvar_cursor *pCur, Tcl_Obj *pObj){
  Tcl_Obj *p;

  अगर( pObj ){
    अगर( !pCur->pList2 ){
      p = Tcl_NewStringObj("array names", -1);
      Tcl_Incr_Ref_गिनती(p);
      Tcl_ListObjAppendElement(0, p, pObj);
      Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
      Tcl_Decr_Ref_गिनती(p);
      pCur->pList2 = Tcl_Get_Obj_परिणाम(interp);
      Tcl_Incr_Ref_गिनती(pCur->pList2);
      निश्चित( pCur->i2==0 );
    }अन्यथा{
      Tcl_आकार n = 0;
      pCur->i2++;
      Tcl_List_Obj_लंबाई(0, pCur->pList2, &n);
      अगर( pCur->i2>=n ){
        Tcl_Decr_Ref_गिनती(pCur->pList2);
        pCur->pList2 = 0;
        pCur->i2 = 0;
        वापस 0;
      }
    }
  }

  वापस 1;
}

अटल पूर्णांक tclvar_अग्र(sqlite3_vtab_cursor *cur){
  Tcl_Obj *pObj;
  Tcl_आकार n = 0;
  पूर्णांक ok = 0;

  tclvar_cursor *pCur = (tclvar_cursor *)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)(cur->pVtab))->interp;

  Tcl_List_Obj_लंबाई(0, pCur->pList1, &n);
  जबतक( !ok && pCur->i1<n ){
    Tcl_List_Obj_सूचकांक(0, pCur->pList1, pCur->i1, &pObj);
    ok = next2(interp, pCur, pObj);
    अगर( !ok ){
      pCur->i1++;
    }
  }

  वापस 0;
}

अटल पूर्णांक tclvarFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  tclvar_cursor *pCur = (tclvar_cursor *)pVtabCursor;
  Tcl_Interp *interp = ((tclvar_vtab *)(pVtabCursor->pVtab))->interp;
  Tcl_Obj *p = Tcl_NewStringObj("tclvar_filter_cmd", -1);

  स्थिर अक्षर *zEq = "";
  स्थिर अक्षर *zMatch = "";
  स्थिर अक्षर *zGlob = "";
  स्थिर अक्षर *zRegexp = "";
  स्थिर अक्षर *zLike = "";
  पूर्णांक i;

  क्रम(i=0; idxStr[i]; i++){
    चयन( idxStr[i] ){
      हाल TCLVAR_NAME_EQ:
        zEq = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
        अवरोध;
      हाल TCLVAR_NAME_MATCH:
        zMatch = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
        अवरोध;
      हाल TCLVAR_मूल्य_GLOB:
        zGlob = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
        अवरोध;
      हाल TCLVAR_मूल्य_REGEXP:
        zRegexp = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
        अवरोध;
      हाल TCLVAR_मूल्य_LIKE:
        zLike = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[i]);
        अवरोध;
      शेष:
        निश्चित( 0 );
    }
  }

  Tcl_Incr_Ref_गिनती(p);
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zEq, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zMatch, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zGlob, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zRegexp, -1));
  Tcl_ListObjAppendElement(0, p, Tcl_NewStringObj(zLike, -1));

  Tcl_EvalObjEx(interp, p, TCL_EVAL_GLOBAL);
  अगर( pCur->pList1 ){
    Tcl_Decr_Ref_गिनती(pCur->pList1);
  }
  अगर( pCur->pList2 ){
    Tcl_Decr_Ref_गिनती(pCur->pList2);
    pCur->pList2 = 0;
  }
  pCur->i1 = 0;
  pCur->i2 = 0;
  pCur->pList1 = Tcl_Get_Obj_परिणाम(interp);
  Tcl_Incr_Ref_गिनती(pCur->pList1);

  Tcl_Decr_Ref_गिनती(p);
  वापस tclvar_अग्र(pVtabCursor);
}

अटल पूर्णांक tclvarColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  Tcl_Obj *p1;
  Tcl_Obj *p2;
  स्थिर अक्षर *z1; 
  स्थिर अक्षर *z2 = "";
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  Tcl_Interp *interp = ((tclvar_vtab *)cur->pVtab)->interp;

  Tcl_List_Obj_सूचकांक(interp, pCur->pList1, pCur->i1, &p1);
  Tcl_List_Obj_सूचकांक(interp, pCur->pList2, pCur->i2, &p2);
  z1 = Tcl_GetString(p1);
  अगर( p2 ){
    z2 = Tcl_GetString(p2);
  }
  चयन (i) {
    हाल 0: {
      sqlite3_परिणाम_text(ctx, z1, -1, SQLITE_TRANSIENT);
      अवरोध;
    }
    हाल 1: {
      sqlite3_परिणाम_text(ctx, z2, -1, SQLITE_TRANSIENT);
      अवरोध;
    }
    हाल 2: {
      Tcl_Obj *pVal = Tcl_GetVar2Ex(interp, z1, *z2?z2:0, TCL_GLOBAL_ONLY);
      sqlite3_परिणाम_text(ctx, Tcl_GetString(pVal), -1, SQLITE_TRANSIENT);
      अवरोध;
    }
    हाल 3: {
      अक्षर *z3;
      अगर( p2 ){
        z3 = sqlite3_mprintf("%s(%s)", z1, z2);
        sqlite3_परिणाम_text(ctx, z3, -1, sqlite3_free);
      }अन्यथा{
        sqlite3_परिणाम_text(ctx, z1, -1, SQLITE_TRANSIENT);
      }
      अवरोध;
    }
  }
  वापस SQLITE_OK;
}

अटल पूर्णांक tclvarRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  वापस SQLITE_OK;
}

अटल पूर्णांक tclvarEof(sqlite3_vtab_cursor *cur){
  tclvar_cursor *pCur = (tclvar_cursor*)cur;
  वापस (pCur->pList2?0:1);
}

/*
** If nul-terminated string zStr does not already contain the character 
** passed as the second argument, append it and return 0. Or, if there is
** already an instance of x in zStr, do nothing return 1;
**
** There is guaranteed to be enough room in the buffer pointed to by zStr
** for the new character and nul-terminator.
*/
अटल पूर्णांक tclvarAddToIdxstr(अक्षर *zStr, अक्षर x){
  पूर्णांक i;
  क्रम(i=0; zStr[i]; i++){
    अगर( zStr[i]==x ) वापस 1;
  }
  zStr[i] = x;
  zStr[i+1] = '\0';
  वापस 0;
}

/*
** Return true if variable $::tclvar_set_omit exists and is set to true.
** False otherwise.
*/
अटल पूर्णांक tclvarSetOmit(Tcl_Interp *interp){
  पूर्णांक rc;
  पूर्णांक res = 0;
  Tcl_Obj *pRes;
  rc = Tcl_Eval(interp,
    "expr {[info exists ::tclvar_set_omit] && $::tclvar_set_omit}"
  );
  अगर( rc==TCL_OK ){
    pRes = Tcl_Get_Obj_परिणाम(interp);
    rc = Tcl_GetBooleanFromObj(0, pRes, &res);
  }
  वापस (rc==TCL_OK && res);
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
अटल पूर्णांक tclvar_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  पूर्णांक ii;
  अक्षर *zStr = sqlite3_malloc(32);
  पूर्णांक iStr = 0;

  अगर( zStr==0 ) वापस SQLITE_NOMEM;
  zStr[0] = '\0';

  क्रम(ii=0; ii<pIdxInfo->nConstraint; ii++){
    काष्ठा sqlite3_सूचकांक_constraint स्थिर *pCons = &pIdxInfo->aConstraint[ii];
    काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage;
    
    pUsage = &pIdxInfo->aConstraintUsage[ii];
    अगर( pCons->usable ){
      /* name = ? */
      अगर( pCons->op==SQLITE_सूचकांक_CONSTRAINT_EQ && pCons->iColumn==0 ){
        अगर( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_EQ) ){
          pUsage->argv_सूचकांक = ++iStr;
          pUsage->omit = 0;
        }
      }

      /* name MATCH ? */
      अगर( pCons->op==SQLITE_सूचकांक_CONSTRAINT_MATCH && pCons->iColumn==0 ){
        अगर( 0==tclvarAddToIdxstr(zStr, TCLVAR_NAME_MATCH) ){
          pUsage->argv_सूचकांक = ++iStr;
          pUsage->omit = 1;
        }
      }

      /* value GLOB ? */
      अगर( pCons->op==SQLITE_सूचकांक_CONSTRAINT_GLOB && pCons->iColumn==2 ){
        अगर( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_GLOB) ){
          pUsage->argv_सूचकांक = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value REGEXP ? */
      अगर( pCons->op==SQLITE_सूचकांक_CONSTRAINT_REGEXP && pCons->iColumn==2 ){
        अगर( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_REGEXP) ){
          pUsage->argv_सूचकांक = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }

      /* value LIKE ? */
      अगर( pCons->op==SQLITE_सूचकांक_CONSTRAINT_LIKE && pCons->iColumn==2 ){
        अगर( 0==tclvarAddToIdxstr(zStr, TCLVAR_मूल्य_LIKE) ){
          pUsage->argv_सूचकांक = ++iStr;
          pUsage->omit = tclvarSetOmit(pTab->interp);
        }
      }
    }
  }
  pIdxInfo->idxStr = zStr;
  pIdxInfo->needToFreeIdxStr = 1;

  वापस SQLITE_OK;
}

/*
** Invoked for any UPDATE, INSERT, or DELETE against a tclvar table
*/
अटल पूर्णांक tclvarUpdate(
  sqlite3_vtab *tab,
  पूर्णांक argc,
  sqlite3_मूल्य **argv,
  sqlite_int64 *pRowid
){
  tclvar_vtab *pTab = (tclvar_vtab*)tab;
  अगर( argc==1 ){
    /* A DELETE operation.  The variable to be deleted is stored in argv[0] */
    स्थिर अक्षर *zVar = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
    Tcl_UnsetVar(pTab->interp, zVar, TCL_GLOBAL_ONLY);
    वापस SQLITE_OK;
  }
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_NULL ){
    /* An INSERT operation */
    स्थिर अक्षर *z_मूल्य = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[4]);
    स्थिर अक्षर *zName;
    अगर( sqlite3_मूल्य_type(argv[5])!=SQLITE_TEXT ){
      tab->z_Err_संदेश = sqlite3_mprintf("the 'fullname' column must be TEXT");
      वापस SQLITE_ERROR;
    }
    zName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[5]);
    अगर( z_मूल्य ){
      Tcl_SetVar(pTab->interp, zName, z_मूल्य, TCL_GLOBAL_ONLY);
    }अन्यथा{
      Tcl_UnsetVar(pTab->interp, zName, TCL_GLOBAL_ONLY);
    }
    वापस SQLITE_OK;
  }
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_TEXT
   && sqlite3_मूल्य_type(argv[1])==SQLITE_TEXT
  ){
    /* An UPDATE operation */
    स्थिर अक्षर *zOldName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
    स्थिर अक्षर *zNewName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[1]);
    स्थिर अक्षर *z_मूल्य = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[4]);

    अगर( म_भेद(zOldName, zNewName)!=0 || z_मूल्य==0 ){
      Tcl_UnsetVar(pTab->interp, zOldName, TCL_GLOBAL_ONLY);
    }
    अगर( z_मूल्य!=0 ){
      Tcl_SetVar(pTab->interp, zNewName, z_मूल्य, TCL_GLOBAL_ONLY);
    }
    वापस SQLITE_OK;
  }
  tab->z_Err_संदेश = sqlite3_mprintf("prohibited TCL variable change");
  वापस SQLITE_ERROR;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
अटल sqlite3_module tclvarModule = {
  0,                         /* iVersion */
  tclvarConnect,
  tclvarConnect,
  tclvar_Best_सूचकांक,
  tclvarDisconnect, 
  tclvarDisconnect,
  tclvarOpen,                  /* xOpen - open a cursor */
  tclvarClose,                 /* xClose - close a cursor */
  tclvarFilter,                /* xFilter - configure scan constraints */
  tclvar_अग्र,                  /* xNext - advance a cursor */
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
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
अटल पूर्णांक SQLITE_TCLAPI register_tclvar_module(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  पूर्णांक rc = TCL_OK;
  sqlite3 *db;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "tclvar", &tclvarModule, (व्योम*)interp);
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
#पूर्ण_अगर
  वापस rc;
}

#पूर्ण_अगर


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetesttclvar_Init(Tcl_Interp *interp){
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     व्योम *client_तथ्य;
  } aObjCmd[] = {
     { "register_tclvar_module",   register_tclvar_module, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_तथ्य, 0);
  }
#पूर्ण_अगर
  वापस TCL_OK;
}
