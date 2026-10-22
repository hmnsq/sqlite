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

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
ساكن i64 ڳوليو_ايندڙ_Host_Parameter(ثابت حرف *zSql, i64 *pnToken){
  صحيح tokenType;
  i64 n_ڪل = 0;
  i64 n;

  *pnToken = 0;
  طالما( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    أكد( n>0 && tokenType!=TK_ILLEGAL );
    إذا( tokenType==TK_VARIABLE ){
      *pnToken = n;
      اكسر;
    }
    n_ڪل += n;
    zSql += n;
  }
  أرجع n_ڪل;
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
  صحيح ايندڙ_اشاريو = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  صحيح i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum ٻاهريون;            /* Accumulate the output here */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#جيڪڏهن_پورو

  db = p->db;
  sqlite3StrAccumInit(&ٻاهريون, 0, 0, 0, db->a_حد_بندي[SQLITE_حد_بندي_ڊيگهائي]);
  إذا( db->nVdbeExec>1 ){
    طالما( *zRawSql ){
      ثابت حرف *z_شروعات = zRawSql;
      طالما( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&ٻاهريون, "-- ", 3);
      أكد( (zRawSql - z_شروعات) > 0 );
      sqlite3_str_append(&ٻاهريون, z_شروعات, (صحيح)(zRawSql-z_شروعات));
    }
  }وإلا إذا( p->nVar==0 ){
    sqlite3_str_append(&ٻاهريون, zRawSql, sqlite3Strlen30(zRawSql));
  }وإلا{
    طالما( zRawSql[0] ){
      n = ڳوليو_ايندڙ_Host_Parameter(zRawSql, &nToken);
      أكد( n>0 );
      sqlite3_str_append(&ٻاهريون, zRawSql, n);
      zRawSql += n;
      أكد( zRawSql[0] || nToken==0 );
      إذا( nToken==0 ) اكسر;
      إذا( zRawSql[0]=='?' ){
        إذا( nToken>1 ){
          أكد( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }وإلا{
          idx = ايندڙ_اشاريو;
        }
      }وإلا{
        أكد( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_اشاريو(p, zRawSql, nToken);
        أكد( idx>0 );
      }
      zRawSql += nToken;
      ايندڙ_اشاريو = MAX(idx + 1, ايندڙ_اشاريو);
      أكد( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      إذا( pVar->flags & MEM_Null ){
        sqlite3_str_append(&ٻاهريون, "NULL", 4);
      }وإلا إذا( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&ٻاهريون, "%lld", pVar->u.i);
      }وإلا إذا( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&ٻاهريون, "%!.15g", pVar->u.r);
      }وإلا إذا( pVar->flags & MEM_Str ){
        صحيح n_ٻاهريون;  /* Number of bytes of the string text to include in output */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        إذا( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, حجم(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_سيٽ_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          إذا( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            ٻاهريون.accError = SQLITE_NOMEM;
            ٻاهريون.nAlloc = 0;
          }
          pVar = &utf8;
        }
#جيڪڏهن_پورو
        n_ٻاهريون = pVar->n;
#جيڪڏهن_وصف_ٿيل SQLITE_TRACE_سائيز_حد_بندي
        إذا( n_ٻاهريون>SQLITE_TRACE_سائيز_حد_بندي ){
          n_ٻاهريون = SQLITE_TRACE_سائيز_حد_بندي;
          طالما( n_ٻاهريون<pVar->n && (pVar->z[n_ٻاهريون]&0xc0)==0x80 ){ n_ٻاهريون++; }
        }
#جيڪڏهن_پورو    
        sqlite3_str_appendf(&ٻاهريون, "'%.*q'", n_ٻاهريون, pVar->z);
#جيڪڏهن_وصف_ٿيل SQLITE_TRACE_سائيز_حد_بندي
        إذا( n_ٻاهريون<pVar->n ){
          sqlite3_str_appendf(&ٻاهريون, "/*+%d bytes*/", pVar->n-n_ٻاهريون);
        }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_UTF16
        إذا( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#جيڪڏهن_پورو
      }وإلا إذا( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&ٻاهريون, "zeroblob(%d)", pVar->u.nZero);
      }وإلا{
        صحيح n_ٻاهريون;  /* Number of bytes of the blob to include in output */
        أكد( pVar->flags & MEM_Blob );
        sqlite3_str_append(&ٻاهريون, "x'", 2);
        n_ٻاهريون = pVar->n;
#جيڪڏهن_وصف_ٿيل SQLITE_TRACE_سائيز_حد_بندي
        إذا( n_ٻاهريون>SQLITE_TRACE_سائيز_حد_بندي ) n_ٻاهريون = SQLITE_TRACE_سائيز_حد_بندي;
#جيڪڏهن_پورو
        لكل(i=0; i<n_ٻاهريون; i++){
          sqlite3_str_appendf(&ٻاهريون, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&ٻاهريون, "'", 1);
#جيڪڏهن_وصف_ٿيل SQLITE_TRACE_سائيز_حد_بندي
        إذا( n_ٻاهريون<pVar->n ){
          sqlite3_str_appendf(&ٻاهريون, "/*+%d bytes*/", pVar->n-n_ٻاهريون);
        }
#جيڪڏهن_پورو
      }
    }
  }
  إذا( ٻاهريون.accError ) sqlite3_str_reset(&ٻاهريون);
  أرجع sqlite3_Str_Accum_پڄاڻي(&ٻاهريون);
}

#جيڪڏهن_پورو /* #ifndef SQLITE_OMIT_TRACE */
