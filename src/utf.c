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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

#ಯದಿ !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ sqlite3one = 1;
#endif /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ sqlite3Utf8Trans1[] = {
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
  ಯದಿ( c<0x00080 ){                                     \
    *zOut++ = (u8)(c&0xFF);                            \
  }                                                    \
  ಅನ್ಯಥಾ ಯದಿ( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  ಅನ್ಯಥಾ ಯದಿ( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }ಅನ್ಯಥಾ{                                               \
    *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#define WRITE_UTF16LE(zOut, c) {                                    \
  ಯದಿ( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
  }ಅನ್ಯಥಾ{                                                            \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#define WRITE_UTF16BE(zOut, c) {                                    \
  ಯದಿ( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }ಅನ್ಯಥಾ{                                                            \
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
ಪೂರ್ಣಾಙ್ಕ sqlite3AppendOneUtf8Character(ವರ್ಣ *zOut, u32 v){
  ಯದಿ( v<0x00080 ){
    zOut[0] = (u8)(v & 0xff);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( v<0x00800 ){
    zOut[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    zOut[1] = 0x80 + (u8)(v & 0x3f);
    ಪ್ರತ್ಯಾಗಮ 2;
  }
  ಯದಿ( v<0x10000 ){
    zOut[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    zOut[1] = 0x80 + (u8)((v>>6) & 0x3f);
    zOut[2] = 0x80 + (u8)(v & 0x3f);
    ಪ್ರತ್ಯಾಗಮ 3;
  }
  zOut[0] = 0xf0 + (u8)((v>>18) & 0x07);
  zOut[1] = 0x80 + (u8)((v>>12) & 0x3f);
  zOut[2] = 0x80 + (u8)((v>>6) & 0x3f);
  zOut[3] = 0x80 + (u8)(v & 0x3f);
  ಪ್ರತ್ಯಾಗಮ 4;
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
  ಯದಿ( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    ಯಾವತ್( zIn<zTerm && (*zIn & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    ಯದಿ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ಧ್ರುವ unsigned ವರ್ಣ **pz    /* Pointer to string from which to read char */
){
  unsigned ಪೂರ್ಣಾಙ್ಕ c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  ಯದಿ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ಯಾವತ್( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    ಯದಿ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  ಪ್ರತ್ಯಾಗಮ c;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3Utf8ReadLimited(
  ಧ್ರುವ u8 *z,
  ಪೂರ್ಣಾಙ್ಕ n,
  u32 *piOut
){
  u32 c;
  ಪೂರ್ಣಾಙ್ಕ i = 1;
  ಪ್ರತಿಜ್ಞಾ( n>0 );
  c = z[0];
  ಯದಿ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ಯದಿ( n>4 ) n = 4;
    ಯಾವತ್( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *piOut = c;
  ಪ್ರತ್ಯಾಗಮ i;
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
SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  unsigned ವರ್ಣ *zOut;        /* Output buffer */
  unsigned ವರ್ಣ *zIn;         /* Input iterator */
  unsigned ವರ್ಣ *zTerm;       /* End of input */
  unsigned ವರ್ಣ *z;           /* Output iterator */
  unsigned ಪೂರ್ಣಾಙ್ಕ c;

  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pMem->flags&MEM_Str );
  ಪ್ರತಿಜ್ಞಾ( pMem->enc!=desiredEnc );
  ಪ್ರತಿಜ್ಞಾ( pMem->enc!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->n>=0 );

#ಯದಿ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    ವರ್ಣ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ಪರಿಮಾಣ(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3StrAccumFinish(&acc));
  }
#endif

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  ಯದಿ( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 temp;
    ಪೂರ್ಣಾಙ್ಕ rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ಯದಿ( rc!=SQLITE_OK ){
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_NOMEM );
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }
    zIn = (u8*)pMem->z;
    zTerm = &zIn[pMem->n&~1];
    ಯಾವತ್( zIn<zTerm ){
      temp = *zIn;
      *zIn = *(zIn+1);
      zIn++;
      *zIn++ = temp;
    }
    pMem->enc = desiredEnc;
    ಗಚ್ಛ translate_out;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  ಯದಿ( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }ಅನ್ಯಥಾ{
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
  ಯದಿ( !zOut ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
  z = zOut;

  ಯದಿ( pMem->enc==SQLITE_UTF8 ){
    ಯದಿ( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      ಯಾವತ್( zIn<zTerm ){
        READ_UTF8(zIn, zTerm, c);
        WRITE_UTF16LE(z, c);
      }
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      ಯಾವತ್( zIn<zTerm ){
        READ_UTF8(zIn, zTerm, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (ಪೂರ್ಣಾಙ್ಕ)(z - zOut);
    *z++ = 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( desiredEnc==SQLITE_UTF8 );
    ಯದಿ( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      ಯಾವತ್( zIn<zTerm ){
        c = *(zIn++);
        c += (*(zIn++))<<8;
        ಯದಿ( c>=0xd800 && c<0xe000 ){
#ifdef SQLITE_REPLACE_INVALID_UTF
          ಯದಿ( c>=0xdc00 || zIn>=zTerm ){
            c = 0xfffd;
          }ಅನ್ಯಥಾ{
            ಪೂರ್ಣಾಙ್ಕ c2 = *(zIn++);
            c2 += (*(zIn++))<<8;
            ಯದಿ( c2<0xdc00 || c2>=0xe000 ){
              zIn -= 2;
              c = 0xfffd;
            }ಅನ್ಯಥಾ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ಅನ್ಯಥಾ
          ಯದಿ( zIn<zTerm ){
            ಪೂರ್ಣಾಙ್ಕ c2 = (*zIn++);
            c2 += ((*zIn++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#endif
        }
        WRITE_UTF8(z, c);
      }
    }ಅನ್ಯಥಾ{
      /* UTF-16 Big-endian -> UTF-8 */
      ಯಾವತ್( zIn<zTerm ){
        c = (*(zIn++))<<8;
        c += *(zIn++);
        ಯದಿ( c>=0xd800 && c<0xe000 ){
#ifdef SQLITE_REPLACE_INVALID_UTF
          ಯದಿ( c>=0xdc00 || zIn>=zTerm ){
            c = 0xfffd;
          }ಅನ್ಯಥಾ{
            ಪೂರ್ಣಾಙ್ಕ c2 = (*(zIn++))<<8;
            c2 += *(zIn++);
            ಯದಿ( c2<0xdc00 || c2>=0xe000 ){
              zIn -= 2;
              c = 0xfffd;
            }ಅನ್ಯಥಾ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ಅನ್ಯಥಾ
          ಯದಿ( zIn<zTerm ){
            ಪೂರ್ಣಾಙ್ಕ c2 = ((*zIn++)<<8);
            c2 += (*zIn++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#endif
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (ಪೂರ್ಣಾಙ್ಕ)(z - zOut);
  }
  *z = 0;
  ಪ್ರತಿಜ್ಞಾ( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_Term|(pMem->flags&(MEM_AffMask|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (ವರ್ಣ*)zOut;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->z);

translate_out:
#ಯದಿ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    ವರ್ಣ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ಪರಿಮಾಣ(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3StrAccumFinish(&acc));
  }
#endif
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3VdbeMemHandleBom(Mem *pMem){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  u8 bom = 0;

  ಪ್ರತಿಜ್ಞಾ( pMem->n>=0 );
  ಯದಿ( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    ಯದಿ( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    ಯದಿ( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  ಯದಿ( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ಯದಿ( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_Term;
      pMem->enc = bom;
    }
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3Utf8CharLen(ಧ್ರುವ ವರ್ಣ *zIn, ಪೂರ್ಣಾಙ್ಕ nByte){
  ಪೂರ್ಣಾಙ್ಕ r = 0;
  ಧ್ರುವ u8 *z = (ಧ್ರುವ u8*)zIn;
  ಧ್ರುವ u8 *zTerm;
  ಯದಿ( nByte>=0 ){
    zTerm = &z[nByte];
  }ಅನ್ಯಥಾ{
    zTerm = (ಧ್ರುವ u8*)(-1);
  }
  ಪ್ರತಿಜ್ಞಾ( z<=zTerm );
  ಯಾವತ್( *z!=0 && z<zTerm ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  ಪ್ರತ್ಯಾಗಮ r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#ಯದಿ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3Utf8To8(unsigned ವರ್ಣ *zIn){
  unsigned ವರ್ಣ *zOut = zIn;
  unsigned ವರ್ಣ *zStart = zIn;
  u32 c;

  ಯಾವತ್( zIn[0] && zOut<=zIn ){
    c = sqlite3Utf8Read((ಧ್ರುವ u8**)&zIn);
    ಯದಿ( c!=0xfffd ){
      WRITE_UTF8(zOut, c);
    }
  }
  *zOut = 0;
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)(zOut - zStart);
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
ವರ್ಣ *sqlite3Utf16to8(sqlite3 *db, ಧ್ರುವ ಶೂನ್ಯ *z, ಪೂರ್ಣಾಙ್ಕ nByte, u8 enc){
  Mem m;
  memset(&m, 0, ಪರಿಮಾಣ(m));
  m.db = db;
  sqlite3VdbeMemSetStr(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  ಯದಿ( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( (m.flags & MEM_Term)!=0 || db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( m.z || db->mallocFailed );
  ಪ್ರತ್ಯಾಗಮ m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3Utf16ByteLen(ಧ್ರುವ ಶೂನ್ಯ *zIn, ಪೂರ್ಣಾಙ್ಕ nByte, ಪೂರ್ಣಾಙ್ಕ nChar){
  ಪೂರ್ಣಾಙ್ಕ c;
  unsigned ವರ್ಣ ಧ್ರುವ *z = zIn;
  unsigned ವರ್ಣ ಧ್ರುವ *zEnd = &z[nByte-1];
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  
  ಯದಿ( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  ಯಾವತ್( n<nChar && z<=zEnd ){
    c = z[0];
    z += 2;
    ಯದಿ( c>=0xd8 && c<0xdc && z<=zEnd && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)(z-(unsigned ವರ್ಣ ಧ್ರುವ *)zIn) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#ಯದಿ defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
ಶೂನ್ಯ sqlite3UtfSelfTest(ಶೂನ್ಯ){
  unsigned ಪೂರ್ಣಾಙ್ಕ i, t;
  unsigned ವರ್ಣ zBuf[20];
  unsigned ವರ್ಣ *z;
  ಪೂರ್ಣಾಙ್ಕ n;
  unsigned ಪೂರ್ಣಾಙ್ಕ c;

  ಪ್ರತಿ(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (ಪೂರ್ಣಾಙ್ಕ)(z-zBuf);
    ಪ್ರತಿಜ್ಞಾ( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ಧ್ರುವ u8**)&z);
    t = i;
    ಯದಿ( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    ಪ್ರತಿಜ್ಞಾ( c==t );
    ಪ್ರತಿಜ್ಞಾ( (z-zBuf)==n );
  }
}
#endif /* SQLITE_TEST */
#endif /* SQLITE_OMIT_UTF16 */
