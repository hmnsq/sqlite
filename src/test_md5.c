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
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

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
#ifndef uint32
#  define uint32 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
#endif

ସଂରଚନା MD5Context {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isInit;
  uint32 buf[4];
  uint32 bits[2];
  unsigned ଅକ୍ଷର in[64];
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
ସ୍ଥିର ଶୂନ୍ୟ byteReverse (unsigned ଅକ୍ଷର *buf, unsigned longs){
        uint32 t;
        କର {
                t = (uint32)((unsigned)buf[3]<<8 | buf[2]) << 16 |
                            ((unsigned)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } ଯେତେବେଳେ (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
ସ୍ଥିର ଶୂନ୍ୟ MD5Transform(uint32 buf[4], ଅଚଳ uint32 in[16]){
        ପଞ୍ଜିକରଣ uint32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
        MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
        MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
        MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
        MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
        MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
        MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
        MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
        MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
        MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
        MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
        MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
        MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
        MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
        MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
        MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

        MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
        MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
        MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
        MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
        MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
        MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
        MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
        MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
        MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
        MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
        MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
        MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
        MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
        MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
        MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
        MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

        MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
        MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
        MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
        MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
        MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
        MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
        MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
        MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
        MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
        MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
        MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
        MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
        MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
        MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
        MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
        MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

        MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
        MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
        MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
        MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
        MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
        MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
        MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
        MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
        MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
        MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
        MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
        MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
        MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
        MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
        MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
        MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
ସ୍ଥିର ଶୂନ୍ୟ MD5Init(MD5Context *ctx){
        ctx->isInit = 1;
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;
        ctx->bits[0] = 0;
        ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
ସ୍ଥିର
ଶୂନ୍ୟ MD5Update(MD5Context *ctx, ଅଚଳ unsigned ଅକ୍ଷର *buf, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା len){
        uint32 t;

        /* Update bitcount */

        t = ctx->bits[0];
        ଯଦି ((ctx->bits[0] = t + ((uint32)len << 3)) < t)
                ctx->bits[1]++; /* Carry from low to high */
        ctx->bits[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        ଯଦି ( t ) {
                unsigned ଅକ୍ଷର *p = (unsigned ଅକ୍ଷର *)ctx->in + t;

                t = 64-t;
                ଯଦି (len < t) {
                        memcpy(p, buf, len);
                        ଫେରନ୍ତୁ;
                }
                memcpy(p, buf, t);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        ଯେତେବେଳେ (len >= 64) {
                memcpy(ctx->in, buf, 64);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
ସ୍ଥିର ଶୂନ୍ୟ MD5Final(unsigned ଅକ୍ଷର digest[16], MD5Context *ctx){
        unsigned ଗଣନା_2;
        unsigned ଅକ୍ଷର *p;

        /* Compute number of bytes mod 64 */
        ଗଣନା_2 = (ctx->bits[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->in + ଗଣନା_2;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        ଗଣନା_2 = 64 - 1 - ଗଣନା_2;

        /* Pad out to 56 mod 64 */
        ଯଦି (ଗଣନା_2 < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset(p, 0, ଗଣନା_2);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);

                /* Now fill the next block with 56 bytes */
                memset(ctx->in, 0, 56);
        } ଅନ୍ୟଥା {
                /* Pad block to 56 bytes */
                memset(p, 0, ଗଣନା_2-8);
        }
        byteReverse(ctx->in, 14);

        /* Append length in bits and transform */
        memcpy(ctx->in + 14*4, ctx->bits, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->in);
        byteReverse((unsigned ଅକ୍ଷର *)ctx->buf, 4);
        memcpy(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
ସ୍ଥିର ଶୂନ୍ୟ MD5DigestToBase16(unsigned ଅକ୍ଷର *digest, ଅକ୍ଷର *zBuf){
  ସ୍ଥିର ଅକ୍ଷର ଅଚଳ zEncode[] = "0123456789abcdef";
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;

  ପାଇଁ(j=i=0; i<16; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = digest[i];
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
ସ୍ଥିର ଶୂନ୍ୟ MD5DigestToBase10x8(unsigned ଅକ୍ଷର digest[16], ଅକ୍ଷର zDigest[50]){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
  ପାଇଁ(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    ଯଦି( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI md5_cmd(
  ଶୂନ୍ୟ*cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  ଅଚଳ ଅକ୍ଷର **argv
){
  MD5Context ctx;
  unsigned ଅକ୍ଷର digest[16];
  ଅକ୍ଷର zBuf[50];
  ଶୂନ୍ୟ (*converter)(unsigned ଅକ୍ଷର*, ଅକ୍ଷର*);

  ଯଦି( argc!=2 ){
    Tcl_AppendResult(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (unsigned ଅକ୍ଷର*)argv[1], (unsigned)strlen(argv[1]));
  MD5Final(digest, &ctx);
  converter = (ଶୂନ୍ୟ(*)(unsigned ଅକ୍ଷର*,ଅକ୍ଷର*))cd;
  converter(digest, zBuf);
  Tcl_AppendResult(interp, zBuf, (ଅକ୍ଷର*)0);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI md5file_cmd(
  ଶୂନ୍ୟ*cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
  ଅଚଳ ଅକ୍ଷର **argv
){
  FILE *in;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା amt;
  MD5Context ctx;
  ଶୂନ୍ୟ (*converter)(unsigned ଅକ୍ଷର*, ଅକ୍ଷର*);
  unsigned ଅକ୍ଷର digest[16];
  ଅକ୍ଷର zBuf[10240];

  ଯଦି( argc!=2 && argc!=4 ){
    Tcl_AppendResult(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }ଅନ୍ୟଥା{
    ofst = 0;
    amt = 2147483647;
  }
  in = fopen(argv[1],"rb");
  ଯଦି( in==0 ){
    Tcl_AppendResult(interp,"unable to open file \"", argv[1],
         "\" for reading", (ଅକ୍ଷର*)0);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  fseek(in, ofst, SEEK_SET);
  MD5Init(&ctx);
  ଯେତେବେଳେ( amt>0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
    n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fread(zBuf, 1, ଆକାର(zBuf)<=amt ? ଆକାର(zBuf) : amt, in);
    ଯଦି( n<=0 ) ଭାଙ୍ଗ;
    MD5Update(&ctx, (unsigned ଅକ୍ଷର*)zBuf, (unsigned)n);
    amt -= n;
  }
  fclose(in);
  MD5Final(digest, &ctx);
  converter = (ଶୂନ୍ୟ(*)(unsigned ଅକ୍ଷର*,ଅକ୍ଷର*))cd;
  converter(digest, zBuf);
  Tcl_AppendResult(interp, zBuf, (ଅକ୍ଷର*)0);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5DigestToBase16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5DigestToBase10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5DigestToBase16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5DigestToBase10x8, 0);
  ଫେରନ୍ତୁ TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
ସ୍ଥିର ଶୂନ୍ୟ md5step(sqlite3_context *context, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv){
  MD5Context *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( argc<1 ) ଫେରନ୍ତୁ;
  p = sqlite3_aggregate_context(context, ଆକାର(*p));
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  ଯଦି( !p->isInit ){
    MD5Init(p);
  }
  ପାଇଁ(i=0; i<argc; i++){
    ଅଚଳ ଅକ୍ଷର *zData = (ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[i]);
    ଯଦି( zData ){
      MD5Update(p, (unsigned ଅକ୍ଷର*)zData, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zData));
    }
  }
}
ସ୍ଥିର ଶୂନ୍ୟ md5finalize(sqlite3_context *context){
  MD5Context *p;
  unsigned ଅକ୍ଷର digest[16];
  ଅକ୍ଷର zBuf[33];
  p = sqlite3_aggregate_context(context, ଆକାର(*p));
  MD5Final(digest,p);
  MD5DigestToBase16(digest, zBuf);
  sqlite3_result_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Md5_Register(
  sqlite3 *db,
  ଅକ୍ଷର **pz_Err_ସନ୍ଦେଶ,
  ଅଚଳ sqlite3_api_routines *pThunk
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  ଫେରନ୍ତୁ rc;
}
