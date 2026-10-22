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
#அந்தர்பாவ <string.h>
#அந்தர்பாவ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
நிலையான வெறுமை halfFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_மதிப்பு_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
நிலையான வெறுமை statusFunc(
  sqlite3_context *context,
  முழு argc,
  sqlite3_மதிப்பு **argv
){
  முழு op = 0, mx, cur, resetFlag, rc;
  எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_மதிப்பு_int(argv[0]);
  }இல்லையெனில் எனில்( sqlite3_மதிப்பு_type(argv[0])==SQLITE_TEXT ){
    முழு i;
    மாறிலி எழுத்து *zName;
    நிலையான மாறிலி கட்டமைப்பு {
      மாறிலி எழுத்து *zName;
      முழு op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    முழு nOp = அளவு(aOp)/அளவு(aOp[0]);
    zName = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    க்கு(i=0; i<nOp; i++){
      எனில்( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        நிறுத்து;
      }
    }
    எனில்( i>=nOp ){
      எழுத்து *z_செய்தி = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_செய்தி, -1);
      sqlite3_free(z_செய்தி);
      திரும்பு;
    }
  }இல்லையெனில்{
    sqlite3_result_error(context, "unknown status type", -1);
    திரும்பு;
  }
  எனில்( argc==2 ){
    resetFlag = sqlite3_மதிப்பு_int(argv[1]);
  }இல்லையெனில்{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  எனில்( rc!=SQLITE_OK ){
    எழுத்து *z_செய்தி = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_செய்தி, -1);
    sqlite3_free(z_செய்தி);
    திரும்பு;
  } 
  எனில்( argc==2 ){
    sqlite3_result_int(context, mx);
  }இல்லையெனில்{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
முழு testloadext_init(
  sqlite3 *db, 
  எழுத்து **pz_Err_செய்தி, 
  மாறிலி sqlite3_api_routines *pApi
){
  முழு nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  திரும்பு nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
முழு testbrokenext_init(
  sqlite3 *db, 
  எழுத்து **pz_Err_செய்தி, 
  மாறிலி sqlite3_api_routines *pApi
){
  எழுத்து *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_செய்தி = zErr;
  திரும்பு 1;
}
