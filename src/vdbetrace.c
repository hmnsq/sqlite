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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "vdbeInt.h"

#ifndef SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
സ്ഥിതം i64 findNextHostParameter(സ്ഥിരം അക്ഷരം *zSql, i64 *pnToken){
  പൂർണ്ണസംഖ്യ tokenType;
  i64 nTotal = 0;
  i64 n;

  *pnToken = 0;
  ആകുമ്പോൾ( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    പ്രതിജ്ഞാ( n>0 && tokenType!=TK_ILLEGAL );
    എങ്കിൽ( tokenType==TK_VARIABLE ){
      *pnToken = n;
      നിർത്തുക;
    }
    nTotal += n;
    zSql += n;
  }
  തിരികെ nTotal;
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
അക്ഷരം *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  സ്ഥിരം അക്ഷരം *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  പൂർണ്ണസംഖ്യ idx = 0;             /* Index of a host parameter */
  പൂർണ്ണസംഖ്യ nextIndex = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  പൂർണ്ണസംഖ്യ i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
#ifndef SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#endif

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  എങ്കിൽ( db->nVdbeExec>1 ){
    ആകുമ്പോൾ( *zRawSql ){
      സ്ഥിരം അക്ഷരം *zStart = zRawSql;
      ആകുമ്പോൾ( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&out, "-- ", 3);
      പ്രതിജ്ഞാ( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&out, zStart, (പൂർണ്ണസംഖ്യ)(zRawSql-zStart));
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( p->nVar==0 ){
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  }അല്ലെങ്കിൽ{
    ആകുമ്പോൾ( zRawSql[0] ){
      n = findNextHostParameter(zRawSql, &nToken);
      പ്രതിജ്ഞാ( n>0 );
      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;
      പ്രതിജ്ഞാ( zRawSql[0] || nToken==0 );
      എങ്കിൽ( nToken==0 ) നിർത്തുക;
      എങ്കിൽ( zRawSql[0]=='?' ){
        എങ്കിൽ( nToken>1 ){
          പ്രതിജ്ഞാ( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }അല്ലെങ്കിൽ{
          idx = nextIndex;
        }
      }അല്ലെങ്കിൽ{
        പ്രതിജ്ഞാ( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3VdbeParameterIndex(p, zRawSql, nToken);
        പ്രതിജ്ഞാ( idx>0 );
      }
      zRawSql += nToken;
      nextIndex = MAX(idx + 1, nextIndex);
      പ്രതിജ്ഞാ( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      എങ്കിൽ( pVar->flags & MEM_Null ){
        sqlite3_str_append(&out, "NULL", 4);
      }അല്ലെങ്കിൽ എങ്കിൽ( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      }അല്ലെങ്കിൽ എങ്കിൽ( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      }അല്ലെങ്കിൽ എങ്കിൽ( pVar->flags & MEM_Str ){
        പൂർണ്ണസംഖ്യ nOut;  /* Number of bytes of the string text to include in output */
#ifndef SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        എങ്കിൽ( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, വലുപ്പം(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          എങ്കിൽ( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }
#endif
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        എങ്കിൽ( nOut>SQLITE_TRACE_SIZE_LIMIT ){
          nOut = SQLITE_TRACE_SIZE_LIMIT;
          ആകുമ്പോൾ( nOut<pVar->n && (pVar->z[nOut]&0xc0)==0x80 ){ nOut++; }
        }
#endif    
        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        എങ്കിൽ( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
#ifndef SQLITE_OMIT_UTF16
        എങ്കിൽ( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#endif
      }അല്ലെങ്കിൽ എങ്കിൽ( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ nOut;  /* Number of bytes of the blob to include in output */
        പ്രതിജ്ഞാ( pVar->flags & MEM_Blob );
        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        എങ്കിൽ( nOut>SQLITE_TRACE_SIZE_LIMIT ) nOut = SQLITE_TRACE_SIZE_LIMIT;
#endif
        വേണ്ടി(i=0; i<nOut; i++){
          sqlite3_str_appendf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&out, "'", 1);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        എങ്കിൽ( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
      }
    }
  }
  എങ്കിൽ( out.accError ) sqlite3_str_reset(&out);
  തിരികെ sqlite3StrAccumFinish(&out);
}

#endif /* #ifndef SQLITE_OMIT_TRACE */
