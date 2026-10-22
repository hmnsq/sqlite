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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 ಹೊರಹೋಗು[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#ವ್ಯಾಖ್ಯಾನಿಸು QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
ಸ್ಥಿರ ಶೂನ್ಯ chacha_ಖಂಡ(u32 *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ u32 *ಒಳ){
  ಪೂರ್ಣಾಂಕ i;
  u32 x[16];
  ಸ್ಮರಣೆ_ನಕಲು(x, ಒಳ, 64);
  ಗಾಗಿ(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  ಗಾಗಿ(i=0; i<16; i++) ಹೊರಹೋಗು[i] = x[i]+ಒಳ[i];
}

/*
** Return N random bytes.
*/
ಶೂನ್ಯ sqlite3_randomness(ಪೂರ್ಣಾಂಕ N, ಶೂನ್ಯ *pBuf){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WSD
  ರಚನೆ sqlite3PrngType *p = &GLOBAL(ರಚನೆ sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#ಇಲ್ಲದಿದ್ದರೆ
# define wsdPrng sqlite3Prng
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_mutex_enter(mutex);
  ಆದರೆ( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    ಹಿಂತಿರುಗಿ;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  ಆದರೆ( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    ಸ್ಮರಣೆ_ನಕಲು(&wsdPrng.s[0], chacha20_init, 16);
    ಆದರೆ( NEVER(pVfs==0) ){
      memset(&wsdPrng.s[4], 0, 44);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3OsRandomness(pVfs, 44, (ಅಕ್ಷರ*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  ಪ್ರತಿಜ್ಞಾ( N>0 );
  ಆಗ( 1 /* exit by break */ ){
    ಆದರೆ( N<=wsdPrng.n ){
      ಸ್ಮರಣೆ_ನಕಲು(zBuf, &wsdPrng.ಹೊರಹೋಗು[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( wsdPrng.n>0 ){
      ಸ್ಮರಣೆ_ನಕಲು(zBuf, wsdPrng.ಹೊರಹೋಗು, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_ಖಂಡ((u32*)wsdPrng.ಹೊರಹೋಗು, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ sqlite3PrngType sqlite3SavedPrng;
ಶೂನ್ಯ sqlite3PrngSaveState(ಶೂನ್ಯ){
  ಸ್ಮರಣೆ_ನಕಲು(
    &GLOBAL(ರಚನೆ sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(ರಚನೆ sqlite3PrngType, sqlite3Prng),
    ಗಾತ್ರ(sqlite3Prng)
  );
}
ಶೂನ್ಯ sqlite3PrngRestoreState(ಶೂನ್ಯ){
  ಸ್ಮರಣೆ_ನಕಲು(
    &GLOBAL(ರಚನೆ sqlite3PrngType, sqlite3Prng),
    &GLOBAL(ರಚನೆ sqlite3PrngType, sqlite3SavedPrng),
    ಗಾತ್ರ(sqlite3Prng)
  );
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_UNTESTABLE */
