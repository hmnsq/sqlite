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
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
ਸਥਿਰ ਖਾਲੀ halfFunc(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  sqlite3_ਨਤੀਜਾ_double(context, 0.5*sqlite3_ਮੁੱਲ_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ਸਥਿਰ ਖਾਲੀ ਸਥਿਤੀ_Func(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ op = 0, mx, cur, resetFlag, rc;
  ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_ਮੁੱਲ_int(argv[0]);
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_ਮੁੱਲ_type(argv[0])==SQLITE_ਪਾਠ ){
    ਪੂਰਨ i;
    ਅਚਲ ਅੱਖਰ *zName;
    ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
      ਅਚਲ ਅੱਖਰ *zName;
      ਪੂਰਨ op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_ਸਥਿਤੀ_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ         },
      { "PAGECACHE_USED",      SQLITE_ਸਥਿਤੀ_PAGECACHE_ਵਰਤਿਆ      },
      { "PAGECACHE_OVERFLOW",  SQLITE_ਸਥਿਤੀ_PAGECACHE_ਉਛਾਲ  },
      { "SCRATCH_USED",        SQLITE_ਸਥਿਤੀ_SCRATCH_ਵਰਤਿਆ        },
      { "SCRATCH_OVERFLOW",    SQLITE_ਸਥਿਤੀ_SCRATCH_ਉਛਾਲ    },
      { "MALLOC_SIZE",         SQLITE_ਸਥਿਤੀ_MALLOC_ਮਾਪ         },
    };
    ਪੂਰਨ nOp = ਆਕਾਰ(aOp)/ਆਕਾਰ(aOp[0]);
    zName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    ਲਈ(i=0; i<nOp; i++){
      ਜੇ( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        ਰੋਕੋ;
      }
    }
    ਜੇ( i>=nOp ){
      ਅੱਖਰ *z_ਸੁਨੇਹਾ = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_ਨਤੀਜਾ_error(context, z_ਸੁਨੇਹਾ, -1);
      sqlite3_free(z_ਸੁਨੇਹਾ);
      ਵਾਪਸ;
    }
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_error(context, "unknown status type", -1);
    ਵਾਪਸ;
  }
  ਜੇ( argc==2 ){
    resetFlag = sqlite3_ਮੁੱਲ_int(argv[1]);
  }ਨਹੀਂਤਾਂ{
    resetFlag = 0;
  }
  rc = sqlite3_ਸਥਿਤੀ(op, &cur, &mx, resetFlag);
  ਜੇ( rc!=SQLITE_OK ){
    ਅੱਖਰ *z_ਸੁਨੇਹਾ = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_ਨਤੀਜਾ_error(context, z_ਸੁਨੇਹਾ, -1);
    sqlite3_free(z_ਸੁਨੇਹਾ);
    ਵਾਪਸ;
  } 
  ਜੇ( argc==2 ){
    sqlite3_ਨਤੀਜਾ_int(context, mx);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਨਤੀਜਾ_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
__declspec(dllexport)
#ਜੇ_ਅੰਤ
ਪੂਰਨ testloadext_init(
  sqlite3 *db, 
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, 
  ਅਚਲ sqlite3_api_routines *pApi
){
  ਪੂਰਨ nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          ਸਥਿਤੀ_Func, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          ਸਥਿਤੀ_Func, 0, 0);
  ਵਾਪਸ nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
__declspec(dllexport)
#ਜੇ_ਅੰਤ
ਪੂਰਨ testbrokenext_init(
  sqlite3 *db, 
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ, 
  ਅਚਲ sqlite3_api_routines *pApi
){
  ਅੱਖਰ *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_ਸੁਨੇਹਾ = zErr;
  ਵਾਪਸ 1;
}
