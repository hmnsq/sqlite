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
#ضمن "sqlite3.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "sqlite3.h"
#ضمن "tclsqlite.h"

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
#جے_بنا_تعریف uint32
#  define uint32 بنانشانی صحيح
#جے_انت

بنية MD5Context {
  صحيح isInit;
  uint32 buf[4];
  uint32 بٹ[2];
  بنانشانی حرف وچ[64];
};
تعريفنوع بنية MD5Context MD5Context;

/*
 * Note: this code is harmless on little-endian machines.
 */
ساكن فراغ byte_الٹ (بنانشانی حرف *buf, بنانشانی longs){
        uint32 t;
        افعل {
                t = (uint32)((بنانشانی)buf[3]<<8 | buf[2]) << 16 |
                            ((بنانشانی)buf[1]<<8 | buf[0]);
                *(uint32 *)buf = t;
                buf += 4;
        } طالما (--longs);
}
/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#تعریف_کرو F1(x, y, z) (z ^ (x & (y ^ z)))
#تعریف_کرو F2(x, y, z) F1(z, x, y)
#تعریف_کرو F3(x, y, z) (x ^ y ^ z)
#تعریف_کرو F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#تعریف_کرو MD5_قدم(f, w, x, y, z, انکڑے, s) \
        ( w += f(x, y, z) + انکڑے,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
ساكن فراغ MD5Transform(uint32 buf[4], ثابت uint32 وچ[16]){
        سجل uint32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5_قدم(F1, a, b, c, d, وچ[ 0]+0xd76aa478,  7);
        MD5_قدم(F1, d, a, b, c, وچ[ 1]+0xe8c7b756, 12);
        MD5_قدم(F1, c, d, a, b, وچ[ 2]+0x242070db, 17);
        MD5_قدم(F1, b, c, d, a, وچ[ 3]+0xc1bdceee, 22);
        MD5_قدم(F1, a, b, c, d, وچ[ 4]+0xf57c0faf,  7);
        MD5_قدم(F1, d, a, b, c, وچ[ 5]+0x4787c62a, 12);
        MD5_قدم(F1, c, d, a, b, وچ[ 6]+0xa8304613, 17);
        MD5_قدم(F1, b, c, d, a, وچ[ 7]+0xfd469501, 22);
        MD5_قدم(F1, a, b, c, d, وچ[ 8]+0x698098d8,  7);
        MD5_قدم(F1, d, a, b, c, وچ[ 9]+0x8b44f7af, 12);
        MD5_قدم(F1, c, d, a, b, وچ[10]+0xffff5bb1, 17);
        MD5_قدم(F1, b, c, d, a, وچ[11]+0x895cd7be, 22);
        MD5_قدم(F1, a, b, c, d, وچ[12]+0x6b901122,  7);
        MD5_قدم(F1, d, a, b, c, وچ[13]+0xfd987193, 12);
        MD5_قدم(F1, c, d, a, b, وچ[14]+0xa679438e, 17);
        MD5_قدم(F1, b, c, d, a, وچ[15]+0x49b40821, 22);

        MD5_قدم(F2, a, b, c, d, وچ[ 1]+0xf61e2562,  5);
        MD5_قدم(F2, d, a, b, c, وچ[ 6]+0xc040b340,  9);
        MD5_قدم(F2, c, d, a, b, وچ[11]+0x265e5a51, 14);
        MD5_قدم(F2, b, c, d, a, وچ[ 0]+0xe9b6c7aa, 20);
        MD5_قدم(F2, a, b, c, d, وچ[ 5]+0xd62f105d,  5);
        MD5_قدم(F2, d, a, b, c, وچ[10]+0x02441453,  9);
        MD5_قدم(F2, c, d, a, b, وچ[15]+0xd8a1e681, 14);
        MD5_قدم(F2, b, c, d, a, وچ[ 4]+0xe7d3fbc8, 20);
        MD5_قدم(F2, a, b, c, d, وچ[ 9]+0x21e1cde6,  5);
        MD5_قدم(F2, d, a, b, c, وچ[14]+0xc33707d6,  9);
        MD5_قدم(F2, c, d, a, b, وچ[ 3]+0xf4d50d87, 14);
        MD5_قدم(F2, b, c, d, a, وچ[ 8]+0x455a14ed, 20);
        MD5_قدم(F2, a, b, c, d, وچ[13]+0xa9e3e905,  5);
        MD5_قدم(F2, d, a, b, c, وچ[ 2]+0xfcefa3f8,  9);
        MD5_قدم(F2, c, d, a, b, وچ[ 7]+0x676f02d9, 14);
        MD5_قدم(F2, b, c, d, a, وچ[12]+0x8d2a4c8a, 20);

        MD5_قدم(F3, a, b, c, d, وچ[ 5]+0xfffa3942,  4);
        MD5_قدم(F3, d, a, b, c, وچ[ 8]+0x8771f681, 11);
        MD5_قدم(F3, c, d, a, b, وچ[11]+0x6d9d6122, 16);
        MD5_قدم(F3, b, c, d, a, وچ[14]+0xfde5380c, 23);
        MD5_قدم(F3, a, b, c, d, وچ[ 1]+0xa4beea44,  4);
        MD5_قدم(F3, d, a, b, c, وچ[ 4]+0x4bdecfa9, 11);
        MD5_قدم(F3, c, d, a, b, وچ[ 7]+0xf6bb4b60, 16);
        MD5_قدم(F3, b, c, d, a, وچ[10]+0xbebfbc70, 23);
        MD5_قدم(F3, a, b, c, d, وچ[13]+0x289b7ec6,  4);
        MD5_قدم(F3, d, a, b, c, وچ[ 0]+0xeaa127fa, 11);
        MD5_قدم(F3, c, d, a, b, وچ[ 3]+0xd4ef3085, 16);
        MD5_قدم(F3, b, c, d, a, وچ[ 6]+0x04881d05, 23);
        MD5_قدم(F3, a, b, c, d, وچ[ 9]+0xd9d4d039,  4);
        MD5_قدم(F3, d, a, b, c, وچ[12]+0xe6db99e5, 11);
        MD5_قدم(F3, c, d, a, b, وچ[15]+0x1fa27cf8, 16);
        MD5_قدم(F3, b, c, d, a, وچ[ 2]+0xc4ac5665, 23);

        MD5_قدم(F4, a, b, c, d, وچ[ 0]+0xf4292244,  6);
        MD5_قدم(F4, d, a, b, c, وچ[ 7]+0x432aff97, 10);
        MD5_قدم(F4, c, d, a, b, وچ[14]+0xab9423a7, 15);
        MD5_قدم(F4, b, c, d, a, وچ[ 5]+0xfc93a039, 21);
        MD5_قدم(F4, a, b, c, d, وچ[12]+0x655b59c3,  6);
        MD5_قدم(F4, d, a, b, c, وچ[ 3]+0x8f0ccc92, 10);
        MD5_قدم(F4, c, d, a, b, وچ[10]+0xffeff47d, 15);
        MD5_قدم(F4, b, c, d, a, وچ[ 1]+0x85845dd1, 21);
        MD5_قدم(F4, a, b, c, d, وچ[ 8]+0x6fa87e4f,  6);
        MD5_قدم(F4, d, a, b, c, وچ[15]+0xfe2ce6e0, 10);
        MD5_قدم(F4, c, d, a, b, وچ[ 6]+0xa3014314, 15);
        MD5_قدم(F4, b, c, d, a, وچ[13]+0x4e0811a1, 21);
        MD5_قدم(F4, a, b, c, d, وچ[ 4]+0xf7537e82,  6);
        MD5_قدم(F4, d, a, b, c, وچ[11]+0xbd3af235, 10);
        MD5_قدم(F4, c, d, a, b, وچ[ 2]+0x2ad7d2bb, 15);
        MD5_قدم(F4, b, c, d, a, وچ[ 9]+0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
}

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
ساكن فراغ MD5Init(MD5Context *ctx){
        ctx->isInit = 1;
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;
        ctx->بٹ[0] = 0;
        ctx->بٹ[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
ساكن
فراغ MD5Update(MD5Context *ctx, ثابت بنانشانی حرف *buf, بنانشانی صحيح len){
        uint32 t;

        /* Update bitcount */

        t = ctx->بٹ[0];
        إذا ((ctx->بٹ[0] = t + ((uint32)len << 3)) < t)
                ctx->بٹ[1]++; /* Carry from low to high */
        ctx->بٹ[1] += len >> 29;

        t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

        /* Handle any leading odd-sized chunks */

        إذا ( t ) {
                بنانشانی حرف *p = (بنانشانی حرف *)ctx->وچ + t;

                t = 64-t;
                إذا (len < t) {
                        حافظہ_نقل(p, buf, len);
                        أرجع;
                }
                حافظہ_نقل(p, buf, t);
                byte_الٹ(ctx->وچ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->وچ);
                buf += t;
                len -= t;
        }

        /* Process data in 64-byte chunks */

        طالما (len >= 64) {
                حافظہ_نقل(ctx->وچ, buf, 64);
                byte_الٹ(ctx->وچ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->وچ);
                buf += 64;
                len -= 64;
        }

        /* Handle any remaining bytes of data. */

        حافظہ_نقل(ctx->وچ, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
ساكن فراغ MD5Final(بنانشانی حرف digest[16], MD5Context *ctx){
        بنانشانی گنتی;
        بنانشانی حرف *p;

        /* Compute number of bytes mod 64 */
        گنتی = (ctx->بٹ[0] >> 3) & 0x3F;

        /* Set the first char of padding to 0x80.  This is safe since there is
           always at least one byte free */
        p = ctx->وچ + گنتی;
        *p++ = 0x80;

        /* Bytes of padding needed to make 64 bytes */
        گنتی = 64 - 1 - گنتی;

        /* Pad out to 56 mod 64 */
        إذا (گنتی < 8) {
                /* Two lots of padding:  Pad the first block to 64 bytes */
                memset(p, 0, گنتی);
                byte_الٹ(ctx->وچ, 16);
                MD5Transform(ctx->buf, (uint32 *)ctx->وچ);

                /* Now fill the next block with 56 bytes */
                memset(ctx->وچ, 0, 56);
        } وإلا {
                /* Pad block to 56 bytes */
                memset(p, 0, گنتی-8);
        }
        byte_الٹ(ctx->وچ, 14);

        /* Append length in bits and transform */
        حافظہ_نقل(ctx->وچ + 14*4, ctx->بٹ, 8);

        MD5Transform(ctx->buf, (uint32 *)ctx->وچ);
        byte_الٹ((بنانشانی حرف *)ctx->buf, 4);
        حافظہ_نقل(digest, ctx->buf, 16);
}

/*
** Convert a 128-bit MD5 digest into a 32-digit base-16 number.
*/
ساكن فراغ MD5_Digest_منزل_Base16(بنانشانی حرف *digest, حرف *zBuf){
  ساكن حرف ثابت zEncode[] = "0123456789abcdef";
  صحيح i, j;

  لكل(j=i=0; i<16; i++){
    صحيح a = digest[i];
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
ساكن فراغ MD5_Digest_منزل_Base10x8(بنانشانی حرف digest[16], حرف zDigest[50]){
  صحيح i, j;
  بنانشانی صحيح x;
  لكل(i=j=0; i<16; i+=2){
    x = digest[i]*256 + digest[i+1];
    إذا( i>0 ) zDigest[j++] = '-';
    sqlite3_snprintf(50-j, &zDigest[j], "%05u", x);
    j += 5;
  }
  zDigest[j] = 0;
}

/*
** A TCL command for md5.  The argument is the text to be hashed.  The
** Result is the hash in base64.
*/
ساكن صحيح SQLITE_TCLAPI md5_cmd(
  فراغ*cd,
  Tcl_Interp *interp,
  صحيح argc,
  ثابت حرف **argv
){
  MD5Context ctx;
  بنانشانی حرف digest[16];
  حرف zBuf[50];
  فراغ (*converter)(بنانشانی حرف*, حرف*);

  إذا( argc!=2 ){
    Tcl_Append_نتیجہ(interp,"wrong # args: should be \"", argv[0],
        " TEXT\"", (حرف*)0);
    أرجع TCL_ERROR;
  }
  MD5Init(&ctx);
  MD5Update(&ctx, (بنانشانی حرف*)argv[1], (بنانشانی)لڑی_لمبائی(argv[1]));
  MD5Final(digest, &ctx);
  converter = (فراغ(*)(بنانشانی حرف*,حرف*))cd;
  converter(digest, zBuf);
  Tcl_Append_نتیجہ(interp, zBuf, (حرف*)0);
  أرجع TCL_OK;
}

/*
** A TCL command to take the md5 hash of a file.  The argument is the
** name of the file.
*/
ساكن صحيح SQLITE_TCLAPI md5file_cmd(
  فراغ*cd,
  Tcl_Interp *interp,
  صحيح argc,
  ثابت حرف **argv
){
  FILE *وچ;
  صحيح ofst;
  صحيح amt;
  MD5Context ctx;
  فراغ (*converter)(بنانشانی حرف*, حرف*);
  بنانشانی حرف digest[16];
  حرف zBuf[10240];

  إذا( argc!=2 && argc!=4 ){
    Tcl_Append_نتیجہ(interp,"wrong # args: should be \"", argv[0],
        " FILENAME [OFFSET AMT]\"", (حرف*)0);
    أرجع TCL_ERROR;
  }
  إذا( argc==4 ){
    ofst = atoi(argv[2]);
    amt = atoi(argv[3]);
  }وإلا{
    ofst = 0;
    amt = 2147483647;
  }
  وچ = fopen(argv[1],"rb");
  إذا( وچ==0 ){
    Tcl_Append_نتیجہ(interp,"unable to open file \"", argv[1],
         "\" for reading", (حرف*)0);
    أرجع TCL_ERROR;
  }
  fseek(وچ, ofst, SEEK_SET);
  MD5Init(&ctx);
  طالما( amt>0 ){
    صحيح n;
    n = (صحيح)fread(zBuf, 1, حجم(zBuf)<=amt ? حجم(zBuf) : amt, وچ);
    إذا( n<=0 ) اكسر;
    MD5Update(&ctx, (بنانشانی حرف*)zBuf, (بنانشانی)n);
    amt -= n;
  }
  fclose(وچ);
  MD5Final(digest, &ctx);
  converter = (فراغ(*)(بنانشانی حرف*,حرف*))cd;
  converter(digest, zBuf);
  Tcl_Append_نتیجہ(interp, zBuf, (حرف*)0);
  أرجع TCL_OK;
}

/*
** Register the four new TCL commands for generating MD5 checksums
** with the TCL interpreter.
*/
صحيح Md5_Init(Tcl_Interp *interp){
  Tcl_CreateCommand(interp, "md5", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_منزل_Base16, 0);
  Tcl_CreateCommand(interp, "md5-10x8", (Tcl_CmdProc*)md5_cmd,
                    MD5_Digest_منزل_Base10x8, 0);
  Tcl_CreateCommand(interp, "md5file", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_منزل_Base16, 0);
  Tcl_CreateCommand(interp, "md5file-10x8", (Tcl_CmdProc*)md5file_cmd,
                    MD5_Digest_منزل_Base10x8, 0);
  أرجع TCL_OK;
}

/*
** During testing, the special md5sum() aggregate function is available.
** inside SQLite.  The following routines implement that function.
*/
ساكن فراغ md5step(sqlite3_context *context, صحيح argc, sqlite3_قدر **argv){
  MD5Context *p;
  صحيح i;
  إذا( argc<1 ) أرجع;
  p = sqlite3_aggregate_context(context, حجم(*p));
  إذا( p==0 ) أرجع;
  إذا( !p->isInit ){
    MD5Init(p);
  }
  لكل(i=0; i<argc; i++){
    ثابت حرف *z_انکڑے = (حرف*)sqlite3_قدر_متن(argv[i]);
    إذا( z_انکڑے ){
      MD5Update(p, (بنانشانی حرف*)z_انکڑے, (صحيح)لڑی_لمبائی(z_انکڑے));
    }
  }
}
ساكن فراغ md5finalize(sqlite3_context *context){
  MD5Context *p;
  بنانشانی حرف digest[16];
  حرف zBuf[33];
  p = sqlite3_aggregate_context(context, حجم(*p));
  MD5Final(digest,p);
  MD5_Digest_منزل_Base16(digest, zBuf);
  sqlite3_نتیجہ_متن(context, zBuf, -1, SQLITE_TRANSIENT);
}
صحيح Md5_رجسٹر(
  sqlite3 *db,
  حرف **pz_Err_پیغام,
  ثابت sqlite3_api_routines *pThunk
){
  صحيح rc = sqlite3_create_function(db, "md5sum", -1, SQLITE_UTF8, 0, 0,
                                 md5step, md5finalize);
  sqlite3_overload_function(db, "md5sum", -1);  /* To exercise this API */
  أرجع rc;
}
