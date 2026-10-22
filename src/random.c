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
#अन्तर्भाव "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
स्थिर SQLITE_WSD संरचना sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 out[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
स्थिर शून्य chacha_block(u32 *out, ध्रुव u32 *in){
  पूर्णाङ्क i;
  u32 x[16];
  memcpy(x, in, 64);
  प्रति(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  प्रति(i=0; i<16; i++) out[i] = x[i]+in[i];
}

/*
** Return N random bytes.
*/
शून्य sqlite3_randomness(पूर्णाङ्क N, शून्य *pBuf){
  unsigned वर्ण *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#ifdef SQLITE_OMIT_WSD
  संरचना sqlite3PrngType *p = &GLOBAL(संरचना sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#अण्णहा
# define wsdPrng sqlite3Prng
#endif

#जइ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#endif

#ifndef SQLITE_OMIT_AUTOINIT
  जइ( sqlite3_initialize() ) प्रत्यागम;
#endif

#जइ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#endif

  sqlite3_mutex_enter(mutex);
  जइ( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    प्रत्यागम;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  जइ( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
    स्थिर ध्रुव u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    memcpy(&wsdPrng.s[0], chacha20_init, 16);
    जइ( NEVER(pVfs==0) ){
      memset(&wsdPrng.s[4], 0, 44);
    }अण्णहा{
      sqlite3OsRandomness(pVfs, 44, (वर्ण*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  प्रतिज्ञा( N>0 );
  जाव( 1 /* exit by break */ ){
    जइ( N<=wsdPrng.n ){
      memcpy(zBuf, &wsdPrng.out[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      भङ्ग;
    }
    जइ( wsdPrng.n>0 ){
      memcpy(zBuf, wsdPrng.out, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_block((u32*)wsdPrng.out, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#ifndef SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
स्थिर SQLITE_WSD संरचना sqlite3PrngType sqlite3SavedPrng;
शून्य sqlite3PrngSaveState(शून्य){
  memcpy(
    &GLOBAL(संरचना sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(संरचना sqlite3PrngType, sqlite3Prng),
    परिमाण(sqlite3Prng)
  );
}
शून्य sqlite3PrngRestoreState(शून्य){
  memcpy(
    &GLOBAL(संरचना sqlite3PrngType, sqlite3Prng),
    &GLOBAL(संरचना sqlite3PrngType, sqlite3SavedPrng),
    परिमाण(sqlite3Prng)
  );
}
#endif /* SQLITE_UNTESTABLE */
