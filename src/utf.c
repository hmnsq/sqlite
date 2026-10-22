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
#ضمن "sqliteInt.h"
#ضمن <assert.h>
#ضمن "vdbeInt.h"

#إذا !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ثابت صحيح sqlite3one = 1;
#پایان_اگر /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
ساكن ثابت بیعلامت حرف sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#تعریف_کن WRITE_UTF8(z_بیرون, c) {                          \
  إذا( c<0x00080 ){                                     \
    *z_بیرون++ = (u8)(c&0xFF);                            \
  }                                                    \
  وإلا إذا( c<0x00800 ){                                \
    *z_بیرون++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *z_بیرون++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  وإلا إذا( c<0x10000 ){                                \
    *z_بیرون++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *z_بیرون++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_بیرون++ = 0x80 + (u8)(c & 0x3F);                   \
  }وإلا{                                               \
    *z_بیرون++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *z_بیرون++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *z_بیرون++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_بیرون++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#تعریف_کن WRITE_UTF16LE(z_بیرون, c) {                                    \
  إذا( c<=0xFFFF ){                                                  \
    *z_بیرون++ = (u8)(c&0x00FF);                                       \
    *z_بیرون++ = (u8)((c>>8)&0x00FF);                                  \
  }وإلا{                                                            \
    *z_بیرون++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_بیرون++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_بیرون++ = (u8)(c&0x00FF);                                       \
    *z_بیرون++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#تعریف_کن WRITE_UTF16BE(z_بیرون, c) {                                    \
  إذا( c<=0xFFFF ){                                                  \
    *z_بیرون++ = (u8)((c>>8)&0x00FF);                                  \
    *z_بیرون++ = (u8)(c&0x00FF);                                       \
  }وإلا{                                                            \
    *z_بیرون++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_بیرون++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_بیرون++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *z_بیرون++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
صحيح sqlite3AppendOneUtf8Character(حرف *z_بیرون, u32 v){
  إذا( v<0x00080 ){
    z_بیرون[0] = (u8)(v & 0xff);
    أرجع 1;
  }
  إذا( v<0x00800 ){
    z_بیرون[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    z_بیرون[1] = 0x80 + (u8)(v & 0x3f);
    أرجع 2;
  }
  إذا( v<0x10000 ){
    z_بیرون[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    z_بیرون[1] = 0x80 + (u8)((v>>6) & 0x3f);
    z_بیرون[2] = 0x80 + (u8)(v & 0x3f);
    أرجع 3;
  }
  z_بیرون[0] = 0xf0 + (u8)((v>>18) & 0x07);
  z_بیرون[1] = 0x80 + (u8)((v>>12) & 0x3f);
  z_بیرون[2] = 0x80 + (u8)((v>>6) & 0x3f);
  z_بیرون[3] = 0x80 + (u8)(v & 0x3f);
  أرجع 4;
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
#تعریف_کن READ_UTF8(z_درون, z_جمله, c)                           \
  c = *(z_درون++);                                            \
  إذا( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    طالما( z_درون<z_جمله && (*z_درون & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(z_درون++));                      \
    }                                                      \
    إذا( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ثابت بیعلامت حرف **pz    /* Pointer to string from which to read char */
){
  بیعلامت صحيح c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  إذا( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    طالما( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    إذا( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  أرجع c;
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
صحيح sqlite3Utf8ReadLimited(
  ثابت u8 *z,
  صحيح n,
  u32 *pi_بیرون
){
  u32 c;
  صحيح i = 1;
  أكد( n>0 );
  c = z[0];
  إذا( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    إذا( n>4 ) n = 4;
    طالما( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *pi_بیرون = c;
  أرجع i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#اگر_تعریف_نشده SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE صحيح sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  بیعلامت حرف *z_بیرون;        /* Output buffer */
  بیعلامت حرف *z_درون;         /* Input iterator */
  بیعلامت حرف *z_جمله;       /* End of input */
  بیعلامت حرف *z;           /* Output iterator */
  بیعلامت صحيح c;

  أكد( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  أكد( pMem->flags&MEM_Str );
  أكد( pMem->enc!=desiredEnc );
  أكد( pMem->enc!=0 );
  أكد( pMem->n>=0 );

#إذا defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    حرف zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3_Str_Accum_پایان(&acc));
  }
#پایان_اگر

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  إذا( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 موقت;
    صحيح rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    إذا( rc!=SQLITE_OK ){
      أكد( rc==SQLITE_NOMEM );
      أرجع SQLITE_NOMEM_BKPT;
    }
    z_درون = (u8*)pMem->z;
    z_جمله = &z_درون[pMem->n&~1];
    طالما( z_درون<z_جمله ){
      موقت = *z_درون;
      *z_درون = *(z_درون+1);
      z_درون++;
      *z_درون++ = موقت;
    }
    pMem->enc = desiredEnc;
    اذهب translate_بیرون;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  إذا( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }وإلا{
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
  z_درون = (u8*)pMem->z;
  z_جمله = &z_درون[pMem->n];
  z_بیرون = sqlite3DbMallocRaw(pMem->db, len);
  إذا( !z_بیرون ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  z = z_بیرون;

  إذا( pMem->enc==SQLITE_UTF8 ){
    إذا( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      طالما( z_درون<z_جمله ){
        READ_UTF8(z_درون, z_جمله, c);
        WRITE_UTF16LE(z, c);
      }
    }وإلا{
      أكد( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      طالما( z_درون<z_جمله ){
        READ_UTF8(z_درون, z_جمله, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (صحيح)(z - z_بیرون);
    *z++ = 0;
  }وإلا{
    أكد( desiredEnc==SQLITE_UTF8 );
    إذا( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      طالما( z_درون<z_جمله ){
        c = *(z_درون++);
        c += (*(z_درون++))<<8;
        إذا( c>=0xd800 && c<0xe000 ){
#اگر_تعریف_شده SQLITE_REPLACE_INVALID_UTF
          إذا( c>=0xdc00 || z_درون>=z_جمله ){
            c = 0xfffd;
          }وإلا{
            صحيح c2 = *(z_درون++);
            c2 += (*(z_درون++))<<8;
            إذا( c2<0xdc00 || c2>=0xe000 ){
              z_درون -= 2;
              c = 0xfffd;
            }وإلا{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#وإلا
          إذا( z_درون<z_جمله ){
            صحيح c2 = (*z_درون++);
            c2 += ((*z_درون++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#پایان_اگر
        }
        WRITE_UTF8(z, c);
      }
    }وإلا{
      /* UTF-16 Big-endian -> UTF-8 */
      طالما( z_درون<z_جمله ){
        c = (*(z_درون++))<<8;
        c += *(z_درون++);
        إذا( c>=0xd800 && c<0xe000 ){
#اگر_تعریف_شده SQLITE_REPLACE_INVALID_UTF
          إذا( c>=0xdc00 || z_درون>=z_جمله ){
            c = 0xfffd;
          }وإلا{
            صحيح c2 = (*(z_درون++))<<8;
            c2 += *(z_درون++);
            إذا( c2<0xdc00 || c2>=0xe000 ){
              z_درون -= 2;
              c = 0xfffd;
            }وإلا{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#وإلا
          إذا( z_درون<z_جمله ){
            صحيح c2 = ((*z_درون++)<<8);
            c2 += (*z_درون++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#پایان_اگر
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (صحيح)(z - z_بیرون);
  }
  *z = 0;
  أكد( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_جمله|(pMem->flags&(MEM_Aff_ماسک|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (حرف*)z_بیرون;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_اندازه(pMem->db, pMem->z);

translate_بیرون:
#إذا defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    حرف zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3_Str_Accum_پایان(&acc));
  }
#پایان_اگر
  أرجع SQLITE_OK;
}
#پایان_اگر /* SQLITE_OMIT_UTF16 */

#اگر_تعریف_نشده SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
صحيح sqlite3VdbeMemHandleBom(Mem *pMem){
  صحيح rc = SQLITE_OK;
  u8 bom = 0;

  أكد( pMem->n>=0 );
  إذا( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    إذا( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    إذا( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  إذا( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    إذا( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_جمله;
      pMem->enc = bom;
    }
  }
  أرجع rc;
}
#پایان_اگر /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
صحيح sqlite3Utf8CharLen(ثابت حرف *z_درون, صحيح nByte){
  صحيح r = 0;
  ثابت u8 *z = (ثابت u8*)z_درون;
  ثابت u8 *z_جمله;
  إذا( nByte>=0 ){
    z_جمله = &z[nByte];
  }وإلا{
    z_جمله = (ثابت u8*)(-1);
  }
  أكد( z<=z_جمله );
  طالما( *z!=0 && z<z_جمله ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  أرجع r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#إذا defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
صحيح sqlite3Utf8To8(بیعلامت حرف *z_درون){
  بیعلامت حرف *z_بیرون = z_درون;
  بیعلامت حرف *z_شروع = z_درون;
  u32 c;

  طالما( z_درون[0] && z_بیرون<=z_درون ){
    c = sqlite3Utf8Read((ثابت u8**)&z_درون);
    إذا( c!=0xfffd ){
      WRITE_UTF8(z_بیرون, c);
    }
  }
  *z_بیرون = 0;
  أرجع (صحيح)(z_بیرون - z_شروع);
}
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
حرف *sqlite3Utf16to8(sqlite3 *db, ثابت فراغ *z, صحيح nByte, u8 enc){
  Mem m;
  memset(&m, 0, حجم(m));
  m.db = db;
  sqlite3_Vdbe_Mem_مجموعه_Str(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  إذا( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  أكد( (m.flags & MEM_جمله)!=0 || db->mallocFailed );
  أكد( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  أكد( m.z || db->mallocFailed );
  أرجع m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
صحيح sqlite3Utf16ByteLen(ثابت فراغ *z_درون, صحيح nByte, صحيح nChar){
  صحيح c;
  بیعلامت حرف ثابت *z = z_درون;
  بیعلامت حرف ثابت *z_انتها = &z[nByte-1];
  صحيح n = 0;
  
  إذا( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  طالما( n<nChar && z<=z_انتها ){
    c = z[0];
    z += 2;
    إذا( c>=0xd8 && c<0xdc && z<=z_انتها && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  أرجع (صحيح)(z-(بیعلامت حرف ثابت *)z_درون) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#إذا defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
فراغ sqlite3UtfSelfTest(فراغ){
  بیعلامت صحيح i, t;
  بیعلامت حرف zBuf[20];
  بیعلامت حرف *z;
  صحيح n;
  بیعلامت صحيح c;

  لكل(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (صحيح)(z-zBuf);
    أكد( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ثابت u8**)&z);
    t = i;
    إذا( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    أكد( c==t );
    أكد( (z-zBuf)==n );
  }
}
#پایان_اگر /* SQLITE_TEST */
#پایان_اگر /* SQLITE_OMIT_UTF16 */
