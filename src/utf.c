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

#ಆದರೆ !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ sqlite3one = 1;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#ವ್ಯಾಖ್ಯಾನಿಸು WRITE_UTF8(z_ಹೊರಹೋಗು, c) {                          \
  ಆದರೆ( c<0x00080 ){                                     \
    *z_ಹೊರಹೋಗು++ = (u8)(c&0xFF);                            \
  }                                                    \
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c<0x00800 ){                                \
    *z_ಹೊರಹೋಗು++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *z_ಹೊರಹೋಗು++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c<0x10000 ){                                \
    *z_ಹೊರಹೋಗು++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *z_ಹೊರಹೋಗು++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_ಹೊರಹೋಗು++ = 0x80 + (u8)(c & 0x3F);                   \
  }ಇಲ್ಲದಿದ್ದರೆ{                                               \
    *z_ಹೊರಹೋಗು++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *z_ಹೊರಹೋಗು++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *z_ಹೊರಹೋಗು++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_ಹೊರಹೋಗು++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#ವ್ಯಾಖ್ಯಾನಿಸು WRITE_UTF16LE(z_ಹೊರಹೋಗು, c) {                                    \
  ಆದರೆ( c<=0xFFFF ){                                                  \
    *z_ಹೊರಹೋಗು++ = (u8)(c&0x00FF);                                       \
    *z_ಹೊರಹೋಗು++ = (u8)((c>>8)&0x00FF);                                  \
  }ಇಲ್ಲದಿದ್ದರೆ{                                                            \
    *z_ಹೊರಹೋಗು++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_ಹೊರಹೋಗು++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_ಹೊರಹೋಗು++ = (u8)(c&0x00FF);                                       \
    *z_ಹೊರಹೋಗು++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#ವ್ಯಾಖ್ಯಾನಿಸು WRITE_UTF16BE(z_ಹೊರಹೋಗು, c) {                                    \
  ಆದರೆ( c<=0xFFFF ){                                                  \
    *z_ಹೊರಹೋಗು++ = (u8)((c>>8)&0x00FF);                                  \
    *z_ಹೊರಹೋಗು++ = (u8)(c&0x00FF);                                       \
  }ಇಲ್ಲದಿದ್ದರೆ{                                                            \
    *z_ಹೊರಹೋಗು++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_ಹೊರಹೋಗು++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_ಹೊರಹೋಗು++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *z_ಹೊರಹೋಗು++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
ಪೂರ್ಣಾಂಕ sqlite3AppendOneUtf8Character(ಅಕ್ಷರ *z_ಹೊರಹೋಗು, u32 v){
  ಆದರೆ( v<0x00080 ){
    z_ಹೊರಹೋಗು[0] = (u8)(v & 0xff);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( v<0x00800 ){
    z_ಹೊರಹೋಗು[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    z_ಹೊರಹೋಗು[1] = 0x80 + (u8)(v & 0x3f);
    ಹಿಂತಿರುಗಿ 2;
  }
  ಆದರೆ( v<0x10000 ){
    z_ಹೊರಹೋಗು[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    z_ಹೊರಹೋಗು[1] = 0x80 + (u8)((v>>6) & 0x3f);
    z_ಹೊರಹೋಗು[2] = 0x80 + (u8)(v & 0x3f);
    ಹಿಂತಿರುಗಿ 3;
  }
  z_ಹೊರಹೋಗು[0] = 0xf0 + (u8)((v>>18) & 0x07);
  z_ಹೊರಹೋಗು[1] = 0x80 + (u8)((v>>12) & 0x3f);
  z_ಹೊರಹೋಗು[2] = 0x80 + (u8)((v>>6) & 0x3f);
  z_ಹೊರಹೋಗು[3] = 0x80 + (u8)(v & 0x3f);
  ಹಿಂತಿರುಗಿ 4;
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
#ವ್ಯಾಖ್ಯಾನಿಸು READ_UTF8(z_ಒಳ, z_ಪದ, c)                           \
  c = *(z_ಒಳ++);                                            \
  ಆದರೆ( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    ಆಗ( z_ಒಳ<z_ಪದ && (*z_ಒಳ & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(z_ಒಳ++));                      \
    }                                                      \
    ಆದರೆ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **pz    /* Pointer to string from which to read char */
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  ಆದರೆ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ಆಗ( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    ಆದರೆ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  ಹಿಂತಿರುಗಿ c;
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
ಪೂರ್ಣಾಂಕ sqlite3Utf8ReadLimited(
  ಸ್ಥಿರಾಂಕ u8 *z,
  ಪೂರ್ಣಾಂಕ n,
  u32 *pi_ಹೊರಹೋಗು
){
  u32 c;
  ಪೂರ್ಣಾಂಕ i = 1;
  ಪ್ರತಿಜ್ಞಾ( n>0 );
  c = z[0];
  ಆದರೆ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ಆದರೆ( n>4 ) n = 4;
    ಆಗ( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *pi_ಹೊರಹೋಗು = c;
  ಹಿಂತಿರುಗಿ i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಹೊರಹೋಗು;        /* Output buffer */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಒಳ;         /* Input iterator */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಪದ;       /* End of input */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z;           /* Output iterator */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ c;

  ಪ್ರತಿಜ್ಞಾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pMem->flags&MEM_Str );
  ಪ್ರತಿಜ್ಞಾ( pMem->enc!=desiredEnc );
  ಪ್ರತಿಜ್ಞಾ( pMem->enc!=0 );
  ಪ್ರತಿಜ್ಞಾ( pMem->n>=0 );

#ಆದರೆ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    ಅಕ್ಷರ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ಗಾತ್ರ(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc));
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  ಆದರೆ( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 ತಾತ್ಕಾಲಿಕ;
    ಪೂರ್ಣಾಂಕ rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_NOMEM );
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    z_ಒಳ = (u8*)pMem->z;
    z_ಪದ = &z_ಒಳ[pMem->n&~1];
    ಆಗ( z_ಒಳ<z_ಪದ ){
      ತಾತ್ಕಾಲಿಕ = *z_ಒಳ;
      *z_ಒಳ = *(z_ಒಳ+1);
      z_ಒಳ++;
      *z_ಒಳ++ = ತಾತ್ಕಾಲಿಕ;
    }
    pMem->enc = desiredEnc;
    ಹೋಗು translate_ಹೊರಹೋಗು;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  ಆದರೆ( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
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
  z_ಒಳ = (u8*)pMem->z;
  z_ಪದ = &z_ಒಳ[pMem->n];
  z_ಹೊರಹೋಗು = sqlite3DbMallocRaw(pMem->db, len);
  ಆದರೆ( !z_ಹೊರಹೋಗು ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  z = z_ಹೊರಹೋಗು;

  ಆದರೆ( pMem->enc==SQLITE_UTF8 ){
    ಆದರೆ( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      ಆಗ( z_ಒಳ<z_ಪದ ){
        READ_UTF8(z_ಒಳ, z_ಪದ, c);
        WRITE_UTF16LE(z, c);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      ಆಗ( z_ಒಳ<z_ಪದ ){
        READ_UTF8(z_ಒಳ, z_ಪದ, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (ಪೂರ್ಣಾಂಕ)(z - z_ಹೊರಹೋಗು);
    *z++ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( desiredEnc==SQLITE_UTF8 );
    ಆದರೆ( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      ಆಗ( z_ಒಳ<z_ಪದ ){
        c = *(z_ಒಳ++);
        c += (*(z_ಒಳ++))<<8;
        ಆದರೆ( c>=0xd800 && c<0xe000 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_REPLACE_INVALID_UTF
          ಆದರೆ( c>=0xdc00 || z_ಒಳ>=z_ಪದ ){
            c = 0xfffd;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ಣಾಂಕ c2 = *(z_ಒಳ++);
            c2 += (*(z_ಒಳ++))<<8;
            ಆದರೆ( c2<0xdc00 || c2>=0xe000 ){
              z_ಒಳ -= 2;
              c = 0xfffd;
            }ಇಲ್ಲದಿದ್ದರೆ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ಇಲ್ಲದಿದ್ದರೆ
          ಆದರೆ( z_ಒಳ<z_ಪದ ){
            ಪೂರ್ಣಾಂಕ c2 = (*z_ಒಳ++);
            c2 += ((*z_ಒಳ++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#ಆದರೆ_ಅಂತ್ಯ
        }
        WRITE_UTF8(z, c);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* UTF-16 Big-endian -> UTF-8 */
      ಆಗ( z_ಒಳ<z_ಪದ ){
        c = (*(z_ಒಳ++))<<8;
        c += *(z_ಒಳ++);
        ಆದರೆ( c>=0xd800 && c<0xe000 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_REPLACE_INVALID_UTF
          ಆದರೆ( c>=0xdc00 || z_ಒಳ>=z_ಪದ ){
            c = 0xfffd;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ಣಾಂಕ c2 = (*(z_ಒಳ++))<<8;
            c2 += *(z_ಒಳ++);
            ಆದರೆ( c2<0xdc00 || c2>=0xe000 ){
              z_ಒಳ -= 2;
              c = 0xfffd;
            }ಇಲ್ಲದಿದ್ದರೆ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ಇಲ್ಲದಿದ್ದರೆ
          ಆದರೆ( z_ಒಳ<z_ಪದ ){
            ಪೂರ್ಣಾಂಕ c2 = ((*z_ಒಳ++)<<8);
            c2 += (*z_ಒಳ++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#ಆದರೆ_ಅಂತ್ಯ
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (ಪೂರ್ಣಾಂಕ)(z - z_ಹೊರಹೋಗು);
  }
  *z = 0;
  ಪ್ರತಿಜ್ಞಾ( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_ಪದ|(pMem->flags&(MEM_Aff_ಮುಖವಾಡ|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (ಅಕ್ಷರ*)z_ಹೊರಹೋಗು;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_ಗಾತ್ರ(pMem->db, pMem->z);

translate_ಹೊರಹೋಗು:
#ಆದರೆ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    ಅಕ್ಷರ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ಗಾತ್ರ(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc));
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeMemHandleBom(Mem *pMem){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  u8 bom = 0;

  ಪ್ರತಿಜ್ಞಾ( pMem->n>=0 );
  ಆದರೆ( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    ಆದರೆ( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    ಆದರೆ( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  ಆದರೆ( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ಆದರೆ( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_ಪದ;
      pMem->enc = bom;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
ಪೂರ್ಣಾಂಕ sqlite3Utf8CharLen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ, ಪೂರ್ಣಾಂಕ nByte){
  ಪೂರ್ಣಾಂಕ r = 0;
  ಸ್ಥಿರಾಂಕ u8 *z = (ಸ್ಥಿರಾಂಕ u8*)z_ಒಳ;
  ಸ್ಥಿರಾಂಕ u8 *z_ಪದ;
  ಆದರೆ( nByte>=0 ){
    z_ಪದ = &z[nByte];
  }ಇಲ್ಲದಿದ್ದರೆ{
    z_ಪದ = (ಸ್ಥಿರಾಂಕ u8*)(-1);
  }
  ಪ್ರತಿಜ್ಞಾ( z<=z_ಪದ );
  ಆಗ( *z!=0 && z<z_ಪದ ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  ಹಿಂತಿರುಗಿ r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#ಆದರೆ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
ಪೂರ್ಣಾಂಕ sqlite3Utf8To8(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಒಳ){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಹೊರಹೋಗು = z_ಒಳ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z_ಆರಂಭ = z_ಒಳ;
  u32 c;

  ಆಗ( z_ಒಳ[0] && z_ಹೊರಹೋಗು<=z_ಒಳ ){
    c = sqlite3Utf8Read((ಸ್ಥಿರಾಂಕ u8**)&z_ಒಳ);
    ಆದರೆ( c!=0xfffd ){
      WRITE_UTF8(z_ಹೊರಹೋಗು, c);
    }
  }
  *z_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(z_ಹೊರಹೋಗು - z_ಆರಂಭ);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
ಅಕ್ಷರ *sqlite3Utf16to8(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z, ಪೂರ್ಣಾಂಕ nByte, u8 enc){
  Mem m;
  memset(&m, 0, ಗಾತ್ರ(m));
  m.db = db;
  sqlite3_Vdbe_Mem_ಗಣ_Str(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  ಆದರೆ( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( (m.flags & MEM_ಪದ)!=0 || db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( m.z || db->mallocFailed );
  ಹಿಂತಿರುಗಿ m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
ಪೂರ್ಣಾಂಕ sqlite3Utf16ByteLen(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z_ಒಳ, ಪೂರ್ಣಾಂಕ nByte, ಪೂರ್ಣಾಂಕ nChar){
  ಪೂರ್ಣಾಂಕ c;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z = z_ಒಳ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z_ಕೊನೆ = &z[nByte-1];
  ಪೂರ್ಣಾಂಕ n = 0;
  
  ಆದರೆ( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  ಆಗ( n<nChar && z<=z_ಕೊನೆ ){
    c = z[0];
    z += 2;
    ಆದರೆ( c>=0xd8 && c<0xdc && z<=z_ಕೊನೆ && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(z-(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *)z_ಒಳ) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#ಆದರೆ defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
ಶೂನ್ಯ sqlite3UtfSelfTest(ಶೂನ್ಯ){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, t;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zBuf[20];
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z;
  ಪೂರ್ಣಾಂಕ n;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ c;

  ಗಾಗಿ(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (ಪೂರ್ಣಾಂಕ)(z-zBuf);
    ಪ್ರತಿಜ್ಞಾ( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ಸ್ಥಿರಾಂಕ u8**)&z);
    t = i;
    ಆದರೆ( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    ಪ್ರತಿಜ್ಞಾ( c==t );
    ಪ್ರತಿಜ್ಞಾ( (z-zBuf)==n );
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
