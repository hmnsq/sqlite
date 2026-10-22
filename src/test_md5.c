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
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "tclsqlite.h"

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
#यदि_अपरिभाषितम् uint32
#  define uint32 अचिह्नित पूर्णाङ्क
#यदि_अन्तः

संरचना MD5Context {
  पूर्णाङ्क isInit;
  uint32 buf[4];
  uint32 द्वयङ्काः[2];
  अचिह्नित वर्ण अन्तर्[64];
};
प्रकारनिर्देश संरचना MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
स्थिर शून्य byteReverse (अचिह्नित वर्ण *buf, अचिह्नित longs){
        uint32 t;
        कुरु {
                t = (uint32)((अचिह्नित)buf[3]<<8 | buf[2]) << 16 |
                            ((अचिह्नित)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } यावत् (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#परिभाषय F1(x, y, z) (z ^ (x & (y ^ z)))
#परिभाषय F2(x, y, z) F1(z, x, y)
#परिभाषय F3(x, y, z) (x ^ y ^ z)
#परिभाषय F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#परिभाषय MD5_चरणः(f, w, x, y, z, दत्त, s) \
        ( w += f(x, y, z) + दत्त,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
स्थिर शून्य MD5Transform(uint32 buf[4], ध्रुव uint32 अन्तर्[16]){
        पञ्जिका uint32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5_चरणः(F1, a, b, c, d, अन्तर्[ 0]+0xd76aa478,  7);
        MD5_चरणः(F1, d, a, b, c, अन्तर्[ 1]+0xe8c7b756, 12);
        MD5_चरणः(F1, c, d, a, b, अन्तर्[ 2]+0x242070db, 17);
        MD5_चरणः(F1, b, c, d, a, अन्तर्[ 3]+0xc1bdceee, 22);
        MD5_चरणः(F1, a, b, c, d, अन्तर्[ 4]+0xf57c0faf,  7);
        MD5_चरणः(F1, d, a, b, c, अन्तर्[ 5]+0x4787c62a, 12);
        MD5_चरणः(F1, c, d, a, b, अन्तर्[ 6]+0xa8304613, 17);
        MD5_चरणः(F1, b, c, d, a, अन्तर्[ 7]+0xfd469501, 22);
        MD5_चरणः(F1, a, b, c, d, अन्तर्[ 8]+0x698098d8,  7);
        MD5_चरणः(F1, d, a, b, c, अन्तर्[ 9]+0x8b44f7af, 12);
        MD5_चरणः(F1, c, d, a, b, अन्तर्[10]+0xffff5bb1, 17);
        MD5_चरणः(F1, b, c, d, a, अन्तर्[11]+0x895cd7be, 22);
        MD5_चरणः(F1, a, b, c, d, अन्तर्[12]+0x6b901122,  7);
        MD5_चरणः(F1, d, a, b, c, अन्तर्[13]+0xfd987193, 12);
        MD5_चरणः(F1, c, d, a, b, अन्तर्[14]+0xa679438e, 17);
        MD5_चरणः(F1, b, c, d, a, अन्तर्[15]+0x49b40821, 22);

        MD5_चरणः(F2, a, b, c, d, अन्तर्[ 1]+0xf61e2562,  5);
        MD5_चरणः(F2, d, a, b, c, अन्तर्[ 6]+0xc040b340,  9);
        MD5_चरणः(F2, c, d, a, b, अन्तर्[11]+0x265e5a51, 14);
        MD5_चरणः(F2, b, c, d, a, अन्तर्[ 0]+0xe9b6c7aa, 20);
        MD5_चरणः(F2, a, b, c, d, अन्तर्[ 5]+0xd62f105d,  5);
        MD5_चरणः(F2, d, a, b, c, अन्तर्[10]+0x02441453,  9);
        MD5_चरणः(F2, c, d, a, b, अन्तर्[15]+0xd8a1e681, 14);
        MD5_चरणः(F2, b, c, d, a, अन्तर्[ 4]+0xe7d3fbc8, 20);
        MD5_चरणः(F2, a, b, c, d, अन्तर्[ 9]+0x21e1cde6,  5);
        MD5_चरणः(F2, d, a, b, c, अन्तर्[14]+0xc33707d6,  9);
        MD5_चरणः(F2, c, d, a, b, अन्तर्[ 3]+0xf4d50d87, 14);
        MD5_चरणः(F2, b, c, d, a, अन्तर्[ 8]+0x455a14ed, 20);
        MD5_चरणः(F2, a, b, c, d, अन्तर्[13]+0xa9e3e905,  5);
        MD5_चरणः(F2, d, a, b, c, अन्तर्[ 2]+0xfcefa3f8,  9);
        MD5_चरणः(F2, c, d, a, b, अन्तर्[ 7]+0x676f02d9, 14);
        MD5_चरणः(F2, b, c, d, a, अन्तर्[12]+0x8d2a4c8a, 20);

        MD5_चरणः(F3, a, b, c, d, अन्तर्[ 5]+0xfffa3942,  4);
        MD5_चरणः(F3, d, a, b, c, अन्तर्[ 8]+0x8771f681, 11);
        MD5_चरणः(F3, c, d, a, b, अन्तर्[11]+0x6d9d6122, 16);
        MD5_चरणः(F3, b, c, d, a, अन्तर्[14]+0xfde5380c, 23);
        MD5_चरणः(F3, a, b, c, d, अन्तर्[ 1]+0xa4beea44,  4);
        MD5_चरणः(F3, d, a, b, c, अन्तर्[ 4]+0x4bdecfa9, 11);
        MD5_चरणः(F3, c, d, a, b, अन्तर्[ 7]+0xf6bb4b60, 16);
        MD5_चरणः(F3, b, c, d, a, अन्तर्[10]+0xbebfbc70, 23);
        MD5_चरणः(F3, a, b, c, d, अन्तर्[13]+0x289b7ec6,  4);
        MD5_चरणः(F3, d, a, b, c, अन्तर्[ 0]+0xeaa127fa, 11);
        MD5_चरणः(F3, c, d, a, b, अन्तर्[ 3]+0xd4ef3085, 16);
        MD5_चरणः(F3, b, c, d, a, अन्तर्[ 6]+0x04881d05, 23);
        MD5_चरणः(F3, a, b, c, d, अन्तर्[ 9]+0xd9d4d039,  4);
        MD5_चरणः(F3, d, a, b, c, अन्तर्[12]+0xe6db99e5, 11);
        MD5_चरणः(F3, c, d, a, b, अन्तर्[15]+0x1fa27cf8, 16);
        MD5_चरणः(F3, b, c, d, a, अन्तर्[ 2]+0xc4ac5665, 23);

        MD5_चरणः(F4, a, b, c, d, अन्तर्[ 0]+0xf4292244,  6);
        MD5_चरणः(F4, d, a, b, c, अन्तर्[ 7]+0x432aff97, 10);
        MD5_चरणः(F4, c, d, a, b, अन्तर्[14]+0xab9423a7, 15);
        MD5_चरणः(F4, b, c, d, a, अन्तर्[ 5]+0xfc93a039, 21);
        MD5_चरणः(F4, a, b, c, d, अन्तर्[12]+0x655b59c3,  6);
        MD5_चरणः(F4, d, a, b, c, अन्तर्[ 3]+0x8f0ccc92, 10);
        MD5_चरणः(F4, c, d, a, b, अन्तर्[10]+0xffeff47d, 15);
        MD5_चरणः(F4, b, c, d, a, अन्तर्[ 1]+0x85845dd1, 21);
        MD5_चरणः(F4, a, b, c, d, अन्तर्[ 8]+0x6fa87e4f,  6);
        MD5_चरणः(F4, d, a, b, c, अन्तर्[15]+0xfe2ce6e0, 10);
        MD5_चरणः(F4, c, d, a, b, अन्तर्[ 6]+0xa3014314, 15);
        MD5_चरणः(F4, b, c, d, a, अन्तर्[13]+0x4e0811a1, 21);
        MD5_चरणः(F4, a, b, c, d, अन्तर्[ 4]+0xf7537e82,  6);
        MD5_चरणः(F4, d, a, b, c, अन्तर्[11]+0xbd3af235, 10);
        MD5_चरणः(F4, c, d, a, b, अन्तर्[ 2]+0x2ad7d2bb, 15);
        MD5_चरणः(F4, b, c, d, a, अन्तर्[ 9]+0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
स्थिर शून्य MD5Init(MD5Context *ctx){
        ctx->isInit = 1;
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;
        ctx->द्वयङ्काः[0] = 0;
        ctx->द्वयङ्काः[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
स्थिर
शून्य MD5Update(MD5Context *ctx, ध्रुव अचिह्नित वर्ण *buf, अचिह्नित पूर्णाङ्क len){
        uint32 t;

        /* Update bitcount */

        t = ctx->द्वयङ्काः[0];
        यदि ((ctx->द्वयङ्काः[0] = t + ((uint32)len << 3)) < t)
                ctx->द्वयङ्काः[1]++; /* Carry from low to high */
        ctx->द्वयङ्काः[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        यदि ( t ) {
                अचिह्नित वर्ण *p = (अचिह्नित वर्ण *)ctx->अन्तर् + t;

                t = 64-t;
                यदि (len < t) {
                        स्मृतिं_प्रतिलिख(p, buf, len);
                        प्रत्यागम;
                }
                स्मृतिं_प्रतिलिख(p, buf, t);
                byteReverse(ctx->अन्तर्, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->अन्तर्);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        यावत् (len >= 64) {
                स्मृतिं_प्रतिलिख(ctx->अन्तर्, buf, 64);
                byteReverse(ctx->अन्तर्, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->अन्तर्);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        स्मृतिं_प्रतिलिख(ctx->अन्तर्, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
स्थिर शून्य MD5Final(अचिह्नित वर्ण digest[16], MD5Context *ctx){
        अचिह्नित गणनम्;
        अचिह्नित वर्ण *p;

        /* Compute number of bytes mod 64 */
        गणनम् = (ctx->द्वयङ्काः[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->अन्तर् + गणनम्;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        गणनम् = 64 - 1 - गणनम्;

        /* Pad out to 56 mod 64 */
        यदि (गणनम् < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                स्मृतिं_पूरय(p, 0, गणनम्);
                byteReverse(ctx->अन्तर्, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->अन्तर्);

                /* Now fill the next block with 56 bytes */
                स्मृतिं_पूरय(ctx->अन्तर्, 0, 56);
        } अन्यथा {
                /* Pad block to 56 bytes */
                स्मृतिं_पूरय(p, 0, गणनम्-8);
        }
        byteReverse(ctx->अन्तर्, 14);

        /* Append length in bits and transform */
        स्मृतिं_प्रतिलिख(ctx->अन्तर् + 14*4, ctx->द्वयङ्काः, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->अन्तर्);
        byteReverse((अचिह्नित वर्ण *)ctx->buf, 4);
        स्मृतिं_प्रतिलिख(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
स्थिर शून्य MD5_Digest_गन्तव्यम्_Base16(अचिह्नित वर्ण *digest, वर्ण *zBuf){
  स्थिर वर्ण ध्रुव zEncode[] = "0123456789abcdef";
  पूर्णाङ्क i, j;

  प्रति(j=i=0; i<16; i++){
    पूर्णाङ्क a = digest[i];
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
स्थिर शून्य MD5_Digest_गन्तव्यम्_Base10x8(अचिह्नित वर्ण digest[16], वर्ण zDigest[50]){
  पूर्णाङ्क i, j;
  अचिह्नित पूर्णाङ्क x;
  प्रति(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    यदि( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI md5_cmd(
  शून्य*cd,
  Tcl_Interp *interp,
  पूर्णाङ्क argc,
  ध्रुव वर्ण **argv
){
  MD5Context ctx;
  अचिह्नित वर्ण digest[16];
  वर्ण zBuf[50];
  शून्य (*converter)(अचिह्नित वर्ण*, वर्ण*);

  यदि( argc!=2 ){
    Tcl_Append_परिणाम(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (अचिह्नित वर्ण*)argv[1], (अचिह्नित)वाक्यदैर्घ्यम्(argv[1]));
  MD5Final(digest, &ctx);
  converter = (शून्य(*)(अचिह्नित वर्ण*,वर्ण*))cd;
  converter(digest, zBuf);
  Tcl_Append_परिणाम(interp, zBuf, (वर्ण*)0);
  प्रत्यागम TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI md5file_cmd(
  शून्य*cd,
  Tcl_Interp *interp,
  पूर्णाङ्क argc,
  ध्रुव वर्ण **argv
){
  FILE *अन्तर्;
  पूर्णाङ्क ofst;
  पूर्णाङ्क amt;
  MD5Context ctx;
  शून्य (*converter)(अचिह्नित वर्ण*, वर्ण*);
  अचिह्नित वर्ण digest[16];
  वर्ण zBuf[10240];

  यदि( argc!=2 && argc!=4 ){
    Tcl_Append_परिणाम(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  यदि( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }अन्यथा{
    ofst = 0;
    amt = 2147483647;
  }
  अन्तर् = fopen(argv[1],"rb");
  यदि( अन्तर्==0 ){
    Tcl_Append_परिणाम(interp,"unable to open file \"", argv[1],
         "\" for reading", (वर्ण*)0);
    प्रत्यागम TCL_ERROR;
  }
  fseek(अन्तर्, ofst, SEEK_SET);
  MD5Init(&ctx);
  यावत्( amt>0 ){
    पूर्णाङ्क n;
    n = (पूर्णाङ्क)fread(zBuf, 1, परिमाण(zBuf)<=amt ? परिमाण(zBuf) : amt, अन्तर्);
    यदि( n<=0 ) भङ्ग;
    MD5Update(&ctx, (अचिह्नित वर्ण*)zBuf, (अचिह्नित)n);
    amt -= n;
  }
  fclose(अन्तर्);
  MD5Final(digest, &ctx);
  converter = (शून्य(*)(अचिह्नित वर्ण*,वर्ण*))cd;
  converter(digest, zBuf);
  Tcl_Append_परिणाम(interp, zBuf, (वर्ण*)0);
  प्रत्यागम TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
पूर्णाङ्क Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_गन्तव्यम्_Base16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_गन्तव्यम्_Base10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_गन्तव्यम्_Base16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_गन्तव्यम्_Base10x8, 0);
  प्रत्यागम TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
स्थिर शून्य md5step(sqlite3_context *context, पूर्णाङ्क argc, sqlite3_मानम् **argv){
  MD5Context *p;
  पूर्णाङ्क i;
  यदि( argc<1 ) प्रत्यागम;
  p = sqlite3_aggregate_context(context, परिमाण(*p));
  यदि( p==0 ) प्रत्यागम;
  यदि( !p->isInit ){
    MD5Init(p);
  }
  प्रति(i=0; i<argc; i++){
    ध्रुव वर्ण *z_दत्त = (वर्ण*)sqlite3_मानम्_text(argv[i]);
    यदि( z_दत्त ){
      MD5Update(p, (अचिह्नित वर्ण*)z_दत्त, (पूर्णाङ्क)वाक्यदैर्घ्यम्(z_दत्त));
    }
  }
}
स्थिर शून्य md5finalize(sqlite3_context *context){
  MD5Context *p;
  अचिह्नित वर्ण digest[16];
  वर्ण zBuf[33];
  p = sqlite3_aggregate_context(context, परिमाण(*p));
  MD5Final(digest,p);
  MD5_Digest_गन्तव्यम्_Base16(digest, zBuf);
  sqlite3_परिणाम_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
पूर्णाङ्क Md5_पञ्जिकाकोशः(
  sqlite3 *db,
  वर्ण **pz_Err_सन्देशः,
  ध्रुव sqlite3_api_routines *pThunk
){
  पूर्णाङ्क rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  प्रत्यागम rc;
}
