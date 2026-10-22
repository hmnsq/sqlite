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
ساکن خالی halfFunc(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  sqlite3_نتیجہ_double(context, 0.5*sqlite3_قدر_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ساکن خالی حالت_Func(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح op = 0, mx, cur, resetFlag, rc;
  اگر( sqlite3_قدر_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_قدر_int(argv[0]);
  }ورنہ اگر( sqlite3_قدر_type(argv[0])==SQLITE_متن ){
    صحیح i;
    ثابت حرف *zName;
    ساکن ثابت ساخت {
      ثابت حرف *zName;
      صحیح op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_حالت_حافظہ_مستعمل         },
      { "PAGECACHE_USED",      SQLITE_حالت_PAGECACHE_مستعمل      },
      { "PAGECACHE_OVERFLOW",  SQLITE_حالت_PAGECACHE_حد_سے_تجاوز  },
      { "SCRATCH_USED",        SQLITE_حالت_SCRATCH_مستعمل        },
      { "SCRATCH_OVERFLOW",    SQLITE_حالت_SCRATCH_حد_سے_تجاوز    },
      { "MALLOC_SIZE",         SQLITE_حالت_MALLOC_سائز         },
    };
    صحیح nOp = حجم(aOp)/حجم(aOp[0]);
    zName = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    برائے(i=0; i<nOp; i++){
      اگر( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        توڑو;
      }
    }
    اگر( i>=nOp ){
      حرف *z_پیغام = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_نتیجہ_error(context, z_پیغام, -1);
      sqlite3_free(z_پیغام);
      واپس;
    }
  }ورنہ{
    sqlite3_نتیجہ_error(context, "unknown status type", -1);
    واپس;
  }
  اگر( argc==2 ){
    resetFlag = sqlite3_قدر_int(argv[1]);
  }ورنہ{
    resetFlag = 0;
  }
  rc = sqlite3_حالت(op, &cur, &mx, resetFlag);
  اگر( rc!=SQLITE_OK ){
    حرف *z_پیغام = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_نتیجہ_error(context, z_پیغام, -1);
    sqlite3_free(z_پیغام);
    واپس;
  } 
  اگر( argc==2 ){
    sqlite3_نتیجہ_int(context, mx);
  }ورنہ{
    sqlite3_نتیجہ_int(context, cur);
  }
}

/*
** Extension load function.
*/
#اگر_تعریف_شدہ _WIN32
__declspec(dllexport)
#اگر_ختم
صحیح testloadext_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  صحیح nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          حالت_Func, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          حالت_Func, 0, 0);
  واپس nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#اگر_تعریف_شدہ _WIN32
__declspec(dllexport)
#اگر_ختم
صحیح testbrokenext_init(
  sqlite3 *db, 
  حرف **pz_Err_پیغام, 
  ثابت sqlite3_api_routines *pApi
){
  حرف *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_پیغام = zErr;
  واپس 1;
}
