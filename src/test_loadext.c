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
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ halfFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_ಮೌಲ್ಯ_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ಸ್ಥಿರ ಶೂನ್ಯ statusFunc(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ op = 0, mx, cur, resetFlag, rc;
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_TEXT ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಧ್ರುವ ವರ್ಣ *zName;
    ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
      ಧ್ರುವ ವರ್ಣ *zName;
      ಪೂರ್ಣಾಙ್ಕ op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    ಪೂರ್ಣಾಙ್ಕ nOp = ಪರಿಮಾಣ(aOp)/ಪರಿಮಾಣ(aOp[0]);
    zName = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    ಪ್ರತಿ(i=0; i<nOp; i++){
      ಯದಿ( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        ಭಙ್ಗ;
      }
    }
    ಯದಿ( i>=nOp ){
      ವರ್ಣ *z_ಸಂದೇಶ = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_ಸಂದೇಶ, -1);
      sqlite3_free(z_ಸಂದೇಶ);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }ಅನ್ಯಥಾ{
    sqlite3_result_error(context, "unknown status type", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( argc==2 ){
    resetFlag = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  }ಅನ್ಯಥಾ{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  ಯದಿ( rc!=SQLITE_OK ){
    ವರ್ಣ *z_ಸಂದೇಶ = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_ಸಂದೇಶ, -1);
    sqlite3_free(z_ಸಂದೇಶ);
    ಪ್ರತ್ಯಾಗಮ;
  } 
  ಯದಿ( argc==2 ){
    sqlite3_result_int(context, mx);
  }ಅನ್ಯಥಾ{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ಪೂರ್ಣಾಙ್ಕ testloadext_init(
  sqlite3 *db, 
  ವರ್ಣ **pz_Err_ಸಂದೇಶ, 
  ಧ್ರುವ sqlite3_api_routines *pApi
){
  ಪೂರ್ಣಾಙ್ಕ nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  ಪ್ರತ್ಯಾಗಮ nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
ಪೂರ್ಣಾಙ್ಕ testbrokenext_init(
  sqlite3 *db, 
  ವರ್ಣ **pz_Err_ಸಂದೇಶ, 
  ಧ್ರುವ sqlite3_api_routines *pApi
){
  ವರ್ಣ *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_ಸಂದೇಶ = zErr;
  ಪ್ರತ್ಯಾಗಮ 1;
}
