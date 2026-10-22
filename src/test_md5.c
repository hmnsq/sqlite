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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ uint32
#  define uint32 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
#ਜੇ_ਅੰਤ

ਬਣਤਰ MD5Context {
  ਪੂਰਨ isInit;
  uint32 buf[4];
  uint32 ਬਿੱਟ[2];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਵਿੱਚ[64];
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
ਸਥਿਰ ਖਾਲੀ byte_ਉਲਟ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ longs){
        uint32 t;
        ਕਰੋ {
                t = (uint32)((ਬਿਨਾਨਿਸ਼ਾਨੀ)buf[3]<<8 | buf[2]) << 16 |
                            ((ਬਿਨਾਨਿਸ਼ਾਨੀ)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } ਜਦੋਂ (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F1(x, y, z) (z ^ (x & (y ^ z)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2(x, y, z) F1(z, x, y)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F3(x, y, z) (x ^ y ^ z)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MD5_ਕਦਮ(f, w, x, y, z, ਅੰਕੜੇ, s) \
        ( w += f(x, y, z) + ਅੰਕੜੇ,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
ਸਥਿਰ ਖਾਲੀ MD5Transform(uint32 buf[4], ਅਚਲ uint32 ਵਿੱਚ[16]){
        ਰਜਿਸਟਰ uint32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5_ਕਦਮ(F1, a, b, c, d, ਵਿੱਚ[ 0]+0xd76aa478,  7);
        MD5_ਕਦਮ(F1, d, a, b, c, ਵਿੱਚ[ 1]+0xe8c7b756, 12);
        MD5_ਕਦਮ(F1, c, d, a, b, ਵਿੱਚ[ 2]+0x242070db, 17);
        MD5_ਕਦਮ(F1, b, c, d, a, ਵਿੱਚ[ 3]+0xc1bdceee, 22);
        MD5_ਕਦਮ(F1, a, b, c, d, ਵਿੱਚ[ 4]+0xf57c0faf,  7);
        MD5_ਕਦਮ(F1, d, a, b, c, ਵਿੱਚ[ 5]+0x4787c62a, 12);
        MD5_ਕਦਮ(F1, c, d, a, b, ਵਿੱਚ[ 6]+0xa8304613, 17);
        MD5_ਕਦਮ(F1, b, c, d, a, ਵਿੱਚ[ 7]+0xfd469501, 22);
        MD5_ਕਦਮ(F1, a, b, c, d, ਵਿੱਚ[ 8]+0x698098d8,  7);
        MD5_ਕਦਮ(F1, d, a, b, c, ਵਿੱਚ[ 9]+0x8b44f7af, 12);
        MD5_ਕਦਮ(F1, c, d, a, b, ਵਿੱਚ[10]+0xffff5bb1, 17);
        MD5_ਕਦਮ(F1, b, c, d, a, ਵਿੱਚ[11]+0x895cd7be, 22);
        MD5_ਕਦਮ(F1, a, b, c, d, ਵਿੱਚ[12]+0x6b901122,  7);
        MD5_ਕਦਮ(F1, d, a, b, c, ਵਿੱਚ[13]+0xfd987193, 12);
        MD5_ਕਦਮ(F1, c, d, a, b, ਵਿੱਚ[14]+0xa679438e, 17);
        MD5_ਕਦਮ(F1, b, c, d, a, ਵਿੱਚ[15]+0x49b40821, 22);

        MD5_ਕਦਮ(F2, a, b, c, d, ਵਿੱਚ[ 1]+0xf61e2562,  5);
        MD5_ਕਦਮ(F2, d, a, b, c, ਵਿੱਚ[ 6]+0xc040b340,  9);
        MD5_ਕਦਮ(F2, c, d, a, b, ਵਿੱਚ[11]+0x265e5a51, 14);
        MD5_ਕਦਮ(F2, b, c, d, a, ਵਿੱਚ[ 0]+0xe9b6c7aa, 20);
        MD5_ਕਦਮ(F2, a, b, c, d, ਵਿੱਚ[ 5]+0xd62f105d,  5);
        MD5_ਕਦਮ(F2, d, a, b, c, ਵਿੱਚ[10]+0x02441453,  9);
        MD5_ਕਦਮ(F2, c, d, a, b, ਵਿੱਚ[15]+0xd8a1e681, 14);
        MD5_ਕਦਮ(F2, b, c, d, a, ਵਿੱਚ[ 4]+0xe7d3fbc8, 20);
        MD5_ਕਦਮ(F2, a, b, c, d, ਵਿੱਚ[ 9]+0x21e1cde6,  5);
        MD5_ਕਦਮ(F2, d, a, b, c, ਵਿੱਚ[14]+0xc33707d6,  9);
        MD5_ਕਦਮ(F2, c, d, a, b, ਵਿੱਚ[ 3]+0xf4d50d87, 14);
        MD5_ਕਦਮ(F2, b, c, d, a, ਵਿੱਚ[ 8]+0x455a14ed, 20);
        MD5_ਕਦਮ(F2, a, b, c, d, ਵਿੱਚ[13]+0xa9e3e905,  5);
        MD5_ਕਦਮ(F2, d, a, b, c, ਵਿੱਚ[ 2]+0xfcefa3f8,  9);
        MD5_ਕਦਮ(F2, c, d, a, b, ਵਿੱਚ[ 7]+0x676f02d9, 14);
        MD5_ਕਦਮ(F2, b, c, d, a, ਵਿੱਚ[12]+0x8d2a4c8a, 20);

        MD5_ਕਦਮ(F3, a, b, c, d, ਵਿੱਚ[ 5]+0xfffa3942,  4);
        MD5_ਕਦਮ(F3, d, a, b, c, ਵਿੱਚ[ 8]+0x8771f681, 11);
        MD5_ਕਦਮ(F3, c, d, a, b, ਵਿੱਚ[11]+0x6d9d6122, 16);
        MD5_ਕਦਮ(F3, b, c, d, a, ਵਿੱਚ[14]+0xfde5380c, 23);
        MD5_ਕਦਮ(F3, a, b, c, d, ਵਿੱਚ[ 1]+0xa4beea44,  4);
        MD5_ਕਦਮ(F3, d, a, b, c, ਵਿੱਚ[ 4]+0x4bdecfa9, 11);
        MD5_ਕਦਮ(F3, c, d, a, b, ਵਿੱਚ[ 7]+0xf6bb4b60, 16);
        MD5_ਕਦਮ(F3, b, c, d, a, ਵਿੱਚ[10]+0xbebfbc70, 23);
        MD5_ਕਦਮ(F3, a, b, c, d, ਵਿੱਚ[13]+0x289b7ec6,  4);
        MD5_ਕਦਮ(F3, d, a, b, c, ਵਿੱਚ[ 0]+0xeaa127fa, 11);
        MD5_ਕਦਮ(F3, c, d, a, b, ਵਿੱਚ[ 3]+0xd4ef3085, 16);
        MD5_ਕਦਮ(F3, b, c, d, a, ਵਿੱਚ[ 6]+0x04881d05, 23);
        MD5_ਕਦਮ(F3, a, b, c, d, ਵਿੱਚ[ 9]+0xd9d4d039,  4);
        MD5_ਕਦਮ(F3, d, a, b, c, ਵਿੱਚ[12]+0xe6db99e5, 11);
        MD5_ਕਦਮ(F3, c, d, a, b, ਵਿੱਚ[15]+0x1fa27cf8, 16);
        MD5_ਕਦਮ(F3, b, c, d, a, ਵਿੱਚ[ 2]+0xc4ac5665, 23);

        MD5_ਕਦਮ(F4, a, b, c, d, ਵਿੱਚ[ 0]+0xf4292244,  6);
        MD5_ਕਦਮ(F4, d, a, b, c, ਵਿੱਚ[ 7]+0x432aff97, 10);
        MD5_ਕਦਮ(F4, c, d, a, b, ਵਿੱਚ[14]+0xab9423a7, 15);
        MD5_ਕਦਮ(F4, b, c, d, a, ਵਿੱਚ[ 5]+0xfc93a039, 21);
        MD5_ਕਦਮ(F4, a, b, c, d, ਵਿੱਚ[12]+0x655b59c3,  6);
        MD5_ਕਦਮ(F4, d, a, b, c, ਵਿੱਚ[ 3]+0x8f0ccc92, 10);
        MD5_ਕਦਮ(F4, c, d, a, b, ਵਿੱਚ[10]+0xffeff47d, 15);
        MD5_ਕਦਮ(F4, b, c, d, a, ਵਿੱਚ[ 1]+0x85845dd1, 21);
        MD5_ਕਦਮ(F4, a, b, c, d, ਵਿੱਚ[ 8]+0x6fa87e4f,  6);
        MD5_ਕਦਮ(F4, d, a, b, c, ਵਿੱਚ[15]+0xfe2ce6e0, 10);
        MD5_ਕਦਮ(F4, c, d, a, b, ਵਿੱਚ[ 6]+0xa3014314, 15);
        MD5_ਕਦਮ(F4, b, c, d, a, ਵਿੱਚ[13]+0x4e0811a1, 21);
        MD5_ਕਦਮ(F4, a, b, c, d, ਵਿੱਚ[ 4]+0xf7537e82,  6);
        MD5_ਕਦਮ(F4, d, a, b, c, ਵਿੱਚ[11]+0xbd3af235, 10);
        MD5_ਕਦਮ(F4, c, d, a, b, ਵਿੱਚ[ 2]+0x2ad7d2bb, 15);
        MD5_ਕਦਮ(F4, b, c, d, a, ਵਿੱਚ[ 9]+0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
ਸਥਿਰ ਖਾਲੀ MD5Init(MD5Context *ctx){
        ctx->isInit = 1;
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;
        ctx->ਬਿੱਟ[0] = 0;
        ctx->ਬਿੱਟ[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
ਸਥਿਰ
ਖਾਲੀ MD5Update(MD5Context *ctx, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ len){
        uint32 t;

        /* Update bitcount */

        t = ctx->ਬਿੱਟ[0];
        ਜੇ ((ctx->ਬਿੱਟ[0] = t + ((uint32)len << 3)) < t)
                ctx->ਬਿੱਟ[1]++; /* Carry from low to high */
        ctx->ਬਿੱਟ[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        ਜੇ ( t ) {
                ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ctx->ਵਿੱਚ + t;

                t = 64-t;
                ਜੇ (len < t) {
                        ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, buf, len);
                        ਵਾਪਸ;
                }
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, buf, t);
                byte_ਉਲਟ(ctx->ਵਿੱਚ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->ਵਿੱਚ);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        ਜਦੋਂ (len >= 64) {
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(ctx->ਵਿੱਚ, buf, 64);
                byte_ਉਲਟ(ctx->ਵਿੱਚ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->ਵਿੱਚ);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ctx->ਵਿੱਚ, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
ਸਥਿਰ ਖਾਲੀ MD5Final(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ digest[16], MD5Context *ctx){
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p;

        /* Compute number of bytes mod 64 */
        ਗਿਣਤੀ_2 = (ctx->ਬਿੱਟ[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->ਵਿੱਚ + ਗਿਣਤੀ_2;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        ਗਿਣਤੀ_2 = 64 - 1 - ਗਿਣਤੀ_2;

        /* Pad out to 56 mod 64 */
        ਜੇ (ਗਿਣਤੀ_2 < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset(p, 0, ਗਿਣਤੀ_2);
                byte_ਉਲਟ(ctx->ਵਿੱਚ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->ਵਿੱਚ);

                /* Now fill the next block with 56 bytes */
                memset(ctx->ਵਿੱਚ, 0, 56);
        } ਨਹੀਂਤਾਂ {
                /* Pad block to 56 bytes */
                memset(p, 0, ਗਿਣਤੀ_2-8);
        }
        byte_ਉਲਟ(ctx->ਵਿੱਚ, 14);

        /* Append length in bits and transform */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ctx->ਵਿੱਚ + 14*4, ctx->ਬਿੱਟ, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->ਵਿੱਚ);
        byte_ਉਲਟ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ctx->buf, 4);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
ਸਥਿਰ ਖਾਲੀ MD5_Digest_ਮੰਜ਼ਿਲ_Base16(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *digest, ਅੱਖਰ *zBuf){
  ਸਥਿਰ ਅੱਖਰ ਅਚਲ zEncode[] = "0123456789abcdef";
  ਪੂਰਨ i, j;

  ਲਈ(j=i=0; i<16; i++){
    ਪੂਰਨ a = digest[i];
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
ਸਥਿਰ ਖਾਲੀ MD5_Digest_ਮੰਜ਼ਿਲ_Base10x8(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ digest[16], ਅੱਖਰ zDigest[50]){
  ਪੂਰਨ i, j;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x;
  ਲਈ(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    ਜੇ( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI md5_cmd(
  ਖਾਲੀ*cd,
  Tcl_Interp *interp,
  ਪੂਰਨ argc,
  ਅਚਲ ਅੱਖਰ **argv
){
  MD5Context ctx;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ digest[16];
  ਅੱਖਰ zBuf[50];
  ਖਾਲੀ (*converter)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*, ਅੱਖਰ*);

  ਜੇ( argc!=2 ){
    Tcl_Append_ਨਤੀਜਾ(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)argv[1], (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਲੜੀ_ਲੰਬਾਈ(argv[1]));
  MD5Final(digest, &ctx);
  converter = (ਖਾਲੀ(*)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*,ਅੱਖਰ*))cd;
  converter(digest, zBuf);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, (ਅੱਖਰ*)0);
  ਵਾਪਸ TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI md5file_cmd(
  ਖਾਲੀ*cd,
  Tcl_Interp *interp,
  ਪੂਰਨ argc,
  ਅਚਲ ਅੱਖਰ **argv
){
  FILE *ਵਿੱਚ;
  ਪੂਰਨ ofst;
  ਪੂਰਨ amt;
  MD5Context ctx;
  ਖਾਲੀ (*converter)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*, ਅੱਖਰ*);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ digest[16];
  ਅੱਖਰ zBuf[10240];

  ਜੇ( argc!=2 && argc!=4 ){
    Tcl_Append_ਨਤੀਜਾ(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }ਨਹੀਂਤਾਂ{
    ofst = 0;
    amt = 2147483647;
  }
  ਵਿੱਚ = fopen(argv[1],"rb");
  ਜੇ( ਵਿੱਚ==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp,"unable to open file \"", argv[1],
         "\" for reading", (ਅੱਖਰ*)0);
    ਵਾਪਸ TCL_ERROR;
  }
  fseek(ਵਿੱਚ, ofst, SEEK_SET);
  MD5Init(&ctx);
  ਜਦੋਂ( amt>0 ){
    ਪੂਰਨ n;
    n = (ਪੂਰਨ)fread(zBuf, 1, ਆਕਾਰ(zBuf)<=amt ? ਆਕਾਰ(zBuf) : amt, ਵਿੱਚ);
    ਜੇ( n<=0 ) ਰੋਕੋ;
    MD5Update(&ctx, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)zBuf, (ਬਿਨਾਨਿਸ਼ਾਨੀ)n);
    amt -= n;
  }
  fclose(ਵਿੱਚ);
  MD5Final(digest, &ctx);
  converter = (ਖਾਲੀ(*)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*,ਅੱਖਰ*))cd;
  converter(digest, zBuf);
  Tcl_Append_ਨਤੀਜਾ(interp, zBuf, (ਅੱਖਰ*)0);
  ਵਾਪਸ TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
ਪੂਰਨ Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_ਮੰਜ਼ਿਲ_Base16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_ਮੰਜ਼ਿਲ_Base10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_ਮੰਜ਼ਿਲ_Base16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_ਮੰਜ਼ਿਲ_Base10x8, 0);
  ਵਾਪਸ TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
ਸਥਿਰ ਖਾਲੀ md5step(sqlite3_context *context, ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv){
  MD5Context *p;
  ਪੂਰਨ i;
  ਜੇ( argc<1 ) ਵਾਪਸ;
  p = sqlite3_aggregate_context(context, ਆਕਾਰ(*p));
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਜੇ( !p->isInit ){
    MD5Init(p);
  }
  ਲਈ(i=0; i<argc; i++){
    ਅਚਲ ਅੱਖਰ *z_ਅੰਕੜੇ = (ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[i]);
    ਜੇ( z_ਅੰਕੜੇ ){
      MD5Update(p, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)z_ਅੰਕੜੇ, (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਅੰਕੜੇ));
    }
  }
}
ਸਥਿਰ ਖਾਲੀ md5finalize(sqlite3_context *context){
  MD5Context *p;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ digest[16];
  ਅੱਖਰ zBuf[33];
  p = sqlite3_aggregate_context(context, ਆਕਾਰ(*p));
  MD5Final(digest,p);
  MD5_Digest_ਮੰਜ਼ਿਲ_Base16(digest, zBuf);
  sqlite3_ਨਤੀਜਾ_ਪਾਠ(context, zBuf, -1, SQLITE_TRANSIENT);
}
ਪੂਰਨ Md5_ਰਜਿਸਟਰ(
  sqlite3 *db,
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ,
  ਅਚਲ sqlite3_api_routines *pThunk
){
  ਪੂਰਨ rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  ਵਾਪਸ rc;
}
