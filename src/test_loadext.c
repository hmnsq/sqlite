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
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
സ്ഥിതം ശൂന്യം halfFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_മൂല്യം_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
സ്ഥിതം ശൂന്യം statusFunc(
  sqlite3_context *context,
  പൂർണ്ണസംഖ്യ argc,
  sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ op = 0, mx, cur, resetFlag, rc;
  എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_മൂല്യം_int(argv[0]);
  }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3_മൂല്യം_type(argv[0])==SQLITE_TEXT ){
    പൂർണ്ണസംഖ്യ i;
    സ്ഥിരം അക്ഷരം *zName;
    സ്ഥിതം സ്ഥിരം ഘടന {
      സ്ഥിരം അക്ഷരം *zName;
      പൂർണ്ണസംഖ്യ op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    പൂർണ്ണസംഖ്യ nOp = വലുപ്പം(aOp)/വലുപ്പം(aOp[0]);
    zName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    വേണ്ടി(i=0; i<nOp; i++){
      എങ്കിൽ( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        നിർത്തുക;
      }
    }
    എങ്കിൽ( i>=nOp ){
      അക്ഷരം *z_സന്ദേശം = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_സന്ദേശം, -1);
      sqlite3_free(z_സന്ദേശം);
      തിരികെ;
    }
  }അല്ലെങ്കിൽ{
    sqlite3_result_error(context, "unknown status type", -1);
    തിരികെ;
  }
  എങ്കിൽ( argc==2 ){
    resetFlag = sqlite3_മൂല്യം_int(argv[1]);
  }അല്ലെങ്കിൽ{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  എങ്കിൽ( rc!=SQLITE_OK ){
    അക്ഷരം *z_സന്ദേശം = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_സന്ദേശം, -1);
    sqlite3_free(z_സന്ദേശം);
    തിരികെ;
  } 
  എങ്കിൽ( argc==2 ){
    sqlite3_result_int(context, mx);
  }അല്ലെങ്കിൽ{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
പൂർണ്ണസംഖ്യ testloadext_init(
  sqlite3 *db, 
  അക്ഷരം **pz_Err_സന്ദേശം, 
  സ്ഥിരം sqlite3_api_routines *pApi
){
  പൂർണ്ണസംഖ്യ nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  തിരികെ nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
പൂർണ്ണസംഖ്യ testbrokenext_init(
  sqlite3 *db, 
  അക്ഷരം **pz_Err_സന്ദേശം, 
  സ്ഥിരം sqlite3_api_routines *pApi
){
  അക്ഷരം *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_സന്ദേശം = zErr;
  തിരികെ 1;
}
