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
#समावेश "sqlite3.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश "sqlite3.h"
#समावेश "tclsqlite.h"

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
#अगर_अघोषित uint32
#  define uint32 अचिन्हित पूर्णांक
#पूर्ण_अगर

काष्ठा MD5Context {
  पूर्णांक isInit;
  uint32 buf[4];
  uint32 bits[2];
  अचिन्हित अक्षर in[64];
};
प्रकार काष्ठा MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
अटल व्योम byteReverse (अचिन्हित अक्षर *buf, अचिन्हित longs){
        uint32 t;
        करो {
                t = (uint32)((अचिन्हित)buf[3]<<8 | buf[2]) << 16 |
                            ((अचिन्हित)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } जबतक (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#घोषणा F1(x, y, z) (z ^ (x & (y ^ z)))
#घोषणा F2(x, y, z) F1(z, x, y)
#घोषणा F3(x, y, z) (x ^ y ^ z)
#घोषणा F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#घोषणा MD5STEP(f, w, x, y, z, तथ्य, s) \
        ( w += f(x, y, z) + तथ्य,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
अटल व्योम MD5Transform(uint32 buf[4], स्थिर uint32 in[16]){
        रेजिस्टर uint32 a, b, c, d;

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
अटल व्योम MD5Init(MD5Context *ctx){
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
अटल
व्योम MD5Update(MD5Context *ctx, स्थिर अचिन्हित अक्षर *buf, अचिन्हित पूर्णांक len){
        uint32 t;

        /* Update bitcount */

        t = ctx->bits[0];
        अगर ((ctx->bits[0] = t + ((uint32)len << 3)) < t)
                ctx->bits[1]++; /* Carry from low to high */
        ctx->bits[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        अगर ( t ) {
                अचिन्हित अक्षर *p = (अचिन्हित अक्षर *)ctx->in + t;

                t = 64-t;
                अगर (len < t) {
                        स_नकल(p, buf, len);
                        वापस;
                }
                स_नकल(p, buf, t);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        जबतक (len >= 64) {
                स_नकल(ctx->in, buf, 64);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        स_नकल(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
अटल व्योम MD5Final(अचिन्हित अक्षर digest[16], MD5Context *ctx){
        अचिन्हित गिनती;
        अचिन्हित अक्षर *p;

        /* Compute number of bytes mod 64 */
        गिनती = (ctx->bits[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->in + गिनती;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        गिनती = 64 - 1 - गिनती;

        /* Pad out to 56 mod 64 */
        अगर (गिनती < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                स_रखो(p, 0, गिनती);
                byteReverse(ctx->in, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->in);

                /* Now fill the next block with 56 bytes */
                स_रखो(ctx->in, 0, 56);
        } अन्यथा {
                /* Pad block to 56 bytes */
                स_रखो(p, 0, गिनती-8);
        }
        byteReverse(ctx->in, 14);

        /* Append length in bits and transform */
        स_नकल(ctx->in + 14*4, ctx->bits, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->in);
        byteReverse((अचिन्हित अक्षर *)ctx->buf, 4);
        स_नकल(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
अटल व्योम MD5DigestToBase16(अचिन्हित अक्षर *digest, अक्षर *zBuf){
  अटल अक्षर स्थिर zEncode[] = "0123456789abcdef";
  पूर्णांक i, j;

  क्रम(j=i=0; i<16; i++){
    पूर्णांक a = digest[i];
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
अटल व्योम MD5DigestToBase10x8(अचिन्हित अक्षर digest[16], अक्षर zDigest[50]){
  पूर्णांक i, j;
  अचिन्हित पूर्णांक x;
  क्रम(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    अगर( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
अटल पूर्णांक SQLITE_TCLAPI md5_cmd(
  व्योम*cd,
  Tcl_Interp *interp,
  पूर्णांक argc,
  स्थिर अक्षर **argv
){
  MD5Context ctx;
  अचिन्हित अक्षर digest[16];
  अक्षर zBuf[50];
  व्योम (*converter)(अचिन्हित अक्षर*, अक्षर*);

  अगर( argc!=2 ){
    Tcl_Append_परिणाम(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (अचिन्हित अक्षर*)argv[1], (अचिन्हित)म_माप(argv[1]));
  MD5Final(digest, &ctx);
  converter = (व्योम(*)(अचिन्हित अक्षर*,अक्षर*))cd;
  converter(digest, zBuf);
  Tcl_Append_परिणाम(interp, zBuf, (अक्षर*)0);
  वापस TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
अटल पूर्णांक SQLITE_TCLAPI md5file_cmd(
  व्योम*cd,
  Tcl_Interp *interp,
  पूर्णांक argc,
  स्थिर अक्षर **argv
){
  खाता *in;
  पूर्णांक ofst;
  पूर्णांक amt;
  MD5Context ctx;
  व्योम (*converter)(अचिन्हित अक्षर*, अक्षर*);
  अचिन्हित अक्षर digest[16];
  अक्षर zBuf[10240];

  अगर( argc!=2 && argc!=4 ){
    Tcl_Append_परिणाम(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  अगर( argc==4 ){
    ofst = म_से_प(argv[2]);
    amt = म_से_प(argv[3]);
  }अन्यथा{
    ofst = 0;
    amt = 2147483647;
  }
  in = ख_खोलो(argv[1],"rb");
  अगर( in==0 ){
    Tcl_Append_परिणाम(interp,"unable to open file \"", argv[1],
         "\" for reading", (अक्षर*)0);
    वापस TCL_ERROR;
  }
  ख_जाओ(in, ofst, शुरू_से);
  MD5Init(&ctx);
  जबतक( amt>0 ){
    पूर्णांक n;
    n = (पूर्णांक)ख_पढ़ो(zBuf, 1, माप(zBuf)<=amt ? माप(zBuf) : amt, in);
    अगर( n<=0 ) अवरोध;
    MD5Update(&ctx, (अचिन्हित अक्षर*)zBuf, (अचिन्हित)n);
    amt -= n;
  }
  ख_बंद(in);
  MD5Final(digest, &ctx);
  converter = (व्योम(*)(अचिन्हित अक्षर*,अक्षर*))cd;
  converter(digest, zBuf);
  Tcl_Append_परिणाम(interp, zBuf, (अक्षर*)0);
  वापस TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
पूर्णांक Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5DigestToBase16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5DigestToBase10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5DigestToBase16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5DigestToBase10x8, 0);
  वापस TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
अटल व्योम md5step(sqlite3_context *context, पूर्णांक argc, sqlite3_मूल्य **argv){
  MD5Context *p;
  पूर्णांक i;
  अगर( argc<1 ) वापस;
  p = sqlite3_aggregate_context(context, माप(*p));
  अगर( p==0 ) वापस;
  अगर( !p->isInit ){
    MD5Init(p);
  }
  क्रम(i=0; i<argc; i++){
    स्थिर अक्षर *z_तथ्य = (अक्षर*)sqlite3_मूल्य_text(argv[i]);
    अगर( z_तथ्य ){
      MD5Update(p, (अचिन्हित अक्षर*)z_तथ्य, (पूर्णांक)म_माप(z_तथ्य));
    }
  }
}
अटल व्योम md5finalize(sqlite3_context *context){
  MD5Context *p;
  अचिन्हित अक्षर digest[16];
  अक्षर zBuf[33];
  p = sqlite3_aggregate_context(context, माप(*p));
  MD5Final(digest,p);
  MD5DigestToBase16(digest, zBuf);
  sqlite3_परिणाम_text(context, zBuf, -1, SQLITE_TRANSIENT);
}
पूर्णांक Md5_Register(
  sqlite3 *db,
  अक्षर **pz_Err_संदेश,
  स्थिर sqlite3_api_routines *pThunk
){
  पूर्णांक rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  वापस rc;
}
