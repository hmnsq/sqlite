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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব "vdbeInt.h"

#যদি !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ধ্রুবক পূর্ণসংখ্যা sqlite3one = 1;
#endif /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
স্থির ধ্রুবক unsigned অক্ষর sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#define WRITE_UTF8(zOut, c) {                          \
  যদি( c<0x00080 ){                                     \
    *zOut++ = (u8)(c&0xFF);                            \
  }                                                    \
  অন্যথা যদি( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  অন্যথা যদি( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }অন্যথা{                                               \
    *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#define WRITE_UTF16LE(zOut, c) {                                    \
  যদি( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
  }অন্যথা{                                                            \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#define WRITE_UTF16BE(zOut, c) {                                    \
  যদি( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }অন্যথা{                                                            \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
পূর্ণসংখ্যা sqlite3AppendOneUtf8Character(অক্ষর *zOut, u32 v){
  যদি( v<0x00080 ){
    zOut[0] = (u8)(v & 0xff);
    ফেরত 1;
  }
  যদি( v<0x00800 ){
    zOut[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    zOut[1] = 0x80 + (u8)(v & 0x3f);
    ফেরত 2;
  }
  যদি( v<0x10000 ){
    zOut[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    zOut[1] = 0x80 + (u8)((v>>6) & 0x3f);
    zOut[2] = 0x80 + (u8)(v & 0x3f);
    ফেরত 3;
  }
  zOut[0] = 0xf0 + (u8)((v>>18) & 0x07);
  zOut[1] = 0x80 + (u8)((v>>12) & 0x3f);
  zOut[2] = 0x80 + (u8)((v>>6) & 0x3f);
  zOut[3] = 0x80 + (u8)(v & 0x3f);
  ফেরত 4;
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
#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  যদি( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    যখন( zIn<zTerm && (*zIn & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    যদি( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ধ্রুবক unsigned অক্ষর **pz    /* Pointer to string from which to read char */
){
  unsigned পূর্ণসংখ্যা c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  যদি( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    যখন( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    যদি( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  ফেরত c;
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
পূর্ণসংখ্যা sqlite3Utf8ReadLimited(
  ধ্রুবক u8 *z,
  পূর্ণসংখ্যা n,
  u32 *piOut
){
  u32 c;
  পূর্ণসংখ্যা i = 1;
  প্রতিজ্ঞা( n>0 );
  c = z[0];
  যদি( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    যদি( n>4 ) n = 4;
    যখন( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *piOut = c;
  ফেরত i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#ifndef SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE পূর্ণসংখ্যা sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  unsigned অক্ষর *zOut;        /* Output buffer */
  unsigned অক্ষর *zIn;         /* Input iterator */
  unsigned অক্ষর *zTerm;       /* End of input */
  unsigned অক্ষর *z;           /* Output iterator */
  unsigned পূর্ণসংখ্যা c;

  প্রতিজ্ঞা( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  প্রতিজ্ঞা( pMem->flags&MEM_Str );
  প্রতিজ্ঞা( pMem->enc!=desiredEnc );
  প্রতিজ্ঞা( pMem->enc!=0 );
  প্রতিজ্ঞা( pMem->n>=0 );

#যদি defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    অক্ষর zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, আকার(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3StrAccumFinish(&acc));
  }
#endif

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  যদি( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 অস্থায়ী;
    পূর্ণসংখ্যা rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    যদি( rc!=SQLITE_OK ){
      প্রতিজ্ঞা( rc==SQLITE_NOMEM );
      ফেরত SQLITE_NOMEM_BKPT;
    }
    zIn = (u8*)pMem->z;
    zTerm = &zIn[pMem->n&~1];
    যখন( zIn<zTerm ){
      অস্থায়ী = *zIn;
      *zIn = *(zIn+1);
      zIn++;
      *zIn++ = অস্থায়ী;
    }
    pMem->enc = desiredEnc;
    যাও translate_out;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  যদি( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }অন্যথা{
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
  zIn = (u8*)pMem->z;
  zTerm = &zIn[pMem->n];
  zOut = sqlite3DbMallocRaw(pMem->db, len);
  যদি( !zOut ){
    ফেরত SQLITE_NOMEM_BKPT;
  }
  z = zOut;

  যদি( pMem->enc==SQLITE_UTF8 ){
    যদি( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      যখন( zIn<zTerm ){
        READ_UTF8(zIn, zTerm, c);
        WRITE_UTF16LE(z, c);
      }
    }অন্যথা{
      প্রতিজ্ঞা( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      যখন( zIn<zTerm ){
        READ_UTF8(zIn, zTerm, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (পূর্ণসংখ্যা)(z - zOut);
    *z++ = 0;
  }অন্যথা{
    প্রতিজ্ঞা( desiredEnc==SQLITE_UTF8 );
    যদি( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      যখন( zIn<zTerm ){
        c = *(zIn++);
        c += (*(zIn++))<<8;
        যদি( c>=0xd800 && c<0xe000 ){
#ifdef SQLITE_REPLACE_INVALID_UTF
          যদি( c>=0xdc00 || zIn>=zTerm ){
            c = 0xfffd;
          }অন্যথা{
            পূর্ণসংখ্যা c2 = *(zIn++);
            c2 += (*(zIn++))<<8;
            যদি( c2<0xdc00 || c2>=0xe000 ){
              zIn -= 2;
              c = 0xfffd;
            }অন্যথা{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#অন্যথা
          যদি( zIn<zTerm ){
            পূর্ণসংখ্যা c2 = (*zIn++);
            c2 += ((*zIn++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#endif
        }
        WRITE_UTF8(z, c);
      }
    }অন্যথা{
      /* UTF-16 Big-endian -> UTF-8 */
      যখন( zIn<zTerm ){
        c = (*(zIn++))<<8;
        c += *(zIn++);
        যদি( c>=0xd800 && c<0xe000 ){
#ifdef SQLITE_REPLACE_INVALID_UTF
          যদি( c>=0xdc00 || zIn>=zTerm ){
            c = 0xfffd;
          }অন্যথা{
            পূর্ণসংখ্যা c2 = (*(zIn++))<<8;
            c2 += *(zIn++);
            যদি( c2<0xdc00 || c2>=0xe000 ){
              zIn -= 2;
              c = 0xfffd;
            }অন্যথা{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#অন্যথা
          যদি( zIn<zTerm ){
            পূর্ণসংখ্যা c2 = ((*zIn++)<<8);
            c2 += (*zIn++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#endif
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (পূর্ণসংখ্যা)(z - zOut);
  }
  *z = 0;
  প্রতিজ্ঞা( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_Term|(pMem->flags&(MEM_AffMask|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (অক্ষর*)zOut;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_আকার(pMem->db, pMem->z);

translate_out:
#যদি defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    অক্ষর zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, আকার(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3StrAccumFinish(&acc));
  }
#endif
  ফেরত SQLITE_OK;
}
#endif /* SQLITE_OMIT_UTF16 */

#ifndef SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
পূর্ণসংখ্যা sqlite3VdbeMemHandleBom(Mem *pMem){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  u8 bom = 0;

  প্রতিজ্ঞা( pMem->n>=0 );
  যদি( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    যদি( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    যদি( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  যদি( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    যদি( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_Term;
      pMem->enc = bom;
    }
  }
  ফেরত rc;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
পূর্ণসংখ্যা sqlite3Utf8CharLen(ধ্রুবক অক্ষর *zIn, পূর্ণসংখ্যা nByte){
  পূর্ণসংখ্যা r = 0;
  ধ্রুবক u8 *z = (ধ্রুবক u8*)zIn;
  ধ্রুবক u8 *zTerm;
  যদি( nByte>=0 ){
    zTerm = &z[nByte];
  }অন্যথা{
    zTerm = (ধ্রুবক u8*)(-1);
  }
  প্রতিজ্ঞা( z<=zTerm );
  যখন( *z!=0 && z<zTerm ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  ফেরত r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#যদি defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
পূর্ণসংখ্যা sqlite3Utf8To8(unsigned অক্ষর *zIn){
  unsigned অক্ষর *zOut = zIn;
  unsigned অক্ষর *zStart = zIn;
  u32 c;

  যখন( zIn[0] && zOut<=zIn ){
    c = sqlite3Utf8Read((ধ্রুবক u8**)&zIn);
    যদি( c!=0xfffd ){
      WRITE_UTF8(zOut, c);
    }
  }
  *zOut = 0;
  ফেরত (পূর্ণসংখ্যা)(zOut - zStart);
}
#endif

#ifndef SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
অক্ষর *sqlite3Utf16to8(sqlite3 *db, ধ্রুবক শূন্য *z, পূর্ণসংখ্যা nByte, u8 enc){
  Mem m;
  memset(&m, 0, আকার(m));
  m.db = db;
  sqlite3VdbeMemSetStr(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  যদি( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  প্রতিজ্ঞা( (m.flags & MEM_Term)!=0 || db->mallocFailed );
  প্রতিজ্ঞা( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  প্রতিজ্ঞা( m.z || db->mallocFailed );
  ফেরত m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
পূর্ণসংখ্যা sqlite3Utf16ByteLen(ধ্রুবক শূন্য *zIn, পূর্ণসংখ্যা nByte, পূর্ণসংখ্যা nChar){
  পূর্ণসংখ্যা c;
  unsigned অক্ষর ধ্রুবক *z = zIn;
  unsigned অক্ষর ধ্রুবক *zEnd = &z[nByte-1];
  পূর্ণসংখ্যা n = 0;
  
  যদি( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  যখন( n<nChar && z<=zEnd ){
    c = z[0];
    z += 2;
    যদি( c>=0xd8 && c<0xdc && z<=zEnd && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  ফেরত (পূর্ণসংখ্যা)(z-(unsigned অক্ষর ধ্রুবক *)zIn) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#যদি defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
শূন্য sqlite3UtfSelfTest(শূন্য){
  unsigned পূর্ণসংখ্যা i, t;
  unsigned অক্ষর zBuf[20];
  unsigned অক্ষর *z;
  পূর্ণসংখ্যা n;
  unsigned পূর্ণসংখ্যা c;

  জন্য(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (পূর্ণসংখ্যা)(z-zBuf);
    প্রতিজ্ঞা( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ধ্রুবক u8**)&z);
    t = i;
    যদি( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    প্রতিজ্ঞা( c==t );
    প্রতিজ্ঞা( (z-zBuf)==n );
  }
}
#endif /* SQLITE_TEST */
#endif /* SQLITE_OMIT_UTF16 */
