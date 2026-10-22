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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  sqlite3_ಫಲಿತಾಂಶ_double(context, 0.5*sqlite3_ಮೌಲ್ಯ_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸ್ಥಿತಿ_Func(
  sqlite3_context *context,
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ op = 0, mx, cur, resetFlag, rc;
  ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[0])==SQLITE_ಪಠ್ಯ ){
    ಪೂರ್ಣಾಂಕ i;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
      ಪೂರ್ಣಾಂಕ op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_ಸ್ಥಿತಿ_ಸ್ಮರಣೆ_ಬಳಸಿದ         },
      { "PAGECACHE_USED",      SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಬಳಸಿದ      },
      { "PAGECACHE_OVERFLOW",  SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಅತಿಪ್ರವಾಹ  },
      { "SCRATCH_USED",        SQLITE_ಸ್ಥಿತಿ_SCRATCH_ಬಳಸಿದ        },
      { "SCRATCH_OVERFLOW",    SQLITE_ಸ್ಥಿತಿ_SCRATCH_ಅತಿಪ್ರವಾಹ    },
      { "MALLOC_SIZE",         SQLITE_ಸ್ಥಿತಿ_MALLOC_ಗಾತ್ರ         },
    };
    ಪೂರ್ಣಾಂಕ nOp = ಗಾತ್ರ(aOp)/ಗಾತ್ರ(aOp[0]);
    zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    ಗಾಗಿ(i=0; i<nOp; i++){
      ಆದರೆ( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        ನಿಲ್ಲಿಸು;
      }
    }
    ಆದರೆ( i>=nOp ){
      ಅಕ್ಷರ *z_ಸಂದೇಶ = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_ಫಲಿತಾಂಶ_error(context, z_ಸಂದೇಶ, -1);
      sqlite3_free(z_ಸಂದೇಶ);
      ಹಿಂತಿರುಗಿ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_error(context, "unknown status type", -1);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( argc==2 ){
    resetFlag = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  }ಇಲ್ಲದಿದ್ದರೆ{
    resetFlag = 0;
  }
  rc = sqlite3_ಸ್ಥಿತಿ(op, &cur, &mx, resetFlag);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಅಕ್ಷರ *z_ಸಂದೇಶ = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_ಫಲಿತಾಂಶ_error(context, z_ಸಂದೇಶ, -1);
    sqlite3_free(z_ಸಂದೇಶ);
    ಹಿಂತಿರುಗಿ;
  } 
  ಆದರೆ( argc==2 ){
    sqlite3_ಫಲಿತಾಂಶ_int(context, mx);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ಫಲಿತಾಂಶ_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
__declspec(dllexport)
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ testloadext_init(
  sqlite3 *db, 
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, 
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  ಪೂರ್ಣಾಂಕ nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          ಸ್ಥಿತಿ_Func, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          ಸ್ಥಿತಿ_Func, 0, 0);
  ಹಿಂತಿರುಗಿ nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
__declspec(dllexport)
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ testbrokenext_init(
  sqlite3 *db, 
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ, 
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pApi
){
  ಅಕ್ಷರ *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_ಸಂದೇಶ = zErr;
  ಹಿಂತಿರುಗಿ 1;
}
