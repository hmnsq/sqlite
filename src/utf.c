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
#شامل "sqliteInt.h"
#شامل <assert.h>
#شامل "vdbeInt.h"

#اگر !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ثابت صحیح sqlite3one = 1;
#اگر_ختم /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
ساکن ثابت غیرعلامتی حرف sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#تعریف_کرو WRITE_UTF8(z_برآمد, c) {                          \
  اگر( c<0x00080 ){                                     \
    *z_برآمد++ = (u8)(c&0xFF);                            \
  }                                                    \
  ورنہ اگر( c<0x00800 ){                                \
    *z_برآمد++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *z_برآمد++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  ورنہ اگر( c<0x10000 ){                                \
    *z_برآمد++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *z_برآمد++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_برآمد++ = 0x80 + (u8)(c & 0x3F);                   \
  }ورنہ{                                               \
    *z_برآمد++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *z_برآمد++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *z_برآمد++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_برآمد++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#تعریف_کرو WRITE_UTF16LE(z_برآمد, c) {                                    \
  اگر( c<=0xFFFF ){                                                  \
    *z_برآمد++ = (u8)(c&0x00FF);                                       \
    *z_برآمد++ = (u8)((c>>8)&0x00FF);                                  \
  }ورنہ{                                                            \
    *z_برآمد++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_برآمد++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_برآمد++ = (u8)(c&0x00FF);                                       \
    *z_برآمد++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#تعریف_کرو WRITE_UTF16BE(z_برآمد, c) {                                    \
  اگر( c<=0xFFFF ){                                                  \
    *z_برآمد++ = (u8)((c>>8)&0x00FF);                                  \
    *z_برآمد++ = (u8)(c&0x00FF);                                       \
  }ورنہ{                                                            \
    *z_برآمد++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_برآمد++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_برآمد++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *z_برآمد++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
صحیح sqlite3AppendOneUtf8Character(حرف *z_برآمد, u32 v){
  اگر( v<0x00080 ){
    z_برآمد[0] = (u8)(v & 0xff);
    واپس 1;
  }
  اگر( v<0x00800 ){
    z_برآمد[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    z_برآمد[1] = 0x80 + (u8)(v & 0x3f);
    واپس 2;
  }
  اگر( v<0x10000 ){
    z_برآمد[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    z_برآمد[1] = 0x80 + (u8)((v>>6) & 0x3f);
    z_برآمد[2] = 0x80 + (u8)(v & 0x3f);
    واپس 3;
  }
  z_برآمد[0] = 0xf0 + (u8)((v>>18) & 0x07);
  z_برآمد[1] = 0x80 + (u8)((v>>12) & 0x3f);
  z_برآمد[2] = 0x80 + (u8)((v>>6) & 0x3f);
  z_برآمد[3] = 0x80 + (u8)(v & 0x3f);
  واپس 4;
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
#تعریف_کرو READ_UTF8(z_اندر, z_رکن, c)                           \
  c = *(z_اندر++);                                            \
  اگر( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    جب( z_اندر<z_رکن && (*z_اندر & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(z_اندر++));                      \
    }                                                      \
    اگر( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ثابت غیرعلامتی حرف **pz    /* Pointer to string from which to read char */
){
  غیرعلامتی صحیح c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  اگر( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    جب( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    اگر( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  واپس c;
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
صحیح sqlite3Utf8ReadLimited(
  ثابت u8 *z,
  صحیح n,
  u32 *pi_برآمد
){
  u32 c;
  صحیح i = 1;
  أكد( n>0 );
  c = z[0];
  اگر( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    اگر( n>4 ) n = 4;
    جب( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *pi_برآمد = c;
  واپس i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE صحیح sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  غیرعلامتی حرف *z_برآمد;        /* Output buffer */
  غیرعلامتی حرف *z_اندر;         /* Input iterator */
  غیرعلامتی حرف *z_رکن;       /* End of input */
  غیرعلامتی حرف *z;           /* Output iterator */
  غیرعلامتی صحیح c;

  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( pMem->flags&MEM_Str );
  أكد( pMem->enc!=desiredEnc );
  أكد( pMem->enc!=0 );
  أكد( pMem->n>=0 );

#اگر defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    حرف zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3_Str_Accum_اختتام(&acc));
  }
#اگر_ختم

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  اگر( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 عارضی;
    صحیح rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    اگر( rc!=SQLITE_OK ){
      أكد( rc==SQLITE_NOMEM );
      واپس SQLITE_NOMEM_BKPT;
    }
    z_اندر = (u8*)pMem->z;
    z_رکن = &z_اندر[pMem->n&~1];
    جب( z_اندر<z_رکن ){
      عارضی = *z_اندر;
      *z_اندر = *(z_اندر+1);
      z_اندر++;
      *z_اندر++ = عارضی;
    }
    pMem->enc = desiredEnc;
    جاؤ translate_برآمد;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  اگر( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }ورنہ{
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
  z_اندر = (u8*)pMem->z;
  z_رکن = &z_اندر[pMem->n];
  z_برآمد = sqlite3DbMallocRaw(pMem->db, len);
  اگر( !z_برآمد ){
    واپس SQLITE_NOMEM_BKPT;
  }
  z = z_برآمد;

  اگر( pMem->enc==SQLITE_UTF8 ){
    اگر( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      جب( z_اندر<z_رکن ){
        READ_UTF8(z_اندر, z_رکن, c);
        WRITE_UTF16LE(z, c);
      }
    }ورنہ{
      أكد( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      جب( z_اندر<z_رکن ){
        READ_UTF8(z_اندر, z_رکن, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (صحیح)(z - z_برآمد);
    *z++ = 0;
  }ورنہ{
    أكد( desiredEnc==SQLITE_UTF8 );
    اگر( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      جب( z_اندر<z_رکن ){
        c = *(z_اندر++);
        c += (*(z_اندر++))<<8;
        اگر( c>=0xd800 && c<0xe000 ){
#اگر_تعریف_شدہ SQLITE_REPLACE_INVALID_UTF
          اگر( c>=0xdc00 || z_اندر>=z_رکن ){
            c = 0xfffd;
          }ورنہ{
            صحیح c2 = *(z_اندر++);
            c2 += (*(z_اندر++))<<8;
            اگر( c2<0xdc00 || c2>=0xe000 ){
              z_اندر -= 2;
              c = 0xfffd;
            }ورنہ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ورنہ
          اگر( z_اندر<z_رکن ){
            صحیح c2 = (*z_اندر++);
            c2 += ((*z_اندر++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#اگر_ختم
        }
        WRITE_UTF8(z, c);
      }
    }ورنہ{
      /* UTF-16 Big-endian -> UTF-8 */
      جب( z_اندر<z_رکن ){
        c = (*(z_اندر++))<<8;
        c += *(z_اندر++);
        اگر( c>=0xd800 && c<0xe000 ){
#اگر_تعریف_شدہ SQLITE_REPLACE_INVALID_UTF
          اگر( c>=0xdc00 || z_اندر>=z_رکن ){
            c = 0xfffd;
          }ورنہ{
            صحیح c2 = (*(z_اندر++))<<8;
            c2 += *(z_اندر++);
            اگر( c2<0xdc00 || c2>=0xe000 ){
              z_اندر -= 2;
              c = 0xfffd;
            }ورنہ{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#ورنہ
          اگر( z_اندر<z_رکن ){
            صحیح c2 = ((*z_اندر++)<<8);
            c2 += (*z_اندر++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#اگر_ختم
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (صحیح)(z - z_برآمد);
  }
  *z = 0;
  أكد( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_رکن|(pMem->flags&(MEM_Aff_نقاب|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (حرف*)z_برآمد;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_سائز(pMem->db, pMem->z);

translate_برآمد:
#اگر defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    حرف zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3_Str_Accum_اختتام(&acc));
  }
#اگر_ختم
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
صحیح sqlite3VdbeMemHandleBom(Mem *pMem){
  صحیح rc = SQLITE_OK;
  u8 bom = 0;

  أكد( pMem->n>=0 );
  اگر( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    اگر( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    اگر( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  اگر( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    اگر( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_رکن;
      pMem->enc = bom;
    }
  }
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
صحیح sqlite3Utf8CharLen(ثابت حرف *z_اندر, صحیح nByte){
  صحیح r = 0;
  ثابت u8 *z = (ثابت u8*)z_اندر;
  ثابت u8 *z_رکن;
  اگر( nByte>=0 ){
    z_رکن = &z[nByte];
  }ورنہ{
    z_رکن = (ثابت u8*)(-1);
  }
  أكد( z<=z_رکن );
  جب( *z!=0 && z<z_رکن ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  واپس r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#اگر defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
صحیح sqlite3Utf8To8(غیرعلامتی حرف *z_اندر){
  غیرعلامتی حرف *z_برآمد = z_اندر;
  غیرعلامتی حرف *z_آغاز = z_اندر;
  u32 c;

  جب( z_اندر[0] && z_برآمد<=z_اندر ){
    c = sqlite3Utf8Read((ثابت u8**)&z_اندر);
    اگر( c!=0xfffd ){
      WRITE_UTF8(z_برآمد, c);
    }
  }
  *z_برآمد = 0;
  واپس (صحیح)(z_برآمد - z_آغاز);
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
حرف *sqlite3Utf16to8(sqlite3 *db, ثابت خالی *z, صحیح nByte, u8 enc){
  Mem m;
  memset(&m, 0, حجم(m));
  m.db = db;
  sqlite3_Vdbe_Mem_سیٹ_Str(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  اگر( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  أكد( (m.flags & MEM_رکن)!=0 || db->mallocFailed );
  أكد( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  أكد( m.z || db->mallocFailed );
  واپس m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
صحیح sqlite3Utf16ByteLen(ثابت خالی *z_اندر, صحیح nByte, صحیح nChar){
  صحیح c;
  غیرعلامتی حرف ثابت *z = z_اندر;
  غیرعلامتی حرف ثابت *z_انتہا = &z[nByte-1];
  صحیح n = 0;
  
  اگر( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  جب( n<nChar && z<=z_انتہا ){
    c = z[0];
    z += 2;
    اگر( c>=0xd8 && c<0xdc && z<=z_انتہا && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  واپس (صحیح)(z-(غیرعلامتی حرف ثابت *)z_اندر) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#اگر defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
خالی sqlite3UtfSelfTest(خالی){
  غیرعلامتی صحیح i, t;
  غیرعلامتی حرف zBuf[20];
  غیرعلامتی حرف *z;
  صحیح n;
  غیرعلامتی صحیح c;

  برائے(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (صحیح)(z-zBuf);
    أكد( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ثابت u8**)&z);
    t = i;
    اگر( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    أكد( c==t );
    أكد( (z-zBuf)==n );
  }
}
#اگر_ختم /* SQLITE_TEST */
#اگر_ختم /* SQLITE_OMIT_UTF16 */
