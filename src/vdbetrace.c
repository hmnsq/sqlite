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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "vdbeInt.h"

#ifndef SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
स्थिर i64 findNextHostParameter(अचर वर्ण *zSql, i64 *pnToken){
  पूर्णाङ्क tokenType;
  i64 nTotal = 0;
  i64 n;

  *pnToken = 0;
  जबसम्म( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    प्रतिज्ञा( n>0 && tokenType!=TK_ILLEGAL );
    यदि( tokenType==TK_VARIABLE ){
      *pnToken = n;
      रोक;
    }
    nTotal += n;
    zSql += n;
  }
  फर्काऊ nTotal;
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
वर्ण *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  अचर वर्ण *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  पूर्णाङ्क idx = 0;             /* Index of a host parameter */
  पूर्णाङ्क nextIndex = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  पूर्णाङ्क i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
#ifndef SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#endif

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  यदि( db->nVdbeExec>1 ){
    जबसम्म( *zRawSql ){
      अचर वर्ण *zStart = zRawSql;
      जबसम्म( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&out, "-- ", 3);
      प्रतिज्ञा( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&out, zStart, (पूर्णाङ्क)(zRawSql-zStart));
    }
  }अन्यथा यदि( p->nVar==0 ){
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  }अन्यथा{
    जबसम्म( zRawSql[0] ){
      n = findNextHostParameter(zRawSql, &nToken);
      प्रतिज्ञा( n>0 );
      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;
      प्रतिज्ञा( zRawSql[0] || nToken==0 );
      यदि( nToken==0 ) रोक;
      यदि( zRawSql[0]=='?' ){
        यदि( nToken>1 ){
          प्रतिज्ञा( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }अन्यथा{
          idx = nextIndex;
        }
      }अन्यथा{
        प्रतिज्ञा( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3VdbeParameterIndex(p, zRawSql, nToken);
        प्रतिज्ञा( idx>0 );
      }
      zRawSql += nToken;
      nextIndex = MAX(idx + 1, nextIndex);
      प्रतिज्ञा( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      यदि( pVar->flags & MEM_Null ){
        sqlite3_str_append(&out, "NULL", 4);
      }अन्यथा यदि( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      }अन्यथा यदि( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      }अन्यथा यदि( pVar->flags & MEM_Str ){
        पूर्णाङ्क nOut;  /* Number of bytes of the string text to include in output */
#ifndef SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        यदि( enc!=SQLITE_UTF8 ){
          स_रखो(&utf8, 0, परिमाण(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          यदि( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }
#endif
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        यदि( nOut>SQLITE_TRACE_SIZE_LIMIT ){
          nOut = SQLITE_TRACE_SIZE_LIMIT;
          जबसम्म( nOut<pVar->n && (pVar->z[nOut]&0xc0)==0x80 ){ nOut++; }
        }
#endif    
        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        यदि( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
#ifndef SQLITE_OMIT_UTF16
        यदि( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#endif
      }अन्यथा यदि( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      }अन्यथा{
        पूर्णाङ्क nOut;  /* Number of bytes of the blob to include in output */
        प्रतिज्ञा( pVar->flags & MEM_Blob );
        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;
#ifdef SQLITE_TRACE_SIZE_LIMIT
        यदि( nOut>SQLITE_TRACE_SIZE_LIMIT ) nOut = SQLITE_TRACE_SIZE_LIMIT;
#endif
        लागि(i=0; i<nOut; i++){
          sqlite3_str_appendf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&out, "'", 1);
#ifdef SQLITE_TRACE_SIZE_LIMIT
        यदि( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#endif
      }
    }
  }
  यदि( out.accError ) sqlite3_str_reset(&out);
  फर्काऊ sqlite3StrAccumFinish(&out);
}

#endif /* #ifndef SQLITE_OMIT_TRACE */
