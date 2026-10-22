/*
** 2005-07-08
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code associated with the ANALYZE command.
**
** The ANALYZE command gather statistics about the content of tables
** and indices.  These statistics are made available to the query planner
** to help it make better decisions about how to perform queries.
**
** The following system tables are or have been supported:
**
**    CREATE TABLE sqlite_stat1(tbl, idx, stat);
**    CREATE TABLE sqlite_stat2(tbl, idx, sampleno, sample);
**    CREATE TABLE sqlite_stat3(tbl, idx, nEq, nLt, nDLt, sample);
**    CREATE TABLE sqlite_stat4(tbl, idx, nEq, nLt, nDLt, sample);
**
** Additional tables might be added in future releases of SQLite.
** The sqlite_stat2 table is not created or used unless the SQLite version
** is between 3.6.18 and 3.7.8, inclusive, and unless SQLite is compiled
** with SQLITE_ENABLE_STAT2.  The sqlite_stat2 table is deprecated.
** The sqlite_stat2 table is superseded by sqlite_stat3, which is only
** created and used by SQLite versions 3.7.9 through 3.29.0 when
** SQLITE_ENABLE_STAT3 defined.  The functionality of sqlite_stat3
** is a superset of sqlite_stat2 and is also now deprecated.  The
** sqlite_stat4 is an enhanced version of sqlite_stat3 and is only 
** available when compiled with SQLITE_ENABLE_STAT4 and in SQLite
** versions 3.8.1 and later.  STAT4 is the only variant that is still
** supported.
**
** For most applications, sqlite_stat1 provides all the statistics required
** for the query planner to make good choices.
**
** Format of sqlite_stat1:
**
** There is normally one row per index, with the index identified by the
** name in the idx column.  The tbl column is the name of the table to
** which the index belongs.  In each such row, the stat column will be
** a string consisting of a list of integers.  The first integer in this
** list is the number of rows in the index.  (This is the same as the
** number of rows in the table, except for partial indices.)  The second
** integer is the average number of rows in the index that have the same
** value in the first column of the index.  The third integer is the average
** number of rows in the index that have the same value for the first two
** columns.  The N-th integer (for N>1) is the average number of rows in 
** the index which have the same value for the first N-1 columns.  For
** a K-column index, there will be K+1 integers in the stat column.  If
** the index is unique, then the last integer will be 1.
**
** The list of integers in the stat column can optionally be followed
** by the keyword "unordered".  The "unordered" keyword, if it is present,
** must be separated from the last integer by a single space.  If the
** "unordered" keyword is present, then the query planner assumes that
** the index is unordered and will not use the index for a range query.
** 
** If the sqlite_stat1.idx column is NULL, then the sqlite_stat1.stat
** column contains a single integer which is the (estimated) number of
** rows in the table identified by sqlite_stat1.tbl.
**
** Format of sqlite_stat2:
**
** The sqlite_stat2 is only created and is only used if SQLite is compiled
** with SQLITE_ENABLE_STAT2 and if the SQLite version number is between
** 3.6.18 and 3.7.8.  The "stat2" table contains additional information
** about the distribution of keys within an index.  The index is identified by
** the "idx" column and the "tbl" column is the name of the table to which
** the index belongs.  There are usually 10 rows in the sqlite_stat2
** table for each index.
**
** The sqlite_stat2 entries for an index that have sampleno between 0 and 9
** inclusive are samples of the left-most key value in the index taken at
** evenly spaced points along the index.  Let the number of samples be S
** (10 in the standard build) and let C be the number of rows in the index.
** Then the sampled rows are given by:
**
**     rownumber = (i*C*2 + C)/(S*2)
**
** For i between 0 and S-1.  Conceptually, the index space is divided into
** S uniform buckets and the samples are the middle row from each bucket.
**
** The format for sqlite_stat2 is recorded here for legacy reference.  This
** version of SQLite does not support sqlite_stat2.  It neither reads nor
** writes the sqlite_stat2 table.  This version of SQLite only supports
** sqlite_stat3.
**
** Format for sqlite_stat3:
**
** The sqlite_stat3 format is a subset of sqlite_stat4.  Hence, the
** sqlite_stat4 format will be described first.  Further information
** about sqlite_stat3 follows the sqlite_stat4 description.
**
** Format for sqlite_stat4:
**
** As with sqlite_stat2, the sqlite_stat4 table contains histogram data
** to aid the query planner in choosing good indices based on the values
** that indexed columns are compared against in the WHERE clauses of
** queries.
**
** The sqlite_stat4 table contains multiple entries for each index.
** The idx column names the index and the tbl column is the table of the
** index.  If the idx and tbl columns are the same, then the sample is
** of the INTEGER PRIMARY KEY.  The sample column is a blob which is the
** binary encoding of a key from the index.  The nEq column is a
** list of integers.  The first integer is the approximate number
** of entries in the index whose left-most column exactly matches
** the left-most column of the sample.  The second integer in nEq
** is the approximate number of entries in the index where the
** first two columns match the first two columns of the sample.
** And so forth.  nLt is another list of integers that show the approximate
** number of entries that are strictly less than the sample.  The first
** integer in nLt contains the number of entries in the index where the
** left-most column is less than the left-most column of the sample.
** The K-th integer in the nLt entry is the number of index entries 
** where the first K columns are less than the first K columns of the
** sample.  The nDLt column is like nLt except that it contains the 
** number of distinct entries in the index that are less than the
** sample.
**
** There can be an arbitrary number of sqlite_stat4 entries per index.
** The ANALYZE command will typically generate sqlite_stat4 tables
** that contain between 10 and 40 samples which are distributed across
** the key space, though not uniformly, and which include samples with
** large nEq values.
**
** Format for sqlite_stat3 redux:
**
** The sqlite_stat3 table is like sqlite_stat4 except that it only
** looks at the left-most column of the index.  The sqlite_stat3.sample
** column contains the actual value of the left-most column instead
** of a blob encoding of the complete index key as is found in
** sqlite_stat4.sample.  The nEq, nLt, and nDLt entries of sqlite_stat3
** all contain just a single integer which is the same as the first
** integer in the equivalent columns in sqlite_stat4.
*/
#ifndef SQLITE_OMIT_ANALYZE
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ಯದಿ defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#ಅನ್ಯಥಾ
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#endif

