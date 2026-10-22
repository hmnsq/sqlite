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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRACE

/*
** zSql is a zero-terminated string of UTF-8 SQL text.  Return the number of
** bytes in this text up to but excluding the first character in
** a host parameter.  If the text contains no host parameters, return
** the total number of bytes in the text.
*/
స్థిర i64 కనుగొను_తదుపరి_Host_Parameter(స్థిరరాశి అక్షరం *zSql, i64 *pnToken){
  పూర్ణాంకం tokenType;
  i64 n_మొత్తం = 0;
  i64 n;

  *pnToken = 0;
  అయితే( zSql[0] ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    ప్రతిజ్ఞా( n>0 && tokenType!=TK_ILLEGAL );
    ఒకవేళ( tokenType==TK_VARIABLE ){
      *pnToken = n;
      ఆపు;
    }
    n_మొత్తం += n;
    zSql += n;
  }
  తిరిగి n_మొత్తం;
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
అక్షరం *sqlite3VdbeExpandSql(
  Vdbe *p,                 /* The prepared statement being evaluated */
  స్థిరరాశి అక్షరం *zRawSql      /* Raw text of the SQL statement */
){
  sqlite3 *db;             /* The database connection */
  పూర్ణాంకం idx = 0;             /* Index of a host parameter */
  పూర్ణాంకం తదుపరి_సూచీ = 1;       /* Index of next ? host parameter */
  i64 n;                   /* Length of a token prefix */
  i64 nToken;              /* Length of the parameter token */
  పూర్ణాంకం i;                   /* Loop counter */
  Mem *pVar;               /* Value of a host parameter */
  StrAccum బహిర్గతం;            /* Accumulate the output here */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
  Mem utf8;                /* Used to convert UTF16 into UTF8 for display */
#అయితే_ముగింపు

  db = p->db;
  sqlite3StrAccumInit(&బహిర్గతం, 0, 0, 0, db->a_హద్దు[SQLITE_హద్దు_నిడివి]);
  ఒకవేళ( db->nVdbeExec>1 ){
    అయితే( *zRawSql ){
      స్థిరరాశి అక్షరం *z_ప్రారంభం = zRawSql;
      అయితే( *(zRawSql++)!='\n' && *zRawSql );
      sqlite3_str_append(&బహిర్గతం, "-- ", 3);
      ప్రతిజ్ఞా( (zRawSql - z_ప్రారంభం) > 0 );
      sqlite3_str_append(&బహిర్గతం, z_ప్రారంభం, (పూర్ణాంకం)(zRawSql-z_ప్రారంభం));
    }
  }లేకపోతే ఒకవేళ( p->nVar==0 ){
    sqlite3_str_append(&బహిర్గతం, zRawSql, sqlite3Strlen30(zRawSql));
  }లేకపోతే{
    అయితే( zRawSql[0] ){
      n = కనుగొను_తదుపరి_Host_Parameter(zRawSql, &nToken);
      ప్రతిజ్ఞా( n>0 );
      sqlite3_str_append(&బహిర్గతం, zRawSql, n);
      zRawSql += n;
      ప్రతిజ్ఞా( zRawSql[0] || nToken==0 );
      ఒకవేళ( nToken==0 ) ఆపు;
      ఒకవేళ( zRawSql[0]=='?' ){
        ఒకవేళ( nToken>1 ){
          ప్రతిజ్ఞా( sqlite3Isdigit(zRawSql[1]) );
          sqlite3GetInt32(&zRawSql[1], &idx);
        }లేకపోతే{
          idx = తదుపరి_సూచీ;
        }
      }లేకపోతే{
        ప్రతిజ్ఞా( zRawSql[0]==':' || zRawSql[0]=='$' ||
                zRawSql[0]=='@' || zRawSql[0]=='#' );
        testcase( zRawSql[0]==':' );
        testcase( zRawSql[0]=='$' );
        testcase( zRawSql[0]=='@' );
        testcase( zRawSql[0]=='#' );
        idx = sqlite3_Vdbe_Parameter_సూచీ(p, zRawSql, nToken);
        ప్రతిజ్ఞా( idx>0 );
      }
      zRawSql += nToken;
      తదుపరి_సూచీ = MAX(idx + 1, తదుపరి_సూచీ);
      ప్రతిజ్ఞా( idx>0 && idx<=p->nVar );
      pVar = &p->aVar[idx-1];
      ఒకవేళ( pVar->flags & MEM_Null ){
        sqlite3_str_append(&బహిర్గతం, "NULL", 4);
      }లేకపోతే ఒకవేళ( pVar->flags & (MEM_Int|MEM_IntReal) ){
        sqlite3_str_appendf(&బహిర్గతం, "%lld", pVar->u.i);
      }లేకపోతే ఒకవేళ( pVar->flags & MEM_Real ){
        sqlite3_str_appendf(&బహిర్గతం, "%!.15g", pVar->u.r);
      }లేకపోతే ఒకవేళ( pVar->flags & MEM_Str ){
        పూర్ణాంకం n_బహిర్గతం;  /* Number of bytes of the string text to include in output */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
        u8 enc = ENC(db);
        ఒకవేళ( enc!=SQLITE_UTF8 ){
          memset(&utf8, 0, పరిమాణం(utf8));
          utf8.db = db;
          sqlite3_Vdbe_Mem_సమితి_Str(&utf8, pVar->z, pVar->n, enc, SQLITE_STATIC);
          ఒకవేళ( SQLITE_NOMEM==sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8) ){
            బహిర్గతం.accError = SQLITE_NOMEM;
            బహిర్గతం.nAlloc = 0;
          }
          pVar = &utf8;
        }
#అయితే_ముగింపు
        n_బహిర్గతం = pVar->n;
#నిర్వచితమైతే SQLITE_TRACE_కొలత_హద్దు
        ఒకవేళ( n_బహిర్గతం>SQLITE_TRACE_కొలత_హద్దు ){
          n_బహిర్గతం = SQLITE_TRACE_కొలత_హద్దు;
          అయితే( n_బహిర్గతం<pVar->n && (pVar->z[n_బహిర్గతం]&0xc0)==0x80 ){ n_బహిర్గతం++; }
        }
#అయితే_ముగింపు    
        sqlite3_str_appendf(&బహిర్గతం, "'%.*q'", n_బహిర్గతం, pVar->z);
#నిర్వచితమైతే SQLITE_TRACE_కొలత_హద్దు
        ఒకవేళ( n_బహిర్గతం<pVar->n ){
          sqlite3_str_appendf(&బహిర్గతం, "/*+%d bytes*/", pVar->n-n_బహిర్గతం);
        }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
        ఒకవేళ( enc!=SQLITE_UTF8 ) sqlite3VdbeMemRelease(&utf8);
#అయితే_ముగింపు
      }లేకపోతే ఒకవేళ( pVar->flags & MEM_Zero ){
        sqlite3_str_appendf(&బహిర్గతం, "zeroblob(%d)", pVar->u.nZero);
      }లేకపోతే{
        పూర్ణాంకం n_బహిర్గతం;  /* Number of bytes of the blob to include in output */
        ప్రతిజ్ఞా( pVar->flags & MEM_Blob );
        sqlite3_str_append(&బహిర్గతం, "x'", 2);
        n_బహిర్గతం = pVar->n;
#నిర్వచితమైతే SQLITE_TRACE_కొలత_హద్దు
        ఒకవేళ( n_బహిర్గతం>SQLITE_TRACE_కొలత_హద్దు ) n_బహిర్గతం = SQLITE_TRACE_కొలత_హద్దు;
#అయితే_ముగింపు
        కొరకు(i=0; i<n_బహిర్గతం; i++){
          sqlite3_str_appendf(&బహిర్గతం, "%02x", pVar->z[i]&0xff);
        }
        sqlite3_str_append(&బహిర్గతం, "'", 1);
#నిర్వచితమైతే SQLITE_TRACE_కొలత_హద్దు
        ఒకవేళ( n_బహిర్గతం<pVar->n ){
          sqlite3_str_appendf(&బహిర్గతం, "/*+%d bytes*/", pVar->n-n_బహిర్గతం);
        }
#అయితే_ముగింపు
      }
    }
  }
  ఒకవేళ( బహిర్గతం.accError ) sqlite3_str_reset(&బహిర్గతం);
  తిరిగి sqlite3_Str_Accum_ముగింపు(&బహిర్గతం);
}

#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_TRACE */
