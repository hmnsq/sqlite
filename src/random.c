/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code to implement a pseudo-random number
** generator (PRNG) for SQLite.
**
** Random numbers are used by some of the database backends in order
** to generate random integer keys for tables or random filenames.
*/
#ضمن "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
ساكن SQLITE_WSD بنية sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 بیرون[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#تعریف_کن ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#تعریف_کن QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
ساكن فراغ chacha_بلوک(u32 *بیرون, ثابت u32 *درون){
  صحيح i;
  u32 x[16];
  کپی_حافظه(x, درون, 64);
  لكل(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  لكل(i=0; i<16; i++) بیرون[i] = x[i]+درون[i];
}

/*
** Return N random bytes.
*/
فراغ sqlite3_randomness(صحيح N, فراغ *pBuf){
  بیعلامت حرف *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#اگر_تعریف_شده SQLITE_OMIT_WSD
  بنية sqlite3PrngType *p = &GLOBAL(بنية sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#وإلا
# define wsdPrng sqlite3Prng
#پایان_اگر

#إذا SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع;
#پایان_اگر

#إذا SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#پایان_اگر

  sqlite3_mutex_enter(mutex);
  إذا( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    أرجع;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  إذا( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_پیدا_کن(0);
    ساكن ثابت u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    کپی_حافظه(&wsdPrng.s[0], chacha20_init, 16);
    إذا( NEVER(pVfs==0) ){
      memset(&wsdPrng.s[4], 0, 44);
    }وإلا{
      sqlite3OsRandomness(pVfs, 44, (حرف*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  أكد( N>0 );
  طالما( 1 /* exit by break */ ){
    إذا( N<=wsdPrng.n ){
      کپی_حافظه(zBuf, &wsdPrng.بیرون[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      اكسر;
    }
    إذا( wsdPrng.n>0 ){
      کپی_حافظه(zBuf, wsdPrng.بیرون, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_بلوک((u32*)wsdPrng.بیرون, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#اگر_تعریف_نشده SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
ساكن SQLITE_WSD بنية sqlite3PrngType sqlite3SavedPrng;
فراغ sqlite3PrngSaveState(فراغ){
  کپی_حافظه(
    &GLOBAL(بنية sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(بنية sqlite3PrngType, sqlite3Prng),
    حجم(sqlite3Prng)
  );
}
فراغ sqlite3PrngRestoreState(فراغ){
  کپی_حافظه(
    &GLOBAL(بنية sqlite3PrngType, sqlite3Prng),
    &GLOBAL(بنية sqlite3PrngType, sqlite3SavedPrng),
    حجم(sqlite3Prng)
  );
}
#پایان_اگر /* SQLITE_UNTESTABLE */
