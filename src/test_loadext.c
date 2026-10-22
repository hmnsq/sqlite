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
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
સ્થિર શૂન્ય halfFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  sqlite3_result_double(context, 0.5*sqlite3_કિંમત_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
સ્થિર શૂન્ય statusFunc(
  sqlite3_context *context,
  પૂર્ણાંક argc,
  sqlite3_કિંમત **argv
){
  પૂર્ણાંક op = 0, mx, cur, resetFlag, rc;
  જો( sqlite3_કિંમત_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_કિંમત_int(argv[0]);
  }નહીંતર જો( sqlite3_કિંમત_type(argv[0])==SQLITE_TEXT ){
    પૂર્ણાંક i;
    અચલ અક્ષર *zName;
    સ્થિર અચલ સંરચના {
      અચલ અક્ષર *zName;
      પૂર્ણાંક op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_STATUS_MEMORY_USED         },
      { "PAGECACHE_USED",      SQLITE_STATUS_PAGECACHE_USED      },
      { "PAGECACHE_OVERFLOW",  SQLITE_STATUS_PAGECACHE_OVERFLOW  },
      { "SCRATCH_USED",        SQLITE_STATUS_SCRATCH_USED        },
      { "SCRATCH_OVERFLOW",    SQLITE_STATUS_SCRATCH_OVERFLOW    },
      { "MALLOC_SIZE",         SQLITE_STATUS_MALLOC_SIZE         },
    };
    પૂર્ણાંક nOp = કદ(aOp)/કદ(aOp[0]);
    zName = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    માટે(i=0; i<nOp; i++){
      જો( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        તોડો;
      }
    }
    જો( i>=nOp ){
      અક્ષર *z_સંદેશ = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_result_error(context, z_સંદેશ, -1);
      sqlite3_free(z_સંદેશ);
      પાછા;
    }
  }નહીંતર{
    sqlite3_result_error(context, "unknown status type", -1);
    પાછા;
  }
  જો( argc==2 ){
    resetFlag = sqlite3_કિંમત_int(argv[1]);
  }નહીંતર{
    resetFlag = 0;
  }
  rc = sqlite3_status(op, &cur, &mx, resetFlag);
  જો( rc!=SQLITE_OK ){
    અક્ષર *z_સંદેશ = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_result_error(context, z_સંદેશ, -1);
    sqlite3_free(z_સંદેશ);
    પાછા;
  } 
  જો( argc==2 ){
    sqlite3_result_int(context, mx);
  }નહીંતર{
    sqlite3_result_int(context, cur);
  }
}

/*
** Extension load function.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
પૂર્ણાંક testloadext_init(
  sqlite3 *db, 
  અક્ષર **pz_Err_સંદેશ, 
  અચલ sqlite3_api_routines *pApi
){
  પૂર્ણાંક nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          statusFunc, 0, 0);
  પાછા nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
પૂર્ણાંક testbrokenext_init(
  sqlite3 *db, 
  અક્ષર **pz_Err_સંદેશ, 
  અચલ sqlite3_api_routines *pApi
){
  અક્ષર *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_સંદેશ = zErr;
  પાછા 1;
}
