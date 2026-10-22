/*
** 2004 April 13
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains routines used to translate between UTF-8, 
** UTF-16, UTF-16BE, and UTF-16LE.
**
** Notes on UTF-8:
**
**   Byte-0    Byte-1    Byte-2    Byte-3    Value
**  0xxxxxxx                                 00000000 00000000 0xxxxxxx
**  110yyyyy  10xxxxxx                       00000000 00000yyy yyxxxxxx
**  1110zzzz  10yyyyyy  10xxxxxx             00000000 zzzzyyyy yyxxxxxx
**  11110uuu  10uuzzzz  10yyyyyy  10xxxxxx   000uuuuu zzzzyyyy yyxxxxxx
**
**
** Notes on UTF-16:  (with wwww+1==uuuuu)
**
**      Word-0               Word-1          Value
**  110110ww wwzzzzyy   110111yy yyxxxxxx    000uuuuu zzzzyyyy yyxxxxxx
**  zzzzyyyy yyxxxxxx                        00000000 zzzzyyyy yyxxxxxx
**
**
** BOM or Byte Order Mark:
**     0xff 0xfe   little-endian utf-16 follows
**     0xfe 0xff   big-endian utf-16 follows
**
*/
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <assert.h>
#అన్తర్భావ "vdbeInt.h"

