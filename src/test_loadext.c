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
#అన్తర్భావ <string.h>
#అన్తర్భావ "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

/*
** The half() SQL function returns half of its input value.
*/
స్థిర శూన్య halfFunc(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  sqlite3_ఫలితం_double(context, 0.5*sqlite3_విలువ_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
స్థిర శూన్య స్థితి_Func(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం op = 0, mx, cur, resetFlag, rc;
  ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_విలువ_int(argv[0]);
  }లేకపోతే ఒకవేళ( sqlite3_విలువ_type(argv[0])==SQLITE_పాఠ్యం ){
    పూర్ణాంకం i;
    స్థిరరాశి అక్షరం *zName;
    స్థిర స్థిరరాశి నిర్మాణం {
      స్థిరరాశి అక్షరం *zName;
      పూర్ణాంకం op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_స్థితి_జ్ఞాపకం_వాడిన         },
      { "PAGECACHE_USED",      SQLITE_స్థితి_PAGECACHE_వాడిన      },
      { "PAGECACHE_OVERFLOW",  SQLITE_స్థితి_PAGECACHE_అధికప్రవాహం  },
      { "SCRATCH_USED",        SQLITE_స్థితి_SCRATCH_వాడిన        },
      { "SCRATCH_OVERFLOW",    SQLITE_స్థితి_SCRATCH_అధికప్రవాహం    },
      { "MALLOC_SIZE",         SQLITE_స్థితి_MALLOC_కొలత         },
    };
    పూర్ణాంకం nOp = పరిమాణం(aOp)/పరిమాణం(aOp[0]);
    zName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    కొరకు(i=0; i<nOp; i++){
      ఒకవేళ( strcmp(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        ఆపు;
      }
    }
    ఒకవేళ( i>=nOp ){
      అక్షరం *z_సందేశం = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_ఫలితం_error(context, z_సందేశం, -1);
      sqlite3_free(z_సందేశం);
      తిరిగి;
    }
  }లేకపోతే{
    sqlite3_ఫలితం_error(context, "unknown status type", -1);
    తిరిగి;
  }
  ఒకవేళ( argc==2 ){
    resetFlag = sqlite3_విలువ_int(argv[1]);
  }లేకపోతే{
    resetFlag = 0;
  }
  rc = sqlite3_స్థితి(op, &cur, &mx, resetFlag);
  ఒకవేళ( rc!=SQLITE_OK ){
    అక్షరం *z_సందేశం = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_ఫలితం_error(context, z_సందేశం, -1);
    sqlite3_free(z_సందేశం);
    తిరిగి;
  } 
  ఒకవేళ( argc==2 ){
    sqlite3_ఫలితం_int(context, mx);
  }లేకపోతే{
    sqlite3_ఫలితం_int(context, cur);
  }
}

/*
** Extension load function.
*/
#నిర్వచితమైతే _WIN32
__declspec(dllexport)
#అయితే_ముగింపు
పూర్ణాంకం testloadext_init(
  sqlite3 *db, 
  అక్షరం **pz_Err_సందేశం, 
  స్థిరరాశి sqlite3_api_routines *pApi
){
  పూర్ణాంకం nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          స్థితి_Func, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          స్థితి_Func, 0, 0);
  తిరిగి nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#నిర్వచితమైతే _WIN32
__declspec(dllexport)
#అయితే_ముగింపు
పూర్ణాంకం testbrokenext_init(
  sqlite3 *db, 
  అక్షరం **pz_Err_సందేశం, 
  స్థిరరాశి sqlite3_api_routines *pApi
){
  అక్షరం *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_సందేశం = zErr;
  తిరిగి 1;
}