/*
** This routine generates code that opens the sqlite_statN tables.
** The sqlite_stat1 table is always relevant.  sqlite_stat2 is now
** obsolete.  sqlite_stat3 and sqlite_stat4 are only opened when
** appropriate compile-time options are provided.
**
** If the sqlite_statN tables do not previously exist, it is created.
**
** Argument zWhere may be a pointer to a buffer containing a table name,
** or it may be a NULL pointer. If it is not NULL, then all entries in
** the sqlite_statN tables associated with the named table are deleted.
** If zWhere==0, then code is generated to delete all stat table entries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ openStatTable(
  Parse *pParse,          /* Parsing context */
  ಪೂರ್ಣಾಙ್ಕ iDb,                /* The database we are looking in */
  ಪೂರ್ಣಾಙ್ಕ iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  ಧ್ರುವ ವರ್ಣ *zWhere,     /* Delete entries for this table or index */
  ಧ್ರುವ ವರ್ಣ *zWhereType  /* Either "tbl" or "idx" */
){
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
    ಧ್ರುವ ವರ್ಣ *zName;
    ಧ್ರುವ ವರ್ಣ *zCols;
  } aTable[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#ಯದಿ defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#ಅನ್ಯಥಾ
    { "sqlite_stat4", 0 },
#endif
    { "sqlite_stat3", 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_ಬೇರು[ArraySize(aTable)];
  u8 aCreateTbl[ArraySize(aTable)];
#ifdef SQLITE_ENABLE_STAT4
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ nToOpen = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#ಅನ್ಯಥಾ
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ nToOpen = 1;
#endif

  ಯದಿ( v==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  ಪ್ರತಿ(i=0; i<ArraySize(aTable); i++){
    ಧ್ರುವ ವರ್ಣ *zTab = aTable[i].zName;
    Table *pStat;
    aCreateTbl[i] = 0;
    ಯದಿ( (pStat = sqlite3FindTable(db, zTab, pDb->zDbSName))==0 ){
      ಯದಿ( i<nToOpen ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, aTable[i].zCols
        );
        ಪ್ರತಿಜ್ಞಾ( pParse->isCreate || pParse->nErr );
        a_ಬೇರು[i] = (u32)pParse->u1.cr.reg_ಬೇರು;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }ಅನ್ಯಥಾ{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_ಬೇರು[i] = pStat->tnum;
      sqlite3TableLock(pParse, iDb, a_ಬೇರು[i], 1, zTab);
      ಯದಿ( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
      }ಅನ್ಯಥಾ ಯದಿ( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#endif
      }ಅನ್ಯಥಾ{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3VdbeAddOp2(v, OP_Clear, (ಪೂರ್ಣಾಙ್ಕ)a_ಬೇರು[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  ಪ್ರತಿ(i=0; i<nToOpen; i++){
    ಪ್ರತಿಜ್ಞಾ( i<ArraySize(aTable) );
    sqlite3VdbeAddOp4Int(v, OP_OpenWrite, iStatCur+i, (ಪೂರ್ಣಾಙ್ಕ)a_ಬೇರು[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, aTable[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#ifndef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#endif

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ StatAccum StatAccum;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ StatSample StatSample;
ಸಂರಚನಾ StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#ifdef SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  ಸಙ್ಘ {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  ಪೂರ್ಣಾಙ್ಕ iCol;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#endif
};                                                    
ಸಂರಚನಾ StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt nRow;             /* Number of rows visited so far */
  ಪೂರ್ಣಾಙ್ಕ nLimit;               /* Analysis row-scan limit */
  ಪೂರ್ಣಾಙ್ಕ nCol;                 /* Number of columns in index + pk/rowid */
  ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ_Col;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#ifdef SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  ಪೂರ್ಣಾಙ್ಕ mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *aBest;        /* Array of nCol best samples */
  ಪೂರ್ಣಾಙ್ಕ iMin;                 /* Index in a[] of entry with minimum score */
  ಪೂರ್ಣಾಙ್ಕ nSample;              /* Current number of samples */
  ಪೂರ್ಣಾಙ್ಕ nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  ಪೂರ್ಣಾಙ್ಕ iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#endif
};

/* Reclaim memory used by a StatSample
*/
#ifdef SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sampleClear(sqlite3 *db, StatSample *p){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#endif

/* Initialize the BLOB value of a ROWID
*/
#ifdef SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sampleSetRowid(sqlite3 *db, StatSample *p, ಪೂರ್ಣಾಙ್ಕ n, ಧ್ರುವ u8 *pData){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  ಯದಿ( p->u.aRowid ){
    p->nRowid = n;
    memcpy(p->u.aRowid, pData, n);
  }ಅನ್ಯಥಾ{
    p->nRowid = 0;
  }
}
#endif

/* Initialize the INTEGER value of a ROWID.
*/
#ifdef SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sampleSetRowidInt64(sqlite3 *db, StatSample *p, i64 iRowid){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯದಿ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#endif


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#ifdef SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sampleCopy(StatAccum *p, StatSample *pTo, StatSample *pFrom){
  pTo->isPSample = pFrom->isPSample;
  pTo->iCol = pFrom->iCol;
  pTo->iHash = pFrom->iHash;
  memcpy(pTo->anEq, pFrom->anEq, ಪರಿಮಾಣ(tRowcnt)*p->nCol);
  memcpy(pTo->anLt, pFrom->anLt, ಪರಿಮಾಣ(tRowcnt)*p->nCol);
  memcpy(pTo->anDLt, pFrom->anDLt, ಪರಿಮಾಣ(tRowcnt)*p->nCol);
  ಯದಿ( pFrom->nRowid ){
    sampleSetRowid(p->db, pTo, pFrom->nRowid, pFrom->u.aRowid);
  }ಅನ್ಯಥಾ{
    sampleSetRowidInt64(p->db, pTo, pFrom->u.iRowid);
  }
}
#endif

/*
** Reclaim all memory of a StatAccum structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ statAccumDestructor(ಶೂನ್ಯ *pOld){
  StatAccum *p = (StatAccum*)pOld;
#ifdef SQLITE_ENABLE_STAT4
  ಯದಿ( p->mxSample ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<p->nCol; i++) sampleClear(p->db, p->aBest+i);
    ಪ್ರತಿ(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#endif
  sqlite3DbFree(p->db, p);
}

/*
** Implementation of the stat_init(N,K,C,L) SQL function. The four parameters
** are:
**     N:    The number of columns in the index including the rowid/pk (note 1)
**     K:    The number of columns in the index excluding the rowid/pk.
**     C:    Estimated number of rows in the index
**     L:    A limit on the number of rows to scan, or 0 for no-limit 
**
** Note 1:  In the special case of the covering index that implements a
** WITHOUT ROWID table, N is the number of PRIMARY KEY columns, not the
** total number of columns in the table.
**
** For indexes on ordinary rowid tables, N==K+1.  But for indexes on
** WITHOUT ROWID tables, N=K+P where P is the number of columns in the
** PRIMARY KEY of the table.  The covering index that implements the
** original WITHOUT ROWID table as N==K as a special case.
**
** This routine allocates the StatAccum object in heap memory. The return 
** value is a pointer to the StatAccum object.  The datatype of the
** return value is BLOB, but it is really just a pointer to the StatAccum
** object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ statInit(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  StatAccum *p;
  ಪೂರ್ಣಾಙ್ಕ nCol;                       /* Number of columns in index being sampled */
  ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ_Col;                    /* Number of key columns */
  ಪೂರ್ಣಾಙ್ಕ nColUp;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#ifdef SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  ಪೂರ್ಣಾಙ್ಕ mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#endif

  /* Decode the three function arguments */
  UNUSED_PARAMETER(argc);
  nCol = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( nCol>0 );
  nColUp = ಪರಿಮಾಣ(tRowcnt)<8 ? (nCol+1)&~1 : nCol;
  n_ಕೀಲಿ_Col = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ_Col<=nCol );
  ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ_Col>0 );

  /* Allocate the space required for the StatAccum object */
  n = ಪರಿಮಾಣ(*p) 
    + ಪರಿಮಾಣ(tRowcnt)*nColUp;                    /* StatAccum.anDLt */
#ifdef SQLITE_ENABLE_STAT4
  n += ಪರಿಮಾಣ(tRowcnt)*nColUp;                   /* StatAccum.anEq */
  ಯದಿ( mxSample ){
    n += ಪರಿಮಾಣ(tRowcnt)*nColUp                  /* StatAccum.anLt */
      + ಪರಿಮಾಣ(StatSample)*(nCol+mxSample)       /* StatAccum.aBest[], a[] */
      + ಪರಿಮಾಣ(tRowcnt)*3*nColUp*(nCol+mxSample);
  }
#endif
  p = sqlite3DbMallocZero(db, n);
  ಯದಿ( p==0 ){
    sqlite3_result_error_nomem(context);
    ಪ್ರತ್ಯಾಗಮ;
  }

  p->db = db;
  p->nEst = sqlite3_ಮೌಲ್ಯ_int64(argv[2]);
  p->nRow = 0;
  p->nLimit = sqlite3_ಮೌಲ್ಯ_int(argv[3]);
  p->nCol = nCol;
  p->n_ಕೀಲಿ_Col = n_ಕೀಲಿ_Col;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#ifdef SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[nColUp];
  p->mxSample = p->nLimit==0 ? mxSample : 0;
  ಯದಿ( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    ಪೂರ್ಣಾಙ್ಕ i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[nColUp];
    p->iPrn = 0x689e962d*(u32)nCol ^ 0xd0944565*(u32)sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (ಸಂರಚನಾ StatSample*)&p->current.anLt[nColUp];
    p->aBest = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+nCol]);
    ಪ್ರತಿ(i=0; i<(mxSample+nCol); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (ಪರಿಮಾಣ(tRowcnt) * nColUp);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (ಪರಿಮಾಣ(tRowcnt) * nColUp);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (ಪರಿಮಾಣ(tRowcnt) * nColUp);
    }
    ಪ್ರತಿಜ್ಞಾ( (pSpace - (u8*)p)==n );
  
    ಪ್ರತಿ(i=0; i<nCol; i++){
      p->aBest[i].iCol = i;
    }
  }
#endif

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_result_blob(context, p, ಪರಿಮಾಣ(*p), statAccumDestructor);
}
ಸ್ಥಿರ ಧ್ರುವ FuncDef statInitFuncdef = {
  4,               /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  statInit,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_init",     /* zName */
  {0}
};

#ifdef SQLITE_ENABLE_STAT4
/*
** pNew and pOld are both candidate non-periodic samples selected for 
** the same column (pNew->iCol==pOld->iCol). Ignoring this column and 
** considering only any trailing columns and the sample hash value, this
** function returns true if sample pNew is to be preferred over pOld.
** In other words, if we assume that the cardinalities of the selected
** column for pNew and pOld are equal, is pNew to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol+1] onwards are valid. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  ಪೂರ್ಣಾಙ್ಕ nCol = pAccum->nCol;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( pNew->iCol==pOld->iCol );
  ಪ್ರತಿ(i=pNew->iCol+1; i<nCol; i++){
    ಯದಿ( pNew->anEq[i]>pOld->anEq[i] ) ಪ್ರತ್ಯಾಗಮ 1;
    ಯದಿ( pNew->anEq[i]<pOld->anEq[i] ) ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( pNew->iHash>pOld->iHash ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

#ifdef SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->iCol];
  tRowcnt nEqOld = pOld->anEq[pOld->iCol];

  ಪ್ರತಿಜ್ಞಾ( pOld->isPSample==0 && pNew->isPSample==0 );
  ಪ್ರತಿಜ್ಞಾ( IsStat4 || (pNew->iCol==0 && pOld->iCol==0) );

  ಯದಿ( (nEqNew>nEqOld) ) ಪ್ರತ್ಯಾಗಮ 1;
  ಯದಿ( nEqNew==nEqOld ){
    ಯದಿ( pNew->iCol<pOld->iCol ) ಪ್ರತ್ಯಾಗಮ 1;
    ಪ್ರತ್ಯಾಗಮ (pNew->iCol==pOld->iCol && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sample_ಸೇರಿಸು(StatAccum *p, StatSample *pNew, ಪೂರ್ಣಾಙ್ಕ nEqZero){
  StatSample *pSample = 0;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿಜ್ಞಾ( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  ಯದಿ( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  ಯದಿ( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    ಪ್ರತಿಜ್ಞಾ( pNew->anEq[pNew->iCol]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    ಪ್ರತಿ(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      ಯದಿ( pOld->anEq[pNew->iCol]==0 ){
        ಯದಿ( pOld->isPSample ) ಪ್ರತ್ಯಾಗಮ;
        ಪ್ರತಿಜ್ಞಾ( pOld->iCol>pNew->iCol );
        ಪ್ರತಿಜ್ಞಾ( sampleIsBetter(p, pNew, pOld) );
        ಯದಿ( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    ಯದಿ( pUpgrade ){
      pUpgrade->iCol = pNew->iCol;
      pUpgrade->anEq[pUpgrade->iCol] = pNew->anEq[pUpgrade->iCol];
      ಗಚ್ಛ find_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  ಯದಿ( p->nSample>=p->mxSample ){
    StatSample *pMin = &p->a[p->iMin];
    tRowcnt *anEq = pMin->anEq;
    tRowcnt *anLt = pMin->anLt;
    tRowcnt *anDLt = pMin->anDLt;
    sampleClear(p->db, pMin);
    memmove(pMin, &pMin[1], ಪರಿಮಾಣ(p->a[0])*(p->nSample-p->iMin-1));
    pSample = &p->a[p->nSample-1];
    pSample->nRowid = 0;
    pSample->anEq = anEq;
    pSample->anDLt = anDLt;
    pSample->anLt = anLt;
    p->nSample = p->mxSample-1;
  }

  /* The "rows less-than" for the rowid column must be greater than that
  ** for the last sample in the p->a[] array. Otherwise, the samples would
  ** be out of order. */
  ಪ್ರತಿಜ್ಞಾ( p->nSample==0 
       || pNew->anLt[p->nCol-1] > p->a[p->nSample-1].anLt[p->nCol-1] );

  /* Insert the new sample */
  pSample = &p->a[p->nSample];
  sampleCopy(p, pSample, pNew);
  p->nSample++;

  /* Zero the first nEqZero entries in the anEq[] array. */
  memset(pSample->anEq, 0, ಪರಿಮಾಣ(tRowcnt)*nEqZero);

find_new_min:
  ಯದಿ( p->nSample>=p->mxSample ){
    ಪೂರ್ಣಾಙ್ಕ iMin = -1;
    ಪ್ರತಿ(i=0; i<p->mxSample; i++){
      ಯದಿ( p->a[i].isPSample ) ಅನುವರ್ತ;
      ಯದಿ( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( iMin>=0 );
    p->iMin = iMin;
  }
}
#endif /* SQLITE_ENABLE_STAT4 */

#ifdef SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sample_ತಳ್ಳು_Previous(StatAccum *p, ಪೂರ್ಣಾಙ್ಕ iChng){
  ಪೂರ್ಣಾಙ್ಕ i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  ಪ್ರತಿ(i=(p->nCol-2); i>=iChng; i--){
    StatSample *pBest = &p->aBest[i];
    pBest->anEq[i] = p->current.anEq[i];
    ಯದಿ( p->nSample<p->mxSample || sampleIsBetter(p, pBest, &p->a[p->iMin]) ){
      sample_ಸೇರಿಸು(p, pBest, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  ಪ್ರತಿ(i=p->nSample-1; i>=0; i--){
    ಪೂರ್ಣಾಙ್ಕ j;
    ಪ್ರತಿ(j=p->nMaxEqZero; j<p->nCol; j++) ಪ್ರತಿಜ್ಞಾ( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  ಯದಿ( iChng<p->nMaxEqZero ){
    ಪ್ರತಿ(i=p->nSample-1; i>=0; i--){
      ಪೂರ್ಣಾಙ್ಕ j;
      ಪ್ರತಿ(j=iChng; j<p->nCol; j++){
        ಯದಿ( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#endif /* SQLITE_ENABLE_STAT4 */

/*
** Implementation of the stat_push SQL function:  stat_push(P,C,R)
** Arguments:
**
**    P     Pointer to the StatAccum object created by stat_init()
**    C     Index of left-most column to differ from previous row
**    R     Rowid for the current row.  Might be a key record for
**          WITHOUT ROWID tables.
**
** The purpose of this routine is to collect statistical data and/or
** samples from the index being analyzed into the StatAccum object.
** The stat_get() SQL function will be used afterwards to
** retrieve the information gathered.
**
** This SQL function usually returns NULL, but might return an integer
** if it wants the byte-code to do special processing.
**
** The R parameter is only used for STAT4
*/
ಸ್ಥಿರ ಶೂನ್ಯ stat_ತಳ್ಳು(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
  ಪೂರ್ಣಾಙ್ಕ iChng = sqlite3_ಮೌಲ್ಯ_int(argv[1]);

  UNUSED_PARAMETER( argc );
  UNUSED_PARAMETER( context );
  ಪ್ರತಿಜ್ಞಾ( p->nCol>0 );
  ಪ್ರತಿಜ್ಞಾ( iChng<p->nCol );

  ಯದಿ( p->nRow==0 ){
    /* This is the first call to this function. Do initialization. */
#ifdef SQLITE_ENABLE_STAT4
    ಪ್ರತಿ(i=0; i<p->nCol; i++) p->current.anEq[i] = 1;
#endif
  }ಅನ್ಯಥಾ{
    /* Second and subsequent calls get processed here */
#ifdef SQLITE_ENABLE_STAT4
    ಯದಿ( p->mxSample ) sample_ತಳ್ಳು_Previous(p, iChng);
#endif

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#ifdef SQLITE_ENABLE_STAT4
    ಪ್ರತಿ(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#endif
    ಪ್ರತಿ(i=iChng; i<p->nCol; i++){
      p->current.anDLt[i]++;
#ifdef SQLITE_ENABLE_STAT4
      ಯದಿ( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#endif
    }
  }

  p->nRow++;
#ifdef SQLITE_ENABLE_STAT4
  ಯದಿ( p->mxSample ){
    tRowcnt nLt;
    ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(argv[2])==SQLITE_INTEGER ){
      sampleSetRowidInt64(p->db, &p->current, sqlite3_ಮೌಲ್ಯ_int64(argv[2]));
    }ಅನ್ಯಥಾ{
      sampleSetRowid(p->db, &p->current, sqlite3_ಮೌಲ್ಯ_bytes(argv[2]),
                                         sqlite3_ಮೌಲ್ಯ_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->nCol-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    ಯದಿ( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.iCol = 0;
      sample_ಸೇರಿಸು(p, &p->current, p->nCol-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    ಪ್ರತಿ(i=0; i<(p->nCol-1); i++){
      p->current.iCol = i;
      ಯದಿ( i>=iChng || sampleIsBetterPost(p, &p->current, &p->aBest[i]) ){
        sampleCopy(p, &p->aBest[i], &p->current);
      }
    }
  }ಅನ್ಯಥಾ
#endif
  ಯದಿ( p->nLimit && p->nRow>(tRowcnt)p->nLimit*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_result_int(context, p->current.anDLt[0]>0);
  }
}

ಸ್ಥಿರ ಧ್ರುವ FuncDef stat_ತಳ್ಳು_Funcdef = {
  2+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  stat_ತಳ್ಳು,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_push",     /* zName */
  {0}
};

#define STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#define STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#define STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#define STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#define STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

/*
** Implementation of the stat_get(P,J) SQL function.  This routine is
** used to query statistical information that has been gathered into
** the StatAccum object by prior calls to stat_push().  The P parameter
** has type BLOB but it is really just a pointer to the StatAccum object.
** The content to returned is determined by the parameter J
** which is one of the STAT_GET_xxxx values defined above.
**
** The stat_get(P,J) function is not available to generic SQL.  It is
** inserted as part of a manually constructed bytecode program.  (See
** the callStatGet() routine below.)  It is guaranteed that the P
** parameter will always be a pointer to a StatAccum object, never a
** NULL.
**
** If STAT4 is not enabled, then J is always
** STAT_GET_STAT1 and is hence omitted and this routine becomes
** a one-parameter function, stat_get(P), that always returns the
** stat1 table entry information.
*/
ಸ್ಥಿರ ಶೂನ್ಯ statGet(
  sqlite3_context *context,
  ಪೂರ್ಣಾಙ್ಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  StatAccum *p = (StatAccum*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
#ifdef SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  ಪೂರ್ಣಾಙ್ಕ eCall = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  ಪ್ರತಿಜ್ಞಾ( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  ಪ್ರತಿಜ್ಞಾ( eCall==STAT_GET_STAT1 || p->mxSample );
  ಯದಿ( eCall==STAT_GET_STAT1 )
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
#endif
  {
    /* Return the value to store in the "stat" column of the sqlite_stat1
    ** table for this index.
    **
    ** The value is a string composed of a list of integers describing 
    ** the index. The first integer in the list is the total number of 
    ** entries in the index. There is one additional integer in the list 
    ** for each indexed column. This additional integer is an estimate of
    ** the number of rows matched by a equality query on the index using
    ** a key with the corresponding number of fields. In other words,
    ** if the index is on columns (a,b) and the sqlite_stat1 value is 
    ** "100 10 2", then SQLite estimates that:
    **
    **   * the index contains 100 rows,
    **   * "WHERE a=?" matches 10 rows, and
    **   * "WHERE a=? AND b=?" matches 2 rows.
    **
    ** If D is the count of distinct values and K is the total number of 
    ** rows, then each estimate is usually computed as:
    **
    **        I = (K+D-1)/D
    **
    ** In other words, I is K/D rounded up to the next whole integer.
    ** However, if I is between 1.0 and 1.1 (in other words if I is
    ** close to 1.0 but just a little larger) then do not round up but
    ** instead keep the I value at 1.0.
    */
    sqlite3_str sStat;   /* Text of the constructed "stat" line */
    ಪೂರ್ಣಾಙ್ಕ i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_ಕೀಲಿ_Col+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->nRow);
    ಪ್ರತಿ(i=0; i<p->n_ಕೀಲಿ_Col; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->nRow + nDistinct - 1) / nDistinct;
      ಯದಿ( iVal==2 && p->nRow*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#ifdef SQLITE_ENABLE_STAT4
      ಪ್ರತಿಜ್ಞಾ( p->current.anEq[i] || p->nRow==0 );
#endif
    }
    sqlite3_result_str(context, &sStat, SQLITE_XFER);
  }
#ifdef SQLITE_ENABLE_STAT4
  ಅನ್ಯಥಾ ಯದಿ( eCall==STAT_GET_ROWID ){
    ಯದಿ( p->iGet<0 ){
      sample_ತಳ್ಳು_Previous(p, 0);
      p->iGet = 0;
    }
    ಯದಿ( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      ಯದಿ( pS->nRowid==0 ){
        sqlite3_result_int64(context, pS->u.iRowid);
      }ಅನ್ಯಥಾ{
        sqlite3_result_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }ಅನ್ಯಥಾ{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    ಪೂರ್ಣಾಙ್ಕ i;

    ಪ್ರತಿಜ್ಞಾ( p->iGet<p->nSample );
    ವಿಕಲ್ಪ( eCall ){
      ಪಕ್ಷ STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; ಭಙ್ಗ;
      ಪಕ್ಷ STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; ಭಙ್ಗ;
      ಸ್ವಭಾವ: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        ಭಙ್ಗ;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->nCol*100);
    ಪ್ರತಿ(i=0; i<p->nCol; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    ಯದಿ( sStat.nChar ) sStat.nChar--;
    sqlite3_result_str(context, &sStat, SQLITE_XFER);
  }
#endif /* SQLITE_ENABLE_STAT4 */
#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER( argc );
#endif
}
ಸ್ಥಿರ ಧ್ರುವ FuncDef statGetFuncdef = {
  1+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  statGet,         /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_get",      /* zName */
  {0}
};

ಸ್ಥಿರ ಶೂನ್ಯ callStatGet(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ regStat, ಪೂರ್ಣಾಙ್ಕ iParam, ಪೂರ್ಣಾಙ್ಕ regOut){
#ifdef SQLITE_ENABLE_STAT4
  sqlite3VdbeAddOp2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#ನೋಚೇದ್ಯದಿ SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( iParam==STAT_GET_STAT1 );
#ಅನ್ಯಥಾ
  UNUSED_PARAMETER( iParam );
#endif
  ಪ್ರತಿಜ್ಞಾ( regOut!=regStat && regOut!=regStat+1 );
  sqlite3VdbeAddFunctionCall(pParse, 0, regStat, regOut, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyzeVdbeCommentIndexWithColumnName(
  Vdbe *v,         /* Prepared statement under construction */
  Index *pIdx,     /* Index whose column is being loaded */
  ಪೂರ್ಣಾಙ್ಕ k            /* Which column index */
){
  ಪೂರ್ಣಾಙ್ಕ i;           /* Index of column in the table */
  ಪ್ರತಿಜ್ಞಾ( k>=0 && k<pIdx->nColumn );
  i = pIdx->aiColumn[k];
  ಯದಿ( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }ಅನ್ಯಥಾ ಯದಿ( i==XN_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }ಅನ್ಯಥಾ{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->pTable->aCol[i].zCnName));
  }
}
#ಅನ್ಯಥಾ
# define analyzeVdbeCommentIndexWithColumnName(a,b,c)
#endif /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyzeOneTable(
  Parse *pParse,   /* Parser context */
  Table *pTab,     /* Table whose indices are to be analyzed */
  Index *pOnlyIdx, /* If not NULL, only analyze this one index */
  ಪೂರ್ಣಾಙ್ಕ iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  ಪೂರ್ಣಾಙ್ಕ iMem,        /* Available memory locations begin here */
  ಪೂರ್ಣಾಙ್ಕ iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  Index *pIdx;                 /* An index to being analyzed */
  ಪೂರ್ಣಾಙ್ಕ iIdxCur;                 /* Cursor open on index being analyzed */
  ಪೂರ್ಣಾಙ್ಕ iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  ಪೂರ್ಣಾಙ್ಕ i;                       /* Loop counter */
  ಪೂರ್ಣಾಙ್ಕ jZeroRows = -1;          /* Jump from here if number of rows is zero */
  ಪೂರ್ಣಾಙ್ಕ iDb;                     /* Index of database containing pTab */
  u8 needTableCnt = 1;         /* True to count the table */
  ಪೂರ್ಣಾಙ್ಕ regNewRowid = iMem++;    /* Rowid for the inserted record */
  ಪೂರ್ಣಾಙ್ಕ regStat = iMem++;        /* Register to hold StatAccum object */
  ಪೂರ್ಣಾಙ್ಕ regChng = iMem++;        /* Index of changed index field */
  ಪೂರ್ಣಾಙ್ಕ regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  ಪೂರ್ಣಾಙ್ಕ regTemp = iMem++;        /* Temporary use register */
  ಪೂರ್ಣಾಙ್ಕ regTemp2 = iMem++;       /* Second temporary use register */
  ಪೂರ್ಣಾಙ್ಕ regTabname = iMem++;     /* Register containing table name */
  ಪೂರ್ಣಾಙ್ಕ regIdxname = iMem++;     /* Register containing index name */
  ಪೂರ್ಣಾಙ್ಕ regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  ಪೂರ್ಣಾಙ್ಕ regPrev = iMem;          /* MUST BE LAST (see below) */
#ifdef SQLITE_ENABLE_STAT4
  ಪೂರ್ಣಾಙ್ಕ doOnce = 1;              /* Flag for a one-time computation */
#endif
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
  Table *pStat1 = 0;
#endif

  sqlite3TouchRegister(pParse, iMem);
  ಪ್ರತಿಜ್ಞಾ( sqlite3NoTempsInRange(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  ಯದಿ( v==0 || NEVER(pTab==0) ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( !IsOrdinaryTable(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
#ifndef SQLITE_OMIT_AUTHORIZATION
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
  ಯದಿ( db->xPreUpdateCallback ){
    pStat1 = (Table*)sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Table) + 13);
    ಯದಿ( pStat1==0 ) ಪ್ರತ್ಯಾಗಮ;
    pStat1->zName = (ವರ್ಣ*)&pStat1[1];
    memcpy(pStat1->zName, "sqlite_stat1", 13);
    pStat1->nCol = 3;
    pStat1->iPKey = -1;
    sqlite3VdbeAddOp4(pParse->pVdbe, OP_Noop, 0, 0, 0,(ವರ್ಣ*)pStat1,P4_DYNAMIC);
  }
#endif

  /* Establish a read-lock on the table at the shared-cache level. 
  ** Open a read-only cursor on the table. Also allocate a cursor number
  ** to use for scanning indexes (iIdxCur). No index cursor is opened at
  ** this time though.  */
  sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = MAX(pParse->nTab, iTab);
  sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
  sqlite3VdbeLoadString(v, regTabname, pTab->zName);

  ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    ಪೂರ್ಣಾಙ್ಕ nCol;                     /* Number of columns in pIdx. "N" */
    ಪೂರ್ಣಾಙ್ಕ addrGotoEnd;               /* Address of "OP_Rewind iIdxCur" */
    ಪೂರ್ಣಾಙ್ಕ addrNextRow;              /* Address of "next_row:" */
    ಧ್ರುವ ವರ್ಣ *zIdxName;         /* Name of the index */
    ಪೂರ್ಣಾಙ್ಕ nColTest;                 /* Number of columns to test for changes */

    ಯದಿ( pOnlyIdx && pOnlyIdx!=pIdx ) ಅನುವರ್ತ;
    ಯದಿ( pIdx->pPartIdxWhere==0 ) needTableCnt = 0;
    ಯದಿ( !HasRowid(pTab) && Is_Primary_ಕೀಲಿ_Index(pIdx) ){
      nCol = pIdx->n_ಕೀಲಿ_Col;
      zIdxName = pTab->zName;
      nColTest = nCol - 1;
    }ಅನ್ಯಥಾ{
      nCol = pIdx->nColumn;
      zIdxName = pIdx->zName;
      nColTest = pIdx->uniqNotNull ? pIdx->n_ಕೀಲಿ_Col-1 : nCol-1;
    }

    /* Populate the register containing the index name. */
    sqlite3VdbeLoadString(v, regIdxname, zIdxName);
    VdbeComment((v, "Analysis for %s.%s", pTab->zName, zIdxName));

    /*
    ** Pseudo-code for loop that calls stat_push():
    **
    **   regChng = 0
    **   Rewind csr
    **   if eof(csr){
    **      stat_init() with count = 0;
    **      goto end_of_scan;
    **   }
    **   count()
    **   stat_init()
    **   goto chng_addr_0;
    **
    **  next_row:
    **   regChng = 0
    **   if( idx(0) != regPrev(0) ) goto chng_addr_0
    **   regChng = 1
    **   if( idx(1) != regPrev(1) ) goto chng_addr_1
    **   ...
    **   regChng = N
    **   goto chng_addr_N
    **
    **  chng_addr_0:
    **   regPrev(0) = idx(0)
    **  chng_addr_1:
    **   regPrev(1) = idx(1)
    **  ...
    **
    **  endDistinctTest:
    **   regRowid = idx(rowid)
    **   stat_push(P, regChng, regRowid)
    **   Next csr
    **   if !eof(csr) goto next_row;
    **
    **  end_of_scan:
    */

    /* Make sure there are enough memory cells allocated to accommodate 
    ** the regPrev array and a trailing rowid (the rowid slot is required
    ** when building a record to insert into the sample column of 
    ** the sqlite_stat4 table.  */
    sqlite3TouchRegister(pParse, regPrev+nColTest);

    /* Open a read-only cursor on the index being analyzed. */
    ಪ್ರತಿಜ್ಞಾ( iDb==sqlite3SchemaToIndex(db, pIdx->pSchema) );
    sqlite3VdbeAddOp3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_Set_P4_ಕೀಲಿ_Info(pParse, pIdx);
    VdbeComment((v, "%s", pIdx->zName));

    /* Implementation of the following:
    **
    **   regChng = 0
    **   Rewind csr
    **   if eof(csr){
    **      stat_init() with count = 0;
    **      goto end_of_scan;
    **   }
    **   count()
    **   stat_init()
    **   goto chng_addr_0;
    */
    ಪ್ರತಿಜ್ಞಾ( regTemp2==regStat+4 );
    sqlite3VdbeAddOp2(v, OP_Integer, db->nAnalysisLimit, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3VdbeAddOp2(v, OP_Integer, nCol, regStat+1);
    ಪ್ರತಿಜ್ಞಾ( regRowid==regStat+2 );
    sqlite3VdbeAddOp2(v, OP_Integer, pIdx->n_ಕೀಲಿ_Col, regRowid);
    sqlite3VdbeAddOp3(v, OP_ಎಣಿಕೆ, iIdxCur, regTemp,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3VdbeAddFunctionCall(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addrGotoEnd = sqlite3VdbeAddOp1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3VdbeAddOp2(v, OP_Integer, 0, regChng);
    addrNextRow = sqlite3VdbeCurrentAddr(v);

    ಯದಿ( nColTest>0 ){
      ಪೂರ್ಣಾಙ್ಕ endDistinctTest = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)*nColTest);
      ಯದಿ( aGotoChng==0 ) ಅನುವರ್ತ;

      /*
      **  next_row:
      **   regChng = 0
      **   if( idx(0) != regPrev(0) ) goto chng_addr_0
      **   regChng = 1
      **   if( idx(1) != regPrev(1) ) goto chng_addr_1
      **   ...
      **   regChng = N
      **   goto endDistinctTest
      */
      sqlite3VdbeAddOp0(v, OP_Goto);
      addrNextRow = sqlite3VdbeCurrentAddr(v);
      ಯದಿ( nColTest==1 && pIdx->n_ಕೀಲಿ_Col==1 && IsUniqueIndex(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3VdbeAddOp2(v, OP_NotNull, regPrev, endDistinctTest);
        VdbeCoverage(v);
      }
      ಪ್ರತಿ(i=0; i<nColTest; i++){
        ವರ್ಣ *pColl = (ವರ್ಣ*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3VdbeAddOp2(v, OP_Integer, i, regChng);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, i, regTemp);
        analyzeVdbeCommentIndexWithColumnName(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3VdbeAddOp4(v, OP_Ne, regTemp, 0, regPrev+i, pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp2(v, OP_Integer, nColTest, regChng);
      sqlite3VdbeGoto(v, endDistinctTest);
  
  
      /*
      **  chng_addr_0:
      **   regPrev(0) = idx(0)
      **  chng_addr_1:
      **   regPrev(1) = idx(1)
      **  ...
      */
      sqlite3VdbeJumpHere(v, addrNextRow-1);
      ಪ್ರತಿ(i=0; i<nColTest; i++){
        sqlite3VdbeJumpHere(v, aGotoChng[i]);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, i, regPrev+i);
        analyzeVdbeCommentIndexWithColumnName(v,pIdx,i);
      }
      sqlite3VdbeResolveLabel(v, endDistinctTest);
      sqlite3DbFree(db, aGotoChng);
    }
  
    /*
    **  chng_addr_N:
    **   regRowid = idx(rowid)            // STAT4 only
    **   stat_push(P, regChng, regRowid)  // 3rd parameter STAT4 only
    **   Next csr
    **   if !eof(csr) goto next_row;
    */
#ifdef SQLITE_ENABLE_STAT4
    ಯದಿ( OptimizationEnabled(db, SQLITE_Stat4) ){
      ಪ್ರತಿಜ್ಞಾ( regRowid==(regStat+2) );
      ಯದಿ( HasRowid(pTab) ){
        sqlite3VdbeAddOp2(v, OP_IdxRowid, iIdxCur, regRowid);
      }ಅನ್ಯಥಾ{
        Index *pPk = sqlite3_Primary_ಕೀಲಿ_Index(pIdx->pTable);
        ಪೂರ್ಣಾಙ್ಕ j, k, reg_ಕೀಲಿ;
        reg_ಕೀಲಿ = sqlite3GetTempRange(pParse, pPk->n_ಕೀಲಿ_Col);
        ಪ್ರತಿ(j=0; j<pPk->n_ಕೀಲಿ_Col; j++){
          k = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[j]);
          ಪ್ರತಿಜ್ಞಾ( k>=0 && k<pIdx->nColumn );
          sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, k, reg_ಕೀಲಿ+j);
          analyzeVdbeCommentIndexWithColumnName(v,pIdx,k);
        }
        sqlite3VdbeAddOp3(v, OP_MakeRecord, reg_ಕೀಲಿ, pPk->n_ಕೀಲಿ_Col, regRowid);
        sqlite3ReleaseTempRange(pParse, reg_ಕೀಲಿ, pPk->n_ಕೀಲಿ_Col);
      }
    }
#endif
    ಪ್ರತಿಜ್ಞಾ( regChng==(regStat+1) );
    {
      sqlite3VdbeAddFunctionCall(pParse, 1, regStat, regTemp, 2+IsStat4,
                                 &stat_ತಳ್ಳು_Funcdef, 0);
      ಯದಿ( db->nAnalysisLimit ){
        ಪೂರ್ಣಾಙ್ಕ j1, j2, j3;
        j1 = sqlite3VdbeAddOp1(v, OP_IsNull, regTemp); VdbeCoverage(v);
        j2 = sqlite3VdbeAddOp1(v, OP_If, regTemp); VdbeCoverage(v);
        j3 = sqlite3VdbeAddOp4Int(v, OP_SeekGT, iIdxCur, 0, regPrev, 1);
        VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, j1);
        sqlite3VdbeAddOp2(v, OP_Next, iIdxCur, addrNextRow); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, j2);
        sqlite3VdbeJumpHere(v, j3);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp2(v, OP_Next, iIdxCur, addrNextRow); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    ಯದಿ( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3VdbeJumpHere(v, addrGotoEnd);
      addrGotoEnd = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    ಪ್ರತಿಜ್ಞಾ( "BBB"[0]==SQLITE_AFF_TEXT );
    sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 3, regTemp, "BBB", 0);
    sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iStatCur, regTemp, regNewRowid);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (ವರ್ಣ*)pStat1, P4_TABLE);
#endif
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#ifdef SQLITE_ENABLE_STAT4
    ಯದಿ( OptimizationEnabled(db, SQLITE_Stat4) && db->nAnalysisLimit==0 ){
      ಪೂರ್ಣಾಙ್ಕ regEq = regStat1;
      ಪೂರ್ಣಾಙ್ಕ regLt = regStat1+1;
      ಪೂರ್ಣಾಙ್ಕ regDLt = regStat1+2;
      ಪೂರ್ಣಾಙ್ಕ regSample = regStat1+3;
      ಪೂರ್ಣಾಙ್ಕ regCol = regStat1+4;
      ಪೂರ್ಣಾಙ್ಕ regSampleRowid = regCol + nCol;
      ಪೂರ್ಣಾಙ್ಕ addrNext;
      ಪೂರ್ಣಾಙ್ಕ addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_NotFound;

      /* No STAT4 data is generated if the number of rows is zero */
      ಯದಿ( addrGotoEnd==0 ){
        sqlite3VdbeAddOp2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addrGotoEnd = sqlite3VdbeAddOp1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      ಯದಿ( doOnce ){
        ಪೂರ್ಣಾಙ್ಕ mxCol = nCol;
        Index *pX;

        /* Compute the maximum number of columns in any index */
        ಪ್ರತಿ(pX=pTab->pIndex; pX; pX=pX->pNext){
          ಪೂರ್ಣಾಙ್ಕ nColX;                     /* Number of columns in pX */
          ಯದಿ( !HasRowid(pTab) && Is_Primary_ಕೀಲಿ_Index(pX) ){
            nColX = pX->n_ಕೀಲಿ_Col;
          }ಅನ್ಯಥಾ{
            nColX = pX->nColumn;
          }
          ಯದಿ( nColX>mxCol ) mxCol = nColX;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3TouchRegister(pParse, regCol+mxCol);
        doOnce = 0;
#ifdef SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3NoTempsInRange(pParse, regEq, regCol+mxCol) );
#endif
        sqlite3ClearTempRegCache(pParse);  /* tag-20230325-1 */
        ಪ್ರತಿಜ್ಞಾ( sqlite3NoTempsInRange(pParse, regEq, regCol+mxCol) );
      }
      ಪ್ರತಿಜ್ಞಾ( sqlite3NoTempsInRange(pParse, regEq, regCol+nCol) );

      addrNext = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3VdbeAddOp4Int(v, seekOp, iTabCur, addrNext, regSampleRowid, 0);
      VdbeCoverage(v);
      ಪ್ರತಿ(i=0; i<nCol; i++){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Load_Index_Column(pParse, pIdx, iTabCur, i, regCol+i);
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regCol, nCol, regSample);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regTabname, 6, regTemp);
      sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iStatCur+1, regTemp, regNewRowid);
      sqlite3VdbeAddOp2(v, OP_Goto, 1, addrNext); /* P1==1 for end-of-loop */
      sqlite3VdbeJumpHere(v, addrIsNull);
    }
#endif /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    ಯದಿ( addrGotoEnd ) sqlite3VdbeJumpHere(v, addrGotoEnd);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  ಯದಿ( pOnlyIdx==0 && needTableCnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3VdbeAddOp2(v, OP_ಎಣಿಕೆ, iTabCur, regStat1);
    jZeroRows = sqlite3VdbeAddOp1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Null, 0, regIdxname);
    ಪ್ರತಿಜ್ಞಾ( "BBB"[0]==SQLITE_AFF_TEXT );
    sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 3, regTemp, "BBB", 0);
    sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iStatCur, regTemp, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (ವರ್ಣ*)pStat1, P4_TABLE);
#endif
    sqlite3VdbeJumpHere(v, jZeroRows);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
ಸ್ಥಿರ ಶೂನ್ಯ loadAnalysis(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  ಯದಿ( v ){
    sqlite3VdbeAddOp1(v, OP_LoadAnalysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyzeDatabase(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  ಪೂರ್ಣಾಙ್ಕ iStatCur;
  ಪೂರ್ಣಾಙ್ಕ iMem;
  ಪೂರ್ಣಾಙ್ಕ iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  openStatTable(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಪ್ರತಿ(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
    Table *pTab = (Table*)sqliteHashData(k);
    analyzeOneTable(pParse, pTab, 0, iStatCur, iMem, iTab);
#ifdef SQLITE_ENABLE_STAT4
    iMem = sqlite3FirstAvailableRegister(pParse, iMem);
#ಅನ್ಯಥಾ
    ಪ್ರತಿಜ್ಞಾ( iMem==sqlite3FirstAvailableRegister(pParse,iMem) );
#endif
  }
  loadAnalysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyzeTable(Parse *pParse, Table *pTab, Index *pOnlyIdx){
  ಪೂರ್ಣಾಙ್ಕ iDb;
  ಪೂರ್ಣಾಙ್ಕ iStatCur;

  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  ಯದಿ( pOnlyIdx ){
    openStatTable(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }ಅನ್ಯಥಾ{
    openStatTable(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyzeOneTable(pParse, pTab, pOnlyIdx, iStatCur,pParse->nMem+1,pParse->nTab);
  loadAnalysis(pParse, iDb);
}

/*
** Generate code for the ANALYZE command.  The parser calls this routine
** when it recognizes an ANALYZE command.
**
**        ANALYZE                            -- 1
**        ANALYZE  <database>                -- 2
**        ANALYZE  ?<database>.?<tablename>  -- 3
**
** Form 1 causes all indices in all attached databases to be analyzed.
** Form 2 analyzes all indices the single database named.
** Form 3 analyzes all indices associated with the named table.
*/
ಶೂನ್ಯ sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ iDb;
  ಪೂರ್ಣಾಙ್ಕ i;
  ವರ್ಣ *z, *zDb;
  Table *pTab;
  Index *pIdx;
  Token *pTableName;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  ಯದಿ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಪ್ರತಿಜ್ಞಾ( pName2!=0 || pName1==0 );
  ಯದಿ( pName1==0 ){
    /* Form 1:  Analyze everything */
    ಪ್ರತಿ(i=0; i<db->nDb; i++){
      ಯದಿ( i==1 ) ಅನುವರ್ತ;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }ಅನ್ಯಥಾ ಯದಿ( pName2->n==0 && (iDb = sqlite3FindDb(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }ಅನ್ಯಥಾ{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pTableName);
    ಯದಿ( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, pTableName);
      ಯದಿ( z ){
        ಯದಿ( (pIdx = sqlite3FindIndex(db, z, zDb))!=0 ){
          analyzeTable(pParse, pIdx->pTable, pIdx);
        }ಅನ್ಯಥಾ ಯದಿ( (pTab = sqlite3LocateTable(pParse, 0, z, zDb))!=0 ){
          analyzeTable(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  ಯದಿ( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3VdbeAddOp0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ analysisInfo analysisInfo;
ಸಂರಚನಾ analysisInfo {
  sqlite3 *db;
  ಧ್ರುವ ವರ್ಣ *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
ಸ್ಥಿರ ಶೂನ್ಯ decodeIntArray(
  ವರ್ಣ *zIntArray,       /* String containing int array to decode */
  ಪೂರ್ಣಾಙ್ಕ nOut,              /* Number of slots in aOut[] */
  tRowcnt *aOut,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  Index *pIndex          /* Handle extra flags for this index, if not NULL */
){
  ವರ್ಣ *z = zIntArray;
  ಪೂರ್ಣಾಙ್ಕ c;
  ಪೂರ್ಣಾಙ್ಕ i;
  tRowcnt v;

#ifdef SQLITE_ENABLE_STAT4
  ಯದಿ( z==0 ) z = "";
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( z!=0 );
#endif
  ಪ್ರತಿ(i=0; *z && i<nOut; i++){
    v = 0;
    ಯಾವತ್( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#ifdef SQLITE_ENABLE_STAT4
    ಯದಿ( aOut ) aOut[i] = v;
    ಯದಿ( aLog ) aLog[i] = sqlite3LogEst(v);
#ಅನ್ಯಥಾ
    ಪ್ರತಿಜ್ಞಾ( aOut==0 );
    UNUSED_PARAMETER(aOut);
    ಪ್ರತಿಜ್ಞಾ( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#endif
    ಯದಿ( *z==' ' ) z++;
  }
#ifndef SQLITE_ENABLE_STAT4
  ಪ್ರತಿಜ್ಞಾ( pIndex!=0 ); {
#ಅನ್ಯಥಾ
  ಯದಿ( pIndex ){
#endif
    pIndex->bUnordered = 0;
    pIndex->noSkipScan = 0;
    ಯಾವತ್( z[0] ){
      ಯದಿ( sqlite3_strglob("unordered*", z)==0 ){
        pIndex->bUnordered = 1;
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        ಪೂರ್ಣಾಙ್ಕ sz = sqlite3Atoi(z+3);
        ಯದಿ( sz<2 ) sz = 2;
        pIndex->szIdxRow = sqlite3LogEst(sz);
      }ಅನ್ಯಥಾ ಯದಿ( sqlite3_strglob("noskipscan*", z)==0 ){
        pIndex->noSkipScan = 1;
      }
#ifdef SQLITE_ENABLE_COSTMULT
      ಅನ್ಯಥಾ ಯದಿ( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        pIndex->pTable->costMult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#endif
      ಯಾವತ್( z[0]!=0 && z[0]!=' ' ) z++;
      ಯಾವತ್( z[0]==' ' ) z++;
    }
  }
}

/*
** This callback is invoked once for each index when reading the
** sqlite_stat1 table.  
**
**     argv[0] = name of the table
**     argv[1] = name of the index (might be NULL)
**     argv[2] = results of analysis - on integer for each column
**
** Entries for which argv[1]==NULL simply record the number of rows in
** the table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ analysisLoader(ಶೂನ್ಯ *pData, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv, ವರ್ಣ **NotUsed){
  analysisInfo *pInfo = (analysisInfo*)pData;
  Index *pIndex;
  Table *pTable;
  ಧ್ರುವ ವರ್ಣ *z;

  ಪ್ರತಿಜ್ಞಾ( argc==3 );
  UNUSED_PARAMETER2(NotUsed, argc);

  ಯದಿ( argv==0 || argv[0]==0 || argv[2]==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pTable = sqlite3FindTable(pInfo->db, argv[0], pInfo->zDatabase);
  ಯದಿ( pTable==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( argv[1]==0 ){
    pIndex = 0;
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3_stricmp(argv[0],argv[1])==0 ){
    pIndex = sqlite3_Primary_ಕೀಲಿ_Index(pTable);
  }ಅನ್ಯಥಾ{
    pIndex = sqlite3FindIndex(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  ಯದಿ( pIndex ){
    tRowcnt *aiRowEst = 0;
    ಪೂರ್ಣಾಙ್ಕ nCol = pIndex->n_ಕೀಲಿ_Col+1;
#ifdef SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    ಯದಿ( pIndex->aiRowEst==0 ){
      pIndex->aiRowEst = (tRowcnt*)sqlite3MallocZero(ಪರಿಮಾಣ(tRowcnt) * nCol);
      ಯದಿ( pIndex->aiRowEst==0 ) sqlite3OomFault(pInfo->db);
    }
    aiRowEst = pIndex->aiRowEst;
#endif
    pIndex->bUnordered = 0;
    decodeIntArray((ವರ್ಣ*)z, nCol, aiRowEst, pIndex->aiRowLogEst, pIndex);
    pIndex->hasStat1 = 1;
    ಯದಿ( pIndex->pPartIdxWhere==0 ){
      pTable->nRowLogEst = pIndex->aiRowLogEst[0];
      pTable->tabFlags |= TF_HasStat1;
    }
  }ಅನ್ಯಥಾ{
    Index fakeIdx;
    fakeIdx.szIdxRow = pTable->szTabRow;
#ifdef SQLITE_ENABLE_COSTMULT
    fakeIdx.pTable = pTable;
#endif
    decodeIntArray((ವರ್ಣ*)z, 1, 0, &pTable->nRowLogEst, &fakeIdx);
    pTable->szTabRow = fakeIdx.szIdxRow;
    pTable->tabFlags |= TF_HasStat1;
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
ಶೂನ್ಯ sqlite3DeleteIndexSamples(sqlite3 *db, Index *pIdx){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( pIdx!=0 );
#ifdef SQLITE_ENABLE_STAT4
  ಯದಿ( pIdx->aSample ){
    ಪೂರ್ಣಾಙ್ಕ j;
    ಪ್ರತಿ(j=0; j<pIdx->nSample; j++){
      IndexSample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  ಯದಿ( db->pnBytesFreed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#ಅನ್ಯಥಾ
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(pIdx);
#endif /* SQLITE_ENABLE_STAT4 */
}

#ifdef SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
ಸ್ಥಿರ ಶೂನ್ಯ initAvgEq(Index *pIdx){
  ಯದಿ( pIdx ){
    IndexSample *aSample = pIdx->aSample;
    IndexSample *pFinal = &aSample[pIdx->nSample-1];
    ಪೂರ್ಣಾಙ್ಕ iCol;
    ಪೂರ್ಣಾಙ್ಕ nCol = 1;
    ಯದಿ( pIdx->nSampleCol>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      nCol = pIdx->nSampleCol-1;
      pIdx->aAvgEq[nCol] = 1;
    }
    ಪ್ರತಿ(iCol=0; iCol<nCol; iCol++){
      ಪೂರ್ಣಾಙ್ಕ nSample = pIdx->nSample;
      ಪೂರ್ಣಾಙ್ಕ i;                    /* Used to iterate through samples */
      tRowcnt sumEq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt nRow;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      ಯದಿ( !pIdx->aiRowEst || iCol>=pIdx->n_ಕೀಲಿ_Col || pIdx->aiRowEst[iCol+1]==0 ){
        nRow = pFinal->anLt[iCol];
        nDist100 = (i64)100 * pFinal->anDLt[iCol];
        nSample--;
      }ಅನ್ಯಥಾ{
        nRow = pIdx->aiRowEst[0];
        nDist100 = ((i64)100 * pIdx->aiRowEst[0]) / pIdx->aiRowEst[iCol+1];
      }
      pIdx->nRowEst0 = nRow;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      ಪ್ರತಿ(i=0; i<nSample; i++){
        ಯದಿ( i==(pIdx->nSample-1)
         || aSample[i].anDLt[iCol]!=aSample[i+1].anDLt[iCol] 
        ){
          sumEq += aSample[i].anEq[iCol];
          nSum100 += 100;
        }
      }

      ಯದಿ( nDist100>nSum100 && sumEq<nRow ){
        avgEq = ((i64)100 * (nRow - sumEq))/(nDist100 - nSum100);
      }
      ಯದಿ( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[iCol] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
ಸ್ಥಿರ Index *find_Index_Or_Primary_ಕೀಲಿ(
  sqlite3 *db,
  ಧ್ರುವ ವರ್ಣ *zName,
  ಧ್ರುವ ವರ್ಣ *zDb
){
  Index *pIdx = sqlite3FindIndex(db, zName, zDb);
  ಯದಿ( pIdx==0 ){
    Table *pTab = sqlite3FindTable(db, zName, zDb);
    ಯದಿ( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
  }
  ಪ್ರತ್ಯಾಗಮ pIdx;
}

/*
** Load the content from either the sqlite_stat4
** into the relevant Index.aSample[] arrays.
**
** Arguments zSql1 and zSql2 must point to SQL statements that return
** data equivalent to the following:
**
**    zSql1: SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx
**    zSql2: SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4
**
** where %Q is replaced with the database name before the SQL is executed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loadStatTbl(
  sqlite3 *db,                  /* Database handle */
  ಧ್ರುವ ವರ್ಣ *zSql1,            /* SQL statement 1 (see above) */
  ಧ್ರುವ ವರ್ಣ *zSql2,            /* SQL statement 2 (see above) */
  ಧ್ರುವ ವರ್ಣ *zDb               /* Database name (e.g. "main") */
){
  ಪೂರ್ಣಾಙ್ಕ rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  ವರ್ಣ *zSql;                   /* Text of the SQL statement */
  Index *pPrevIdx = 0;          /* Previous index in the loop */
  IndexSample *pSample;         /* A slot in pIdx->aSample[] */

  ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  ಯದಿ( !zSql ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;

  ಯಾವತ್( sqlite3_step(pStmt)==SQLITE_ROW ){
    ಪೂರ್ಣಾಙ್ಕ nIdxCol = 1;              /* Number of columns in stat4 records */

    ವರ್ಣ *zIndex;    /* Index name */
    Index *pIdx;     /* Pointer to the index object */
    ಪೂರ್ಣಾಙ್ಕ nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    zIndex = (ವರ್ಣ *)sqlite3_column_text(pStmt, 0);
    ಯದಿ( zIndex==0 ) ಅನುವರ್ತ;
    nSample = sqlite3_column_int(pStmt, 1);
    pIdx = find_Index_Or_Primary_ಕೀಲಿ(db, zIndex, zDb);
    ಪ್ರತಿಜ್ಞಾ( pIdx==0 || pIdx->nSample==0 );
    ಯದಿ( pIdx==0 ) ಅನುವರ್ತ;
    ಯದಿ( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      ಅನುವರ್ತ;
    }
    ಪ್ರತಿಜ್ಞಾ( !HasRowid(pIdx->pTable) || pIdx->nColumn==pIdx->n_ಕೀಲಿ_Col+1 );
    ಯದಿ( !HasRowid(pIdx->pTable) && Is_Primary_ಕೀಲಿ_Index(pIdx) ){
      nIdxCol = pIdx->n_ಕೀಲಿ_Col;
    }ಅನ್ಯಥಾ{
      nIdxCol = pIdx->nColumn;
    }
    pIdx->nSampleCol = nIdxCol;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(IndexSample) * nSample);
    nByte += sizeof64(tRowcnt) * nIdxCol * 3 * nSample;
    nByte += nIdxCol * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    ಯದಿ( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += nIdxCol;
    pIdx->pTable->tabFlags |= TF_HasStat4;
    ಪ್ರತಿ(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += nIdxCol;
      pIdx->aSample[i].anLt = pSpace; pSpace += nIdxCol;
      pIdx->aSample[i].anDLt = pSpace; pSpace += nIdxCol;
    }
    ಪ್ರತಿಜ್ಞಾ( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  ಯದಿ( !zSql ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;

  ಯಾವತ್( sqlite3_step(pStmt)==SQLITE_ROW ){
    ವರ್ಣ *zIndex;                 /* Index name */
    Index *pIdx;                  /* Pointer to the index object */
    ಪೂರ್ಣಾಙ್ಕ nCol = 1;                 /* Number of columns in index */

    zIndex = (ವರ್ಣ *)sqlite3_column_text(pStmt, 0);
    ಯದಿ( zIndex==0 ) ಅನುವರ್ತ;
    pIdx = find_Index_Or_Primary_ಕೀಲಿ(db, zIndex, zDb);
    ಯದಿ( pIdx==0 ) ಅನುವರ್ತ;
    ಯದಿ( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      ಅನುವರ್ತ;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    nCol = pIdx->nSampleCol;
    ಯದಿ( pIdx!=pPrevIdx ){
      initAvgEq(pPrevIdx);
      pPrevIdx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((ವರ್ಣ*)sqlite3_column_text(pStmt,1),nCol,pSample->anEq,0,0);
    decodeIntArray((ವರ್ಣ*)sqlite3_column_text(pStmt,2),nCol,pSample->anLt,0,0);
    decodeIntArray((ವರ್ಣ*)sqlite3_column_text(pStmt,3),nCol,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_column_bytes(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    ಯದಿ( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
    }
    ಯದಿ( pSample->n ){
      memcpy(pSample->p, sqlite3_column_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  ಯದಿ( rc==SQLITE_OK ) initAvgEq(pPrevIdx);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loadStat4(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zDb){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;             /* Result codes from subroutines */
  ಧ್ರುವ Table *pStat4;

  ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable );
  ಯದಿ( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3FindTable(db, "sqlite_stat4", zDb))!=0
   && IsOrdinaryTable(pStat4)
  ){
    rc = loadStatTbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* SQLITE_ENABLE_STAT4 */

/*
** Load the content of the sqlite_stat1 and sqlite_stat4 tables. The
** contents of sqlite_stat1 are used to populate the Index.aiRowEst[]
** arrays. The contents of sqlite_stat4 are used to populate the
** Index.aSample[] arrays.
**
** If the sqlite_stat1 table is not present in the database, SQLITE_ERROR
** is returned. In this case, even if SQLITE_ENABLE_STAT4 was defined 
** during compilation and the sqlite_stat4 table is present, no data is 
** read from it.
**
** If SQLITE_ENABLE_STAT4 was defined during compilation and the 
** sqlite_stat4 table is not present in the database, SQLITE_ERROR is
** returned. However, in this case, data is read from the sqlite_stat1
** table (if it is present) before returning.
**
** If an OOM error occurs, this function always sets db->mallocFailed.
** This means if the caller does not care about other errors, the return
** code may be ignored.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3AnalysisLoad(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ iDb){
  analysisInfo sInfo;
  HashElem *i;
  ವರ್ಣ *zSql;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  ಧ್ರುವ Table *pStat1;

  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಪ್ರತಿ(i=sqliteHashFirst(&pSchema->tblHash); i; i=sqliteHashNext(i)){
    Table *pTab = sqliteHashData(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  ಪ್ರತಿ(i=sqliteHashFirst(&pSchema->idxHash); i; i=sqliteHashNext(i)){
    Index *pIdx = sqliteHashData(i);
    pIdx->hasStat1 = 0;
#ifdef SQLITE_ENABLE_STAT4
    sqlite3DeleteIndexSamples(db, pIdx);
    pIdx->aSample = 0;
#endif
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  ಯದಿ( (pStat1 = sqlite3FindTable(db, "sqlite_stat1", sInfo.zDatabase))
   && IsOrdinaryTable(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    ಯದಿ( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ಅನ್ಯಥಾ{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಪ್ರತಿ(i=sqliteHashFirst(&pSchema->idxHash); i; i=sqliteHashNext(i)){
    Index *pIdx = sqliteHashData(i);
    ಯದಿ( !pIdx->hasStat1 ) sqlite3DefaultRowEst(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#ifdef SQLITE_ENABLE_STAT4
  ಯದಿ( rc==SQLITE_OK ){
    DisableLookaside;
    rc = loadStat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  ಪ್ರತಿ(i=sqliteHashFirst(&pSchema->idxHash); i; i=sqliteHashNext(i)){
    Index *pIdx = sqliteHashData(i);
    sqlite3_free(pIdx->aiRowEst);
    pIdx->aiRowEst = 0;
  }
#endif

  ಯದಿ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}


#endif /* SQLITE_OMIT_ANALYZE */
