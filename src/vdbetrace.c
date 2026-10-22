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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
ಸ್ಥಿರ i64 ಕಂಡುಹಿಡಿ_ಮುಂದಿನ_Host_Parameter(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql, i64 *pnToken){
  ಪೂರ್ಣಾಂಕ tokenType;
  i64 n_ಒಟ್ಟು = 0;
  i64 n;

  *pnToken = 0;
  ಆಗ( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    ಪ್ರತಿಜ್ಞಾ( n>0 && tokenType!=TK_ILLEGAL );
    ಆದರೆ( tokenType==TK_VARIABLE ){
      *pnToken = n;
      ನಿಲ್ಲಿಸು;
    }
    n_ಒಟ್ಟು += n;
    zSql += n;
  }
  ಹಿಂತಿರುಗಿ n_ಒಟ್ಟು;
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
ಅಕ್ಷರ *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  ಪೂರ್ಣಾಂಕ idx = 0;             /* Index of a host parameter */
  ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_ಸೂಚ್ಯಂಕ = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  ಪೂರ್ಣಾಂಕ i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum ಹೊರಹೋಗು;            /* Accumulate the output here */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#ಆದರೆ_ಅಂತ್ಯ

  db = p->db;
  sqlite3StrAccumInit(&ಹೊರಹೋಗು, 0, 0, 0, db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಉದ್ದಳತೆ]);
  ಆದರೆ( db->nVdbeExec>1 ){
    ಆಗ( *zRawSql ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಆರಂಭ = zRawSql;
      ಆಗ( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&ಹೊರಹೋಗು, "-- ", 3);
      ಪ್ರತಿಜ್ಞಾ( (zRawSql - z_ಆರಂಭ) > 0 );
      sqlite3_str_append(&ಹೊರಹೋಗು, z_ಆರಂಭ, (ಪೂರ್ಣಾಂಕ)(zRawSql-z_ಆರಂಭ));
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->nVar==0 ){
    sqlite3_str_append(&ಹೊರಹೋಗು, zRawSql, sqlite3Strlen30(zRawSql));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಗ( zRawSql[0] ){
      n = ಕಂಡುಹಿಡಿ_ಮುಂದಿನ_Host_Parameter(zRawSql, &nToken);
      ಪ್ರತಿಜ್ಞಾ( n>0 );
      sqlite3_str_append(&ಹೊರಹೋಗು, zRawSql, n);
      zRawSql += n;
      ಪ್ರತಿಜ್ಞಾ( zRawSql[0] || nToken==0 );
      ಆದರೆ( nToken==0 ) ನಿಲ್ಲಿಸು;
      ಆದರೆ( zRawSql[0]=='?' ){
        ಆದರೆ( nToken>1 ){
          ಪ್ರತಿಜ್ಞಾ( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }ಇಲ್ಲದಿದ್ದರೆ{
          idx = ಮುಂದಿನ_ಸೂಚ್ಯಂಕ;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_ಸೂಚ್ಯಂಕ(p, zRawSql, nToken);
        ಪ್ರತಿಜ್ಞಾ( idx>0 );
      }
      zRawSql += nToken;
      ಮುಂದಿನ_ಸೂಚ್ಯಂಕ = MAX(idx + 1, ಮುಂದಿನ_ಸೂಚ್ಯಂಕ);
      ಪ್ರತಿಜ್ಞಾ( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      ಆದರೆ( pVar->flags & MEM_Null ){
        sqlite3_str_append(&ಹೊರಹೋಗು, "NULL", 4);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&ಹೊರಹೋಗು, "%lld", pVar->u.i);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&ಹೊರಹೋಗು, "%!.15g", pVar->u.r);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVar->flags & MEM_Str ){
        ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು;  /* Number of bytes of the string text to include in output */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        ಆದರೆ( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, ಗಾತ್ರ(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_ಗಣ_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          ಆದರೆ( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            ಹೊರಹೋಗು.accError = SQLITE_NOMEM;
            ಹೊರಹೋಗು.nAlloc = 0;
          }
          pVar = &utf8;
        }
#ಆದರೆ_ಅಂತ್ಯ
        n_ಹೊರಹೋಗು = pVar->n;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ
        ಆದರೆ( n_ಹೊರಹೋಗು>SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ ){
          n_ಹೊರಹೋಗು = SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ;
          ಆಗ( n_ಹೊರಹೋಗು<pVar->n && (pVar->z[n_ಹೊರಹೋಗು]&0xc0)==0x80 ){ n_ಹೊರಹೋಗು++; }
        }
#ಆದರೆ_ಅಂತ್ಯ    
        sqlite3_str_appendf(&ಹೊರಹೋಗು, "'%.*q'", n_ಹೊರಹೋಗು, pVar->z);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ
        ಆದರೆ( n_ಹೊರಹೋಗು<pVar->n ){
          sqlite3_str_appendf(&ಹೊರಹೋಗು, "/*+%d bytes*/", pVar->n-n_ಹೊರಹೋಗು);
        }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
        ಆದರೆ( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#ಆದರೆ_ಅಂತ್ಯ
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&ಹೊರಹೋಗು, "zeroblob(%d)", pVar->u.nZero);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು;  /* Number of bytes of the blob to include in output */
        ಪ್ರತಿಜ್ಞಾ( pVar->flags & MEM_Blob );
        sqlite3_str_append(&ಹೊರಹೋಗು, "x'", 2);
        n_ಹೊರಹೋಗು = pVar->n;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ
        ಆದರೆ( n_ಹೊರಹೋಗು>SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ ) n_ಹೊರಹೋಗು = SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ;
#ಆದರೆ_ಅಂತ್ಯ
        ಗಾಗಿ(i=0; i<n_ಹೊರಹೋಗು; i++){
          sqlite3_str_appendf(&ಹೊರಹೋಗು, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&ಹೊರಹೋಗು, "'", 1);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TRACE_ಗಾತ್ರ_ಎಲ್ಲೆ
        ಆದರೆ( n_ಹೊರಹೋಗು<pVar->n ){
          sqlite3_str_appendf(&ಹೊರಹೋಗು, "/*+%d bytes*/", pVar->n-n_ಹೊರಹೋಗು);
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
    }
  }
  ಆದರೆ( ಹೊರಹೋಗು.accError ) sqlite3_str_reset(&ಹೊರಹೋಗು);
  ಹಿಂತಿರುಗಿ sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&ಹೊರಹೋಗು);
}

#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_TRACE */
