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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ANALYZE
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

#ಆದರೆ defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#ಇಲ್ಲದಿದ್ದರೆ
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಶೂನ್ಯ open_Stat_ಕೋಷ್ಟಕ(
  Parse *pParse,          /* Parsing context */
  ಪೂರ್ಣಾಂಕ iDb,                /* The database we are looking in */
  ಪೂರ್ಣಾಂಕ iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zWhere,     /* Delete entries for this table or index */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zWhereType  /* Either "tbl" or "idx" */
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
  } a_ಕೋಷ್ಟಕ[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#ಆದರೆ defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#ಇಲ್ಲದಿದ್ದರೆ
    { "sqlite_stat4", 0 },
#ಆದರೆ_ಅಂತ್ಯ
    { "sqlite_stat3", 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_ಬೇರು[Array_ಗಾತ್ರ(a_ಕೋಷ್ಟಕ)];
  u8 aCreateTbl[Array_ಗಾತ್ರ(a_ಕೋಷ್ಟಕ)];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ಗಮ್ಯ_Open = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_ಗಮ್ಯ_Open = 1;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( v==0 ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(a_ಕೋಷ್ಟಕ); i++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab = a_ಕೋಷ್ಟಕ[i].zName;
    ಕೋಷ್ಟಕ *pStat;
    aCreateTbl[i] = 0;
    ಆದರೆ( (pStat = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, zTab, pDb->zDbSName))==0 ){
      ಆದರೆ( i<n_ಗಮ್ಯ_Open ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, a_ಕೋಷ್ಟಕ[i].z_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ
        );
        ಪ್ರತಿಜ್ಞಾ( pParse->isCreate || pParse->nErr );
        a_ಬೇರು[i] = (u32)pParse->u1.cr.reg_ಬೇರು;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_ಬೇರು[i] = pStat->tnum;
      sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, a_ಬೇರು[i], 1, zTab);
      ಆದರೆ( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#ಆದರೆ_ಅಂತ್ಯ
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Clear, (ಪೂರ್ಣಾಂಕ)a_ಬೇರು[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  ಗಾಗಿ(i=0; i<n_ಗಮ್ಯ_Open; i++){
    ಪ್ರತಿಜ್ಞಾ( i<Array_ಗಾತ್ರ(a_ಕೋಷ್ಟಕ) );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_OpenWrite, iStatCur+i, (ಪೂರ್ಣಾಂಕ)a_ಬೇರು[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, a_ಕೋಷ್ಟಕ[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#ಆದರೆ_ಅಂತ್ಯ

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ StatAccum StatAccum;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ StatSample StatSample;
ರಚನೆ StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  ಒಕ್ಕೂಟ {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#ಆದರೆ_ಅಂತ್ಯ
};                                                    
ರಚನೆ StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt n_ಅಡ್ಡಸಾಲು;             /* Number of rows visited so far */
  ಪೂರ್ಣಾಂಕ n_ಎಲ್ಲೆ;               /* Analysis row-scan limit */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                 /* Number of columns in index + pk/rowid */
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ_ಲಂಬಸಾಲು;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  ಪೂರ್ಣಾಂಕ mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *a_ಉತ್ತಮ;        /* Array of nCol best samples */
  ಪೂರ್ಣಾಂಕ iMin;                 /* Index in a[] of entry with minimum score */
  ಪೂರ್ಣಾಂಕ nSample;              /* Current number of samples */
  ಪೂರ್ಣಾಂಕ nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  ಪೂರ್ಣಾಂಕ iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#ಆದರೆ_ಅಂತ್ಯ
};

/* Reclaim memory used by a StatSample
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sampleClear(sqlite3 *db, StatSample *p){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/* Initialize the BLOB value of a ROWID
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sample_ಗಣ_Rowid(sqlite3 *db, StatSample *p, ಪೂರ್ಣಾಂಕ n, ಸ್ಥಿರಾಂಕ u8 *p_ದತ್ತಾಂಶ){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  ಆದರೆ( p->u.aRowid ){
    p->nRowid = n;
    ಸ್ಮರಣೆ_ನಕಲು(p->u.aRowid, p_ದತ್ತಾಂಶ, n);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->nRowid = 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/* Initialize the INTEGER value of a ROWID.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sample_ಗಣ_Rowid_Int64(sqlite3 *db, StatSample *p, i64 iRowid){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
ಸ್ಥಿರ ಶೂನ್ಯ sampleCopy(StatAccum *p, StatSample *p_ಗಮ್ಯ, StatSample *pFrom){
  p_ಗಮ್ಯ->isPSample = pFrom->isPSample;
  p_ಗಮ್ಯ->i_ಲಂಬಸಾಲು = pFrom->i_ಲಂಬಸಾಲು;
  p_ಗಮ್ಯ->iHash = pFrom->iHash;
  ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ->anEq, pFrom->anEq, ಗಾತ್ರ(tRowcnt)*p->n_ಲಂಬಸಾಲು);
  ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ->anLt, pFrom->anLt, ಗಾತ್ರ(tRowcnt)*p->n_ಲಂಬಸಾಲು);
  ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ->anDLt, pFrom->anDLt, ಗಾತ್ರ(tRowcnt)*p->n_ಲಂಬಸಾಲು);
  ಆದರೆ( pFrom->nRowid ){
    sample_ಗಣ_Rowid(p->db, p_ಗಮ್ಯ, pFrom->nRowid, pFrom->u.aRowid);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sample_ಗಣ_Rowid_Int64(p->db, p_ಗಮ್ಯ, pFrom->u.iRowid);
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Reclaim all memory of a StatAccum structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ statAccumDestructor(ಶೂನ್ಯ *pOld){
  StatAccum *p = (StatAccum*)pOld;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( p->mxSample ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++) sampleClear(p->db, p->a_ಉತ್ತಮ+i);
    ಗಾಗಿ(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#ಆದರೆ_ಅಂತ್ಯ
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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  StatAccum *p;
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                       /* Number of columns in index being sampled */
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ_ಲಂಬಸಾಲು;                    /* Number of key columns */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು_ಮೇಲೆ;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  ಪೂರ್ಣಾಂಕ mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#ಆದರೆ_ಅಂತ್ಯ

  /* Decode the three function arguments */
  ಬಳಸದ_PARAMETER(argc);
  n_ಲಂಬಸಾಲು = sqlite3_ಮೌಲ್ಯ_int(argv[0]);
  ಪ್ರತಿಜ್ಞಾ( n_ಲಂಬಸಾಲು>0 );
  n_ಲಂಬಸಾಲು_ಮೇಲೆ = ಗಾತ್ರ(tRowcnt)<8 ? (n_ಲಂಬಸಾಲು+1)&~1 : n_ಲಂಬಸಾಲು;
  n_ಕೀಲಿ_ಲಂಬಸಾಲು = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ_ಲಂಬಸಾಲು<=n_ಲಂಬಸಾಲು );
  ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ_ಲಂಬಸಾಲು>0 );

  /* Allocate the space required for the StatAccum object */
  n = ಗಾತ್ರ(*p) 
    + ಗಾತ್ರ(tRowcnt)*n_ಲಂಬಸಾಲು_ಮೇಲೆ;                    /* StatAccum.anDLt */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  n += ಗಾತ್ರ(tRowcnt)*n_ಲಂಬಸಾಲು_ಮೇಲೆ;                   /* StatAccum.anEq */
  ಆದರೆ( mxSample ){
    n += ಗಾತ್ರ(tRowcnt)*n_ಲಂಬಸಾಲು_ಮೇಲೆ                  /* StatAccum.anLt */
      + ಗಾತ್ರ(StatSample)*(n_ಲಂಬಸಾಲು+mxSample)       /* StatAccum.aBest[], a[] */
      + ಗಾತ್ರ(tRowcnt)*3*n_ಲಂಬಸಾಲು_ಮೇಲೆ*(n_ಲಂಬಸಾಲು+mxSample);
  }
#ಆದರೆ_ಅಂತ್ಯ
  p = sqlite3DbMallocZero(db, n);
  ಆದರೆ( p==0 ){
    sqlite3_ಫಲಿತಾಂಶ_error_nomem(context);
    ಹಿಂತಿರುಗಿ;
  }

  p->db = db;
  p->nEst = sqlite3_ಮೌಲ್ಯ_int64(argv[2]);
  p->n_ಅಡ್ಡಸಾಲು = 0;
  p->n_ಎಲ್ಲೆ = sqlite3_ಮೌಲ್ಯ_int(argv[3]);
  p->n_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು;
  p->n_ಕೀಲಿ_ಲಂಬಸಾಲು = n_ಕೀಲಿ_ಲಂಬಸಾಲು;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[n_ಲಂಬಸಾಲು_ಮೇಲೆ];
  p->mxSample = p->n_ಎಲ್ಲೆ==0 ? mxSample : 0;
  ಆದರೆ( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    ಪೂರ್ಣಾಂಕ i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[n_ಲಂಬಸಾಲು_ಮೇಲೆ];
    p->iPrn = 0x689e962d*(u32)n_ಲಂಬಸಾಲು ^ 0xd0944565*(u32)sqlite3_ಮೌಲ್ಯ_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (ರಚನೆ StatSample*)&p->current.anLt[n_ಲಂಬಸಾಲು_ಮೇಲೆ];
    p->a_ಉತ್ತಮ = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+n_ಲಂಬಸಾಲು]);
    ಗಾಗಿ(i=0; i<(mxSample+n_ಲಂಬಸಾಲು); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (ಗಾತ್ರ(tRowcnt) * n_ಲಂಬಸಾಲು_ಮೇಲೆ);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (ಗಾತ್ರ(tRowcnt) * n_ಲಂಬಸಾಲು_ಮೇಲೆ);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (ಗಾತ್ರ(tRowcnt) * n_ಲಂಬಸಾಲು_ಮೇಲೆ);
    }
    ಪ್ರತಿಜ್ಞಾ( (pSpace - (u8*)p)==n );
  
    ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
      p->a_ಉತ್ತಮ[i].i_ಲಂಬಸಾಲು = i;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_ಫಲಿತಾಂಶ_blob(context, p, ಗಾತ್ರ(*p), statAccumDestructor);
}
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ FuncDef statInitFuncdef = {
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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pAccum->n_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( pNew->i_ಲಂಬಸಾಲು==pOld->i_ಲಂಬಸಾಲು );
  ಗಾಗಿ(i=pNew->i_ಲಂಬಸಾಲು+1; i<n_ಲಂಬಸಾಲು; i++){
    ಆದರೆ( pNew->anEq[i]>pOld->anEq[i] ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( pNew->anEq[i]<pOld->anEq[i] ) ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( pNew->iHash>pOld->iHash ) ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->i_ಲಂಬಸಾಲು];
  tRowcnt nEqOld = pOld->anEq[pOld->i_ಲಂಬಸಾಲು];

  ಪ್ರತಿಜ್ಞಾ( pOld->isPSample==0 && pNew->isPSample==0 );
  ಪ್ರತಿಜ್ಞಾ( IsStat4 || (pNew->i_ಲಂಬಸಾಲು==0 && pOld->i_ಲಂಬಸಾಲು==0) );

  ಆದರೆ( (nEqNew>nEqOld) ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( nEqNew==nEqOld ){
    ಆದರೆ( pNew->i_ಲಂಬಸಾಲು<pOld->i_ಲಂಬಸಾಲು ) ಹಿಂತಿರುಗಿ 1;
    ಹಿಂತಿರುಗಿ (pNew->i_ಲಂಬಸಾಲು==pOld->i_ಲಂಬಸಾಲು && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sample_ಸೇರಿಸು(StatAccum *p, StatSample *pNew, ಪೂರ್ಣಾಂಕ nEqZero){
  StatSample *pSample = 0;
  ಪೂರ್ಣಾಂಕ i;

  ಪ್ರತಿಜ್ಞಾ( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  ಆದರೆ( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  ಆದರೆ( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    ಪ್ರತಿಜ್ಞಾ( pNew->anEq[pNew->i_ಲಂಬಸಾಲು]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    ಗಾಗಿ(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      ಆದರೆ( pOld->anEq[pNew->i_ಲಂಬಸಾಲು]==0 ){
        ಆದರೆ( pOld->isPSample ) ಹಿಂತಿರುಗಿ;
        ಪ್ರತಿಜ್ಞಾ( pOld->i_ಲಂಬಸಾಲು>pNew->i_ಲಂಬಸಾಲು );
        ಪ್ರತಿಜ್ಞಾ( sampleIsBetter(p, pNew, pOld) );
        ಆದರೆ( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    ಆದರೆ( pUpgrade ){
      pUpgrade->i_ಲಂಬಸಾಲು = pNew->i_ಲಂಬಸಾಲು;
      pUpgrade->anEq[pUpgrade->i_ಲಂಬಸಾಲು] = pNew->anEq[pUpgrade->i_ಲಂಬಸಾಲು];
      ಹೋಗು ಕಂಡುಹಿಡಿ_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  ಆದರೆ( p->nSample>=p->mxSample ){
    StatSample *pMin = &p->a[p->iMin];
    tRowcnt *anEq = pMin->anEq;
    tRowcnt *anLt = pMin->anLt;
    tRowcnt *anDLt = pMin->anDLt;
    sampleClear(p->db, pMin);
    memmove(pMin, &pMin[1], ಗಾತ್ರ(p->a[0])*(p->nSample-p->iMin-1));
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
       || pNew->anLt[p->n_ಲಂಬಸಾಲು-1] > p->a[p->nSample-1].anLt[p->n_ಲಂಬಸಾಲು-1] );

  /* Insert the new sample */
  pSample = &p->a[p->nSample];
  sampleCopy(p, pSample, pNew);
  p->nSample++;

  /* Zero the first nEqZero entries in the anEq[] array. */
  memset(pSample->anEq, 0, ಗಾತ್ರ(tRowcnt)*nEqZero);

ಕಂಡುಹಿಡಿ_new_min:
  ಆದರೆ( p->nSample>=p->mxSample ){
    ಪೂರ್ಣಾಂಕ iMin = -1;
    ಗಾಗಿ(i=0; i<p->mxSample; i++){
      ಆದರೆ( p->a[i].isPSample ) ಮುಂದುವರಿ;
      ಆದರೆ( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    ಪ್ರತಿಜ್ಞಾ( iMin>=0 );
    p->iMin = iMin;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sample_ತಳ್ಳು_ಹಿಂದಿನ(StatAccum *p, ಪೂರ್ಣಾಂಕ iChng){
  ಪೂರ್ಣಾಂಕ i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  ಗಾಗಿ(i=(p->n_ಲಂಬಸಾಲು-2); i>=iChng; i--){
    StatSample *p_ಉತ್ತಮ = &p->a_ಉತ್ತಮ[i];
    p_ಉತ್ತಮ->anEq[i] = p->current.anEq[i];
    ಆದರೆ( p->nSample<p->mxSample || sampleIsBetter(p, p_ಉತ್ತಮ, &p->a[p->iMin]) ){
      sample_ಸೇರಿಸು(p, p_ಉತ್ತಮ, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  ಗಾಗಿ(i=p->nSample-1; i>=0; i--){
    ಪೂರ್ಣಾಂಕ j;
    ಗಾಗಿ(j=p->nMaxEqZero; j<p->n_ಲಂಬಸಾಲು; j++) ಪ್ರತಿಜ್ಞಾ( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  ಆದರೆ( iChng<p->nMaxEqZero ){
    ಗಾಗಿ(i=p->nSample-1; i>=0; i--){
      ಪೂರ್ಣಾಂಕ j;
      ಗಾಗಿ(j=iChng; j<p->n_ಲಂಬಸಾಲು; j++){
        ಆದರೆ( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 */

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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
  ಪೂರ್ಣಾಂಕ iChng = sqlite3_ಮೌಲ್ಯ_int(argv[1]);

  ಬಳಸದ_PARAMETER( argc );
  ಬಳಸದ_PARAMETER( context );
  ಪ್ರತಿಜ್ಞಾ( p->n_ಲಂಬಸಾಲು>0 );
  ಪ್ರತಿಜ್ಞಾ( iChng<p->n_ಲಂಬಸಾಲು );

  ಆದರೆ( p->n_ಅಡ್ಡಸಾಲು==0 ){
    /* This is the first call to this function. Do initialization. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++) p->current.anEq[i] = 1;
#ಆದರೆ_ಅಂತ್ಯ
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Second and subsequent calls get processed here */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    ಆದರೆ( p->mxSample ) sample_ತಳ್ಳು_ಹಿಂದಿನ(p, iChng);
#ಆದರೆ_ಅಂತ್ಯ

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    ಗಾಗಿ(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಗಾಗಿ(i=iChng; i<p->n_ಲಂಬಸಾಲು; i++){
      p->current.anDLt[i]++;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
      ಆದರೆ( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#ಆದರೆ_ಅಂತ್ಯ
    }
  }

  p->n_ಅಡ್ಡಸಾಲು++;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( p->mxSample ){
    tRowcnt nLt;
    ಆದರೆ( sqlite3_ಮೌಲ್ಯ_type(argv[2])==SQLITE_INTEGER ){
      sample_ಗಣ_Rowid_Int64(p->db, &p->current, sqlite3_ಮೌಲ್ಯ_int64(argv[2]));
    }ಇಲ್ಲದಿದ್ದರೆ{
      sample_ಗಣ_Rowid(p->db, &p->current, sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[2]),
                                         sqlite3_ಮೌಲ್ಯ_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->n_ಲಂಬಸಾಲು-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    ಆದರೆ( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.i_ಲಂಬಸಾಲು = 0;
      sample_ಸೇರಿಸು(p, &p->current, p->n_ಲಂಬಸಾಲು-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    ಗಾಗಿ(i=0; i<(p->n_ಲಂಬಸಾಲು-1); i++){
      p->current.i_ಲಂಬಸಾಲು = i;
      ಆದರೆ( i>=iChng || sampleIsBetterPost(p, &p->current, &p->a_ಉತ್ತಮ[i]) ){
        sampleCopy(p, &p->a_ಉತ್ತಮ[i], &p->current);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->n_ಎಲ್ಲೆ && p->n_ಅಡ್ಡಸಾಲು>(tRowcnt)p->n_ಎಲ್ಲೆ*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_ಫಲಿತಾಂಶ_int(context, p->current.anDLt[0]>0);
  }
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ FuncDef stat_ತಳ್ಳು_Funcdef = {
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

#ವ್ಯಾಖ್ಯಾನಿಸು STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#ವ್ಯಾಖ್ಯಾನಿಸು STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#ವ್ಯಾಖ್ಯಾನಿಸು STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#ವ್ಯಾಖ್ಯಾನಿಸು STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#ವ್ಯಾಖ್ಯಾನಿಸು STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

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
  ಪೂರ್ಣಾಂಕ argc,
  sqlite3_ಮೌಲ್ಯ **argv
){
  StatAccum *p = (StatAccum*)sqlite3_ಮೌಲ್ಯ_blob(argv[0]);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  ಪೂರ್ಣಾಂಕ eCall = sqlite3_ಮೌಲ್ಯ_int(argv[1]);
  ಪ್ರತಿಜ್ಞಾ( argc==2 );
  ಪ್ರತಿಜ್ಞಾ( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  ಪ್ರತಿಜ್ಞಾ( eCall==STAT_GET_STAT1 || p->mxSample );
  ಆದರೆ( eCall==STAT_GET_STAT1 )
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( argc==1 );
#ಆದರೆ_ಅಂತ್ಯ
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
    ಪೂರ್ಣಾಂಕ i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_ಕೀಲಿ_ಲಂಬಸಾಲು+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->n_ಅಡ್ಡಸಾಲು);
    ಗಾಗಿ(i=0; i<p->n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->n_ಅಡ್ಡಸಾಲು + nDistinct - 1) / nDistinct;
      ಆದರೆ( iVal==2 && p->n_ಅಡ್ಡಸಾಲು*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
      ಪ್ರತಿಜ್ಞಾ( p->current.anEq[i] || p->n_ಅಡ್ಡಸಾಲು==0 );
#ಆದರೆ_ಅಂತ್ಯ
    }
    sqlite3_ಫಲಿತಾಂಶ_str(context, &sStat, SQLITE_XFER);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eCall==STAT_GET_ROWID ){
    ಆದರೆ( p->iGet<0 ){
      sample_ತಳ್ಳು_ಹಿಂದಿನ(p, 0);
      p->iGet = 0;
    }
    ಆದರೆ( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      ಆದರೆ( pS->nRowid==0 ){
        sqlite3_ಫಲಿತಾಂಶ_int64(context, pS->u.iRowid);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    ಪೂರ್ಣಾಂಕ i;

    ಪ್ರತಿಜ್ಞಾ( p->iGet<p->nSample );
    ಆಯ್ಕೆ( eCall ){
      ಸ್ಥಿತಿ STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        ನಿಲ್ಲಿಸು;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->n_ಲಂಬಸಾಲು*100);
    ಗಾಗಿ(i=0; i<p->n_ಲಂಬಸಾಲು; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    ಆದರೆ( sStat.nChar ) sStat.nChar--;
    sqlite3_ಫಲಿತಾಂಶ_str(context, &sStat, SQLITE_XFER);
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEBUG
  ಬಳಸದ_PARAMETER( argc );
#ಆದರೆ_ಅಂತ್ಯ
}
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ FuncDef statGetFuncdef = {
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

ಸ್ಥಿರ ಶೂನ್ಯ callStatGet(Parse *pParse, ಪೂರ್ಣಾಂಕ regStat, ಪೂರ್ಣಾಂಕ iParam, ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#ಅಥವಾಆದರೆ SQLITE_DEBUG
  ಪ್ರತಿಜ್ಞಾ( iParam==STAT_GET_STAT1 );
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER( iParam );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( reg_ಹೊರಹೋಗು!=regStat && reg_ಹೊರಹೋಗು!=regStat+1 );
  sqlite3_Vdbe_ಕೂಡಿಸು_Function_Call(pParse, 0, regStat, reg_ಹೊರಹೋಗು, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyze_Vdbe_Comment_ಸೂಚ್ಯಂಕ_With_ಸ್ತಂಭ_Name(
  Vdbe *v,         /* Prepared statement under construction */
  ಸೂಚ್ಯಂಕ *pIdx,     /* Index whose column is being loaded */
  ಪೂರ್ಣಾಂಕ k            /* Which column index */
){
  ಪೂರ್ಣಾಂಕ i;           /* Index of column in the table */
  ಪ್ರತಿಜ್ಞಾ( k>=0 && k<pIdx->n_ಸ್ತಂಭ );
  i = pIdx->ai_ಸ್ತಂಭ[k];
  ಆದರೆ( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i==XN_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪ್ರತಿಜ್ಞಾ( pIdx->b_Has_ಅಭಿವ್ಯಕ್ತಿ );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }ಇಲ್ಲದಿದ್ದರೆ{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->p_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i].zCnName));
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define analyze_Vdbe_Comment_ಸೂಚ್ಯಂಕ_With_ಸ್ತಂಭ_Name(a,b,c)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyze_One_ಕೋಷ್ಟಕ(
  Parse *pParse,   /* Parser context */
  ಕೋಷ್ಟಕ *pTab,     /* Table whose indices are to be analyzed */
  ಸೂಚ್ಯಂಕ *pOnlyIdx, /* If not NULL, only analyze this one index */
  ಪೂರ್ಣಾಂಕ iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  ಪೂರ್ಣಾಂಕ iMem,        /* Available memory locations begin here */
  ಪೂರ್ಣಾಂಕ iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  ಸೂಚ್ಯಂಕ *pIdx;                 /* An index to being analyzed */
  ಪೂರ್ಣಾಂಕ iIdxCur;                 /* Cursor open on index being analyzed */
  ಪೂರ್ಣಾಂಕ iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  ಪೂರ್ಣಾಂಕ i;                       /* Loop counter */
  ಪೂರ್ಣಾಂಕ j_Zero_ಅಡ್ಡಸಾಲುಗಳು = -1;          /* Jump from here if number of rows is zero */
  ಪೂರ್ಣಾಂಕ iDb;                     /* Index of database containing pTab */
  u8 need_ಕೋಷ್ಟಕ_Cnt = 1;         /* True to count the table */
  ಪೂರ್ಣಾಂಕ regNewRowid = iMem++;    /* Rowid for the inserted record */
  ಪೂರ್ಣಾಂಕ regStat = iMem++;        /* Register to hold StatAccum object */
  ಪೂರ್ಣಾಂಕ regChng = iMem++;        /* Index of changed index field */
  ಪೂರ್ಣಾಂಕ regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  ಪೂರ್ಣಾಂಕ reg_ತಾತ್ಕಾಲಿಕ = iMem++;        /* Temporary use register */
  ಪೂರ್ಣಾಂಕ regTemp2 = iMem++;       /* Second temporary use register */
  ಪೂರ್ಣಾಂಕ regTabname = iMem++;     /* Register containing table name */
  ಪೂರ್ಣಾಂಕ regIdxname = iMem++;     /* Register containing index name */
  ಪೂರ್ಣಾಂಕ regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  ಪೂರ್ಣಾಂಕ regPrev = iMem;          /* MUST BE LAST (see below) */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಪೂರ್ಣಾಂಕ doOnce = 1;              /* Flag for a one-time computation */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
  ಕೋಷ್ಟಕ *pStat1 = 0;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_Touch_ರಿಜಿಸ್ಟರ್(pParse, iMem);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_No_Temps_ಒಳ_Range(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v==0 || NEVER(pTab==0) ){
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
  ಆದರೆ( db->xPreUpdateCallback ){
    pStat1 = (ಕೋಷ್ಟಕ*)sqlite3DbMallocZero(db, ಗಾತ್ರ(ಕೋಷ್ಟಕ) + 13);
    ಆದರೆ( pStat1==0 ) ಹಿಂತಿರುಗಿ;
    pStat1->zName = (ಅಕ್ಷರ*)&pStat1[1];
    ಸ್ಮರಣೆ_ನಕಲು(pStat1->zName, "sqlite_stat1", 13);
    pStat1->n_ಲಂಬಸಾಲು = 3;
    pStat1->iPKey = -1;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(pParse->pVdbe, OP_Noop, 0, 0, 0,(ಅಕ್ಷರ*)pStat1,P4_DYNAMIC);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Establish a read-lock on the table at the shared-cache level. 
  ** Open a read-only cursor on the table. Also allocate a cursor number
  ** to use for scanning indexes (iIdxCur). No index cursor is opened at
  ** this time though.  */
  sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = MAX(pParse->nTab, iTab);
  sqlite3_Open_ಕೋಷ್ಟಕ(pParse, iTabCur, iDb, pTab, OP_OpenRead);
  sqlite3_Vdbe_ತುಂಬು_String(v, regTabname, pTab->zName);

  ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                     /* Number of columns in pIdx. "N" */
    ಪೂರ್ಣಾಂಕ addr_Goto_ಕೊನೆ;               /* Address of "OP_Rewind iIdxCur" */
    ಪೂರ್ಣಾಂಕ addr_ಮುಂದಿನ_ಅಡ್ಡಸಾಲು;              /* Address of "next_row:" */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zIdxName;         /* Name of the index */
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು_Test;                 /* Number of columns to test for changes */

    ಆದರೆ( pOnlyIdx && pOnlyIdx!=pIdx ) ಮುಂದುವರಿ;
    ಆದರೆ( pIdx->pPartIdxWhere==0 ) need_ಕೋಷ್ಟಕ_Cnt = 0;
    ಆದರೆ( !HasRowid(pTab) && Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) ){
      n_ಲಂಬಸಾಲು = pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
      zIdxName = pTab->zName;
      n_ಲಂಬಸಾಲು_Test = n_ಲಂಬಸಾಲು - 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_ಲಂಬಸಾಲು = pIdx->n_ಸ್ತಂಭ;
      zIdxName = pIdx->zName;
      n_ಲಂಬಸಾಲು_Test = pIdx->uniqNotNull ? pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು-1 : n_ಲಂಬಸಾಲು-1;
    }

    /* Populate the register containing the index name. */
    sqlite3_Vdbe_ತುಂಬು_String(v, regIdxname, zIdxName);
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
    sqlite3_Touch_ರಿಜಿಸ್ಟರ್(pParse, regPrev+n_ಲಂಬಸಾಲು_Test);

    /* Open a read-only cursor on the index being analyzed. */
    ಪ್ರತಿಜ್ಞಾ( iDb==sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pIdx->pSchema) );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pIdx);
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
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, db->n_Analysis_ಎಲ್ಲೆ, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, n_ಲಂಬಸಾಲು, regStat+1);
    ಪ್ರತಿಜ್ಞಾ( regRowid==regStat+2 );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು, regRowid);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಎಣಿಕೆ, iIdxCur, reg_ತಾತ್ಕಾಲಿಕ,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3_Vdbe_ಕೂಡಿಸು_Function_Call(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addr_Goto_ಕೊನೆ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regChng);
    addr_ಮುಂದಿನ_ಅಡ್ಡಸಾಲು = sqlite3VdbeCurrentAddr(v);

    ಆದರೆ( n_ಲಂಬಸಾಲು_Test>0 ){
      ಪೂರ್ಣಾಂಕ ಕೊನೆ_Distinct_Test = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*n_ಲಂಬಸಾಲು_Test);
      ಆದರೆ( aGotoChng==0 ) ಮುಂದುವರಿ;

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
      sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
      addr_ಮುಂದಿನ_ಅಡ್ಡಸಾಲು = sqlite3VdbeCurrentAddr(v);
      ಆದರೆ( n_ಲಂಬಸಾಲು_Test==1 && pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು==1 && Is_Unique_ಸೂಚ್ಯಂಕ(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NotNull, regPrev, ಕೊನೆ_Distinct_Test);
        VdbeCoverage(v);
      }
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು_Test; i++){
        ಅಕ್ಷರ *pColl = (ಅಕ್ಷರ*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, i, regChng);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iIdxCur, i, reg_ತಾತ್ಕಾಲಿಕ);
        analyze_Vdbe_Comment_ಸೂಚ್ಯಂಕ_With_ಸ್ತಂಭ_Name(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Ne, reg_ತಾತ್ಕಾಲಿಕ, 0, regPrev+i, pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, n_ಲಂಬಸಾಲು_Test, regChng);
      sqlite3VdbeGoto(v, ಕೊನೆ_Distinct_Test);
  
  
      /*
      **  chng_addr_0:
      **   regPrev(0) = idx(0)
      **  chng_addr_1:
      **   regPrev(1) = idx(1)
      **  ...
      */
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_ಮುಂದಿನ_ಅಡ್ಡಸಾಲು-1);
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು_Test; i++){
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, aGotoChng[i]);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iIdxCur, i, regPrev+i);
        analyze_Vdbe_Comment_ಸೂಚ್ಯಂಕ_With_ಸ್ತಂಭ_Name(v,pIdx,i);
      }
      sqlite3VdbeResolveLabel(v, ಕೊನೆ_Distinct_Test);
      sqlite3DbFree(db, aGotoChng);
    }
  
    /*
    **  chng_addr_N:
    **   regRowid = idx(rowid)            // STAT4 only
    **   stat_push(P, regChng, regRowid)  // 3rd parameter STAT4 only
    **   Next csr
    **   if !eof(csr) goto next_row;
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    ಆದರೆ( OptimizationEnabled(db, SQLITE_Stat4) ){
      ಪ್ರತಿಜ್ಞಾ( regRowid==(regStat+2) );
      ಆದರೆ( HasRowid(pTab) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IdxRowid, iIdxCur, regRowid);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx->p_ಕೋಷ್ಟಕ);
        ಪೂರ್ಣಾಂಕ j, k, reg_ಕೀಲಿ;
        reg_ಕೀಲಿ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
        ಗಾಗಿ(j=0; j<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; j++){
          k = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pIdx, pPk->ai_ಸ್ತಂಭ[j]);
          ಪ್ರತಿಜ್ಞಾ( k>=0 && k<pIdx->n_ಸ್ತಂಭ );
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iIdxCur, k, reg_ಕೀಲಿ+j);
          analyze_Vdbe_Comment_ಸೂಚ್ಯಂಕ_With_ಸ್ತಂಭ_Name(v,pIdx,k);
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ಕೀಲಿ, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು, regRowid);
        sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ಕೀಲಿ, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪ್ರತಿಜ್ಞಾ( regChng==(regStat+1) );
    {
      sqlite3_Vdbe_ಕೂಡಿಸು_Function_Call(pParse, 1, regStat, reg_ತಾತ್ಕಾಲಿಕ, 2+IsStat4,
                                 &stat_ತಳ್ಳು_Funcdef, 0);
      ಆದರೆ( db->n_Analysis_ಎಲ್ಲೆ ){
        ಪೂರ್ಣಾಂಕ j1, j2, j3;
        j1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, reg_ತಾತ್ಕಾಲಿಕ); VdbeCoverage(v);
        j2 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_If, reg_ತಾತ್ಕಾಲಿಕ); VdbeCoverage(v);
        j3 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_SeekGT, iIdxCur, 0, regPrev, 1);
        VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, j1);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iIdxCur, addr_ಮುಂದಿನ_ಅಡ್ಡಸಾಲು); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, j2);
        sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, j3);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iIdxCur, addr_ಮುಂದಿನ_ಅಡ್ಡಸಾಲು); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    ಆದರೆ( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_Goto_ಕೊನೆ);
      addr_Goto_ಕೊನೆ = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    ಪ್ರತಿಜ್ಞಾ( "BBB"[0]==SQLITE_AFF_ಪಠ್ಯ );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, regTabname, 3, reg_ತಾತ್ಕಾಲಿಕ, "BBB", 0);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iStatCur, reg_ತಾತ್ಕಾಲಿಕ, regNewRowid);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (ಅಕ್ಷರ*)pStat1, P4_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    ಆದರೆ( OptimizationEnabled(db, SQLITE_Stat4) && db->n_Analysis_ಎಲ್ಲೆ==0 ){
      ಪೂರ್ಣಾಂಕ regEq = regStat1;
      ಪೂರ್ಣಾಂಕ regLt = regStat1+1;
      ಪೂರ್ಣಾಂಕ regDLt = regStat1+2;
      ಪೂರ್ಣಾಂಕ regSample = regStat1+3;
      ಪೂರ್ಣಾಂಕ reg_ಲಂಬಸಾಲು = regStat1+4;
      ಪೂರ್ಣಾಂಕ regSampleRowid = reg_ಲಂಬಸಾಲು + n_ಲಂಬಸಾಲು;
      ಪೂರ್ಣಾಂಕ addr_ಮುಂದಿನ;
      ಪೂರ್ಣಾಂಕ addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_Not_ಸಿಕ್ಕಿತು;

      /* No STAT4 data is generated if the number of rows is zero */
      ಆದರೆ( addr_Goto_ಕೊನೆ==0 ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addr_Goto_ಕೊನೆ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      ಆದರೆ( doOnce ){
        ಪೂರ್ಣಾಂಕ mx_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು;
        ಸೂಚ್ಯಂಕ *pX;

        /* Compute the maximum number of columns in any index */
        ಗಾಗಿ(pX=pTab->p_ಸೂಚ್ಯಂಕ; pX; pX=pX->p_ಮುಂದಿನ){
          ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು_X;                     /* Number of columns in pX */
          ಆದರೆ( !HasRowid(pTab) && Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pX) ){
            n_ಲಂಬಸಾಲು_X = pX->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
          }ಇಲ್ಲದಿದ್ದರೆ{
            n_ಲಂಬಸಾಲು_X = pX->n_ಸ್ತಂಭ;
          }
          ಆದರೆ( n_ಲಂಬಸಾಲು_X>mx_ಲಂಬಸಾಲು ) mx_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು_X;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3_Touch_ರಿಜಿಸ್ಟರ್(pParse, reg_ಲಂಬಸಾಲು+mx_ಲಂಬಸಾಲು);
        doOnce = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3_No_Temps_ಒಳ_Range(pParse, regEq, reg_ಲಂಬಸಾಲು+mx_ಲಂಬಸಾಲು) );
#ಆದರೆ_ಅಂತ್ಯ
        sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(pParse);  /* tag-20230325-1 */
        ಪ್ರತಿಜ್ಞಾ( sqlite3_No_Temps_ಒಳ_Range(pParse, regEq, reg_ಲಂಬಸಾಲು+mx_ಲಂಬಸಾಲು) );
      }
      ಪ್ರತಿಜ್ಞಾ( sqlite3_No_Temps_ಒಳ_Range(pParse, regEq, reg_ಲಂಬಸಾಲು+n_ಲಂಬಸಾಲು) );

      addr_ಮುಂದಿನ = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, seekOp, iTabCur, addr_ಮುಂದಿನ, regSampleRowid, 0);
      VdbeCoverage(v);
      ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತುಂಬು_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ(pParse, pIdx, iTabCur, i, reg_ಲಂಬಸಾಲು+i);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ಲಂಬಸಾಲು, n_ಲಂಬಸಾಲು, regSample);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, regTabname, 6, reg_ತಾತ್ಕಾಲಿಕ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iStatCur+1, reg_ತಾತ್ಕಾಲಿಕ, regNewRowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 1, addr_ಮುಂದಿನ); /* P1==1 for end-of-loop */
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrIsNull);
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    ಆದರೆ( addr_Goto_ಕೊನೆ ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_Goto_ಕೊನೆ);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  ಆದರೆ( pOnlyIdx==0 && need_ಕೋಷ್ಟಕ_Cnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಎಣಿಕೆ, iTabCur, regStat1);
    j_Zero_ಅಡ್ಡಸಾಲುಗಳು = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, regIdxname);
    ಪ್ರತಿಜ್ಞಾ( "BBB"[0]==SQLITE_AFF_ಪಠ್ಯ );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, regTabname, 3, reg_ತಾತ್ಕಾಲಿಕ, "BBB", 0);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iStatCur, reg_ತಾತ್ಕಾಲಿಕ, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (ಅಕ್ಷರ*)pStat1, P4_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, j_Zero_ಅಡ್ಡಸಾಲುಗಳು);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Analysis(Parse *pParse, ಪೂರ್ಣಾಂಕ iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  ಆದರೆ( v ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ತುಂಬು_Analysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyzeDatabase(Parse *pParse, ಪೂರ್ಣಾಂಕ iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  ಪೂರ್ಣಾಂಕ iStatCur;
  ಪೂರ್ಣಾಂಕ iMem;
  ಪೂರ್ಣಾಂಕ iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  open_Stat_ಕೋಷ್ಟಕ(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಗಾಗಿ(k=sqlite_Hash_ಮೊದಲ(&pSchema->tblHash); k; k=sqlite_Hash_ಮುಂದಿನ(k)){
    ಕೋಷ್ಟಕ *pTab = (ಕೋಷ್ಟಕ*)sqlite_Hash_ದತ್ತಾಂಶ(k);
    analyze_One_ಕೋಷ್ಟಕ(pParse, pTab, 0, iStatCur, iMem, iTab);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    iMem = sqlite3_ಮೊದಲ_ಲಭ್ಯ_ರಿಜಿಸ್ಟರ್(pParse, iMem);
#ಇಲ್ಲದಿದ್ದರೆ
    ಪ್ರತಿಜ್ಞಾ( iMem==sqlite3_ಮೊದಲ_ಲಭ್ಯ_ರಿಜಿಸ್ಟರ್(pParse,iMem) );
#ಆದರೆ_ಅಂತ್ಯ
  }
  ತುಂಬು_Analysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ analyze_ಕೋಷ್ಟಕ(Parse *pParse, ಕೋಷ್ಟಕ *pTab, ಸೂಚ್ಯಂಕ *pOnlyIdx){
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ iStatCur;

  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  ಆದರೆ( pOnlyIdx ){
    open_Stat_ಕೋಷ್ಟಕ(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }ಇಲ್ಲದಿದ್ದರೆ{
    open_Stat_ಕೋಷ್ಟಕ(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyze_One_ಕೋಷ್ಟಕ(pParse, pTab, pOnlyIdx, iStatCur,pParse->nMem+1,pParse->nTab);
  ತುಂಬು_Analysis(pParse, iDb);
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
  ಪೂರ್ಣಾಂಕ iDb;
  ಪೂರ್ಣಾಂಕ i;
  ಅಕ್ಷರ *z, *zDb;
  ಕೋಷ್ಟಕ *pTab;
  ಸೂಚ್ಯಂಕ *pIdx;
  Token *p_ಕೋಷ್ಟಕ_Name;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  ಆದರೆ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ಹಿಂತಿರುಗಿ;
  }

  ಪ್ರತಿಜ್ಞಾ( pName2!=0 || pName1==0 );
  ಆದರೆ( pName1==0 ){
    /* Form 1:  Analyze everything */
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      ಆದರೆ( i==1 ) ಮುಂದುವರಿ;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pName2->n==0 && (iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &p_ಕೋಷ್ಟಕ_Name);
    ಆದರೆ( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, p_ಕೋಷ್ಟಕ_Name);
      ಆದರೆ( z ){
        ಆದರೆ( (pIdx = sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(db, z, zDb))!=0 ){
          analyze_ಕೋಷ್ಟಕ(pParse, pIdx->p_ಕೋಷ್ಟಕ, pIdx);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pTab = sqlite3_Locate_ಕೋಷ್ಟಕ(pParse, 0, z, zDb))!=0 ){
          analyze_ಕೋಷ್ಟಕ(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  ಆದರೆ( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ analysisInfo analysisInfo;
ರಚನೆ analysisInfo {
  sqlite3 *db;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
ಸ್ಥಿರ ಶೂನ್ಯ decodeIntArray(
  ಅಕ್ಷರ *zIntArray,       /* String containing int array to decode */
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು,              /* Number of slots in aOut[] */
  tRowcnt *a_ಹೊರಹೋಗು,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ          /* Handle extra flags for this index, if not NULL */
){
  ಅಕ್ಷರ *z = zIntArray;
  ಪೂರ್ಣಾಂಕ c;
  ಪೂರ್ಣಾಂಕ i;
  tRowcnt v;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( z==0 ) z = "";
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( z!=0 );
#ಆದರೆ_ಅಂತ್ಯ
  ಗಾಗಿ(i=0; *z && i<n_ಹೊರಹೋಗು; i++){
    v = 0;
    ಆಗ( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    ಆದರೆ( a_ಹೊರಹೋಗು ) a_ಹೊರಹೋಗು[i] = v;
    ಆದರೆ( aLog ) aLog[i] = sqlite3LogEst(v);
#ಇಲ್ಲದಿದ್ದರೆ
    ಪ್ರತಿಜ್ಞಾ( a_ಹೊರಹೋಗು==0 );
    ಬಳಸದ_PARAMETER(a_ಹೊರಹೋಗು);
    ಪ್ರತಿಜ್ಞಾ( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( *z==' ' ) z++;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_STAT4
  ಪ್ರತಿಜ್ಞಾ( p_ಸೂಚ್ಯಂಕ!=0 ); {
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( p_ಸೂಚ್ಯಂಕ ){
#ಆದರೆ_ಅಂತ್ಯ
    p_ಸೂಚ್ಯಂಕ->bUnordered = 0;
    p_ಸೂಚ್ಯಂಕ->no_Skip_ಪರಿಶೀಲನೆ = 0;
    ಆಗ( z[0] ){
      ಆದರೆ( sqlite3_strglob("unordered*", z)==0 ){
        p_ಸೂಚ್ಯಂಕ->bUnordered = 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        ಪೂರ್ಣಾಂಕ sz = sqlite3Atoi(z+3);
        ಆದರೆ( sz<2 ) sz = 2;
        p_ಸೂಚ್ಯಂಕ->sz_Idx_ಅಡ್ಡಸಾಲು = sqlite3LogEst(sz);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_strglob("noskipscan*", z)==0 ){
        p_ಸೂಚ್ಯಂಕ->no_Skip_ಪರಿಶೀಲನೆ = 1;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_COSTMULT
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        p_ಸೂಚ್ಯಂಕ->p_ಕೋಷ್ಟಕ->ವೆಚ್ಚ_Mult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆಗ( z[0]!=0 && z[0]!=' ' ) z++;
      ಆಗ( z[0]==' ' ) z++;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ analysisLoader(ಶೂನ್ಯ *p_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv, ಅಕ್ಷರ **Not_ಬಳಸಿದ){
  analysisInfo *pInfo = (analysisInfo*)p_ದತ್ತಾಂಶ;
  ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ;
  ಕೋಷ್ಟಕ *p_ಕೋಷ್ಟಕ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;

  ಪ್ರತಿಜ್ಞಾ( argc==3 );
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, argc);

  ಆದರೆ( argv==0 || argv[0]==0 || argv[2]==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  p_ಕೋಷ್ಟಕ = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(pInfo->db, argv[0], pInfo->zDatabase);
  ಆದರೆ( p_ಕೋಷ್ಟಕ==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( argv[1]==0 ){
    p_ಸೂಚ್ಯಂಕ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_stricmp(argv[0],argv[1])==0 ){
    p_ಸೂಚ್ಯಂಕ = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(p_ಕೋಷ್ಟಕ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಸೂಚ್ಯಂಕ = sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  ಆದರೆ( p_ಸೂಚ್ಯಂಕ ){
    tRowcnt *ai_ಅಡ್ಡಸಾಲು_Est = 0;
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = p_ಸೂಚ್ಯಂಕ->n_ಕೀಲಿ_ಲಂಬಸಾಲು+1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    ಆದರೆ( p_ಸೂಚ್ಯಂಕ->ai_ಅಡ್ಡಸಾಲು_Est==0 ){
      p_ಸೂಚ್ಯಂಕ->ai_ಅಡ್ಡಸಾಲು_Est = (tRowcnt*)sqlite3MallocZero(ಗಾತ್ರ(tRowcnt) * n_ಲಂಬಸಾಲು);
      ಆದರೆ( p_ಸೂಚ್ಯಂಕ->ai_ಅಡ್ಡಸಾಲು_Est==0 ) sqlite3OomFault(pInfo->db);
    }
    ai_ಅಡ್ಡಸಾಲು_Est = p_ಸೂಚ್ಯಂಕ->ai_ಅಡ್ಡಸಾಲು_Est;
#ಆದರೆ_ಅಂತ್ಯ
    p_ಸೂಚ್ಯಂಕ->bUnordered = 0;
    decodeIntArray((ಅಕ್ಷರ*)z, n_ಲಂಬಸಾಲು, ai_ಅಡ್ಡಸಾಲು_Est, p_ಸೂಚ್ಯಂಕ->ai_ಅಡ್ಡಸಾಲು_Log_Est, p_ಸೂಚ್ಯಂಕ);
    p_ಸೂಚ್ಯಂಕ->hasStat1 = 1;
    ಆದರೆ( p_ಸೂಚ್ಯಂಕ->pPartIdxWhere==0 ){
      p_ಕೋಷ್ಟಕ->n_ಅಡ್ಡಸಾಲು_Log_Est = p_ಸೂಚ್ಯಂಕ->ai_ಅಡ್ಡಸಾಲು_Log_Est[0];
      p_ಕೋಷ್ಟಕ->tabFlags |= TF_HasStat1;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸೂಚ್ಯಂಕ fakeIdx;
    fakeIdx.sz_Idx_ಅಡ್ಡಸಾಲು = p_ಕೋಷ್ಟಕ->sz_Tab_ಅಡ್ಡಸಾಲು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_COSTMULT
    fakeIdx.p_ಕೋಷ್ಟಕ = p_ಕೋಷ್ಟಕ;
#ಆದರೆ_ಅಂತ್ಯ
    decodeIntArray((ಅಕ್ಷರ*)z, 1, 0, &p_ಕೋಷ್ಟಕ->n_ಅಡ್ಡಸಾಲು_Log_Est, &fakeIdx);
    p_ಕೋಷ್ಟಕ->sz_Tab_ಅಡ್ಡಸಾಲು = fakeIdx.sz_Idx_ಅಡ್ಡಸಾಲು;
    p_ಕೋಷ್ಟಕ->tabFlags |= TF_HasStat1;
  }

  ಹಿಂತಿರುಗಿ 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
ಶೂನ್ಯ sqlite3_Delete_ಸೂಚ್ಯಂಕ_Samples(sqlite3 *db, ಸೂಚ್ಯಂಕ *pIdx){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( pIdx!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( pIdx->aSample ){
    ಪೂರ್ಣಾಂಕ j;
    ಗಾಗಿ(j=0; j<pIdx->nSample; j++){
      ಸೂಚ್ಯಂಕ_Sample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  ಆದರೆ( db->pn_ಬೈಟುಗಳು_Freed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(db);
  ಬಳಸದ_PARAMETER(pIdx);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
ಸ್ಥಿರ ಶೂನ್ಯ initAvgEq(ಸೂಚ್ಯಂಕ *pIdx){
  ಆದರೆ( pIdx ){
    ಸೂಚ್ಯಂಕ_Sample *aSample = pIdx->aSample;
    ಸೂಚ್ಯಂಕ_Sample *pFinal = &aSample[pIdx->nSample-1];
    ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = 1;
    ಆದರೆ( pIdx->n_Sample_ಲಂಬಸಾಲು>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      n_ಲಂಬಸಾಲು = pIdx->n_Sample_ಲಂಬಸಾಲು-1;
      pIdx->aAvgEq[n_ಲಂಬಸಾಲು] = 1;
    }
    ಗಾಗಿ(i_ಲಂಬಸಾಲು=0; i_ಲಂಬಸಾಲು<n_ಲಂಬಸಾಲು; i_ಲಂಬಸಾಲು++){
      ಪೂರ್ಣಾಂಕ nSample = pIdx->nSample;
      ಪೂರ್ಣಾಂಕ i;                    /* Used to iterate through samples */
      tRowcnt ಮೊತ್ತ_Eq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt n_ಅಡ್ಡಸಾಲು;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      ಆದರೆ( !pIdx->ai_ಅಡ್ಡಸಾಲು_Est || i_ಲಂಬಸಾಲು>=pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು || pIdx->ai_ಅಡ್ಡಸಾಲು_Est[i_ಲಂಬಸಾಲು+1]==0 ){
        n_ಅಡ್ಡಸಾಲು = pFinal->anLt[i_ಲಂಬಸಾಲು];
        nDist100 = (i64)100 * pFinal->anDLt[i_ಲಂಬಸಾಲು];
        nSample--;
      }ಇಲ್ಲದಿದ್ದರೆ{
        n_ಅಡ್ಡಸಾಲು = pIdx->ai_ಅಡ್ಡಸಾಲು_Est[0];
        nDist100 = ((i64)100 * pIdx->ai_ಅಡ್ಡಸಾಲು_Est[0]) / pIdx->ai_ಅಡ್ಡಸಾಲು_Est[i_ಲಂಬಸಾಲು+1];
      }
      pIdx->n_ಅಡ್ಡಸಾಲು_Est0 = n_ಅಡ್ಡಸಾಲು;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      ಗಾಗಿ(i=0; i<nSample; i++){
        ಆದರೆ( i==(pIdx->nSample-1)
         || aSample[i].anDLt[i_ಲಂಬಸಾಲು]!=aSample[i+1].anDLt[i_ಲಂಬಸಾಲು] 
        ){
          ಮೊತ್ತ_Eq += aSample[i].anEq[i_ಲಂಬಸಾಲು];
          nSum100 += 100;
        }
      }

      ಆದರೆ( nDist100>nSum100 && ಮೊತ್ತ_Eq<n_ಅಡ್ಡಸಾಲು ){
        avgEq = ((i64)100 * (n_ಅಡ್ಡಸಾಲು - ಮೊತ್ತ_Eq))/(nDist100 - nSum100);
      }
      ಆದರೆ( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[i_ಲಂಬಸಾಲು] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
ಸ್ಥಿರ ಸೂಚ್ಯಂಕ *ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_Or_Primary_ಕೀಲಿ(
  sqlite3 *db,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb
){
  ಸೂಚ್ಯಂಕ *pIdx = sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(db, zName, zDb);
  ಆದರೆ( pIdx==0 ){
    ಕೋಷ್ಟಕ *pTab = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, zName, zDb);
    ಆದರೆ( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
  }
  ಹಿಂತಿರುಗಿ pIdx;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಬು_Stat_Tbl(
  sqlite3 *db,                  /* Database handle */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql1,            /* SQL statement 1 (see above) */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql2,            /* SQL statement 2 (see above) */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb               /* Database name (e.g. "main") */
){
  ಪೂರ್ಣಾಂಕ rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  ಅಕ್ಷರ *zSql;                   /* Text of the SQL statement */
  ಸೂಚ್ಯಂಕ *pPrevIdx = 0;          /* Previous index in the loop */
  ಸೂಚ್ಯಂಕ_Sample *pSample;         /* A slot in pIdx->aSample[] */

  ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  ಆದರೆ( !zSql ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;

  ಆಗ( sqlite3_ಹೆಜ್ಜೆ(pStmt)==SQLITE_ಅಡ್ಡಸಾಲು ){
    ಪೂರ್ಣಾಂಕ n_Idx_ಲಂಬಸಾಲು = 1;              /* Number of columns in stat4 records */

    ಅಕ್ಷರ *z_ಸೂಚ್ಯಂಕ;    /* Index name */
    ಸೂಚ್ಯಂಕ *pIdx;     /* Pointer to the index object */
    ಪೂರ್ಣಾಂಕ nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    z_ಸೂಚ್ಯಂಕ = (ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 0);
    ಆದರೆ( z_ಸೂಚ್ಯಂಕ==0 ) ಮುಂದುವರಿ;
    nSample = sqlite3_ಸ್ತಂಭ_int(pStmt, 1);
    pIdx = ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_Or_Primary_ಕೀಲಿ(db, z_ಸೂಚ್ಯಂಕ, zDb);
    ಪ್ರತಿಜ್ಞಾ( pIdx==0 || pIdx->nSample==0 );
    ಆದರೆ( pIdx==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      ಮುಂದುವರಿ;
    }
    ಪ್ರತಿಜ್ಞಾ( !HasRowid(pIdx->p_ಕೋಷ್ಟಕ) || pIdx->n_ಸ್ತಂಭ==pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು+1 );
    ಆದರೆ( !HasRowid(pIdx->p_ಕೋಷ್ಟಕ) && Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) ){
      n_Idx_ಲಂಬಸಾಲು = pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n_Idx_ಲಂಬಸಾಲು = pIdx->n_ಸ್ತಂಭ;
    }
    pIdx->n_Sample_ಲಂಬಸಾಲು = n_Idx_ಲಂಬಸಾಲು;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(ಸೂಚ್ಯಂಕ_Sample) * nSample);
    nByte += sizeof64(tRowcnt) * n_Idx_ಲಂಬಸಾಲು * 3 * nSample;
    nByte += n_Idx_ಲಂಬಸಾಲು * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    ಆದರೆ( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += n_Idx_ಲಂಬಸಾಲು;
    pIdx->p_ಕೋಷ್ಟಕ->tabFlags |= TF_HasStat4;
    ಗಾಗಿ(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += n_Idx_ಲಂಬಸಾಲು;
      pIdx->aSample[i].anLt = pSpace; pSpace += n_Idx_ಲಂಬಸಾಲು;
      pIdx->aSample[i].anDLt = pSpace; pSpace += n_Idx_ಲಂಬಸಾಲು;
    }
    ಪ್ರತಿಜ್ಞಾ( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  ಆದರೆ( !zSql ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;

  ಆಗ( sqlite3_ಹೆಜ್ಜೆ(pStmt)==SQLITE_ಅಡ್ಡಸಾಲು ){
    ಅಕ್ಷರ *z_ಸೂಚ್ಯಂಕ;                 /* Index name */
    ಸೂಚ್ಯಂಕ *pIdx;                  /* Pointer to the index object */
    ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = 1;                 /* Number of columns in index */

    z_ಸೂಚ್ಯಂಕ = (ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 0);
    ಆದರೆ( z_ಸೂಚ್ಯಂಕ==0 ) ಮುಂದುವರಿ;
    pIdx = ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ_Or_Primary_ಕೀಲಿ(db, z_ಸೂಚ್ಯಂಕ, zDb);
    ಆದರೆ( pIdx==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      ಮುಂದುವರಿ;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    n_ಲಂಬಸಾಲು = pIdx->n_Sample_ಲಂಬಸಾಲು;
    ಆದರೆ( pIdx!=pPrevIdx ){
      initAvgEq(pPrevIdx);
      pPrevIdx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt,1),n_ಲಂಬಸಾಲು,pSample->anEq,0,0);
    decodeIntArray((ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt,2),n_ಲಂಬಸಾಲು,pSample->anLt,0,0);
    decodeIntArray((ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt,3),n_ಲಂಬಸಾಲು,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_ಸ್ತಂಭ_ಬೈಟುಗಳು(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    ಆದರೆ( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    ಆದರೆ( pSample->n ){
      ಸ್ಮರಣೆ_ನಕಲು(pSample->p, sqlite3_ಸ್ತಂಭ_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  ಆದರೆ( rc==SQLITE_OK ) initAvgEq(pPrevIdx);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಬು_Stat4(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Result codes from subroutines */
  ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pStat4;

  ಪ್ರತಿಜ್ಞಾ( db->lookaside.bDisable );
  ಆದರೆ( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, "sqlite_stat4", zDb))!=0
   && Is_Ordinary_ಕೋಷ್ಟಕ(pStat4)
  ){
    rc = ತುಂಬು_Stat_Tbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_STAT4 */

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
ಪೂರ್ಣಾಂಕ sqlite3_Analysis_ತುಂಬು(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb){
  analysisInfo sInfo;
  HashElem *i;
  ಅಕ್ಷರ *zSql;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pStat1;

  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಪ್ರತಿಜ್ಞಾ( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&pSchema->tblHash); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&pSchema->idxHash); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    ಸೂಚ್ಯಂಕ *pIdx = sqlite_Hash_ದತ್ತಾಂಶ(i);
    pIdx->hasStat1 = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
    sqlite3_Delete_ಸೂಚ್ಯಂಕ_Samples(db, pIdx);
    pIdx->aSample = 0;
#ಆದರೆ_ಅಂತ್ಯ
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  ಆದರೆ( (pStat1 = sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(db, "sqlite_stat1", sInfo.zDatabase))
   && Is_Ordinary_ಕೋಷ್ಟಕ(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    ಆದರೆ( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  ಪ್ರತಿಜ್ಞಾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&pSchema->idxHash); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    ಸೂಚ್ಯಂಕ *pIdx = sqlite_Hash_ದತ್ತಾಂಶ(i);
    ಆದರೆ( !pIdx->hasStat1 ) sqlite3_Default_ಅಡ್ಡಸಾಲು_Est(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಆದರೆ( rc==SQLITE_OK ){
    DisableLookaside;
    rc = ತುಂಬು_Stat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  ಗಾಗಿ(i=sqlite_Hash_ಮೊದಲ(&pSchema->idxHash); i; i=sqlite_Hash_ಮುಂದಿನ(i)){
    ಸೂಚ್ಯಂಕ *pIdx = sqlite_Hash_ದತ್ತಾಂಶ(i);
    sqlite3_free(pIdx->ai_ಅಡ್ಡಸಾಲು_Est);
    pIdx->ai_ಅಡ್ಡಸಾಲು_Est = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ಹಿಂತಿರುಗಿ rc;
}


#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_ANALYZE */
