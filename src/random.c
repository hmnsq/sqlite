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
#అన్తర్భావ "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
స్థిర SQLITE_WSD నిర్మాణం sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 బహిర్గతం[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#నిర్వచించు ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#నిర్వచించు QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
స్థిర శూన్య chacha_ఖండం(u32 *బహిర్గతం, స్థిరరాశి u32 *లోపల){
  పూర్ణాంకం i;
  u32 x[16];
  జ్ఞాపకం_నకలు(x, లోపల, 64);
  కొరకు(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  కొరకు(i=0; i<16; i++) బహిర్గతం[i] = x[i]+లోపల[i];
}

/*
** Return N random bytes.
*/
శూన్య sqlite3_randomness(పూర్ణాంకం N, శూన్య *pBuf){
  గుర్తులేని అక్షరం *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#నిర్వచితమైతే SQLITE_OMIT_WSD
  నిర్మాణం sqlite3PrngType *p = &GLOBAL(నిర్మాణం sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#లేకపోతే
# define wsdPrng sqlite3Prng
#అయితే_ముగింపు

#ఒకవేళ SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి;
#అయితే_ముగింపు

#ఒకవేళ SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#అయితే_ముగింపు

  sqlite3_mutex_enter(mutex);
  ఒకవేళ( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    తిరిగి;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  ఒకవేళ( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_కనుగొను(0);
    స్థిర స్థిరరాశి u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    జ్ఞాపకం_నకలు(&wsdPrng.s[0], chacha20_init, 16);
    ఒకవేళ( NEVER(pVfs==0) ){
      memset(&wsdPrng.s[4], 0, 44);
    }లేకపోతే{
      sqlite3OsRandomness(pVfs, 44, (అక్షరం*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  ప్రతిజ్ఞా( N>0 );
  అయితే( 1 /* exit by break */ ){
    ఒకవేళ( N<=wsdPrng.n ){
      జ్ఞాపకం_నకలు(zBuf, &wsdPrng.బహిర్గతం[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      ఆపు;
    }
    ఒకవేళ( wsdPrng.n>0 ){
      జ్ఞాపకం_నకలు(zBuf, wsdPrng.బహిర్గతం, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_ఖండం((u32*)wsdPrng.బహిర్గతం, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
స్థిర SQLITE_WSD నిర్మాణం sqlite3PrngType sqlite3SavedPrng;
శూన్య sqlite3PrngSaveState(శూన్య){
  జ్ఞాపకం_నకలు(
    &GLOBAL(నిర్మాణం sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(నిర్మాణం sqlite3PrngType, sqlite3Prng),
    పరిమాణం(sqlite3Prng)
  );
}
శూన్య sqlite3PrngRestoreState(శూన్య){
  జ్ఞాపకం_నకలు(
    &GLOBAL(నిర్మాణం sqlite3PrngType, sqlite3Prng),
    &GLOBAL(నిర్మాణం sqlite3PrngType, sqlite3SavedPrng),
    పరిమాణం(sqlite3Prng)
  );
}
#అయితే_ముగింపు /* SQLITE_UNTESTABLE */
