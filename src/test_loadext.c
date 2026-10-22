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
#अन्तर्भाव <string.h>
#अन्तर्भाव "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
स्थिर शून्य halfFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_मानम्_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
स्थिर शून्य statusFunc(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क op = 0, mx, cur, resetFlag, rc;
  सचे( sqlite3_मानम्_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_मानम्_int(argv[0]);
  }अञ्ञथा सचे( sqlite3_मानम्_type(argv[0])==SQLITE_TEXT ){
    पूर्णाङ्क i;
    ध्रुव वर्ण *zName;
    स्थिर ध्रुव संरचना {
      ध्रुव वर्ण *zName;
      पूर्णाङ्क op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    पूर्णाङ्क nOp = परिमाण(aOp)/परिमाण(aOp[0]);
    zName = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
    अत्थाय(i=0; i<nOp; i++){
      सचे( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        भङ्ग;
      }
    }
    सचे( i>=nOp ){
      वर्ण *z_सन्देशः = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_सन्देशः, -1);
      sqlite3_free(z_सन्देशः);
      पटिनिवत्त;
    }
  }अञ्ञथा{
    sqlite3_result_error(context, "unknown status type", -1);
    पटिनिवत्त;
  }
  सचे( argc==2 ){
    resetFlag = sqlite3_मानम्_int(argv[1]);
  }अञ्ञथा{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  सचे( rc!=SQLITE_OK ){
    वर्ण *z_सन्देशः = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_सन्देशः, -1);
    sqlite3_free(z_सन्देशः);
    पटिनिवत्त;
  } 
  सचे( argc==2 ){
    sqlite3_result_int(context, mx);
  }अञ्ञथा{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
पूर्णाङ्क testloadext_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  पूर्णाङ्क nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  पटिनिवत्त nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
पूर्णाङ्क testbrokenext_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  वर्ण *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_सन्देशः = zErr;
  पटिनिवत्त 1;
}
