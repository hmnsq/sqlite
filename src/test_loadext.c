/*
** 2006 June 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Test extension for testing the sqlite3_load_extension() function.
*/
#समावेश <string.h>
#समावेश "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
अटल व्योम halfFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  sqlite3_परिणाम_double(context, 0.5*sqlite3_मूल्य_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
अटल व्योम statusFunc(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक op = 0, mx, cur, resetFlag, rc;
  अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_मूल्य_int(argv[0]);
  }अन्यथा अगर( sqlite3_मूल्य_type(argv[0])==SQLITE_TEXT ){
    पूर्णांक i;
    स्थिर अक्षर *zName;
    अटल स्थिर काष्ठा {
      स्थिर अक्षर *zName;
      पूर्णांक op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_आकार         },
    };
    पूर्णांक nOp = माप(aOp)/माप(aOp[0]);
    zName = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
    क्रम(i=0; i<nOp; i++){
      अगर( म_भेद(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        अवरोध;
      }
    }
    अगर( i>=nOp ){
      अक्षर *z_संदेश = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_परिणाम_error(context, z_संदेश, -1);
      sqlite3_free(z_संदेश);
      वापस;
    }
  }अन्यथा{
    sqlite3_परिणाम_error(context, "unknown status type", -1);
    वापस;
  }
  अगर( argc==2 ){
    resetFlag = sqlite3_मूल्य_int(argv[1]);
  }अन्यथा{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  अगर( rc!=SQLITE_OK ){
    अक्षर *z_संदेश = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_परिणाम_error(context, z_संदेश, -1);
    sqlite3_free(z_संदेश);
    वापस;
  } 
  अगर( argc==2 ){
    sqlite3_परिणाम_int(context, mx);
  }अन्यथा{
    sqlite3_परिणाम_int(context, cur);
  }
}

/*
** Extension load function.
*/
#अगर_घोषित _WIN32
__declspec(dllexport)
#पूर्ण_अगर
पूर्णांक testloadext_init(
  sqlite3 *db, 
  अक्षर **pz_Err_संदेश, 
  स्थिर sqlite3_api_routines *pApi
){
  पूर्णांक nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  वापस nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#अगर_घोषित _WIN32
__declspec(dllexport)
#पूर्ण_अगर
पूर्णांक testbrokenext_init(
  sqlite3 *db, 
  अक्षर **pz_Err_संदेश, 
  स्थिर sqlite3_api_routines *pApi
){
  अक्षर *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_संदेश = zErr;
  वापस 1;
}
