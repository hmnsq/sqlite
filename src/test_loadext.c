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
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
ସ୍ଥିର ଶୂନ୍ୟ halfFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_ମୂଲ୍ୟ_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ସ୍ଥିର ଶୂନ୍ୟ statusFunc(
  sqlite3_context *context,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = 0, mx, cur, resetFlag, rc;
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_ମୂଲ୍ୟ_int(argv[0]);
  }ଅନ୍ୟଥା ଯଦି( sqlite3_ମୂଲ୍ୟ_type(argv[0])==SQLITE_TEXT ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଅଚଳ ଅକ୍ଷର *zName;
    ସ୍ଥିର ଅଚଳ ସଂରଚନା {
      ଅଚଳ ଅକ୍ଷର *zName;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOp = ଆକାର(aOp)/ଆକାର(aOp[0]);
    zName = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    ପାଇଁ(i=0; i<nOp; i++){
      ଯଦି( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        ଭାଙ୍ଗ;
      }
    }
    ଯଦି( i>=nOp ){
      ଅକ୍ଷର *z_ସନ୍ଦେଶ = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_ସନ୍ଦେଶ, -1);
      sqlite3_free(z_ସନ୍ଦେଶ);
      ଫେରନ୍ତୁ;
    }
  }ଅନ୍ୟଥା{
    sqlite3_result_error(context, "unknown status type", -1);
    ଫେରନ୍ତୁ;
  }
  ଯଦି( argc==2 ){
    resetFlag = sqlite3_ମୂଲ୍ୟ_int(argv[1]);
  }ଅନ୍ୟଥା{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  ଯଦି( rc!=SQLITE_OK ){
    ଅକ୍ଷର *z_ସନ୍ଦେଶ = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_ସନ୍ଦେଶ, -1);
    sqlite3_free(z_ସନ୍ଦେଶ);
    ଫେରନ୍ତୁ;
  } 
  ଯଦି( argc==2 ){
    sqlite3_result_int(context, mx);
  }ଅନ୍ୟଥା{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା testloadext_init(
  sqlite3 *db, 
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ, 
  ଅଚଳ sqlite3_api_routines *pApi
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  ଫେରନ୍ତୁ nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ପୂର୍ଣ୍ଣସଂଖ୍ୟା testbrokenext_init(
  sqlite3 *db, 
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ, 
  ଅଚଳ sqlite3_api_routines *pApi
){
  ଅକ୍ଷର *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_ସନ୍ଦେଶ = zErr;
  ଫେରନ୍ତୁ 1;
}
