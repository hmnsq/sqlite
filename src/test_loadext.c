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
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
স্থির শূন্য halfFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_মান_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
স্থির শূন্য statusFunc(
  sqlite3_context *context,
  পূর্ণাঙ্ক argc,
  sqlite3_মান **argv
){
  পূর্ণাঙ্ক op = 0, mx, cur, resetFlag, rc;
  যদি( sqlite3_মান_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_মান_int(argv[0]);
  }অন্যথা যদি( sqlite3_মান_type(argv[0])==SQLITE_TEXT ){
    পূর্ণাঙ্ক i;
    ধ্রুৱ ৱর্ণ *zName;
    স্থির ধ্রুৱ সংরচনা {
      ধ্রুৱ ৱর্ণ *zName;
      পূর্ণাঙ্ক op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    পূর্ণাঙ্ক nOp = পরিমাণ(aOp)/পরিমাণ(aOp[0]);
    zName = (ধ্রুৱ ৱর্ণ*)sqlite3_মান_text(argv[0]);
    বাবে(i=0; i<nOp; i++){
      যদি( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        ভঙ্গ;
      }
    }
    যদি( i>=nOp ){
      ৱর্ণ *z_বাৰ্তা = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_বাৰ্তা, -1);
      sqlite3_free(z_বাৰ্তা);
      ঘূৰাই;
    }
  }অন্যথা{
    sqlite3_result_error(context, "unknown status type", -1);
    ঘূৰাই;
  }
  যদি( argc==2 ){
    resetFlag = sqlite3_মান_int(argv[1]);
  }অন্যথা{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  যদি( rc!=SQLITE_OK ){
    ৱর্ণ *z_বাৰ্তা = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_বাৰ্তা, -1);
    sqlite3_free(z_বাৰ্তা);
    ঘূৰাই;
  } 
  যদি( argc==2 ){
    sqlite3_result_int(context, mx);
  }অন্যথা{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
পূর্ণাঙ্ক testloadext_init(
  sqlite3 *db, 
  ৱর্ণ **pz_Err_বাৰ্তা, 
  ধ্রুৱ sqlite3_api_routines *pApi
){
  পূর্ণাঙ্ক nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  ঘূৰাই nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
পূর্ণাঙ্ক testbrokenext_init(
  sqlite3 *db, 
  ৱর্ণ **pz_Err_বাৰ্তা, 
  ধ্রুৱ sqlite3_api_routines *pApi
){
  ৱর্ণ *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_বাৰ্তা = zErr;
  ঘূৰাই 1;
}