#ఒకవేళ !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
స్థిరరాశి పూర్ణాంకం sqlite3one = 1;
#అయితే_ముగింపు /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
స్థిర స్థిరరాశి గుర్తులేని అక్షరం sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#నిర్వచించు WRITE_UTF8(z_బహిర్గతం, c) {                          \
  ఒకవేళ( c<0x00080 ){                                     \
    *z_బహిర్గతం++ = (u8)(c&0xFF);                            \
  }                                                    \
  లేకపోతే ఒకవేళ( c<0x00800 ){                                \
    *z_బహిర్గతం++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *z_బహిర్గతం++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  లేకపోతే ఒకవేళ( c<0x10000 ){                                \
    *z_బహిర్గతం++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *z_బహిర్గతం++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_బహిర్గతం++ = 0x80 + (u8)(c & 0x3F);                   \
  }లేకపోతే{                                               \
    *z_బహిర్గతం++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *z_బహిర్గతం++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *z_బహిర్గతం++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_బహిర్గతం++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#నిర్వచించు WRITE_UTF16LE(z_బహిర్గతం, c) {                                    \
  ఒకవేళ( c<=0xFFFF ){                                                  \
    *z_బహిర్గతం++ = (u8)(c&0x00FF);                                       \
    *z_బహిర్గతం++ = (u8)((c>>8)&0x00FF);                                  \
  }లేకపోతే{                                                            \
    *z_బహిర్గతం++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_బహిర్గతం++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_బహిర్గతం++ = (u8)(c&0x00FF);                                       \
    *z_బహిర్గతం++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#నిర్వచించు WRITE_UTF16BE(z_బహిర్గతం, c) {                                    \
  ఒకవేళ( c<=0xFFFF ){                                                  \
    *z_బహిర్గతం++ = (u8)((c>>8)&0x00FF);                                  \
    *z_బహిర్గతం++ = (u8)(c&0x00FF);                                       \
  }లేకపోతే{                                                            \
    *z_బహిర్గతం++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_బహిర్గతం++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_బహిర్గతం++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *z_బహిర్గతం++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
పూర్ణాంకం sqlite3AppendOneUtf8Character(అక్షరం *z_బహిర్గతం, u32 v){
  ఒకవేళ( v<0x00080 ){
    z_బహిర్గతం[0] = (u8)(v & 0xff);
    తిరిగి 1;
  }
  ఒకవేళ( v<0x00800 ){
    z_బహిర్గతం[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    z_బహిర్గతం[1] = 0x80 + (u8)(v & 0x3f);
    తిరిగి 2;
  }
  ఒకవేళ( v<0x10000 ){
    z_బహిర్గతం[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    z_బహిర్గతం[1] = 0x80 + (u8)((v>>6) & 0x3f);
    z_బహిర్గతం[2] = 0x80 + (u8)(v & 0x3f);
    తిరిగి 3;
  }
  z_బహిర్గతం[0] = 0xf0 + (u8)((v>>18) & 0x07);
  z_బహిర్గతం[1] = 0x80 + (u8)((v>>12) & 0x3f);
  z_బహిర్గతం[2] = 0x80 + (u8)((v>>6) & 0x3f);
  z_బహిర్గతం[3] = 0x80 + (u8)(v & 0x3f);
  తిరిగి 4;
}

/*
** Translate a single UTF-8 character.  Return the unicode value.
**
** During translation, assume that the byte that zTerm points
** is a 0x00.
**
** Write a pointer to the next unread byte back into *pzNext.
**
** Notes On Invalid UTF-8:
**
**  *  This routine never allows a 7-bit character (0x00 through 0x7f) to
**     be encoded as a multi-byte character.  Any multi-byte character that
**     attempts to encode a value between 0x00 and 0x7f is rendered as 0xfffd.
**
**  *  This routine never allows a UTF16 surrogate value to be encoded.
**     If a multi-byte character attempts to encode a value between
**     0xd800 and 0xe000 then it is rendered as 0xfffd.
**
**  *  Bytes in the range of 0x80 through 0xbf which occur as the first
**     byte of a character are interpreted as single-byte characters
**     and rendered as themselves even though they are technically
**     invalid characters.
**
**  *  This routine accepts over-length UTF8 encodings
**     for unicode values 0x80 and greater.  It does not change over-length
**     encodings to 0xfffd as some systems recommend.
*/
#నిర్వచించు READ_UTF8(z_లోపల, z_పదం, c)                           \
  c = *(z_లోపల++);                                            \
  ఒకవేళ( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    అయితే( z_లోపల<z_పదం && (*z_లోపల & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(z_లోపల++));                      \
    }                                                      \
    ఒకవేళ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  స్థిరరాశి గుర్తులేని అక్షరం **pz    /* Pointer to string from which to read char */
){
  గుర్తులేని పూర్ణాంకం c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  ఒకవేళ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    అయితే( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    ఒకవేళ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  తిరిగి c;
}

/*
** Read a single UTF8 character out of buffer z[], but reading no
** more than n characters from the buffer.  z[] is not zero-terminated.
**
** Return the number of bytes used to construct the character.
**
** Invalid UTF8 might generate a strange result.  No effort is made
** to detect invalid UTF8.
**
** At most 4 bytes will be read out of z[].  The return value will always
** be between 1 and 4.
*/
పూర్ణాంకం sqlite3Utf8ReadLimited(
  స్థిరరాశి u8 *z,
  పూర్ణాంకం n,
  u32 *pi_బహిర్గతం
){
  u32 c;
  పూర్ణాంకం i = 1;
  ప్రతిజ్ఞా( n>0 );
  c = z[0];
  ఒకవేళ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ఒకవేళ( n>4 ) n = 4;
    అయితే( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *pi_బహిర్గతం = c;
  తిరిగి i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE పూర్ణాంకం sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  గుర్తులేని అక్షరం *z_బహిర్గతం;        /* Output buffer */
  గుర్తులేని అక్షరం *z_లోపల;         /* Input iterator */
  గుర్తులేని అక్షరం *z_పదం;       /* End of input */
  గుర్తులేని అక్షరం *z;           /* Output iterator */
  గుర్తులేని పూర్ణాంకం c;

  ప్రతిజ్ఞా( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ప్రతిజ్ఞా( pMem->flags&MEM_Str );
  ప్రతిజ్ఞా( pMem->enc!=desiredEnc );
  ప్రతిజ్ఞా( pMem->enc!=0 );
  ప్రతిజ్ఞా( pMem->n>=0 );

#ఒకవేళ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    అక్షరం zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, పరిమాణం(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3_Str_Accum_ముగింపు(&acc));
  }
#అయితే_ముగింపు

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  ఒకవేళ( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 తాత్కాలిక;
    పూర్ణాంకం rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ఒకవేళ( rc!=SQLITE_OK ){
      ప్రతిజ్ఞా( rc==SQLITE_NOMEM );
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    z_లోపల = (u8*)pMem->z;
    z_పదం = &z_లోపల[pMem->n&~1];
    అయితే( z_లోపల<z_పదం ){
      తాత్కాలిక = *z_లోపల;
      *z_లోపల = *(z_లోపల+1);
      z_లోపల++;
      *z_లోపల++ = తాత్కాలిక;
    }
    pMem->enc = desiredEnc;
    వెళ్ళు translate_బహిర్గతం;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  ఒకవేళ( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }లేకపోతే{
    /* When converting from UTF-8 to UTF-16 the maximum growth is caused
    ** when a 1-byte UTF-8 character is translated into a 2-byte UTF-16
    ** character. Two bytes are required in the output buffer for the
    ** nul-terminator.
    */
    len = 2 * (sqlite3_int64)pMem->n + 2;
  }

  /* Set zIn to point at the start of the input buffer and zTerm to point 1
  ** byte past the end.
  **
  ** Variable zOut is set to point at the output buffer, space obtained
  ** from sqlite3_malloc().
  */
  z_లోపల = (u8*)pMem->z;
  z_పదం = &z_లోపల[pMem->n];
  z_బహిర్గతం = sqlite3DbMallocRaw(pMem->db, len);
  ఒకవేళ( !z_బహిర్గతం ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  z = z_బహిర్గతం;

  ఒకవేళ( pMem->enc==SQLITE_UTF8 ){
    ఒకవేళ( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      అయితే( z_లోపల<z_పదం ){
        READ_UTF8(z_లోపల, z_పదం, c);
        WRITE_UTF16LE(z, c);
      }
    }లేకపోతే{
      ప్రతిజ్ఞా( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      అయితే( z_లోపల<z_పదం ){
        READ_UTF8(z_లోపల, z_పదం, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (పూర్ణాంకం)(z - z_బహిర్గతం);
    *z++ = 0;
  }లేకపోతే{
    ప్రతిజ్ఞా( desiredEnc==SQLITE_UTF8 );
    ఒకవేళ( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      అయితే( z_లోపల<z_పదం ){
        c = *(z_లోపల++);
        c += (*(z_లోపల++))<<8;
        ఒకవేళ( c>=0xd800 && c<0xe000 ){
#నిర్వచితమైతే SQLITE_REPLACE_INVALID_UTF
          ఒకవేళ( c>=0xdc00 || z_లోపల>=z_పదం ){
            c = 0xfffd;
          }లేకపోతే{
            పూర్ణాంకం c2 = *(z_లోపల++);
            c2 += (*(z_లోపల++))<<8;
            ఒకవేళ( c2<0xdc00 || c2>=0xe000 ){
              z_లోపల -= 2;
              c = 0xfffd;
            }లేకపోతే{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#లేకపోతే
          ఒకవేళ( z_లోపల<z_పదం ){
            పూర్ణాంకం c2 = (*z_లోపల++);
            c2 += ((*z_లోపల++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#అయితే_ముగింపు
        }
        WRITE_UTF8(z, c);
      }
    }లేకపోతే{
      /* UTF-16 Big-endian -> UTF-8 */
      అయితే( z_లోపల<z_పదం ){
        c = (*(z_లోపల++))<<8;
        c += *(z_లోపల++);
        ఒకవేళ( c>=0xd800 && c<0xe000 ){
#నిర్వచితమైతే SQLITE_REPLACE_INVALID_UTF
          ఒకవేళ( c>=0xdc00 || z_లోపల>=z_పదం ){
            c = 0xfffd;
          }లేకపోతే{
            పూర్ణాంకం c2 = (*(z_లోపల++))<<8;
            c2 += *(z_లోపల++);
            ఒకవేళ( c2<0xdc00 || c2>=0xe000 ){
              z_లోపల -= 2;
              c = 0xfffd;
            }లేకపోతే{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#లేకపోతే
          ఒకవేళ( z_లోపల<z_పదం ){
            పూర్ణాంకం c2 = ((*z_లోపల++)<<8);
            c2 += (*z_లోపల++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#అయితే_ముగింపు
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (పూర్ణాంకం)(z - z_బహిర్గతం);
  }
  *z = 0;
  ప్రతిజ్ఞా( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_పదం|(pMem->flags&(MEM_Aff_ముసుగు|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (అక్షరం*)z_బహిర్గతం;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_కొలత(pMem->db, pMem->z);

translate_బహిర్గతం:
#ఒకవేళ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    అక్షరం zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, పరిమాణం(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3_Str_Accum_ముగింపు(&acc));
  }
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
పూర్ణాంకం sqlite3VdbeMemHandleBom(Mem *pMem){
  పూర్ణాంకం rc = SQLITE_OK;
  u8 bom = 0;

  ప్రతిజ్ఞా( pMem->n>=0 );
  ఒకవేళ( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    ఒకవేళ( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    ఒకవేళ( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  ఒకవేళ( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ఒకవేళ( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_పదం;
      pMem->enc = bom;
    }
  }
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
పూర్ణాంకం sqlite3Utf8CharLen(స్థిరరాశి అక్షరం *z_లోపల, పూర్ణాంకం nByte){
  పూర్ణాంకం r = 0;
  స్థిరరాశి u8 *z = (స్థిరరాశి u8*)z_లోపల;
  స్థిరరాశి u8 *z_పదం;
  ఒకవేళ( nByte>=0 ){
    z_పదం = &z[nByte];
  }లేకపోతే{
    z_పదం = (స్థిరరాశి u8*)(-1);
  }
  ప్రతిజ్ఞా( z<=z_పదం );
  అయితే( *z!=0 && z<z_పదం ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  తిరిగి r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#ఒకవేళ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
పూర్ణాంకం sqlite3Utf8To8(గుర్తులేని అక్షరం *z_లోపల){
  గుర్తులేని అక్షరం *z_బహిర్గతం = z_లోపల;
  గుర్తులేని అక్షరం *z_ప్రారంభం = z_లోపల;
  u32 c;

  అయితే( z_లోపల[0] && z_బహిర్గతం<=z_లోపల ){
    c = sqlite3Utf8Read((స్థిరరాశి u8**)&z_లోపల);
    ఒకవేళ( c!=0xfffd ){
      WRITE_UTF8(z_బహిర్గతం, c);
    }
  }
  *z_బహిర్గతం = 0;
  తిరిగి (పూర్ణాంకం)(z_బహిర్గతం - z_ప్రారంభం);
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
అక్షరం *sqlite3Utf16to8(sqlite3 *db, స్థిరరాశి శూన్య *z, పూర్ణాంకం nByte, u8 enc){
  Mem m;
  memset(&m, 0, పరిమాణం(m));
  m.db = db;
  sqlite3_Vdbe_Mem_సమితి_Str(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  ఒకవేళ( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  ప్రతిజ్ఞా( (m.flags & MEM_పదం)!=0 || db->mallocFailed );
  ప్రతిజ్ఞా( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  ప్రతిజ్ఞా( m.z || db->mallocFailed );
  తిరిగి m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
పూర్ణాంకం sqlite3Utf16ByteLen(స్థిరరాశి శూన్య *z_లోపల, పూర్ణాంకం nByte, పూర్ణాంకం nChar){
  పూర్ణాంకం c;
  గుర్తులేని అక్షరం స్థిరరాశి *z = z_లోపల;
  గుర్తులేని అక్షరం స్థిరరాశి *z_చివర = &z[nByte-1];
  పూర్ణాంకం n = 0;
  
  ఒకవేళ( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  అయితే( n<nChar && z<=z_చివర ){
    c = z[0];
    z += 2;
    ఒకవేళ( c>=0xd8 && c<0xdc && z<=z_చివర && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  తిరిగి (పూర్ణాంకం)(z-(గుర్తులేని అక్షరం స్థిరరాశి *)z_లోపల) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#ఒకవేళ defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
శూన్య sqlite3UtfSelfTest(శూన్య){
  గుర్తులేని పూర్ణాంకం i, t;
  గుర్తులేని అక్షరం zBuf[20];
  గుర్తులేని అక్షరం *z;
  పూర్ణాంకం n;
  గుర్తులేని పూర్ణాంకం c;

  కొరకు(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (పూర్ణాంకం)(z-zBuf);
    ప్రతిజ్ఞా( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((స్థిరరాశి u8**)&z);
    t = i;
    ఒకవేళ( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    ప్రతిజ్ఞా( c==t );
    ప్రతిజ్ఞా( (z-zBuf)==n );
  }
}
#అయితే_ముగింపు /* SQLITE_TEST */
#అయితే_ముగింపు /* SQLITE_OMIT_UTF16 */
