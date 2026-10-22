/*
** 2017-10-13
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
** This file contains code to implement an MD5 extension to TCL.
*/
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

/*
 * If compiled on a machine that doesn't have a 32-bit integer,
 * you just set "uint32" to the appropriate datatype for an
 * unsigned 32-bit integer.  For example:
 *
 *       cc -Duint32='unsigned long' md5.c
 *
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ uint32
#  define uint32 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ MD5Context {
  ಪೂರ್ಣಾಂಕ isInit;
  uint32 buf[4];
  uint32 ಬಿಟ್ಟುಗಳು[2];
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಒಳ[64];
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
ಸ್ಥಿರ ಶೂನ್ಯ byte_ಹಿಮ್ಮುಖ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ longs){
        uint32 t;
        ಮಾಡು {
                t = (uint32)((ಚಿಹ್ನೆರಹಿತ)buf[3]<<8 | buf[2]) << 16 |
                            ((ಚಿಹ್ನೆರಹಿತ)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } ಆಗ (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#ವ್ಯಾಖ್ಯಾನಿಸು F1(x, y, z) (z ^ (x & (y ^ z)))
#ವ್ಯಾಖ್ಯಾನಿಸು F2(x, y, z) F1(z, x, y)
#ವ್ಯಾಖ್ಯಾನಿಸು F3(x, y, z) (x ^ y ^ z)
#ವ್ಯಾಖ್ಯಾನಿಸು F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#ವ್ಯಾಖ್ಯಾನಿಸು MD5_ಹೆಜ್ಜೆ(f, w, x, y, z, ದತ್ತಾಂಶ, s) \
        ( w += f(x, y, z) + ದತ್ತಾಂಶ,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
ಸ್ಥಿರ ಶೂನ್ಯ MD5Transform(uint32 buf[4], ಸ್ಥಿರಾಂಕ uint32 ಒಳ[16]){
        ನೋಂದಣಿ uint32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5_ಹೆಜ್ಜೆ(F1, a, b, c, d, ಒಳ[ 0]+0xd76aa478,  7);
        MD5_ಹೆಜ್ಜೆ(F1, d, a, b, c, ಒಳ[ 1]+0xe8c7b756, 12);
        MD5_ಹೆಜ್ಜೆ(F1, c, d, a, b, ಒಳ[ 2]+0x242070db, 17);
        MD5_ಹೆಜ್ಜೆ(F1, b, c, d, a, ಒಳ[ 3]+0xc1bdceee, 22);
        MD5_ಹೆಜ್ಜೆ(F1, a, b, c, d, ಒಳ[ 4]+0xf57c0faf,  7);
        MD5_ಹೆಜ್ಜೆ(F1, d, a, b, c, ಒಳ[ 5]+0x4787c62a, 12);
        MD5_ಹೆಜ್ಜೆ(F1, c, d, a, b, ಒಳ[ 6]+0xa8304613, 17);
        MD5_ಹೆಜ್ಜೆ(F1, b, c, d, a, ಒಳ[ 7]+0xfd469501, 22);
        MD5_ಹೆಜ್ಜೆ(F1, a, b, c, d, ಒಳ[ 8]+0x698098d8,  7);
        MD5_ಹೆಜ್ಜೆ(F1, d, a, b, c, ಒಳ[ 9]+0x8b44f7af, 12);
        MD5_ಹೆಜ್ಜೆ(F1, c, d, a, b, ಒಳ[10]+0xffff5bb1, 17);
        MD5_ಹೆಜ್ಜೆ(F1, b, c, d, a, ಒಳ[11]+0x895cd7be, 22);
        MD5_ಹೆಜ್ಜೆ(F1, a, b, c, d, ಒಳ[12]+0x6b901122,  7);
        MD5_ಹೆಜ್ಜೆ(F1, d, a, b, c, ಒಳ[13]+0xfd987193, 12);
        MD5_ಹೆಜ್ಜೆ(F1, c, d, a, b, ಒಳ[14]+0xa679438e, 17);
        MD5_ಹೆಜ್ಜೆ(F1, b, c, d, a, ಒಳ[15]+0x49b40821, 22);

        MD5_ಹೆಜ್ಜೆ(F2, a, b, c, d, ಒಳ[ 1]+0xf61e2562,  5);
        MD5_ಹೆಜ್ಜೆ(F2, d, a, b, c, ಒಳ[ 6]+0xc040b340,  9);
        MD5_ಹೆಜ್ಜೆ(F2, c, d, a, b, ಒಳ[11]+0x265e5a51, 14);
        MD5_ಹೆಜ್ಜೆ(F2, b, c, d, a, ಒಳ[ 0]+0xe9b6c7aa, 20);
        MD5_ಹೆಜ್ಜೆ(F2, a, b, c, d, ಒಳ[ 5]+0xd62f105d,  5);
        MD5_ಹೆಜ್ಜೆ(F2, d, a, b, c, ಒಳ[10]+0x02441453,  9);
        MD5_ಹೆಜ್ಜೆ(F2, c, d, a, b, ಒಳ[15]+0xd8a1e681, 14);
        MD5_ಹೆಜ್ಜೆ(F2, b, c, d, a, ಒಳ[ 4]+0xe7d3fbc8, 20);
        MD5_ಹೆಜ್ಜೆ(F2, a, b, c, d, ಒಳ[ 9]+0x21e1cde6,  5);
        MD5_ಹೆಜ್ಜೆ(F2, d, a, b, c, ಒಳ[14]+0xc33707d6,  9);
        MD5_ಹೆಜ್ಜೆ(F2, c, d, a, b, ಒಳ[ 3]+0xf4d50d87, 14);
        MD5_ಹೆಜ್ಜೆ(F2, b, c, d, a, ಒಳ[ 8]+0x455a14ed, 20);
        MD5_ಹೆಜ್ಜೆ(F2, a, b, c, d, ಒಳ[13]+0xa9e3e905,  5);
        MD5_ಹೆಜ್ಜೆ(F2, d, a, b, c, ಒಳ[ 2]+0xfcefa3f8,  9);
        MD5_ಹೆಜ್ಜೆ(F2, c, d, a, b, ಒಳ[ 7]+0x676f02d9, 14);
        MD5_ಹೆಜ್ಜೆ(F2, b, c, d, a, ಒಳ[12]+0x8d2a4c8a, 20);

        MD5_ಹೆಜ್ಜೆ(F3, a, b, c, d, ಒಳ[ 5]+0xfffa3942,  4);
        MD5_ಹೆಜ್ಜೆ(F3, d, a, b, c, ಒಳ[ 8]+0x8771f681, 11);
        MD5_ಹೆಜ್ಜೆ(F3, c, d, a, b, ಒಳ[11]+0x6d9d6122, 16);
        MD5_ಹೆಜ್ಜೆ(F3, b, c, d, a, ಒಳ[14]+0xfde5380c, 23);
        MD5_ಹೆಜ್ಜೆ(F3, a, b, c, d, ಒಳ[ 1]+0xa4beea44,  4);
        MD5_ಹೆಜ್ಜೆ(F3, d, a, b, c, ಒಳ[ 4]+0x4bdecfa9, 11);
        MD5_ಹೆಜ್ಜೆ(F3, c, d, a, b, ಒಳ[ 7]+0xf6bb4b60, 16);
        MD5_ಹೆಜ್ಜೆ(F3, b, c, d, a, ಒಳ[10]+0xbebfbc70, 23);
        MD5_ಹೆಜ್ಜೆ(F3, a, b, c, d, ಒಳ[13]+0x289b7ec6,  4);
        MD5_ಹೆಜ್ಜೆ(F3, d, a, b, c, ಒಳ[ 0]+0xeaa127fa, 11);
        MD5_ಹೆಜ್ಜೆ(F3, c, d, a, b, ಒಳ[ 3]+0xd4ef3085, 16);
        MD5_ಹೆಜ್ಜೆ(F3, b, c, d, a, ಒಳ[ 6]+0x04881d05, 23);
        MD5_ಹೆಜ್ಜೆ(F3, a, b, c, d, ಒಳ[ 9]+0xd9d4d039,  4);
        MD5_ಹೆಜ್ಜೆ(F3, d, a, b, c, ಒಳ[12]+0xe6db99e5, 11);
        MD5_ಹೆಜ್ಜೆ(F3, c, d, a, b, ಒಳ[15]+0x1fa27cf8, 16);
        MD5_ಹೆಜ್ಜೆ(F3, b, c, d, a, ಒಳ[ 2]+0xc4ac5665, 23);

        MD5_ಹೆಜ್ಜೆ(F4, a, b, c, d, ಒಳ[ 0]+0xf4292244,  6);
        MD5_ಹೆಜ್ಜೆ(F4, d, a, b, c, ಒಳ[ 7]+0x432aff97, 10);
        MD5_ಹೆಜ್ಜೆ(F4, c, d, a, b, ಒಳ[14]+0xab9423a7, 15);
        MD5_ಹೆಜ್ಜೆ(F4, b, c, d, a, ಒಳ[ 5]+0xfc93a039, 21);
        MD5_ಹೆಜ್ಜೆ(F4, a, b, c, d, ಒಳ[12]+0x655b59c3,  6);
        MD5_ಹೆಜ್ಜೆ(F4, d, a, b, c, ಒಳ[ 3]+0x8f0ccc92, 10);
        MD5_ಹೆಜ್ಜೆ(F4, c, d, a, b, ಒಳ[10]+0xffeff47d, 15);
        MD5_ಹೆಜ್ಜೆ(F4, b, c, d, a, ಒಳ[ 1]+0x85845dd1, 21);
        MD5_ಹೆಜ್ಜೆ(F4, a, b, c, d, ಒಳ[ 8]+0x6fa87e4f,  6);
        MD5_ಹೆಜ್ಜೆ(F4, d, a, b, c, ಒಳ[15]+0xfe2ce6e0, 10);
        MD5_ಹೆಜ್ಜೆ(F4, c, d, a, b, ಒಳ[ 6]+0xa3014314, 15);
        MD5_ಹೆಜ್ಜೆ(F4, b, c, d, a, ಒಳ[13]+0x4e0811a1, 21);
        MD5_ಹೆಜ್ಜೆ(F4, a, b, c, d, ಒಳ[ 4]+0xf7537e82,  6);
        MD5_ಹೆಜ್ಜೆ(F4, d, a, b, c, ಒಳ[11]+0xbd3af235, 10);
        MD5_ಹೆಜ್ಜೆ(F4, c, d, a, b, ಒಳ[ 2]+0x2ad7d2bb, 15);
        MD5_ಹೆಜ್ಜೆ(F4, b, c, d, a, ಒಳ[ 9]+0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
ಸ್ಥಿರ ಶೂನ್ಯ MD5Init(MD5Context *ctx){
        ctx->isInit = 1;
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;
        ctx->ಬಿಟ್ಟುಗಳು[0] = 0;
        ctx->ಬಿಟ್ಟುಗಳು[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
ಸ್ಥಿರ
ಶೂನ್ಯ MD5Update(MD5Context *ctx, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ len){
        uint32 t;

        /* Update bitcount */

        t = ctx->ಬಿಟ್ಟುಗಳು[0];
        ಆದರೆ ((ctx->ಬಿಟ್ಟುಗಳು[0] = t + ((uint32)len << 3)) < t)
                ctx->ಬಿಟ್ಟುಗಳು[1]++; /* Carry from low to high */
        ctx->ಬಿಟ್ಟುಗಳು[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        ಆದರೆ ( t ) {
                ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ctx->ಒಳ + t;

                t = 64-t;
                ಆದರೆ (len < t) {
                        ಸ್ಮರಣೆ_ನಕಲು(p, buf, len);
                        ಹಿಂತಿರುಗಿ;
                }
                ಸ್ಮರಣೆ_ನಕಲು(p, buf, t);
                byte_ಹಿಮ್ಮುಖ(ctx->ಒಳ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->ಒಳ);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        ಆಗ (len >= 64) {
                ಸ್ಮರಣೆ_ನಕಲು(ctx->ಒಳ, buf, 64);
                byte_ಹಿಮ್ಮುಖ(ctx->ಒಳ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->ಒಳ);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        ಸ್ಮರಣೆ_ನಕಲು(ctx->ಒಳ, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
ಸ್ಥಿರ ಶೂನ್ಯ MD5Final(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ digest[16], MD5Context *ctx){
        ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p;

        /* Compute number of bytes mod 64 */
        ಎಣಿಕೆ_2 = (ctx->ಬಿಟ್ಟುಗಳು[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->ಒಳ + ಎಣಿಕೆ_2;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        ಎಣಿಕೆ_2 = 64 - 1 - ಎಣಿಕೆ_2;

        /* Pad out to 56 mod 64 */
        ಆದರೆ (ಎಣಿಕೆ_2 < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset(p, 0, ಎಣಿಕೆ_2);
                byte_ಹಿಮ್ಮುಖ(ctx->ಒಳ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->ಒಳ);

                /* Now fill the next block with 56 bytes */
                memset(ctx->ಒಳ, 0, 56);
        } ಇಲ್ಲದಿದ್ದರೆ {
                /* Pad block to 56 bytes */
                memset(p, 0, ಎಣಿಕೆ_2-8);
        }
        byte_ಹಿಮ್ಮುಖ(ctx->ಒಳ, 14);

        /* Append length in bits and transform */
        ಸ್ಮರಣೆ_ನಕಲು(ctx->ಒಳ + 14*4, ctx->ಬಿಟ್ಟುಗಳು, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->ಒಳ);
        byte_ಹಿಮ್ಮುಖ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)ctx->buf, 4);
        ಸ್ಮರಣೆ_ನಕಲು(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
ಸ್ಥಿರ ಶೂನ್ಯ MD5_Digest_ಗಮ್ಯ_Base16(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *digest, ಅಕ್ಷರ *zBuf){
  ಸ್ಥಿರ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ zEncode[] = "0123456789abcdef";
  ಪೂರ್ಣಾಂಕ i, j;

  ಗಾಗಿ(j=i=0; i<16; i++){
    ಪೂರ್ಣಾಂಕ a = digest[i];
    zBuf[j++] = zEncode[(a>>4)&0xf];
    zBuf[j++] = zEncode[a & 0xf];
  }
  zBuf[j] = 0;
}


/*
** Convert a 128-bit MD5 digest into sequences of eight 5-digit integers
** each representing 16 bits of the digest and separated from each
** other by a "-" character.
*/
ಸ್ಥಿರ ಶೂನ್ಯ MD5_Digest_ಗಮ್ಯ_Base10x8(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ digest[16], ಅಕ್ಷರ zDigest[50]){
  ಪೂರ್ಣಾಂಕ i, j;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x;
  ಗಾಗಿ(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    ಆದರೆ( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI md5_cmd(
  ಶೂನ್ಯ*cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ argc,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv
){
  MD5Context ctx;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ digest[16];
  ಅಕ್ಷರ zBuf[50];
  ಶೂನ್ಯ (*converter)(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*, ಅಕ್ಷರ*);

  ಆದರೆ( argc!=2 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)argv[1], (ಚಿಹ್ನೆರಹಿತ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[1]));
  MD5Final(digest, &ctx);
  converter = (ಶೂನ್ಯ(*)(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*,ಅಕ್ಷರ*))cd;
  converter(digest, zBuf);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, (ಅಕ್ಷರ*)0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI md5file_cmd(
  ಶೂನ್ಯ*cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ argc,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv
){
  FILE *ಒಳ;
  ಪೂರ್ಣಾಂಕ ofst;
  ಪೂರ್ಣಾಂಕ amt;
  MD5Context ctx;
  ಶೂನ್ಯ (*converter)(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*, ಅಕ್ಷರ*);
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ digest[16];
  ಅಕ್ಷರ zBuf[10240];

  ಆದರೆ( argc!=2 && argc!=4 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ofst = 0;
    amt = 2147483647;
  }
  ಒಳ = fopen(argv[1],"rb");
  ಆದರೆ( ಒಳ==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp,"unable to open file \"", argv[1],
         "\" for reading", (ಅಕ್ಷರ*)0);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  fseek(ಒಳ, ofst, SEEK_SET);
  MD5Init(&ctx);
  ಆಗ( amt>0 ){
    ಪೂರ್ಣಾಂಕ n;
    n = (ಪೂರ್ಣಾಂಕ)fread(zBuf, 1, ಗಾತ್ರ(zBuf)<=amt ? ಗಾತ್ರ(zBuf) : amt, ಒಳ);
    ಆದರೆ( n<=0 ) ನಿಲ್ಲಿಸು;
    MD5Update(&ctx, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)zBuf, (ಚಿಹ್ನೆರಹಿತ)n);
    amt -= n;
  }
  fclose(ಒಳ);
  MD5Final(digest, &ctx);
  converter = (ಶೂನ್ಯ(*)(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*,ಅಕ್ಷರ*))cd;
  converter(digest, zBuf);
  Tcl_Append_ಫಲಿತಾಂಶ(interp, zBuf, (ಅಕ್ಷರ*)0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_ಗಮ್ಯ_Base16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_ಗಮ್ಯ_Base10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_ಗಮ್ಯ_Base16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_ಗಮ್ಯ_Base10x8, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ md5step(sqlite3_context *context, ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv){
  MD5Context *p;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( argc<1 ) ಹಿಂತಿರುಗಿ;
  p = sqlite3_aggregate_context(context, ಗಾತ್ರ(*p));
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( !p->isInit ){
    MD5Init(p);
  }
  ಗಾಗಿ(i=0; i<argc; i++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ದತ್ತಾಂಶ = (ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[i]);
    ಆದರೆ( z_ದತ್ತಾಂಶ ){
      MD5Update(p, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)z_ದತ್ತಾಂಶ, (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ದತ್ತಾಂಶ));
    }
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ md5finalize(sqlite3_context *context){
  MD5Context *p;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ digest[16];
  ಅಕ್ಷರ zBuf[33];
  p = sqlite3_aggregate_context(context, ಗಾತ್ರ(*p));
  MD5Final(digest,p);
  MD5_Digest_ಗಮ್ಯ_Base16(digest, zBuf);
  sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(context, zBuf, -1, SQLITE_TRANSIENT);
}
ಪೂರ್ಣಾಂಕ Md5_ರಿಜಿಸ್ಟರ್(
  sqlite3 *db,
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ,
  ಸ್ಥಿರಾಂಕ sqlite3_api_routines *pThunk
){
  ಪೂರ್ಣಾಂಕ rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  ಹಿಂತಿರುಗಿ rc;
}
