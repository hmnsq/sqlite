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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
ਸਥਿਰ i64 ਲੱਭੋ_ਅਗਲਾ_Host_Parameter(ਅਚਲ ਅੱਖਰ *zSql, i64 *pnToken){
  ਪੂਰਨ tokenType;
  i64 n_ਕੁੱਲ = 0;
  i64 n;

  *pnToken = 0;
  ਜਦੋਂ( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    ਪ੍ਰਤਿਜ੍ਞਾ( n>0 && tokenType!=TK_ILLEGAL );
    ਜੇ( tokenType==TK_VARIABLE ){
      *pnToken = n;
      ਰੋਕੋ;
    }
    n_ਕੁੱਲ += n;
    zSql += n;
  }
  ਵਾਪਸ n_ਕੁੱਲ;
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
ਅੱਖਰ *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  ਅਚਲ ਅੱਖਰ *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  ਪੂਰਨ idx = 0;             /* Index of a host parameter */
  ਪੂਰਨ ਅਗਲਾ_ਸੂਚਕਾਂਕ = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  ਪੂਰਨ i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum ਬਾਹਰਲਾ;            /* Accumulate the output here */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#ਜੇ_ਅੰਤ

  db = p->db;
  sqlite3StrAccumInit(&ਬਾਹਰਲਾ, 0, 0, 0, db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਲੰਮਾਈ]);
  ਜੇ( db->nVdbeExec>1 ){
    ਜਦੋਂ( *zRawSql ){
      ਅਚਲ ਅੱਖਰ *z_ਸ਼ੁਰੂ = zRawSql;
      ਜਦੋਂ( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&ਬਾਹਰਲਾ, "-- ", 3);
      ਪ੍ਰਤਿਜ੍ਞਾ( (zRawSql - z_ਸ਼ੁਰੂ) > 0 );
      sqlite3_str_append(&ਬਾਹਰਲਾ, z_ਸ਼ੁਰੂ, (ਪੂਰਨ)(zRawSql-z_ਸ਼ੁਰੂ));
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p->nVar==0 ){
    sqlite3_str_append(&ਬਾਹਰਲਾ, zRawSql, sqlite3Strlen30(zRawSql));
  }ਨਹੀਂਤਾਂ{
    ਜਦੋਂ( zRawSql[0] ){
      n = ਲੱਭੋ_ਅਗਲਾ_Host_Parameter(zRawSql, &nToken);
      ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );
      sqlite3_str_append(&ਬਾਹਰਲਾ, zRawSql, n);
      zRawSql += n;
      ਪ੍ਰਤਿਜ੍ਞਾ( zRawSql[0] || nToken==0 );
      ਜੇ( nToken==0 ) ਰੋਕੋ;
      ਜੇ( zRawSql[0]=='?' ){
        ਜੇ( nToken>1 ){
          ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }ਨਹੀਂਤਾਂ{
          idx = ਅਗਲਾ_ਸੂਚਕਾਂਕ;
        }
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_ਸੂਚਕਾਂਕ(p, zRawSql, nToken);
        ਪ੍ਰਤਿਜ੍ਞਾ( idx>0 );
      }
      zRawSql += nToken;
      ਅਗਲਾ_ਸੂਚਕਾਂਕ = MAX(idx + 1, ਅਗਲਾ_ਸੂਚਕਾਂਕ);
      ਪ੍ਰਤਿਜ੍ਞਾ( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      ਜੇ( pVar->flags & MEM_Null ){
        sqlite3_str_append(&ਬਾਹਰਲਾ, "NULL", 4);
      }ਨਹੀਂਤਾਂ ਜੇ( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&ਬਾਹਰਲਾ, "%lld", pVar->u.i);
      }ਨਹੀਂਤਾਂ ਜੇ( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&ਬਾਹਰਲਾ, "%!.15g", pVar->u.r);
      }ਨਹੀਂਤਾਂ ਜੇ( pVar->flags & MEM_Str ){
        ਪੂਰਨ n_ਬਾਹਰਲਾ;  /* Number of bytes of the string text to include in output */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        ਜੇ( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, ਆਕਾਰ(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          ਜੇ( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            ਬਾਹਰਲਾ.accError = SQLITE_NOMEM;
            ਬਾਹਰਲਾ.nAlloc = 0;
          }
          pVar = &utf8;
        }
#ਜੇ_ਅੰਤ
        n_ਬਾਹਰਲਾ = pVar->n;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ
        ਜੇ( n_ਬਾਹਰਲਾ>SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ ){
          n_ਬਾਹਰਲਾ = SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ;
          ਜਦੋਂ( n_ਬਾਹਰਲਾ<pVar->n && (pVar->z[n_ਬਾਹਰਲਾ]&0xc0)==0x80 ){ n_ਬਾਹਰਲਾ++; }
        }
#ਜੇ_ਅੰਤ    
        sqlite3_str_appendf(&ਬਾਹਰਲਾ, "'%.*q'", n_ਬਾਹਰਲਾ, pVar->z);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ
        ਜੇ( n_ਬਾਹਰਲਾ<pVar->n ){
          sqlite3_str_appendf(&ਬਾਹਰਲਾ, "/*+%d bytes*/", pVar->n-n_ਬਾਹਰਲਾ);
        }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
        ਜੇ( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#ਜੇ_ਅੰਤ
      }ਨਹੀਂਤਾਂ ਜੇ( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&ਬਾਹਰਲਾ, "zeroblob(%d)", pVar->u.nZero);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ n_ਬਾਹਰਲਾ;  /* Number of bytes of the blob to include in output */
        ਪ੍ਰਤਿਜ੍ਞਾ( pVar->flags & MEM_Blob );
        sqlite3_str_append(&ਬਾਹਰਲਾ, "x'", 2);
        n_ਬਾਹਰਲਾ = pVar->n;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ
        ਜੇ( n_ਬਾਹਰਲਾ>SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ ) n_ਬਾਹਰਲਾ = SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ;
#ਜੇ_ਅੰਤ
        ਲਈ(i=0; i<n_ਬਾਹਰਲਾ; i++){
          sqlite3_str_appendf(&ਬਾਹਰਲਾ, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&ਬਾਹਰਲਾ, "'", 1);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TRACE_ਮਾਪ_ਹੱਦਬੰਦੀ
        ਜੇ( n_ਬਾਹਰਲਾ<pVar->n ){
          sqlite3_str_appendf(&ਬਾਹਰਲਾ, "/*+%d bytes*/", pVar->n-n_ਬਾਹਰਲਾ);
        }
#ਜੇ_ਅੰਤ
      }
    }
  }
  ਜੇ( ਬਾਹਰਲਾ.accError ) sqlite3_str_reset(&ਬਾਹਰਲਾ);
  ਵਾਪਸ sqlite3_Str_Accum_ਅੰਤ(&ਬਾਹਰਲਾ);
}

#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_TRACE */
