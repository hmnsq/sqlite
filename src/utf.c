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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <assert.h>
#अन्तर्भाव "vdbeInt.h"

#यदि !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
ध्रुव पूर्णाङ्क sqlite3one = 1;
#यदि_अन्तः /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
स्थिर ध्रुव अचिह्नित वर्ण sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#परिभाषय WRITE_UTF8(z_बहिर्गमनम्, c) {                          \
  यदि( c<0x00080 ){                                     \
    *z_बहिर्गमनम्++ = (u8)(c&0xFF);                            \
  }                                                    \
  अन्यथा यदि( c<0x00800 ){                                \
    *z_बहिर्गमनम्++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *z_बहिर्गमनम्++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  अन्यथा यदि( c<0x10000 ){                                \
    *z_बहिर्गमनम्++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *z_बहिर्गमनम्++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_बहिर्गमनम्++ = 0x80 + (u8)(c & 0x3F);                   \
  }अन्यथा{                                               \
    *z_बहिर्गमनम्++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *z_बहिर्गमनम्++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *z_बहिर्गमनम्++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *z_बहिर्गमनम्++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#परिभाषय WRITE_UTF16LE(z_बहिर्गमनम्, c) {                                    \
  यदि( c<=0xFFFF ){                                                  \
    *z_बहिर्गमनम्++ = (u8)(c&0x00FF);                                       \
    *z_बहिर्गमनम्++ = (u8)((c>>8)&0x00FF);                                  \
  }अन्यथा{                                                            \
    *z_बहिर्गमनम्++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_बहिर्गमनम्++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_बहिर्गमनम्++ = (u8)(c&0x00FF);                                       \
    *z_बहिर्गमनम्++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#परिभाषय WRITE_UTF16BE(z_बहिर्गमनम्, c) {                                    \
  यदि( c<=0xFFFF ){                                                  \
    *z_बहिर्गमनम्++ = (u8)((c>>8)&0x00FF);                                  \
    *z_बहिर्गमनम्++ = (u8)(c&0x00FF);                                       \
  }अन्यथा{                                                            \
    *z_बहिर्गमनम्++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *z_बहिर्गमनम्++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *z_बहिर्गमनम्++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *z_बहिर्गमनम्++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

/*
** Write a single UTF8 character whose value is v into the
** buffer starting at zOut.  zOut must be sized to hold at
** least four bytes.  Return the number of bytes needed
** to encode the new character.
*/
पूर्णाङ्क sqlite3AppendOneUtf8Character(वर्ण *z_बहिर्गमनम्, u32 v){
  यदि( v<0x00080 ){
    z_बहिर्गमनम्[0] = (u8)(v & 0xff);
    प्रत्यागम 1;
  }
  यदि( v<0x00800 ){
    z_बहिर्गमनम्[0] = 0xc0 + (u8)((v>>6) & 0x1f);
    z_बहिर्गमनम्[1] = 0x80 + (u8)(v & 0x3f);
    प्रत्यागम 2;
  }
  यदि( v<0x10000 ){
    z_बहिर्गमनम्[0] = 0xe0 + (u8)((v>>12) & 0x0f);
    z_बहिर्गमनम्[1] = 0x80 + (u8)((v>>6) & 0x3f);
    z_बहिर्गमनम्[2] = 0x80 + (u8)(v & 0x3f);
    प्रत्यागम 3;
  }
  z_बहिर्गमनम्[0] = 0xf0 + (u8)((v>>18) & 0x07);
  z_बहिर्गमनम्[1] = 0x80 + (u8)((v>>12) & 0x3f);
  z_बहिर्गमनम्[2] = 0x80 + (u8)((v>>6) & 0x3f);
  z_बहिर्गमनम्[3] = 0x80 + (u8)(v & 0x3f);
  प्रत्यागम 4;
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
#परिभाषय READ_UTF8(z_अन्तर्, z_पदांशः, c)                           \
  c = *(z_अन्तर्++);                                            \
  यदि( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    यावत्( z_अन्तर्<z_पदांशः && (*z_अन्तर् & 0xc0)==0x80 ){             \
      c = (c<<6) + (0x3f & *(z_अन्तर्++));                      \
    }                                                      \
    यदि( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; } \
  }
u32 sqlite3Utf8Read(
  ध्रुव अचिह्नित वर्ण **pz    /* Pointer to string from which to read char */
){
  अचिह्नित पूर्णाङ्क c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  यदि( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    यावत्( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    यदि( c<0x80 || (c&0xFFFFF800)==0xD800 ){  c = 0xFFFD; }
  }
  प्रत्यागम c;
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
पूर्णाङ्क sqlite3Utf8ReadLimited(
  ध्रुव u8 *z,
  पूर्णाङ्क n,
  u32 *pi_बहिर्गमनम्
){
  u32 c;
  पूर्णाङ्क i = 1;
  प्रतिज्ञा( n>0 );
  c = z[0];
  यदि( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    यदि( n>4 ) n = 4;
    यावत्( i<n && (z[i] & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & z[i]);
      i++;
    }
  }
  *pi_बहिर्गमनम् = c;
  प्रत्यागम i;
}


/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_NOINLINE पूर्णाङ्क sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  sqlite3_int64 len;          /* Maximum length of output string in bytes */
  अचिह्नित वर्ण *z_बहिर्गमनम्;        /* Output buffer */
  अचिह्नित वर्ण *z_अन्तर्;         /* Input iterator */
  अचिह्नित वर्ण *z_पदांशः;       /* End of input */
  अचिह्नित वर्ण *z;           /* Output iterator */
  अचिह्नित पूर्णाङ्क c;

  प्रतिज्ञा( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  प्रतिज्ञा( pMem->flags&MEM_Str );
  प्रतिज्ञा( pMem->enc!=desiredEnc );
  प्रतिज्ञा( pMem->enc!=0 );
  प्रतिज्ञा( pMem->n>=0 );

#यदि defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    वर्ण zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "INPUT:  %s\n", sqlite3StrAccumFinish(&acc));
  }
#यदि_अन्तः

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  यदि( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 अस्थायी;
    पूर्णाङ्क rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    यदि( rc!=SQLITE_OK ){
      प्रतिज्ञा( rc==SQLITE_NOMEM );
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    z_अन्तर् = (u8*)pMem->z;
    z_पदांशः = &z_अन्तर्[pMem->n&~1];
    यावत्( z_अन्तर्<z_पदांशः ){
      अस्थायी = *z_अन्तर्;
      *z_अन्तर् = *(z_अन्तर्+1);
      z_अन्तर्++;
      *z_अन्तर्++ = अस्थायी;
    }
    pMem->enc = desiredEnc;
    गच्छ translate_बहिर्गमनम्;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  यदि( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  }अन्यथा{
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
  z_अन्तर् = (u8*)pMem->z;
  z_पदांशः = &z_अन्तर्[pMem->n];
  z_बहिर्गमनम् = sqlite3DbMallocRaw(pMem->db, len);
  यदि( !z_बहिर्गमनम् ){
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  z = z_बहिर्गमनम्;

  यदि( pMem->enc==SQLITE_UTF8 ){
    यदि( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      यावत्( z_अन्तर्<z_पदांशः ){
        READ_UTF8(z_अन्तर्, z_पदांशः, c);
        WRITE_UTF16LE(z, c);
      }
    }अन्यथा{
      प्रतिज्ञा( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      यावत्( z_अन्तर्<z_पदांशः ){
        READ_UTF8(z_अन्तर्, z_पदांशः, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (पूर्णाङ्क)(z - z_बहिर्गमनम्);
    *z++ = 0;
  }अन्यथा{
    प्रतिज्ञा( desiredEnc==SQLITE_UTF8 );
    यदि( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      यावत्( z_अन्तर्<z_पदांशः ){
        c = *(z_अन्तर्++);
        c += (*(z_अन्तर्++))<<8;
        यदि( c>=0xd800 && c<0xe000 ){
#यदि_परिभाषितम् SQLITE_REPLACE_INVALID_UTF
          यदि( c>=0xdc00 || z_अन्तर्>=z_पदांशः ){
            c = 0xfffd;
          }अन्यथा{
            पूर्णाङ्क c2 = *(z_अन्तर्++);
            c2 += (*(z_अन्तर्++))<<8;
            यदि( c2<0xdc00 || c2>=0xe000 ){
              z_अन्तर् -= 2;
              c = 0xfffd;
            }अन्यथा{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#अन्यथा
          यदि( z_अन्तर्<z_पदांशः ){
            पूर्णाङ्क c2 = (*z_अन्तर्++);
            c2 += ((*z_अन्तर्++)<<8);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#यदि_अन्तः
        }
        WRITE_UTF8(z, c);
      }
    }अन्यथा{
      /* UTF-16 Big-endian -> UTF-8 */
      यावत्( z_अन्तर्<z_पदांशः ){
        c = (*(z_अन्तर्++))<<8;
        c += *(z_अन्तर्++);
        यदि( c>=0xd800 && c<0xe000 ){
#यदि_परिभाषितम् SQLITE_REPLACE_INVALID_UTF
          यदि( c>=0xdc00 || z_अन्तर्>=z_पदांशः ){
            c = 0xfffd;
          }अन्यथा{
            पूर्णाङ्क c2 = (*(z_अन्तर्++))<<8;
            c2 += *(z_अन्तर्++);
            यदि( c2<0xdc00 || c2>=0xe000 ){
              z_अन्तर् -= 2;
              c = 0xfffd;
            }अन्यथा{
              c = ((c&0x3ff)<<10) + (c2&0x3ff) + 0x10000;
            }
          }
#अन्यथा
          यदि( z_अन्तर्<z_पदांशः ){
            पूर्णाङ्क c2 = ((*z_अन्तर्++)<<8);
            c2 += (*z_अन्तर्++);
            c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);
          }
#यदि_अन्तः
        }
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (पूर्णाङ्क)(z - z_बहिर्गमनम्);
  }
  *z = 0;
  प्रतिज्ञा( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = MEM_Str|MEM_पदांशः|(pMem->flags&(MEM_Aff_आवरणम्|MEM_Subtype));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (वर्ण*)z_बहिर्गमनम्;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3_Db_Malloc_आकार(pMem->db, pMem->z);

translate_बहिर्गमनम्:
#यदि defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    StrAccum acc;
    वर्ण zBuf[1000];
    sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);  
    sqlite3VdbeMemPrettyPrint(pMem, &acc);
    fprintf(stderr, "OUTPUT: %s\n", sqlite3StrAccumFinish(&acc));
  }
#यदि_अन्तः
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */

#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
पूर्णाङ्क sqlite3VdbeMemHandleBom(Mem *pMem){
  पूर्णाङ्क rc = SQLITE_OK;
  u8 bom = 0;

  प्रतिज्ञा( pMem->n>=0 );
  यदि( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    यदि( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    यदि( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  यदि( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    यदि( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_पदांशः;
      pMem->enc = bom;
    }
  }
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
पूर्णाङ्क sqlite3Utf8CharLen(ध्रुव वर्ण *z_अन्तर्, पूर्णाङ्क nByte){
  पूर्णाङ्क r = 0;
  ध्रुव u8 *z = (ध्रुव u8*)z_अन्तर्;
  ध्रुव u8 *z_पदांशः;
  यदि( nByte>=0 ){
    z_पदांशः = &z[nByte];
  }अन्यथा{
    z_पदांशः = (ध्रुव u8*)(-1);
  }
  प्रतिज्ञा( z<=z_पदांशः );
  यावत्( *z!=0 && z<z_पदांशः ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  प्रत्यागम r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#यदि defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
पूर्णाङ्क sqlite3Utf8To8(अचिह्नित वर्ण *z_अन्तर्){
  अचिह्नित वर्ण *z_बहिर्गमनम् = z_अन्तर्;
  अचिह्नित वर्ण *zStart = z_अन्तर्;
  u32 c;

  यावत्( z_अन्तर्[0] && z_बहिर्गमनम्<=z_अन्तर् ){
    c = sqlite3Utf8Read((ध्रुव u8**)&z_अन्तर्);
    यदि( c!=0xfffd ){
      WRITE_UTF8(z_बहिर्गमनम्, c);
    }
  }
  *z_बहिर्गमनम् = 0;
  प्रत्यागम (पूर्णाङ्क)(z_बहिर्गमनम् - zStart);
}
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
वर्ण *sqlite3Utf16to8(sqlite3 *db, ध्रुव शून्य *z, पूर्णाङ्क nByte, u8 enc){
  Mem m;
  स्मृतिं_पूरय(&m, 0, परिमाण(m));
  m.db = db;
  sqlite3_Vdbe_Mem_समुच्चयः_Str(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  यदि( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  प्रतिज्ञा( (m.flags & MEM_पदांशः)!=0 || db->mallocFailed );
  प्रतिज्ञा( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  प्रतिज्ञा( m.z || db->mallocFailed );
  प्रत्यागम m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nByte bytes long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.  Surrogate pairs count as a single
** character.
*/
पूर्णाङ्क sqlite3Utf16ByteLen(ध्रुव शून्य *z_अन्तर्, पूर्णाङ्क nByte, पूर्णाङ्क nChar){
  पूर्णाङ्क c;
  अचिह्नित वर्ण ध्रुव *z = z_अन्तर्;
  अचिह्नित वर्ण ध्रुव *z_अन्तः = &z[nByte-1];
  पूर्णाङ्क n = 0;
  
  यदि( SQLITE_UTF16NATIVE==SQLITE_UTF16LE ) z++;
  यावत्( n<nChar && z<=z_अन्तः ){
    c = z[0];
    z += 2;
    यदि( c>=0xd8 && c<0xdc && z<=z_अन्तः && z[0]>=0xdc && z[0]<0xe0 ) z += 2;
    n++;
  }
  प्रत्यागम (पूर्णाङ्क)(z-(अचिह्नित वर्ण ध्रुव *)z_अन्तर्) 
              - (SQLITE_UTF16NATIVE==SQLITE_UTF16LE);
}

#यदि defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
शून्य sqlite3UtfSelfTest(शून्य){
  अचिह्नित पूर्णाङ्क i, t;
  अचिह्नित वर्ण zBuf[20];
  अचिह्नित वर्ण *z;
  पूर्णाङ्क n;
  अचिह्नित पूर्णाङ्क c;

  प्रति(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (पूर्णाङ्क)(z-zBuf);
    प्रतिज्ञा( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((ध्रुव u8**)&z);
    t = i;
    यदि( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    प्रतिज्ञा( c==t );
    प्रतिज्ञा( (z-zBuf)==n );
  }
}
#यदि_अन्तः /* SQLITE_TEST */
#यदि_अन्तः /* SQLITE_OMIT_UTF16 */
