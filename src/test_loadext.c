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
#ضمن <string.h>
#ضمن "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
ساكن فراغ halfFunc(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  sqlite3_نتیجه_double(context, 0.5*sqlite3_مقدار_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
ساكن فراغ وضعیت_Func(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_مقدار **argv
){
  صحيح op = 0, mx, cur, resetFlag, rc;
  إذا( sqlite3_مقدار_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_مقدار_int(argv[0]);
  }وإلا إذا( sqlite3_مقدار_type(argv[0])==SQLITE_متن ){
    صحيح i;
    ثابت حرف *zName;
    ساكن ثابت بنية {
      ثابت حرف *zName;
      صحيح op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_وضعیت_حافظه_استفاده_شده         },
      { "PAGECACHE_USED",      SQLITE_وضعیت_PAGECACHE_استفاده_شده      },
      { "PAGECACHE_OVERFLOW",  SQLITE_وضعیت_PAGECACHE_سرریز  },
      { "SCRATCH_USED",        SQLITE_وضعیت_SCRATCH_استفاده_شده        },
      { "SCRATCH_OVERFLOW",    SQLITE_وضعیت_SCRATCH_سرریز    },
      { "MALLOC_SIZE",         SQLITE_وضعیت_MALLOC_اندازه         },
    };
    صحيح nOp = حجم(aOp)/حجم(aOp[0]);
    zName = (ثابت حرف*)sqlite3_مقدار_متن(argv[0]);
    لكل(i=0; i<nOp; i++){
      إذا( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        اكسر;
      }
    }
    إذا( i>=nOp ){
      حرف *z_پیام = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_نتیجه_error(context, z_پیام, -1);
      sqlite3_free(z_پیام);
      أرجع;
    }
  }وإلا{
    sqlite3_نتیجه_error(context, "unknown status type", -1);
    أرجع;
  }
  إذا( argc==2 ){
    resetFlag = sqlite3_مقدار_int(argv[1]);
  }وإلا{
    resetFlag = 0;
  }
  rc = sqlite3_وضعیت(op, &cur, &mx, resetFlag);
  إذا( rc!=SQLITE_OK ){
    حرف *z_پیام = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_نتیجه_error(context, z_پیام, -1);
    sqlite3_free(z_پیام);
    أرجع;
  } 
  إذا( argc==2 ){
    sqlite3_نتیجه_int(context, mx);
  }وإلا{
    sqlite3_نتیجه_int(context, cur);
  }
}

/*
** Extension load function.
*/
#اگر_تعریف_شده _WIN32
__declspec(dllexport)
#پایان_اگر
صحيح testloadext_init(
  sqlite3 *db, 
  حرف **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  صحيح nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          وضعیت_Func, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          وضعیت_Func, 0, 0);
  أرجع nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#اگر_تعریف_شده _WIN32
__declspec(dllexport)
#پایان_اگر
صحيح testbrokenext_init(
  sqlite3 *db, 
  حرف **pz_Err_پیام, 
  ثابت sqlite3_api_routines *pApi
){
  حرف *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_پیام = zErr;
  أرجع 1;
}
