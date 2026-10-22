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
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "tclsqlite.h"

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
#నిర్వచితం_కాకపోతే uint32
#  define uint32 గుర్తులేని పూర్ణాంకం
#అయితే_ముగింపు

నిర్మాణం MD5Context {
  పూర్ణాంకం isInit;
  uint32 buf[4];
  uint32 బిట్లు[2];
  గుర్తులేని అక్షరం లోపల[64];
};
రకనిర్వచనం నిర్మాణం MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
స్థిర శూన్య byte_తిరగేయి (గుర్తులేని అక్షరం *buf, గుర్తులేని longs){
        uint32 t;
        చేయి {
                t = (uint32)((గుర్తులేని)buf[3]<<8 | buf[2]) << 16 |
                            ((గుర్తులేని)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } అయితే (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#నిర్వచించు F1(x, y, z) (z ^ (x & (y ^ z)))
#నిర్వచించు F2(x, y, z) F1(z, x, y)
#నిర్వచించు F3(x, y, z) (x ^ y ^ z)
#నిర్వచించు F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#నిర్వచించు MD5_అడుగు(f, w, x, y, z, దత్తాంశం, s) \
        ( w += f(x, y, z) + దత్తాంశం,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
స్థిర శూన్య MD5Transform(uint32 buf[4], స్థిరరాశి uint32 లోపల[16]){
        నమోదు uint32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5_అడుగు(F1, a, b, c, d, లోపల[ 0]+0xd76aa478,  7);
        MD5_అడుగు(F1, d, a, b, c, లోపల[ 1]+0xe8c7b756, 12);
        MD5_అడుగు(F1, c, d, a, b, లోపల[ 2]+0x242070db, 17);
        MD5_అడుగు(F1, b, c, d, a, లోపల[ 3]+0xc1bdceee, 22);
        MD5_అడుగు(F1, a, b, c, d, లోపల[ 4]+0xf57c0faf,  7);
        MD5_అడుగు(F1, d, a, b, c, లోపల[ 5]+0x4787c62a, 12);
        MD5_అడుగు(F1, c, d, a, b, లోపల[ 6]+0xa8304613, 17);
        MD5_అడుగు(F1, b, c, d, a, లోపల[ 7]+0xfd469501, 22);
        MD5_అడుగు(F1, a, b, c, d, లోపల[ 8]+0x698098d8,  7);
        MD5_అడుగు(F1, d, a, b, c, లోపల[ 9]+0x8b44f7af, 12);
        MD5_అడుగు(F1, c, d, a, b, లోపల[10]+0xffff5bb1, 17);
        MD5_అడుగు(F1, b, c, d, a, లోపల[11]+0x895cd7be, 22);
        MD5_అడుగు(F1, a, b, c, d, లోపల[12]+0x6b901122,  7);
        MD5_అడుగు(F1, d, a, b, c, లోపల[13]+0xfd987193, 12);
        MD5_అడుగు(F1, c, d, a, b, లోపల[14]+0xa679438e, 17);
        MD5_అడుగు(F1, b, c, d, a, లోపల[15]+0x49b40821, 22);

        MD5_అడుగు(F2, a, b, c, d, లోపల[ 1]+0xf61e2562,  5);
        MD5_అడుగు(F2, d, a, b, c, లోపల[ 6]+0xc040b340,  9);
        MD5_అడుగు(F2, c, d, a, b, లోపల[11]+0x265e5a51, 14);
        MD5_అడుగు(F2, b, c, d, a, లోపల[ 0]+0xe9b6c7aa, 20);
        MD5_అడుగు(F2, a, b, c, d, లోపల[ 5]+0xd62f105d,  5);
        MD5_అడుగు(F2, d, a, b, c, లోపల[10]+0x02441453,  9);
        MD5_అడుగు(F2, c, d, a, b, లోపల[15]+0xd8a1e681, 14);
        MD5_అడుగు(F2, b, c, d, a, లోపల[ 4]+0xe7d3fbc8, 20);
        MD5_అడుగు(F2, a, b, c, d, లోపల[ 9]+0x21e1cde6,  5);
        MD5_అడుగు(F2, d, a, b, c, లోపల[14]+0xc33707d6,  9);
        MD5_అడుగు(F2, c, d, a, b, లోపల[ 3]+0xf4d50d87, 14);
        MD5_అడుగు(F2, b, c, d, a, లోపల[ 8]+0x455a14ed, 20);
        MD5_అడుగు(F2, a, b, c, d, లోపల[13]+0xa9e3e905,  5);
        MD5_అడుగు(F2, d, a, b, c, లోపల[ 2]+0xfcefa3f8,  9);
        MD5_అడుగు(F2, c, d, a, b, లోపల[ 7]+0x676f02d9, 14);
        MD5_అడుగు(F2, b, c, d, a, లోపల[12]+0x8d2a4c8a, 20);

        MD5_అడుగు(F3, a, b, c, d, లోపల[ 5]+0xfffa3942,  4);
        MD5_అడుగు(F3, d, a, b, c, లోపల[ 8]+0x8771f681, 11);
        MD5_అడుగు(F3, c, d, a, b, లోపల[11]+0x6d9d6122, 16);
        MD5_అడుగు(F3, b, c, d, a, లోపల[14]+0xfde5380c, 23);
        MD5_అడుగు(F3, a, b, c, d, లోపల[ 1]+0xa4beea44,  4);
        MD5_అడుగు(F3, d, a, b, c, లోపల[ 4]+0x4bdecfa9, 11);
        MD5_అడుగు(F3, c, d, a, b, లోపల[ 7]+0xf6bb4b60, 16);
        MD5_అడుగు(F3, b, c, d, a, లోపల[10]+0xbebfbc70, 23);
        MD5_అడుగు(F3, a, b, c, d, లోపల[13]+0x289b7ec6,  4);
        MD5_అడుగు(F3, d, a, b, c, లోపల[ 0]+0xeaa127fa, 11);
        MD5_అడుగు(F3, c, d, a, b, లోపల[ 3]+0xd4ef3085, 16);
        MD5_అడుగు(F3, b, c, d, a, లోపల[ 6]+0x04881d05, 23);
        MD5_అడుగు(F3, a, b, c, d, లోపల[ 9]+0xd9d4d039,  4);
        MD5_అడుగు(F3, d, a, b, c, లోపల[12]+0xe6db99e5, 11);
        MD5_అడుగు(F3, c, d, a, b, లోపల[15]+0x1fa27cf8, 16);
        MD5_అడుగు(F3, b, c, d, a, లోపల[ 2]+0xc4ac5665, 23);

        MD5_అడుగు(F4, a, b, c, d, లోపల[ 0]+0xf4292244,  6);
        MD5_అడుగు(F4, d, a, b, c, లోపల[ 7]+0x432aff97, 10);
        MD5_అడుగు(F4, c, d, a, b, లోపల[14]+0xab9423a7, 15);
        MD5_అడుగు(F4, b, c, d, a, లోపల[ 5]+0xfc93a039, 21);
        MD5_అడుగు(F4, a, b, c, d, లోపల[12]+0x655b59c3,  6);
        MD5_అడుగు(F4, d, a, b, c, లోపల[ 3]+0x8f0ccc92, 10);
        MD5_అడుగు(F4, c, d, a, b, లోపల[10]+0xffeff47d, 15);
        MD5_అడుగు(F4, b, c, d, a, లోపల[ 1]+0x85845dd1, 21);
        MD5_అడుగు(F4, a, b, c, d, లోపల[ 8]+0x6fa87e4f,  6);
        MD5_అడుగు(F4, d, a, b, c, లోపల[15]+0xfe2ce6e0, 10);
        MD5_అడుగు(F4, c, d, a, b, లోపల[ 6]+0xa3014314, 15);
        MD5_అడుగు(F4, b, c, d, a, లోపల[13]+0x4e0811a1, 21);
        MD5_అడుగు(F4, a, b, c, d, లోపల[ 4]+0xf7537e82,  6);
        MD5_అడుగు(F4, d, a, b, c, లోపల[11]+0xbd3af235, 10);
        MD5_అడుగు(F4, c, d, a, b, లోపల[ 2]+0x2ad7d2bb, 15);
        MD5_అడుగు(F4, b, c, d, a, లోపల[ 9]+0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
స్థిర శూన్య MD5Init(MD5Context *ctx){
        ctx->isInit = 1;
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;
        ctx->బిట్లు[0] = 0;
        ctx->బిట్లు[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
స్థిర
శూన్య MD5Update(MD5Context *ctx, స్థిరరాశి గుర్తులేని అక్షరం *buf, గుర్తులేని పూర్ణాంకం len){
        uint32 t;

        /* Update bitcount */

        t = ctx->బిట్లు[0];
        ఒకవేళ ((ctx->బిట్లు[0] = t + ((uint32)len << 3)) < t)
                ctx->బిట్లు[1]++; /* Carry from low to high */
        ctx->బిట్లు[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        ఒకవేళ ( t ) {
                గుర్తులేని అక్షరం *p = (గుర్తులేని అక్షరం *)ctx->లోపల + t;

                t = 64-t;
                ఒకవేళ (len < t) {
                        జ్ఞాపకం_నకలు(p, buf, len);
                        తిరిగి;
                }
                జ్ఞాపకం_నకలు(p, buf, t);
                byte_తిరగేయి(ctx->లోపల, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->లోపల);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        అయితే (len >= 64) {
                జ్ఞాపకం_నకలు(ctx->లోపల, buf, 64);
                byte_తిరగేయి(ctx->లోపల, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->లోపల);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        జ్ఞాపకం_నకలు(ctx->లోపల, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
స్థిర శూన్య MD5Final(గుర్తులేని అక్షరం digest[16], MD5Context *ctx){
        గుర్తులేని లెక్క;
        గుర్తులేని అక్షరం *p;

        /* Compute number of bytes mod 64 */
        లెక్క = (ctx->బిట్లు[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->లోపల + లెక్క;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        లెక్క = 64 - 1 - లెక్క;

        /* Pad out to 56 mod 64 */
        ఒకవేళ (లెక్క < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset(p, 0, లెక్క);
                byte_తిరగేయి(ctx->లోపల, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->లోపల);

                /* Now fill the next block with 56 bytes */
                memset(ctx->లోపల, 0, 56);
        } లేకపోతే {
                /* Pad block to 56 bytes */
                memset(p, 0, లెక్క-8);
        }
        byte_తిరగేయి(ctx->లోపల, 14);

        /* Append length in bits and transform */
        జ్ఞాపకం_నకలు(ctx->లోపల + 14*4, ctx->బిట్లు, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->లోపల);
        byte_తిరగేయి((గుర్తులేని అక్షరం *)ctx->buf, 4);
        జ్ఞాపకం_నకలు(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
స్థిర శూన్య MD5_Digest_గమ్యం_Base16(గుర్తులేని అక్షరం *digest, అక్షరం *zBuf){
  స్థిర అక్షరం స్థిరరాశి zEncode[] = "0123456789abcdef";
  పూర్ణాంకం i, j;

  కొరకు(j=i=0; i<16; i++){
    పూర్ణాంకం a = digest[i];
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
స్థిర శూన్య MD5_Digest_గమ్యం_Base10x8(గుర్తులేని అక్షరం digest[16], అక్షరం zDigest[50]){
  పూర్ణాంకం i, j;
  గుర్తులేని పూర్ణాంకం x;
  కొరకు(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    ఒకవేళ( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI md5_cmd(
  శూన్య*cd,
  Tcl_Interp *interp,
  పూర్ణాంకం argc,
  స్థిరరాశి అక్షరం **argv
){
  MD5Context ctx;
  గుర్తులేని అక్షరం digest[16];
  అక్షరం zBuf[50];
  శూన్య (*converter)(గుర్తులేని అక్షరం*, అక్షరం*);

  ఒకవేళ( argc!=2 ){
    Tcl_Append_ఫలితం(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (గుర్తులేని అక్షరం*)argv[1], (గుర్తులేని)అక్షరశ్రేణి_పొడవు(argv[1]));
  MD5Final(digest, &ctx);
  converter = (శూన్య(*)(గుర్తులేని అక్షరం*,అక్షరం*))cd;
  converter(digest, zBuf);
  Tcl_Append_ఫలితం(interp, zBuf, (అక్షరం*)0);
  తిరిగి TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI md5file_cmd(
  శూన్య*cd,
  Tcl_Interp *interp,
  పూర్ణాంకం argc,
  స్థిరరాశి అక్షరం **argv
){
  FILE *లోపల;
  పూర్ణాంకం ofst;
  పూర్ణాంకం amt;
  MD5Context ctx;
  శూన్య (*converter)(గుర్తులేని అక్షరం*, అక్షరం*);
  గుర్తులేని అక్షరం digest[16];
  అక్షరం zBuf[10240];

  ఒకవేళ( argc!=2 && argc!=4 ){
    Tcl_Append_ఫలితం(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }లేకపోతే{
    ofst = 0;
    amt = 2147483647;
  }
  లోపల = fopen(argv[1],"rb");
  ఒకవేళ( లోపల==0 ){
    Tcl_Append_ఫలితం(interp,"unable to open file \"", argv[1],
         "\" for reading", (అక్షరం*)0);
    తిరిగి TCL_ERROR;
  }
  fseek(లోపల, ofst, SEEK_SET);
  MD5Init(&ctx);
  అయితే( amt>0 ){
    పూర్ణాంకం n;
    n = (పూర్ణాంకం)fread(zBuf, 1, పరిమాణం(zBuf)<=amt ? పరిమాణం(zBuf) : amt, లోపల);
    ఒకవేళ( n<=0 ) ఆపు;
    MD5Update(&ctx, (గుర్తులేని అక్షరం*)zBuf, (గుర్తులేని)n);
    amt -= n;
  }
  fclose(లోపల);
  MD5Final(digest, &ctx);
  converter = (శూన్య(*)(గుర్తులేని అక్షరం*,అక్షరం*))cd;
  converter(digest, zBuf);
  Tcl_Append_ఫలితం(interp, zBuf, (అక్షరం*)0);
  తిరిగి TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
పూర్ణాంకం Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_గమ్యం_Base16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_గమ్యం_Base10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_గమ్యం_Base16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_గమ్యం_Base10x8, 0);
  తిరిగి TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
స్థిర శూన్య md5step(sqlite3_context *context, పూర్ణాంకం argc, sqlite3_విలువ **argv){
  MD5Context *p;
  పూర్ణాంకం i;
  ఒకవేళ( argc<1 ) తిరిగి;
  p = sqlite3_aggregate_context(context, పరిమాణం(*p));
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( !p->isInit ){
    MD5Init(p);
  }
  కొరకు(i=0; i<argc; i++){
    స్థిరరాశి అక్షరం *z_దత్తాంశం = (అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[i]);
    ఒకవేళ( z_దత్తాంశం ){
      MD5Update(p, (గుర్తులేని అక్షరం*)z_దత్తాంశం, (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_దత్తాంశం));
    }
  }
}
స్థిర శూన్య md5finalize(sqlite3_context *context){
  MD5Context *p;
  గుర్తులేని అక్షరం digest[16];
  అక్షరం zBuf[33];
  p = sqlite3_aggregate_context(context, పరిమాణం(*p));
  MD5Final(digest,p);
  MD5_Digest_గమ్యం_Base16(digest, zBuf);
  sqlite3_ఫలితం_పాఠ్యం(context, zBuf, -1, SQLITE_TRANSIENT);
}
పూర్ణాంకం Md5_రిజిస్టరు(
  sqlite3 *db,
  అక్షరం **pz_Err_సందేశం,
  స్థిరరాశి sqlite3_api_routines *pThunk
){
  పూర్ణాంకం rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  తిరిగి rc;
}
