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
#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
ساکن i64 ڈھونڈو_اگلا_Host_Parameter(ثابت حرف *zSql, i64 *pnToken){
  صحیح tokenType;
  i64 n_کل = 0;
  i64 n;

  *pnToken = 0;
  جب( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    أكد( n>0 && tokenType!=TK_ILLEGAL );
    اگر( tokenType==TK_VARIABLE ){
      *pnToken = n;
      توڑو;
    }
    n_کل += n;
    zSql += n;
  }
  واپس n_کل;
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
  صحیح idx = 0;             /* Index of a host parameter */
  صحیح اگلا_اشاریہ = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  صحیح i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum برآمد;            /* Accumulate the output here */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#اگر_ختم

  db = p->db;
  sqlite3StrAccumInit(&برآمد, 0, 0, 0, db->a_حد_بندی[SQLITE_حد_بندی_لمبائی]);
  اگر( db->nVdbeExec>1 ){
    جب( *zRawSql ){
      ثابت حرف *z_آغاز = zRawSql;
      جب( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&برآمد, "-- ", 3);
      أكد( (zRawSql - z_آغاز) > 0 );
      sqlite3_str_append(&برآمد, z_آغاز, (صحیح)(zRawSql-z_آغاز));
    }
  }ورنہ اگر( p->nVar==0 ){
    sqlite3_str_append(&برآمد, zRawSql, sqlite3Strlen30(zRawSql));
  }ورنہ{
    جب( zRawSql[0] ){
      n = ڈھونڈو_اگلا_Host_Parameter(zRawSql, &nToken);
      أكد( n>0 );
      sqlite3_str_append(&برآمد, zRawSql, n);
      zRawSql += n;
      أكد( zRawSql[0] || nToken==0 );
      اگر( nToken==0 ) توڑو;
      اگر( zRawSql[0]=='?' ){
        اگر( nToken>1 ){
          أكد( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }ورنہ{
          idx = اگلا_اشاریہ;
        }
      }ورنہ{
        أكد( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_اشاریہ(p, zRawSql, nToken);
        أكد( idx>0 );
      }
      zRawSql += nToken;
      اگلا_اشاریہ = MAX(idx + 1, اگلا_اشاریہ);
      أكد( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      اگر( pVar->flags & MEM_Null ){
        sqlite3_str_append(&برآمد, "NULL", 4);
      }ورنہ اگر( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&برآمد, "%lld", pVar->u.i);
      }ورنہ اگر( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&برآمد, "%!.15g", pVar->u.r);
      }ورنہ اگر( pVar->flags & MEM_Str ){
        صحیح n_برآمد;  /* Number of bytes of the string text to include in output */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        اگر( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, حجم(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_سیٹ_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          اگر( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            برآمد.accError = SQLITE_NOMEM;
            برآمد.nAlloc = 0;
          }
          pVar = &utf8;
        }
#اگر_ختم
        n_برآمد = pVar->n;
#اگر_تعریف_شدہ SQLITE_TRACE_سائز_حد_بندی
        اگر( n_برآمد>SQLITE_TRACE_سائز_حد_بندی ){
          n_برآمد = SQLITE_TRACE_سائز_حد_بندی;
          جب( n_برآمد<pVar->n && (pVar->z[n_برآمد]&0xc0)==0x80 ){ n_برآمد++; }
        }
#اگر_ختم    
        sqlite3_str_appendf(&برآمد, "'%.*q'", n_برآمد, pVar->z);
#اگر_تعریف_شدہ SQLITE_TRACE_سائز_حد_بندی
        اگر( n_برآمد<pVar->n ){
          sqlite3_str_appendf(&برآمد, "/*+%d bytes*/", pVar->n-n_برآمد);
        }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
        اگر( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#اگر_ختم
      }ورنہ اگر( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&برآمد, "zeroblob(%d)", pVar->u.nZero);
      }ورنہ{
        صحیح n_برآمد;  /* Number of bytes of the blob to include in output */
        أكد( pVar->flags & MEM_Blob );
        sqlite3_str_append(&برآمد, "x'", 2);
        n_برآمد = pVar->n;
#اگر_تعریف_شدہ SQLITE_TRACE_سائز_حد_بندی
        اگر( n_برآمد>SQLITE_TRACE_سائز_حد_بندی ) n_برآمد = SQLITE_TRACE_سائز_حد_بندی;
#اگر_ختم
        برائے(i=0; i<n_برآمد; i++){
          sqlite3_str_appendf(&برآمد, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&برآمد, "'", 1);
#اگر_تعریف_شدہ SQLITE_TRACE_سائز_حد_بندی
        اگر( n_برآمد<pVar->n ){
          sqlite3_str_appendf(&برآمد, "/*+%d bytes*/", pVar->n-n_برآمد);
        }
#اگر_ختم
      }
    }
  }
  اگر( برآمد.accError ) sqlite3_str_reset(&برآمد);
  واپس sqlite3_Str_Accum_اختتام(&برآمد);
}

#اگر_ختم /* #ifndef SQLITE_OMIT_TRACE */
