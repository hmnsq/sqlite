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
  sqlite3_परिणाम_double(context, 0.5*sqlite3_मानम्_double(argv[0]));
}

/*
** SQL functions to call the sqlite3_status function and return results.
*/
स्थिर शून्य स्थितिः_Func(
  sqlite3_context *context,
  पूर्णाङ्क argc,
  sqlite3_मानम् **argv
){
  पूर्णाङ्क op = 0, mx, cur, resetFlag, rc;
  यदि( sqlite3_मानम्_type(argv[0])==SQLITE_INTEGER ){
    op = sqlite3_मानम्_int(argv[0]);
  }अन्यथा यदि( sqlite3_मानम्_type(argv[0])==SQLITE_TEXT ){
    पूर्णाङ्क i;
    ध्रुव वर्ण *zName;
    स्थिर ध्रुव संरचना {
      ध्रुव वर्ण *zName;
      पूर्णाङ्क op;
    } aOp[] = {
      { "MEMORY_USED",         SQLITE_स्थितिः_स्मृतिः_प्रयुक्तम्         },
      { "PAGECACHE_USED",      SQLITE_स्थितिः_PAGECACHE_प्रयुक्तम्      },
      { "PAGECACHE_OVERFLOW",  SQLITE_स्थितिः_PAGECACHE_अतिप्रवाहः  },
      { "SCRATCH_USED",        SQLITE_स्थितिः_SCRATCH_प्रयुक्तम्        },
      { "SCRATCH_OVERFLOW",    SQLITE_स्थितिः_SCRATCH_अतिप्रवाहः    },
      { "MALLOC_SIZE",         SQLITE_स्थितिः_MALLOC_आकार         },
    };
    पूर्णाङ्क nOp = परिमाण(aOp)/परिमाण(aOp[0]);
    zName = (ध्रुव वर्ण*)sqlite3_मानम्_text(argv[0]);
    प्रति(i=0; i<nOp; i++){
      यदि( वाक्यं_तुलय(aOp[i].zName, zName)==0 ){
        op = aOp[i].op;
        भङ्ग;
      }
    }
    यदि( i>=nOp ){
      वर्ण *z_सन्देशः = sqlite3_mprintf("unknown status property: %s", zName);
      sqlite3_परिणाम_error(context, z_सन्देशः, -1);
      sqlite3_free(z_सन्देशः);
      प्रत्यागम;
    }
  }अन्यथा{
    sqlite3_परिणाम_error(context, "unknown status type", -1);
    प्रत्यागम;
  }
  यदि( argc==2 ){
    resetFlag = sqlite3_मानम्_int(argv[1]);
  }अन्यथा{
    resetFlag = 0;
  }
  rc = sqlite3_स्थितिः(op, &cur, &mx, resetFlag);
  यदि( rc!=SQLITE_OK ){
    वर्ण *z_सन्देशः = sqlite3_mprintf("sqlite3_status(%d,...) returns %d", op, rc);
    sqlite3_परिणाम_error(context, z_सन्देशः, -1);
    sqlite3_free(z_सन्देशः);
    प्रत्यागम;
  } 
  यदि( argc==2 ){
    sqlite3_परिणाम_int(context, mx);
  }अन्यथा{
    sqlite3_परिणाम_int(context, cur);
  }
}

/*
** Extension load function.
*/
#यदि_परिभाषितम् _WIN32
__declspec(dllexport)
#यदि_अन्तः
पूर्णाङ्क testloadext_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  पूर्णाङ्क nErr = 0;
  SQLITE_EXTENSION_INIT2(pApi);
  nErr |= sqlite3_create_function(db, "half", 1, SQLITE_ANY, 0, halfFunc, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 1, SQLITE_ANY, 0,
                          स्थितिः_Func, 0, 0);
  nErr |= sqlite3_create_function(db, "sqlite3_status", 2, SQLITE_ANY, 0,
                          स्थितिः_Func, 0, 0);
  प्रत्यागम nErr ? SQLITE_ERROR : SQLITE_OK;
}

/*
** Another extension entry point. This one always fails.
*/
#यदि_परिभाषितम् _WIN32
__declspec(dllexport)
#यदि_अन्तः
पूर्णाङ्क testbrokenext_init(
  sqlite3 *db, 
  वर्ण **pz_Err_सन्देशः, 
  ध्रुव sqlite3_api_routines *pApi
){
  वर्ण *zErr;
  SQLITE_EXTENSION_INIT2(pApi);
  zErr = sqlite3_mprintf("broken!");
  *pz_Err_सन्देशः = zErr;
  प्रत्यागम 1;
}
