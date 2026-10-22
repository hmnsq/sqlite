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
#समावेश "sqliteInt.h"


/* All threads share a single random number generator.
** This structure is the current state of the generator.
*/
अटल SQLITE_WSD काष्ठा sqlite3PrngType {
  u32 s[16];                 /* 64 bytes of chacha20 state */
  u8 out[64];                /* Output bytes */
  u8 n;                      /* Output bytes remaining */
} sqlite3Prng;


/* The RFC-7539 ChaCha20 block function
*/
#घोषणा ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#घोषणा QR(a, b, c, d) ( \
    a += b, d ^= a, d = ROTL(d,16), \
    c += d, b ^= c, b = ROTL(b,12), \
    a += b, d ^= a, d = ROTL(d, 8), \
    c += d, b ^= c, b = ROTL(b, 7))
अटल व्योम chacha_block(u32 *out, स्थिर u32 *in){
  पूर्णांक i;
  u32 x[16];
  स_नकल(x, in, 64);
  क्रम(i=0; i<10; i++){
    QR(x[0], x[4], x[ 8], x[12]);
    QR(x[1], x[5], x[ 9], x[13]);
    QR(x[2], x[6], x[10], x[14]);
    QR(x[3], x[7], x[11], x[15]);
    QR(x[0], x[5], x[10], x[15]);
    QR(x[1], x[6], x[11], x[12]);
    QR(x[2], x[7], x[ 8], x[13]);
    QR(x[3], x[4], x[ 9], x[14]);
  }
  क्रम(i=0; i<16; i++) out[i] = x[i]+in[i];
}

/*
** Return N random bytes.
*/
व्योम sqlite3_randomness(पूर्णांक N, व्योम *pBuf){
  अचिन्हित अक्षर *zBuf = pBuf;

  /* The "wsdPrng" macro will resolve to the pseudo-random number generator
  ** state vector.  If writable static data is unsupported on the target,
  ** we have to locate the state vector at run-time.  In the more common
  ** case where writable static data is supported, wsdPrng can refer directly
  ** to the "sqlite3Prng" state vector declared above.
  */
#अगर_घोषित SQLITE_OMIT_WSD
  काष्ठा sqlite3PrngType *p = &GLOBAL(काष्ठा sqlite3PrngType, sqlite3Prng);
# define wsdPrng p[0]
#अन्यथा
# define wsdPrng sqlite3Prng
#पूर्ण_अगर

#अगर SQLITE_THREADSAFE
  sqlite3_mutex *mutex;
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस;
#पूर्ण_अगर

#अगर SQLITE_THREADSAFE
  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);
#पूर्ण_अगर

  sqlite3_mutex_enter(mutex);
  अगर( N<=0 || pBuf==0 ){
    wsdPrng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    वापस;
  }

  /* Initialize the state of the random number generator once,
  ** the first time this routine is called.
  */
  अगर( wsdPrng.s[0]==0 ){
    sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
    अटल स्थिर u32 chacha20_init[] = {
      0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
    };
    स_नकल(&wsdPrng.s[0], chacha20_init, 16);
    अगर( NEVER(pVfs==0) ){
      स_रखो(&wsdPrng.s[4], 0, 44);
    }अन्यथा{
      sqlite3OsRandomness(pVfs, 44, (अक्षर*)&wsdPrng.s[4]);
    }
    wsdPrng.s[15] = wsdPrng.s[12];
    wsdPrng.s[12] = 0;
    wsdPrng.n = 0;
  }

  निश्चित( N>0 );
  जबतक( 1 /* exit by break */ ){
    अगर( N<=wsdPrng.n ){
      स_नकल(zBuf, &wsdPrng.out[wsdPrng.n-N], N);
      wsdPrng.n -= N;
      अवरोध;
    }
    अगर( wsdPrng.n>0 ){
      स_नकल(zBuf, wsdPrng.out, wsdPrng.n);
      N -= wsdPrng.n;
      zBuf += wsdPrng.n;
    }
    wsdPrng.s[12]++;
    chacha_block((u32*)wsdPrng.out, wsdPrng.s);
    wsdPrng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}

#अगर_अघोषित SQLITE_UNTESTABLE
/*
** For testing purposes, we sometimes want to preserve the state of
** PRNG and restore the PRNG to its saved state at a later time, or
** to reset the PRNG to its initial state.  These routines accomplish
** those tasks.
**
** The sqlite3_test_control() interface calls these routines to
** control the PRNG.
*/
अटल SQLITE_WSD काष्ठा sqlite3PrngType sqlite3SavedPrng;
व्योम sqlite3PrngSaveState(व्योम){
  स_नकल(
    &GLOBAL(काष्ठा sqlite3PrngType, sqlite3SavedPrng),
    &GLOBAL(काष्ठा sqlite3PrngType, sqlite3Prng),
    माप(sqlite3Prng)
  );
}
व्योम sqlite3PrngRestoreState(व्योम){
  स_नकल(
    &GLOBAL(काष्ठा sqlite3PrngType, sqlite3Prng),
    &GLOBAL(काष्ठा sqlite3PrngType, sqlite3SavedPrng),
    माप(sqlite3Prng)
  );
}
#पूर्ण_अगर /* SQLITE_UNTESTABLE */
