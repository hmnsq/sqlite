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
#شامل "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
ساکن SQLITE_WSD ساخت sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 برآمد[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#تعریف_کرو ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#تعریف_کرو QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
ساکن خالی chacha_بلاک(u32 *برآمد, ثابت u32 *اندر){
  صحیح i;
  u32 x[16];
  حافظہ_نقل(x, اندر, 64);
  برائے(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  برائے(i=0; i<16; i++) برآمد[i] = x[i]+اندر[i];
}

/*
** Return N random bytes.
*/
خالی sqlite3_randomness(صحیح N, خالی *pBuf){
  غیرعلامتی حرف *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#اگر_تعریف_شدہ SQLITE_OMIT_WSD
  ساخت sqlite3PrngType *p = &GLOBAL(ساخت sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#ورنہ
# define wsdPrng sqlite3Prng
#اگر_ختم

#اگر SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس;
#اگر_ختم

#اگر SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#اگر_ختم

  sqlite3_mutex_enter(mutex);
  اگر( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    واپس;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  اگر( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_ڈھونڈو(0);
    ساکن ثابت u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    حافظہ_نقل(&wsdPrng.s[0], chacha20_init, 16);
    اگر( NEVER(pVfs==0) ){
      memset(&wsdPrng.s[4], 0, 44);
    }ورنہ{
      sqlite3OsRandomness(pVfs, 44, (حرف*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  أكد( N>0 );
  جب( 1 /* exit by break */ ){
    اگر( N<=wsdPrng.n ){
      حافظہ_نقل(zBuf, &wsdPrng.برآمد[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      توڑو;
    }
    اگر( wsdPrng.n>0 ){
      حافظہ_نقل(zBuf, wsdPrng.برآمد, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_بلاک((u32*)wsdPrng.برآمد, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#اگر_غیر_تعریف_شدہ SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
ساکن SQLITE_WSD ساخت sqlite3PrngType sqlite3SavedPrng;
خالی sqlite3PrngSaveState(خالی){
  حافظہ_نقل(
    &GLOBAL(ساخت sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(ساخت sqlite3PrngType, sqlite3Prng),
    حجم(sqlite3Prng)
  );
}
خالی sqlite3PrngRestoreState(خالی){
  حافظہ_نقل(
    &GLOBAL(ساخت sqlite3PrngType, sqlite3Prng),
    &GLOBAL(ساخت sqlite3PrngType, sqlite3SavedPrng),
    حجم(sqlite3Prng)
  );
}
#اگر_ختم /* SQLITE_UNTESTABLE */
