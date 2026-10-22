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

#यदि_अपरिभाषितम् SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
स्थिर i64 find_अग्र_Host_Parameter(ध्रुव वर्ण *zSql, i64 *pnToken){
  पूर्णाङ्क tokenType;
  i64 n_योग = 0;
  i64 n;

  *pnToken = 0;
  यावत्( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    प्रतिज्ञा( n>0 && tokenType!=TK_ILLEGAL );
    यदि( tokenType==TK_VARIABLE ){
      *pnToken = n;
      भङ्ग;
    }
    n_योग += n;
    zSql += n;
  }
  प्रत्यागम n_योग;
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
  ध्रुव वर्ण *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  पूर्णाङ्क idx = 0;             /* Index of a host parameter */
  पूर्णाङ्क अग्र_सूचकांक = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  पूर्णाङ्क i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum बहिर्गमनम्;            /* Accumulate the output here */
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#यदि_अन्तः

  db = p->db;
  sqlite3StrAccumInit(&बहिर्गमनम्, 0, 0, 0, db->a_सीमा[SQLITE_सीमा_दीर्घता]);
  यदि( db->nVdbeExec>1 ){
    यावत्( *zRawSql ){
      ध्रुव वर्ण *zStart = zRawSql;
      यावत्( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&बहिर्गमनम्, "-- ", 3);
      प्रतिज्ञा( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&बहिर्गमनम्, zStart, (पूर्णाङ्क)(zRawSql-zStart));
    }
  }अन्यथा यदि( p->nVar==0 ){
    sqlite3_str_append(&बहिर्गमनम्, zRawSql, sqlite3Strlen30(zRawSql));
  }अन्यथा{
    यावत्( zRawSql[0] ){
      n = find_अग्र_Host_Parameter(zRawSql, &nToken);
      प्रतिज्ञा( n>0 );
      sqlite3_str_append(&बहिर्गमनम्, zRawSql, n);
      zRawSql += n;
      प्रतिज्ञा( zRawSql[0] || nToken==0 );
      यदि( nToken==0 ) भङ्ग;
      यदि( zRawSql[0]=='?' ){
        यदि( nToken>1 ){
          प्रतिज्ञा( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }अन्यथा{
          idx = अग्र_सूचकांक;
        }
      }अन्यथा{
        प्रतिज्ञा( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_सूचकांक(p, zRawSql, nToken);
        प्रतिज्ञा( idx>0 );
      }
      zRawSql += nToken;
      अग्र_सूचकांक = MAX(idx + 1, अग्र_सूचकांक);
      प्रतिज्ञा( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      यदि( pVar->flags & MEM_Null ){
        sqlite3_str_append(&बहिर्गमनम्, "NULL", 4);
      }अन्यथा यदि( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&बहिर्गमनम्, "%lld", pVar->u.i);
      }अन्यथा यदि( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&बहिर्गमनम्, "%!.15g", pVar->u.r);
      }अन्यथा यदि( pVar->flags & MEM_Str ){
        पूर्णाङ्क n_बहिर्गमनम्;  /* Number of bytes of the string text to include in output */
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        यदि( enc!=SQLITE_UTF8 ){
          स्मृतिं_पूरय(&utf8, 0, परिमाण(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_समुच्चयः_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          यदि( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            बहिर्गमनम्.accError = SQLITE_NOMEM;
            बहिर्गमनम्.nAlloc = 0;
          }
          pVar = &utf8;
        }
#यदि_अन्तः
        n_बहिर्गमनम् = pVar->n;
#यदि_परिभाषितम् SQLITE_TRACE_आकार_सीमा
        यदि( n_बहिर्गमनम्>SQLITE_TRACE_आकार_सीमा ){
          n_बहिर्गमनम् = SQLITE_TRACE_आकार_सीमा;
          यावत्( n_बहिर्गमनम्<pVar->n && (pVar->z[n_बहिर्गमनम्]&0xc0)==0x80 ){ n_बहिर्गमनम्++; }
        }
#यदि_अन्तः    
        sqlite3_str_appendf(&बहिर्गमनम्, "'%.*q'", n_बहिर्गमनम्, pVar->z);
#यदि_परिभाषितम् SQLITE_TRACE_आकार_सीमा
        यदि( n_बहिर्गमनम्<pVar->n ){
          sqlite3_str_appendf(&बहिर्गमनम्, "/*+%d bytes*/", pVar->n-n_बहिर्गमनम्);
        }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
        यदि( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#यदि_अन्तः
      }अन्यथा यदि( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&बहिर्गमनम्, "zeroblob(%d)", pVar->u.nZero);
      }अन्यथा{
        पूर्णाङ्क n_बहिर्गमनम्;  /* Number of bytes of the blob to include in output */
        प्रतिज्ञा( pVar->flags & MEM_Blob );
        sqlite3_str_append(&बहिर्गमनम्, "x'", 2);
        n_बहिर्गमनम् = pVar->n;
#यदि_परिभाषितम् SQLITE_TRACE_आकार_सीमा
        यदि( n_बहिर्गमनम्>SQLITE_TRACE_आकार_सीमा ) n_बहिर्गमनम् = SQLITE_TRACE_आकार_सीमा;
#यदि_अन्तः
        प्रति(i=0; i<n_बहिर्गमनम्; i++){
          sqlite3_str_appendf(&बहिर्गमनम्, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&बहिर्गमनम्, "'", 1);
#यदि_परिभाषितम् SQLITE_TRACE_आकार_सीमा
        यदि( n_बहिर्गमनम्<pVar->n ){
          sqlite3_str_appendf(&बहिर्गमनम्, "/*+%d bytes*/", pVar->n-n_बहिर्गमनम्);
        }
#यदि_अन्तः
      }
    }
  }
  यदि( बहिर्गमनम्.accError ) sqlite3_str_reset(&बहिर्गमनम्);
  प्रत्यागम sqlite3StrAccumFinish(&बहिर्गमनम्);
}

#यदि_अन्तः /* #ifndef SQLITE_OMIT_TRACE */
