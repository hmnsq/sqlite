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
#समावेश "sqliteInt.h"
#समावेश "vdbeInt.h"

#अगर_अघोषित SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
अटल i64 find_अग्र_Host_Parameter(स्थिर अक्षर *zSql, i64 *pnToken){
  पूर्णांक tokenType;
  i64 n_योग = 0;
  i64 n;

  *pnToken = 0;
  जबतक( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    निश्चित( n>0 && tokenType!=TK_ILLEGAL );
    अगर( tokenType==TK_VARIABLE ){
      *pnToken = n;
      अवरोध;
    }
    n_योग += n;
    zSql += n;
  }
  वापस n_योग;
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
अक्षर *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  स्थिर अक्षर *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  पूर्णांक idx = 0;             /* Index of a host parameter */
  पूर्णांक अग्र_सूचकांक = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  पूर्णांक i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum out;            /* Accumulate the output here */
#अगर_अघोषित SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#पूर्ण_अगर

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_लंबाई]);
  अगर( db->nVdbeExec>1 ){
    जबतक( *zRawSql ){
      स्थिर अक्षर *zStart = zRawSql;
      जबतक( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&out, "-- ", 3);
      निश्चित( (zRawSql - zStart) > 0 );
      sqlite3_str_append(&out, zStart, (पूर्णांक)(zRawSql-zStart));
    }
  }अन्यथा अगर( p->nVar==0 ){
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  }अन्यथा{
    जबतक( zRawSql[0] ){
      n = find_अग्र_Host_Parameter(zRawSql, &nToken);
      निश्चित( n>0 );
      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;
      निश्चित( zRawSql[0] || nToken==0 );
      अगर( nToken==0 ) अवरोध;
      अगर( zRawSql[0]=='?' ){
        अगर( nToken>1 ){
          निश्चित( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }अन्यथा{
          idx = अग्र_सूचकांक;
        }
      }अन्यथा{
        निश्चित( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_सूचकांक(p, zRawSql, nToken);
        निश्चित( idx>0 );
      }
      zRawSql += nToken;
      अग्र_सूचकांक = MAX(idx + 1, अग्र_सूचकांक);
      निश्चित( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      अगर( pVar->flags & MEM_Null ){
        sqlite3_str_append(&out, "NULL", 4);
      }अन्यथा अगर( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      }अन्यथा अगर( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      }अन्यथा अगर( pVar->flags & MEM_Str ){
        पूर्णांक nOut;  /* Number of bytes of the string text to include in output */
#अगर_अघोषित SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        अगर( enc!=SQLITE_UTF8 ){
          स_रखो(&utf8, 0, माप(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          अगर( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }
#पूर्ण_अगर
        nOut = pVar->n;
#अगर_घोषित SQLITE_TRACE_आकार_LIMIT
        अगर( nOut>SQLITE_TRACE_आकार_LIMIT ){
          nOut = SQLITE_TRACE_आकार_LIMIT;
          जबतक( nOut<pVar->n && (pVar->z[nOut]&0xc0)==0x80 ){ nOut++; }
        }
#पूर्ण_अगर    
        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);
#अगर_घोषित SQLITE_TRACE_आकार_LIMIT
        अगर( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_UTF16
        अगर( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#पूर्ण_अगर
      }अन्यथा अगर( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      }अन्यथा{
        पूर्णांक nOut;  /* Number of bytes of the blob to include in output */
        निश्चित( pVar->flags & MEM_Blob );
        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;
#अगर_घोषित SQLITE_TRACE_आकार_LIMIT
        अगर( nOut>SQLITE_TRACE_आकार_LIMIT ) nOut = SQLITE_TRACE_आकार_LIMIT;
#पूर्ण_अगर
        क्रम(i=0; i<nOut; i++){
          sqlite3_str_appendf(&out, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&out, "'", 1);
#अगर_घोषित SQLITE_TRACE_आकार_LIMIT
        अगर( nOut<pVar->n ){
          sqlite3_str_appendf(&out, "/*+%d bytes*/", pVar->n-nOut);
        }
#पूर्ण_अगर
      }
    }
  }
  अगर( out.accError ) sqlite3_str_reset(&out);
  वापस sqlite3StrAccumFinish(&out);
}

#पूर्ण_अगर /* #ifndef SQLITE_OMIT_TRACE */
