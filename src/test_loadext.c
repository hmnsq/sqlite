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
#অন্তর্ভাব <string.h>
#অন্তর্ভাব "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
স্থির শূন্য halfFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  sqlite3_ফলাফল_double(context, 0.5*sqlite3_মান_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
স্থির শূন্য statusFunc(
  sqlite3_context *context,
  পূর্ণসংখ্যা argc,
  sqlite3_মান **argv
){
  পূর্ণসংখ্যা op = 0, mx, cur, resetFlag, rc;
  যদি( sqlite3_মান_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_মান_int(argv[0]);
  }অন্যথা যদি( sqlite3_মান_type(argv[0])==SQLITE_TEXT ){
    পূর্ণসংখ্যা i;
    ধ্রুবক অক্ষর *zName;
    স্থির ধ্রুবক কাঠামো {
      ধ্রুবক অক্ষর *zName;
      পূর্ণসংখ্যা op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_আকার         },
    };
    পূর্ণসংখ্যা nOp = আকার(aOp)/আকার(aOp[0]);
    zName = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
    জন্য(i=0; i<nOp; i++){
      যদি( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        বিরতি;
      }
    }
    যদি( i>=nOp ){
      অক্ষর *z_বার্তা = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_ফলাফল_error(context, z_বার্তা, -1);
      sqlite3_free(z_বার্তা);
      ফেরত;
    }
  }অন্যথা{
    sqlite3_ফলাফল_error(context, "unknown status type", -1);
    ফেরত;
  }
  যদি( argc==2 ){
    resetFlag = sqlite3_মান_int(argv[1]);
  }অন্যথা{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  যদি( rc!=SQLITE_OK ){
    অক্ষর *z_বার্তা = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_ফলাফল_error(context, z_বার্তা, -1);
    sqlite3_free(z_বার্তা);
    ফেরত;
  } 
  যদি( argc==2 ){
    sqlite3_ফলাফল_int(context, mx);
  }অন্যথা{
    sqlite3_ফলাফল_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
পূর্ণসংখ্যা testloadext_init(
  sqlite3 *db, 
  অক্ষর **pz_Err_বার্তা, 
  ধ্রুবক sqlite3_api_routines *pApi
){
  পূর্ণসংখ্যা nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  ফেরত nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
পূর্ণসংখ্যা testbrokenext_init(
  sqlite3 *db, 
  অক্ষর **pz_Err_বার্তা, 
  ধ্রুবক sqlite3_api_routines *pApi
){
  অক্ষর *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_বার্তা = zErr;
  ফেরত 1;
}
