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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

#ਜੇ !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ਅਚਲ ਪੂਰਨ sqlite3one = 1;
#ਜੇ_ਅੰਤ /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE_UTF8(z_ਬਾਹਰਲਾ, c) {                          \
  ਜੇ( c<0x00080 ){                                     \
    *z_ਬਾਹਰਲਾ++ = (u8)(c&0xFF);                            \
  }                                                    \
  ਨਹੀਂਤਾਂ ਜੇ( c<0x00800 ){                                \
    *z_ਬਾਹਰਲਾ++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  ਨਹੀਂਤਾਂ ਜੇ( c<0x10000 ){                                \
    *z_ਬਾਹਰਲਾ++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)(c & 0x3F);                   \
  }ਨਹੀਂਤਾਂ{                                               \
    *z_ਬਾਹਰਲਾ++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_ਬਾਹਰਲਾ++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE_UTF16LE(z_ਬਾਹਰਲਾ, c) {                                    \
  ਜੇ( c<=0xFFFF ){                                                  \
    *z_ਬਾਹਰਲਾ++ = (u8)(c&0x00FF);                                       \
    *z_ਬਾਹਰਲਾ++ = (u8)((c>>8)&0x00FF);                                  \
  }ਨਹੀਂਤਾਂ{                                                            \
    *z_ਬਾਹਰਲਾ++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_ਬਾਹਰਲਾ++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_ਬਾਹਰਲਾ++ = (u8)(c&0x00FF);                                       \
    *z_ਬਾਹਰਲਾ++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE_UTF16BE(z_ਬਾਹਰਲਾ, c) {                                    \
  ਜੇ( c<=0xFFFF ){                                                  \
    *z_ਬਾਹਰਲਾ++ = (u8)((c>>8)&0x00FF);                                  \
    *z_ਬਾਹਰਲਾ++ = (u8)(c&0x00FF);                                       \
  }ਨਹੀਂਤਾਂ{                                                            \
    *z_ਬਾਹਰਲਾ++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_ਬਾਹਰਲਾ++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_ਬਾਹਰਲਾ++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *z_ਬਾਹਰਲਾ++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
ਪੂਰਨ sqlite3AppendOneUtf8Character(ਅੱਖਰ *z_ਬਾਹਰਲਾ, u32 v){
  ਜੇ( v<0x00080 ){
    z_ਬਾਹਰਲਾ[0] = (u8)(v & 0xff);
    ਵਾਪਸ 1;
  }
  ਜੇ( v<0x00800 ){
    z_ਬਾਹਰਲਾ[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    z_ਬਾਹਰਲਾ[1] = 0x80 + (u8)(v & 0x3f);
    ਵਾਪਸ 2;
  }
  ਜੇ( v<0x10000 ){
    z_ਬਾਹਰਲਾ[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    z_ਬਾਹਰਲਾ[1] = 0x80 + (u8)((v>>6) & 0x3f);
    z_ਬਾਹਰਲਾ[2] = 0x80 + (u8)(v & 0x3f);
    ਵਾਪਸ 3;
  }
  z_ਬਾਹਰਲਾ[0] = 0xf0 + (u8)((v>>18) & 0x07);
  z_ਬਾਹਰਲਾ[1] = 0x80 + (u8)((v>>12) & 0x3f);
  z_ਬਾਹਰਲਾ[2] = 0x80 + (u8)((v>>6) & 0x3f);
  z_ਬਾਹਰਲਾ[3] = 0x80 + (u8)(v & 0x3f);
  ਵਾਪਸ 4;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ_UTF8(z_ਵਿੱਚ, z_ਪਦ, c)                           \
  c = *(z_ਵਿੱਚ++);                                            \
  ਜੇ( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    ਜਦੋਂ( z_ਵਿੱਚ<z_ਪਦ && (*z_ਵਿੱਚ & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(z_ਵਿੱਚ++));                      \
    }                                                      \
    ਜੇ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **pz    /* Pointer to string from which to read char */
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  ਜੇ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ਜਦੋਂ( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    ਜੇ( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  ਵਾਪਸ c;
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
ਪੂਰਨ sqlite3Utf8ReadLimited(
  ਅਚਲ u8 *z,
  ਪੂਰਨ n,
  u32 *pi_ਬਾਹਰਲਾ
){
  u32 c;
  ਪੂਰਨ i = 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( n>0 );
  c = z[0];
  ਜੇ( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    ਜੇ( n>4 ) n = 4;
    ਜਦੋਂ( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *pi_ਬਾਹਰਲਾ = c;
  ਵਾਪਸ i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE ਪੂਰਨ sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਬਾਹਰਲਾ;        /* Output buffer */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਵਿੱਚ;         /* Input iterator */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਪਦ;       /* End of input */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z;           /* Output iterator */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c;

  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->flags&MEM_Str );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->enc!=desiredEnc );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->enc!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->n>=0 );

#ਜੇ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    ਅੱਖਰ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ਆਕਾਰ(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3_Str_Accum_ਅੰਤ(&acc));
  }
#ਜੇ_ਅੰਤ

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  ਜੇ( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 ਆਰਜ਼ੀ;
    ਪੂਰਨ rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ਜੇ( rc!=SQLITE_OK ){
      ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_NOMEM );
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    z_ਵਿੱਚ = (u8*)pMem->z;
    z_ਪਦ = &z_ਵਿੱਚ[pMem->n&~1];
    ਜਦੋਂ( z_ਵਿੱਚ<z_ਪਦ ){
      ਆਰਜ਼ੀ = *z_ਵਿੱਚ;
      *z_ਵਿੱਚ = *(z_ਵਿੱਚ+1);
      z_ਵਿੱਚ++;
      *z_ਵਿੱਚ++ = ਆਰਜ਼ੀ;
    }
    pMem->enc = desiredEnc;
    ਜਾਓ translate_ਬਾਹਰਲਾ;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  ਜੇ( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }ਨਹੀਂਤਾਂ{
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
  z_ਵਿੱਚ = (u8*)pMem->z;
  z_ਪਦ = &z_ਵਿੱਚ[pMem->n];
  z_ਬਾਹਰਲਾ = sqlite3DbMallocRaw(pMem->db, len);
  ਜੇ( !z_ਬਾਹਰਲਾ ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  z = z_ਬਾਹਰਲਾ;

  ਜੇ( pMem->enc==SQLITE_UTF8 ){
    ਜੇ( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      ਜਦੋਂ( z_ਵਿੱਚ<z_ਪਦ ){
        READ_UTF8(z_ਵਿੱਚ, z_ਪਦ, c);
        WRITE_UTF16LE(z, c);
      }
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      ਜਦੋਂ( z_ਵਿੱਚ<z_ਪਦ ){
        READ_UTF8(z_ਵਿੱਚ, z_ਪਦ, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (ਪੂਰਨ)(z - z_ਬਾਹਰਲਾ);
    *z++ = 0;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( desiredEnc==SQLITE_UTF8 );
    ਜੇ( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      ਜਦੋਂ( z_ਵਿੱਚ<z_ਪਦ ){
        c = *(z_ਵਿੱਚ++);
        c += (*(z_ਵਿੱਚ++))<<8;
        ਜੇ( c>=0xd800 && c<0xe000 ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_REPLACE_INVALID_UTF
          ਜੇ( c>=0xdc00 || z_ਵਿੱਚ>=z_ਪਦ ){
            c = 0xfffd;
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ c2 = *(z_ਵਿੱਚ++);
            c2 += (*(z_ਵਿੱਚ++))<<8;
            ਜੇ( c2<0xdc00 || c2>=0xe000 ){
              z_ਵਿੱਚ -= 2;
              c = 0xfffd;
            }ਨਹੀਂਤਾਂ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ਨਹੀਂਤਾਂ
          ਜੇ( z_ਵਿੱਚ<z_ਪਦ ){
            ਪੂਰਨ c2 = (*z_ਵਿੱਚ++);
            c2 += ((*z_ਵਿੱਚ++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#ਜੇ_ਅੰਤ
        }
        WRITE_UTF8(z, c);
      }
    }ਨਹੀਂਤਾਂ{
      /* UTF-16 Big-endian -> UTF-8 */
      ਜਦੋਂ( z_ਵਿੱਚ<z_ਪਦ ){
        c = (*(z_ਵਿੱਚ++))<<8;
        c += *(z_ਵਿੱਚ++);
        ਜੇ( c>=0xd800 && c<0xe000 ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_REPLACE_INVALID_UTF
          ਜੇ( c>=0xdc00 || z_ਵਿੱਚ>=z_ਪਦ ){
            c = 0xfffd;
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ c2 = (*(z_ਵਿੱਚ++))<<8;
            c2 += *(z_ਵਿੱਚ++);
            ਜੇ( c2<0xdc00 || c2>=0xe000 ){
              z_ਵਿੱਚ -= 2;
              c = 0xfffd;
            }ਨਹੀਂਤਾਂ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ਨਹੀਂਤਾਂ
          ਜੇ( z_ਵਿੱਚ<z_ਪਦ ){
            ਪੂਰਨ c2 = ((*z_ਵਿੱਚ++)<<8);
            c2 += (*z_ਵਿੱਚ++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#ਜੇ_ਅੰਤ
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (ਪੂਰਨ)(z - z_ਬਾਹਰਲਾ);
  }
  *z = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_ਪਦ|(pMem->flags&(MEM_Aff_ਨਕਾਬ|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (ਅੱਖਰ*)z_ਬਾਹਰਲਾ;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_ਮਾਪ(pMem->db, pMem->z);

translate_ਬਾਹਰਲਾ:
#ਜੇ defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    ਅੱਖਰ zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, ਆਕਾਰ(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3_Str_Accum_ਅੰਤ(&acc));
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
ਪੂਰਨ sqlite3VdbeMemHandleBom(Mem *pMem){
  ਪੂਰਨ rc = SQLITE_OK;
  u8 bom = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( pMem->n>=0 );
  ਜੇ( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    ਜੇ( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    ਜੇ( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  ਜੇ( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    ਜੇ( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_ਪਦ;
      pMem->enc = bom;
    }
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
ਪੂਰਨ sqlite3Utf8CharLen(ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ, ਪੂਰਨ nByte){
  ਪੂਰਨ r = 0;
  ਅਚਲ u8 *z = (ਅਚਲ u8*)z_ਵਿੱਚ;
  ਅਚਲ u8 *z_ਪਦ;
  ਜੇ( nByte>=0 ){
    z_ਪਦ = &z[nByte];
  }ਨਹੀਂਤਾਂ{
    z_ਪਦ = (ਅਚਲ u8*)(-1);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( z<=z_ਪਦ );
  ਜਦੋਂ( *z!=0 && z<z_ਪਦ ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  ਵਾਪਸ r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#ਜੇ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
ਪੂਰਨ sqlite3Utf8To8(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਵਿੱਚ){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਬਾਹਰਲਾ = z_ਵਿੱਚ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z_ਸ਼ੁਰੂ = z_ਵਿੱਚ;
  u32 c;

  ਜਦੋਂ( z_ਵਿੱਚ[0] && z_ਬਾਹਰਲਾ<=z_ਵਿੱਚ ){
    c = sqlite3Utf8Read((ਅਚਲ u8**)&z_ਵਿੱਚ);
    ਜੇ( c!=0xfffd ){
      WRITE_UTF8(z_ਬਾਹਰਲਾ, c);
    }
  }
  *z_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ (ਪੂਰਨ)(z_ਬਾਹਰਲਾ - z_ਸ਼ੁਰੂ);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
ਅੱਖਰ *sqlite3Utf16to8(sqlite3 *db, ਅਚਲ ਖਾਲੀ *z, ਪੂਰਨ nByte, u8 enc){
  Mem m;
  memset(&m, 0, ਆਕਾਰ(m));
  m.db = db;
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Str(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  ਜੇ( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (m.flags & MEM_ਪਦ)!=0 || db->mallocFailed );
  ਪ੍ਰਤਿਜ੍ਞਾ( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  ਪ੍ਰਤਿਜ੍ਞਾ( m.z || db->mallocFailed );
  ਵਾਪਸ m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
ਪੂਰਨ sqlite3Utf16ByteLen(ਅਚਲ ਖਾਲੀ *z_ਵਿੱਚ, ਪੂਰਨ nByte, ਪੂਰਨ nChar){
  ਪੂਰਨ c;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਚਲ *z = z_ਵਿੱਚ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਚਲ *z_ਸਿਰਾ = &z[nByte-1];
  ਪੂਰਨ n = 0;
  
  ਜੇ( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  ਜਦੋਂ( n<nChar && z<=z_ਸਿਰਾ ){
    c = z[0];
    z += 2;
    ਜੇ( c>=0xd8 && c<0xdc && z<=z_ਸਿਰਾ && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  ਵਾਪਸ (ਪੂਰਨ)(z-(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਚਲ *)z_ਵਿੱਚ) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#ਜੇ defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
ਖਾਲੀ sqlite3UtfSelfTest(ਖਾਲੀ){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, t;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zBuf[20];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z;
  ਪੂਰਨ n;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c;

  ਲਈ(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (ਪੂਰਨ)(z-zBuf);
    ਪ੍ਰਤਿਜ੍ਞਾ( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ਅਚਲ u8**)&z);
    t = i;
    ਜੇ( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    ਪ੍ਰਤਿਜ੍ਞਾ( c==t );
    ਪ੍ਰਤਿਜ੍ਞਾ( (z-zBuf)==n );
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_TEST */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
