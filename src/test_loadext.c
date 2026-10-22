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
#include <string.h>
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
static void halfFunc(
  sqlite3_context *context,
  целое argc,
  sqlite3_value **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_value_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
static void statusFunc(
  sqlite3_context *context,
  целое argc,
  sqlite3_value **argv
){
  целое op = 0, mx, cur, resetFlag, rc;
  если( sqlite3_value_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_value_int(argv[0]);
  }иначе если( sqlite3_value_type(argv[0])==SQLITE_TEXT ){
    целое i;
    const char *zName;
    static const struct {
      const char *zName;
      целое op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    целое nOp = sizeof(aOp)/sizeof(aOp[0]);
    zName = (const char*)sqlite3_value_text(argv[0]);
    для(i=0; i<nOp; i++){
      если( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        break;
      }
    }
    если( i>=nOp ){
      char *zMsg = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, zMsg, -1);
      sqlite3_free(zMsg);
      вернуть;
    }
  }иначе{
    sqlite3_result_error(context, "unknown status type", -1);
    вернуть;
  }
  если( argc==2 ){
    resetFlag = sqlite3_value_int(argv[1]);
  }иначе{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  если( rc!=SQLITE_OK ){
    char *zMsg = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, zMsg, -1);
    sqlite3_free(zMsg);
    вернуть;
  } 
  если( argc==2 ){
    sqlite3_result_int(context, mx);
  }иначе{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
целое testloadext_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  целое nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  вернуть nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
целое testbrokenext_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  char *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pzErrMsg = zErr;
  вернуть 1;
}
