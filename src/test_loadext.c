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
  puur_naa_nka argc,
  sqlite3_value **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_value_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
static void statusFunc(
  sqlite3_context *context,
  puur_naa_nka argc,
  sqlite3_value **argv
){
  puur_naa_nka op = 0, mx, cur, resetFlag, rc;
  ᯀᯮᯝ( sqlite3_value_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_value_int(argv[0]);
  }ᯀᯮᯝᯂᯪ ᯀᯮᯝ( sqlite3_value_type(argv[0])==SQLITE_TEXT ){
    puur_naa_nka i;
    const var_na *zName;
    static const sa_nracanaa {
      const var_na *zName;
      puur_naa_nka op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    puur_naa_nka nOp = parimaa_na(aOp)/parimaa_na(aOp[0]);
    zName = (const var_na*)sqlite3_value_text(argv[0]);
    for(i=0; i<nOp; i++){
      ᯀᯮᯝ( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        bha_nga;
      }
    }
    ᯀᯮᯝ( i>=nOp ){
      var_na *zMsg = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, zMsg, -1);
      sqlite3_free(zMsg);
      ᯀᯮᯞ;
    }
  }ᯀᯮᯝᯂᯪ{
    sqlite3_result_error(context, "unknown status type", -1);
    ᯀᯮᯞ;
  }
  ᯀᯮᯝ( argc==2 ){
    resetFlag = sqlite3_value_int(argv[1]);
  }ᯀᯮᯝᯂᯪ{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  ᯀᯮᯝ( rc!=SQLITE_OK ){
    var_na *zMsg = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, zMsg, -1);
    sqlite3_free(zMsg);
    ᯀᯮᯞ;
  } 
  ᯀᯮᯝ( argc==2 ){
    sqlite3_result_int(context, mx);
  }ᯀᯮᯝᯂᯪ{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
puur_naa_nka testloadext_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  puur_naa_nka nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  ᯀᯮᯞ nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
puur_naa_nka testbrokenext_init(
  sqlite3 *db, 
  var_na **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  var_na *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pzErrMsg = zErr;
  ᯀᯮᯞ 1;
}
