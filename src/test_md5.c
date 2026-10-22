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
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "tclsqlite.h"

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
#  define uint32 unsigned പൂർണ്ണസംഖ്യ
#endif

ഘടന MD5Context {
  പൂർണ്ണസംഖ്യ isInit;
  uint32 buf[4];
  uint32 bits[2];
  unsigned അക്ഷരം in[64];
};
തരനിർവചനം ഘടന MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
സ്ഥിതം ശൂന്യം byteReverse (unsigned അക്ഷരം *buf, unsigned longs){
        uint32 t;
        ചെയ്യുക {
                t = (uint32)((unsigned)buf[3]<<8 | buf[2]) << 16 |
                            ((unsigned)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } ആകുമ്പോൾ (--longs);
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
സ്ഥിതം ശൂന്യം MD5Transform(uint32 buf[4], സ്ഥിരം uint32 in[16]){
        രജിസ്റ്റർ uint32 a, b, c, d;

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
സ്ഥിതം ശൂന്യം MD5Init(MD5Context *ctx){
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
സ്ഥിതം
ശൂന്യം MD5Update(MD5Context *ctx, സ്ഥിരം unsigned അക്ഷരം *buf, unsigned പൂർണ്ണസംഖ്യ len){
        uint32 t;

        /* Update bitcount */

        t = ctx->bits[0];
        എങ്കിൽ ((ctx->bits[0] = t + ((uint32)len << 3)) < t)
                ctx->bits[1]++; /* Carry from low to high */
        ctx->bits[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        എങ്കിൽ ( t ) {
                unsigned അക്ഷരം *p = (unsigned അക്ഷരം *)ctx->in + t;

                t = 64-t;
                എങ്കിൽ (len < t) {
                        memcpy(p, buf, len);
                        തിരികെ;
                }
                memcpy(p, buf, t);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        ആകുമ്പോൾ (len >= 64) {
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
സ്ഥിതം ശൂന്യം MD5Final(unsigned അക്ഷരം digest[16], MD5Context *ctx){
        unsigned എണ്ണം;
        unsigned അക്ഷരം *p;

        /* Compute number of bytes mod 64 */
        എണ്ണം = (ctx->bits[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->in + എണ്ണം;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        എണ്ണം = 64 - 1 - എണ്ണം;

        /* Pad out to 56 mod 64 */
        എങ്കിൽ (എണ്ണം < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset(p, 0, എണ്ണം);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);

                /* Now fill the next block with 56 bytes */
                memset(ctx->in, 0, 56);
        } അല്ലെങ്കിൽ {
                /* Pad block to 56 bytes */
                memset(p, 0, എണ്ണം-8);
        }
        byteReverse(ctx->in, 14);

        /* Append length in bits and transform */
        memcpy(ctx->in + 14*4, ctx->bits, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->in);
        byteReverse((unsigned അക്ഷരം *)ctx->buf, 4);
        memcpy(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
സ്ഥിതം ശൂന്യം MD5DigestToBase16(unsigned അക്ഷരം *digest, അക്ഷരം *zBuf){
  സ്ഥിതം അക്ഷരം സ്ഥിരം zEncode[] = "0123456789abcdef";
  പൂർണ്ണസംഖ്യ i, j;

  വേണ്ടി(j=i=0; i<16; i++){
    പൂർണ്ണസംഖ്യ a = digest[i];
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
സ്ഥിതം ശൂന്യം MD5DigestToBase10x8(unsigned അക്ഷരം digest[16], അക്ഷരം zDigest[50]){
  പൂർണ്ണസംഖ്യ i, j;
  unsigned പൂർണ്ണസംഖ്യ x;
  വേണ്ടി(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    എങ്കിൽ( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI md5_cmd(
  ശൂന്യം*cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ argc,
  സ്ഥിരം അക്ഷരം **argv
){
  MD5Context ctx;
  unsigned അക്ഷരം digest[16];
  അക്ഷരം zBuf[50];
  ശൂന്യം (*converter)(unsigned അക്ഷരം*, അക്ഷരം*);

  എങ്കിൽ( argc!=2 ){
    Tcl_AppendResult(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (unsigned അക്ഷരം*)argv[1], (unsigned)strlen(argv[1]));
  MD5Final(digest, &ctx);
  converter = (ശൂന്യം(*)(unsigned അക്ഷരം*,അക്ഷരം*))cd;
  converter(digest, zBuf);
  Tcl_AppendResult(interp, zBuf, (അക്ഷരം*)0);
  തിരികെ TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI md5file_cmd(
  ശൂന്യം*cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ argc,
  സ്ഥിരം അക്ഷരം **argv
){
  FILE *in;
  പൂർണ്ണസംഖ്യ ofst;
  പൂർണ്ണസംഖ്യ amt;
  MD5Context ctx;
  ശൂന്യം (*converter)(unsigned അക്ഷരം*, അക്ഷരം*);
  unsigned അക്ഷരം digest[16];
  അക്ഷരം zBuf[10240];

  എങ്കിൽ( argc!=2 && argc!=4 ){
    Tcl_AppendResult(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }അല്ലെങ്കിൽ{
    ofst = 0;
    amt = 2147483647;
  }
  in = fopen(argv[1],"rb");
  എങ്കിൽ( in==0 ){
    Tcl_AppendResult(interp,"unable to open file \"", argv[1],
         "\" for reading", (അക്ഷരം*)0);
    തിരികെ TCL_ERROR;
  }
  fseek(in, ofst, SEEK_SET);
  MD5Init(&ctx);
  ആകുമ്പോൾ( amt>0 ){
    പൂർണ്ണസംഖ്യ n;
    n = (പൂർണ്ണസംഖ്യ)fread(zBuf, 1, വലുപ്പം(zBuf)<=amt ? വലുപ്പം(zBuf) : amt, in);
    എങ്കിൽ( n<=0 ) നിർത്തുക;
    MD5Update(&ctx, (unsigned അക്ഷരം*)zBuf, (unsigned)n);
    amt -= n;
  }
  fclose(in);
  MD5Final(digest, &ctx);
  converter = (ശൂന്യം(*)(unsigned അക്ഷരം*,അക്ഷരം*))cd;
  converter(digest, zBuf);
  Tcl_AppendResult(interp, zBuf, (അക്ഷരം*)0);
  തിരികെ TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5DigestToBase16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5DigestToBase10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5DigestToBase16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5DigestToBase10x8, 0);
  തിരികെ TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
സ്ഥിതം ശൂന്യം md5step(sqlite3_context *context, പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv){
  MD5Context *p;
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( argc<1 ) തിരികെ;
  p = sqlite3_aggregate_context(context, വലുപ്പം(*p));
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( !p->isInit ){
    MD5Init(p);
  }
  വേണ്ടി(i=0; i<argc; i++){
    സ്ഥിരം അക്ഷരം *zData = (അക്ഷരം*)sqlite3_മൂല്യം_text(argv[i]);
    എങ്കിൽ( zData ){
      MD5Update(p, (unsigned അക്ഷരം*)zData, (പൂർണ്ണസംഖ്യ)strlen(zData));
    }
  }
}
സ്ഥിതം ശൂന്യം md5finalize(sqlite3_context *context){
  MD5Context *p;
  unsigned അക്ഷരം digest[16];
  അക്ഷരം zBuf[33];
  p = sqlite3_aggregate_context(context, വലുപ്പം(*p));
  MD5Final(digest,p);
  MD5DigestToBase16(digest, zBuf);
  sqlite3_result_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
പൂർണ്ണസംഖ്യ Md5_Register(
  sqlite3 *db,
  അക്ഷരം **pz_Err_സന്ദേശം,
  സ്ഥിരം sqlite3_api_routines *pThunk
){
  പൂർണ്ണസംഖ്യ rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  തിരികെ rc;
}
