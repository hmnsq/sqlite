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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 ਬਾਹਰਲਾ[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
ਸਥਿਰ ਖਾਲੀ chacha_ਖੰਡ(u32 *ਬਾਹਰਲਾ, ਅਚਲ u32 *ਵਿੱਚ){
  ਪੂਰਨ i;
  u32 x[16];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(x, ਵਿੱਚ, 64);
  ਲਈ(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  ਲਈ(i=0; i<16; i++) ਬਾਹਰਲਾ[i] = x[i]+ਵਿੱਚ[i];
}

/*
** Return N random bytes.
*/
ਖਾਲੀ sqlite3_randomness(ਪੂਰਨ N, ਖਾਲੀ *pBuf){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
  ਬਣਤਰ sqlite3PrngType *p = &GLOBAL(ਬਣਤਰ sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#ਨਹੀਂਤਾਂ
# define wsdPrng sqlite3Prng
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ;
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#ਜੇ_ਅੰਤ

  sqlite3_mutex_enter(mutex);
  ਜੇ( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    ਵਾਪਸ;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  ਜੇ( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_ਲੱਭੋ(0);
    ਸਥਿਰ ਅਚਲ u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&wsdPrng.s[0], chacha20_init, 16);
    ਜੇ( NEVER(pVfs==0) ){
      memset(&wsdPrng.s[4], 0, 44);
    }ਨਹੀਂਤਾਂ{
      sqlite3OsRandomness(pVfs, 44, (ਅੱਖਰ*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( N>0 );
  ਜਦੋਂ( 1 /* exit by break */ ){
    ਜੇ( N<=wsdPrng.n ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, &wsdPrng.ਬਾਹਰਲਾ[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      ਰੋਕੋ;
    }
    ਜੇ( wsdPrng.n>0 ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, wsdPrng.ਬਾਹਰਲਾ, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_ਖੰਡ((u32*)wsdPrng.ਬਾਹਰਲਾ, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ sqlite3PrngType sqlite3SavedPrng;
ਖਾਲੀ sqlite3PrngSaveState(ਖਾਲੀ){
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(
    &GLOBAL(ਬਣਤਰ sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(ਬਣਤਰ sqlite3PrngType, sqlite3Prng),
    ਆਕਾਰ(sqlite3Prng)
  );
}
ਖਾਲੀ sqlite3PrngRestoreState(ਖਾਲੀ){
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(
    &GLOBAL(ਬਣਤਰ sqlite3PrngType, sqlite3Prng),
    &GLOBAL(ਬਣਤਰ sqlite3PrngType, sqlite3SavedPrng),
    ਆਕਾਰ(sqlite3Prng)
  );
}
#ਜੇ_ਅੰਤ /* SQLITE_UNTESTABLE */
