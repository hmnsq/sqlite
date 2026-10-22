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
#شامل <string.h>
#شامل "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
ایستا تهی halfFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_مقدار_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ایستا تهی statusFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_مقدار **argv
){
  صحیح op = 0, mx, cur, resetFlag, rc;
  اگر( sqlite3_مقدار_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_مقدار_int(argv[0]);
  }وگرنه اگر( sqlite3_مقدار_type(argv[0])==SQLITE_TEXT ){
    صحیح i;
    ثابت نویسه *zName;
    ایستا ثابت ساختار {
      ثابت نویسه *zName;
      صحیح op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    صحیح nOp = اندازه(aOp)/اندازه(aOp[0]);
    zName = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    برای(i=0; i<nOp; i++){
      اگر( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        بشکن;
      }
    }
    اگر( i>=nOp ){
      نویسه *z_پیام = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_پیام, -1);
      sqlite3_free(z_پیام);
      بازگشت;
    }
  }وگرنه{
    sqlite3_result_error(context, "unknown status type", -1);
    بازگشت;
  }
  اگر( argc==2 ){
    resetFlag = sqlite3_مقدار_int(argv[1]);
  }وگرنه{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  اگر( rc!=SQLITE_OK ){
    نویسه *z_پیام = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_پیام, -1);
    sqlite3_free(z_پیام);
    بازگشت;
  } 
  اگر( argc==2 ){
    sqlite3_result_int(context, mx);
  }وگرنه{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
صحیح testloadext_init(
  sqlite3 *db, 
  نویسه **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  صحیح nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  بازگشت nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
صحیح testbrokenext_init(
  sqlite3 *db, 
  نویسه **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  نویسه *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_پیام = zErr;
  بازگشت 1;
}
