/*
** 2009 November 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code used to insert the values of host parameters
** (aka "wildcards") into the SQL text output by sqlite3_trace().
**
** The Vdbe parse-tree explainer is also found here.
*/
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

#اگر_تعریف_نشده SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
ساكن i64 پیدا_کن_بعدی_Host_Parameter(ثابت حرف *zSql, i64 *pnToken){
  صحيح tokenType;
  i64 n_مجموع = 0;
  i64 n;

  *pnToken = 0;
  طالما( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    أكد( n>0 && tokenType!=TK_ILLEGAL );
    إذا( tokenType==TK_VARIABLE ){
      *pnToken = n;
      اكسر;
    }
    n_مجموع += n;
    zSql += n;
  }
  أرجع n_مجموع;
}

/*
** This function returns a pointer to a nul-terminated string in memory
** obtained from sqlite3DbMalloc(). If sqlite3.nVdbeExec is 1, then the
** string contains a copy of zRawSql but with host parameters expanded to 
** their current bindings. Or, if sqlite3.nVdbeExec is greater than 1, 
** then the returned string holds a copy of zRawSql with "-- " prepended
** to each line of text.
**
** If the SQLITE_TRACE_SIZE_LIMIT macro is defined to an integer, then
** then long strings and blobs are truncated to that many bytes.  This
** can be used to prevent unreasonably large trace strings when dealing
** with large (multi-megabyte) strings and blobs.
**
** The calling function is responsible for making sure the memory returned
** is eventually freed.
**
** ALGORITHM:  Scan the input string looking for host parameters in any of
** these forms:  ?, ?N, $A, @A, :A.  Take care to avoid text within
** string literals, quoted identifier names, and comments.  For text forms,
** the host parameter index is found by scanning the prepared
** statement for the corresponding OP_Variable opcode.  Once the host
** parameter index is known, locate the value in p->aVar[].  Then render
** the value as a literal in place of the host parameter name.
*/
حرف *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  ثابت حرف *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  صحيح idx = 0;             /* Index of a host parameter */
  صحيح بعدی_شاخص = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  صحيح i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum بیرون;            /* Accumulate the output here */
#اگر_تعریف_نشده SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#پایان_اگر

  db = p->db;
  sqlite3StrAccumInit(&بیرون, 0, 0, 0, db->a_حد[SQLITE_حد_درازی]);
  إذا( db->nVdbeExec>1 ){
    طالما( *zRawSql ){
      ثابت حرف *z_شروع = zRawSql;
      طالما( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&بیرون, "-- ", 3);
      أكد( (zRawSql - z_شروع) > 0 );
      sqlite3_str_append(&بیرون, z_شروع, (صحيح)(zRawSql-z_شروع));
    }
  }وإلا إذا( p->nVar==0 ){
    sqlite3_str_append(&بیرون, zRawSql, sqlite3Strlen30(zRawSql));
  }وإلا{
    طالما( zRawSql[0] ){
      n = پیدا_کن_بعدی_Host_Parameter(zRawSql, &nToken);
      أكد( n>0 );
      sqlite3_str_append(&بیرون, zRawSql, n);
      zRawSql += n;
      أكد( zRawSql[0] || nToken==0 );
      إذا( nToken==0 ) اكسر;
      إذا( zRawSql[0]=='?' ){
        إذا( nToken>1 ){
          أكد( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }وإلا{
          idx = بعدی_شاخص;
        }
      }وإلا{
        أكد( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_شاخص(p, zRawSql, nToken);
        أكد( idx>0 );
      }
      zRawSql += nToken;
      بعدی_شاخص = MAX(idx + 1, بعدی_شاخص);
      أكد( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      إذا( pVar->flags & MEM_Null ){
        sqlite3_str_append(&بیرون, "NULL", 4);
      }وإلا إذا( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&بیرون, "%lld", pVar->u.i);
      }وإلا إذا( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&بیرون, "%!.15g", pVar->u.r);
      }وإلا إذا( pVar->flags & MEM_Str ){
        صحيح n_بیرون;  /* Number of bytes of the string text to include in output */
#اگر_تعریف_نشده SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        إذا( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, حجم(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_مجموعه_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          إذا( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            بیرون.accError = SQLITE_NOMEM;
            بیرون.nAlloc = 0;
          }
          pVar = &utf8;
        }
#پایان_اگر
        n_بیرون = pVar->n;
#اگر_تعریف_شده SQLITE_TRACE_اندازه_حد
        إذا( n_بیرون>SQLITE_TRACE_اندازه_حد ){
          n_بیرون = SQLITE_TRACE_اندازه_حد;
          طالما( n_بیرون<pVar->n && (pVar->z[n_بیرون]&0xc0)==0x80 ){ n_بیرون++; }
        }
#پایان_اگر    
        sqlite3_str_appendf(&بیرون, "'%.*q'", n_بیرون, pVar->z);
#اگر_تعریف_شده SQLITE_TRACE_اندازه_حد
        إذا( n_بیرون<pVar->n ){
          sqlite3_str_appendf(&بیرون, "/*+%d bytes*/", pVar->n-n_بیرون);
        }
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_UTF16
        إذا( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#پایان_اگر
      }وإلا إذا( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&بیرون, "zeroblob(%d)", pVar->u.nZero);
      }وإلا{
        صحيح n_بیرون;  /* Number of bytes of the blob to include in output */
        أكد( pVar->flags & MEM_Blob );
        sqlite3_str_append(&بیرون, "x'", 2);
        n_بیرون = pVar->n;
#اگر_تعریف_شده SQLITE_TRACE_اندازه_حد
        إذا( n_بیرون>SQLITE_TRACE_اندازه_حد ) n_بیرون = SQLITE_TRACE_اندازه_حد;
#پایان_اگر
        لكل(i=0; i<n_بیرون; i++){
          sqlite3_str_appendf(&بیرون, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&بیرون, "'", 1);
#اگر_تعریف_شده SQLITE_TRACE_اندازه_حد
        إذا( n_بیرون<pVar->n ){
          sqlite3_str_appendf(&بیرون, "/*+%d bytes*/", pVar->n-n_بیرون);
        }
#پایان_اگر
      }
    }
  }
  إذا( بیرون.accError ) sqlite3_str_reset(&بیرون);
  أرجع sqlite3_Str_Accum_پایان(&بیرون);
}

#پایان_اگر /* #ifndef SQLITE_OMIT_TRACE */
