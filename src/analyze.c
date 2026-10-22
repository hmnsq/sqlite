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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ANALYZE
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#ਨਹੀਂਤਾਂ
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਖਾਲੀ open_Stat_ਸਾਰਣੀ(
  Parse *pParse,          /* Parsing context */
  ਪੂਰਨ iDb,                /* The database we are looking in */
  ਪੂਰਨ iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  ਅਚਲ ਅੱਖਰ *zWhere,     /* Delete entries for this table or index */
  ਅਚਲ ਅੱਖਰ *zWhereType  /* Either "tbl" or "idx" */
){
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
    ਅਚਲ ਅੱਖਰ *zName;
    ਅਚਲ ਅੱਖਰ *z_ਖਾਨੇ_ਗਿਣਤੀ;
  } a_ਸਾਰਣੀ[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#ਜੇ defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#ਨਹੀਂਤਾਂ
    { "sqlite_stat4", 0 },
#ਜੇ_ਅੰਤ
    { "sqlite_stat3", 0 },
  };
  ਪੂਰਨ i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_ਜੜ੍ਹ[Array_ਮਾਪ(a_ਸਾਰਣੀ)];
  u8 aCreateTbl[Array_ਮਾਪ(a_ਸਾਰਣੀ)];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਅਚਲ ਪੂਰਨ n_ਮੰਜ਼ਿਲ_Open = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#ਨਹੀਂਤਾਂ
  ਅਚਲ ਪੂਰਨ n_ਮੰਜ਼ਿਲ_Open = 1;
#ਜੇ_ਅੰਤ

  ਜੇ( v==0 ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  ਲਈ(i=0; i<Array_ਮਾਪ(a_ਸਾਰਣੀ); i++){
    ਅਚਲ ਅੱਖਰ *zTab = a_ਸਾਰਣੀ[i].zName;
    ਸਾਰਣੀ *pStat;
    aCreateTbl[i] = 0;
    ਜੇ( (pStat = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zTab, pDb->zDbSName))==0 ){
      ਜੇ( i<n_ਮੰਜ਼ਿਲ_Open ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, a_ਸਾਰਣੀ[i].z_ਖਾਨੇ_ਗਿਣਤੀ
        );
        ਪ੍ਰਤਿਜ੍ਞਾ( pParse->isCreate || pParse->nErr );
        a_ਜੜ੍ਹ[i] = (u32)pParse->u1.cr.reg_ਜੜ੍ਹ;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }ਨਹੀਂਤਾਂ{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_ਜੜ੍ਹ[i] = pStat->tnum;
      sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, a_ਜੜ੍ਹ[i], 1, zTab);
      ਜੇ( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
      }ਨਹੀਂਤਾਂ ਜੇ( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#ਜੇ_ਅੰਤ
      }ਨਹੀਂਤਾਂ{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Clear, (ਪੂਰਨ)a_ਜੜ੍ਹ[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  ਲਈ(i=0; i<n_ਮੰਜ਼ਿਲ_Open; i++){
    ਪ੍ਰਤਿਜ੍ਞਾ( i<Array_ਮਾਪ(a_ਸਾਰਣੀ) );
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_OpenWrite, iStatCur+i, (ਪੂਰਨ)a_ਜੜ੍ਹ[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, a_ਸਾਰਣੀ[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#ਜੇ_ਅੰਤ

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ StatAccum StatAccum;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ StatSample StatSample;
ਬਣਤਰ StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  ਸੰਘ {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  ਪੂਰਨ i_ਖਾਨਾ;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#ਜੇ_ਅੰਤ
};                                                    
ਬਣਤਰ StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt n_ਪੰਕਤੀ;             /* Number of rows visited so far */
  ਪੂਰਨ n_ਹੱਦਬੰਦੀ;               /* Analysis row-scan limit */
  ਪੂਰਨ n_ਖਾਨਾ;                 /* Number of columns in index + pk/rowid */
  ਪੂਰਨ n_ਕੁੰਜੀ_ਖਾਨਾ;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  ਪੂਰਨ mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *a_ਉੱਤਮ;        /* Array of nCol best samples */
  ਪੂਰਨ iMin;                 /* Index in a[] of entry with minimum score */
  ਪੂਰਨ nSample;              /* Current number of samples */
  ਪੂਰਨ nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  ਪੂਰਨ iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#ਜੇ_ਅੰਤ
};

/* Reclaim memory used by a StatSample
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
ਸਥਿਰ ਖਾਲੀ sampleClear(sqlite3 *db, StatSample *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#ਜੇ_ਅੰਤ

/* Initialize the BLOB value of a ROWID
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
ਸਥਿਰ ਖਾਲੀ sample_ਸੈੱਟ_Rowid(sqlite3 *db, StatSample *p, ਪੂਰਨ n, ਅਚਲ u8 *p_ਅੰਕੜੇ){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  ਜੇ( p->u.aRowid ){
    p->nRowid = n;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->u.aRowid, p_ਅੰਕੜੇ, n);
  }ਨਹੀਂਤਾਂ{
    p->nRowid = 0;
  }
}
#ਜੇ_ਅੰਤ

/* Initialize the INTEGER value of a ROWID.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
ਸਥਿਰ ਖਾਲੀ sample_ਸੈੱਟ_Rowid_Int64(sqlite3 *db, StatSample *p, i64 iRowid){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#ਜੇ_ਅੰਤ


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
ਸਥਿਰ ਖਾਲੀ sampleCopy(StatAccum *p, StatSample *p_ਮੰਜ਼ਿਲ, StatSample *pFrom){
  p_ਮੰਜ਼ਿਲ->isPSample = pFrom->isPSample;
  p_ਮੰਜ਼ਿਲ->i_ਖਾਨਾ = pFrom->i_ਖਾਨਾ;
  p_ਮੰਜ਼ਿਲ->iHash = pFrom->iHash;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ->anEq, pFrom->anEq, ਆਕਾਰ(tRowcnt)*p->n_ਖਾਨਾ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ->anLt, pFrom->anLt, ਆਕਾਰ(tRowcnt)*p->n_ਖਾਨਾ);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ->anDLt, pFrom->anDLt, ਆਕਾਰ(tRowcnt)*p->n_ਖਾਨਾ);
  ਜੇ( pFrom->nRowid ){
    sample_ਸੈੱਟ_Rowid(p->db, p_ਮੰਜ਼ਿਲ, pFrom->nRowid, pFrom->u.aRowid);
  }ਨਹੀਂਤਾਂ{
    sample_ਸੈੱਟ_Rowid_Int64(p->db, p_ਮੰਜ਼ਿਲ, pFrom->u.iRowid);
  }
}
#ਜੇ_ਅੰਤ

/*
** Reclaim all memory of a StatAccum structure.
*/
ਸਥਿਰ ਖਾਲੀ statAccumDestructor(ਖਾਲੀ *pOld){
  StatAccum *p = (StatAccum*)pOld;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( p->mxSample ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++) sampleClear(p->db, p->a_ਉੱਤਮ+i);
    ਲਈ(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਖਾਲੀ statInit(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  StatAccum *p;
  ਪੂਰਨ n_ਖਾਨਾ;                       /* Number of columns in index being sampled */
  ਪੂਰਨ n_ਕੁੰਜੀ_ਖਾਨਾ;                    /* Number of key columns */
  ਪੂਰਨ n_ਖਾਨਾ_ਉੱਪਰ;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  ਪੂਰਨ mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#ਜੇ_ਅੰਤ

  /* Decode the three function arguments */
  ਅਣਵਰਤਿਆ_PARAMETER(argc);
  n_ਖਾਨਾ = sqlite3_ਮੁੱਲ_int(argv[0]);
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਖਾਨਾ>0 );
  n_ਖਾਨਾ_ਉੱਪਰ = ਆਕਾਰ(tRowcnt)<8 ? (n_ਖਾਨਾ+1)&~1 : n_ਖਾਨਾ;
  n_ਕੁੰਜੀ_ਖਾਨਾ = sqlite3_ਮੁੱਲ_int(argv[1]);
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ_ਖਾਨਾ<=n_ਖਾਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ_ਖਾਨਾ>0 );

  /* Allocate the space required for the StatAccum object */
  n = ਆਕਾਰ(*p) 
    + ਆਕਾਰ(tRowcnt)*n_ਖਾਨਾ_ਉੱਪਰ;                    /* StatAccum.anDLt */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  n += ਆਕਾਰ(tRowcnt)*n_ਖਾਨਾ_ਉੱਪਰ;                   /* StatAccum.anEq */
  ਜੇ( mxSample ){
    n += ਆਕਾਰ(tRowcnt)*n_ਖਾਨਾ_ਉੱਪਰ                  /* StatAccum.anLt */
      + ਆਕਾਰ(StatSample)*(n_ਖਾਨਾ+mxSample)       /* StatAccum.aBest[], a[] */
      + ਆਕਾਰ(tRowcnt)*3*n_ਖਾਨਾ_ਉੱਪਰ*(n_ਖਾਨਾ+mxSample);
  }
#ਜੇ_ਅੰਤ
  p = sqlite3DbMallocZero(db, n);
  ਜੇ( p==0 ){
    sqlite3_ਨਤੀਜਾ_error_nomem(context);
    ਵਾਪਸ;
  }

  p->db = db;
  p->nEst = sqlite3_ਮੁੱਲ_int64(argv[2]);
  p->n_ਪੰਕਤੀ = 0;
  p->n_ਹੱਦਬੰਦੀ = sqlite3_ਮੁੱਲ_int(argv[3]);
  p->n_ਖਾਨਾ = n_ਖਾਨਾ;
  p->n_ਕੁੰਜੀ_ਖਾਨਾ = n_ਕੁੰਜੀ_ਖਾਨਾ;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[n_ਖਾਨਾ_ਉੱਪਰ];
  p->mxSample = p->n_ਹੱਦਬੰਦੀ==0 ? mxSample : 0;
  ਜੇ( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    ਪੂਰਨ i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[n_ਖਾਨਾ_ਉੱਪਰ];
    p->iPrn = 0x689e962d*(u32)n_ਖਾਨਾ ^ 0xd0944565*(u32)sqlite3_ਮੁੱਲ_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (ਬਣਤਰ StatSample*)&p->current.anLt[n_ਖਾਨਾ_ਉੱਪਰ];
    p->a_ਉੱਤਮ = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+n_ਖਾਨਾ]);
    ਲਈ(i=0; i<(mxSample+n_ਖਾਨਾ); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (ਆਕਾਰ(tRowcnt) * n_ਖਾਨਾ_ਉੱਪਰ);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (ਆਕਾਰ(tRowcnt) * n_ਖਾਨਾ_ਉੱਪਰ);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (ਆਕਾਰ(tRowcnt) * n_ਖਾਨਾ_ਉੱਪਰ);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( (pSpace - (u8*)p)==n );
  
    ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
      p->a_ਉੱਤਮ[i].i_ਖਾਨਾ = i;
    }
  }
#ਜੇ_ਅੰਤ

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_ਨਤੀਜਾ_blob(context, p, ਆਕਾਰ(*p), statAccumDestructor);
}
ਸਥਿਰ ਅਚਲ FuncDef statInitFuncdef = {
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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
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
ਸਥਿਰ ਪੂਰਨ sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  ਪੂਰਨ n_ਖਾਨਾ = pAccum->n_ਖਾਨਾ;
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->i_ਖਾਨਾ==pOld->i_ਖਾਨਾ );
  ਲਈ(i=pNew->i_ਖਾਨਾ+1; i<n_ਖਾਨਾ; i++){
    ਜੇ( pNew->anEq[i]>pOld->anEq[i] ) ਵਾਪਸ 1;
    ਜੇ( pNew->anEq[i]<pOld->anEq[i] ) ਵਾਪਸ 0;
  }
  ਜੇ( pNew->iHash>pOld->iHash ) ਵਾਪਸ 1;
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
ਸਥਿਰ ਪੂਰਨ sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->i_ਖਾਨਾ];
  tRowcnt nEqOld = pOld->anEq[pOld->i_ਖਾਨਾ];

  ਪ੍ਰਤਿਜ੍ਞਾ( pOld->isPSample==0 && pNew->isPSample==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( IsStat4 || (pNew->i_ਖਾਨਾ==0 && pOld->i_ਖਾਨਾ==0) );

  ਜੇ( (nEqNew>nEqOld) ) ਵਾਪਸ 1;
  ਜੇ( nEqNew==nEqOld ){
    ਜੇ( pNew->i_ਖਾਨਾ<pOld->i_ਖਾਨਾ ) ਵਾਪਸ 1;
    ਵਾਪਸ (pNew->i_ਖਾਨਾ==pOld->i_ਖਾਨਾ && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  ਵਾਪਸ 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
ਸਥਿਰ ਖਾਲੀ sample_ਪਾਓ(StatAccum *p, StatSample *pNew, ਪੂਰਨ nEqZero){
  StatSample *pSample = 0;
  ਪੂਰਨ i;

  ਪ੍ਰਤਿਜ੍ਞਾ( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  ਜੇ( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  ਜੇ( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew->anEq[pNew->i_ਖਾਨਾ]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    ਲਈ(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      ਜੇ( pOld->anEq[pNew->i_ਖਾਨਾ]==0 ){
        ਜੇ( pOld->isPSample ) ਵਾਪਸ;
        ਪ੍ਰਤਿਜ੍ਞਾ( pOld->i_ਖਾਨਾ>pNew->i_ਖਾਨਾ );
        ਪ੍ਰਤਿਜ੍ਞਾ( sampleIsBetter(p, pNew, pOld) );
        ਜੇ( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    ਜੇ( pUpgrade ){
      pUpgrade->i_ਖਾਨਾ = pNew->i_ਖਾਨਾ;
      pUpgrade->anEq[pUpgrade->i_ਖਾਨਾ] = pNew->anEq[pUpgrade->i_ਖਾਨਾ];
      ਜਾਓ ਲੱਭੋ_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  ਜੇ( p->nSample>=p->mxSample ){
    StatSample *pMin = &p->a[p->iMin];
    tRowcnt *anEq = pMin->anEq;
    tRowcnt *anLt = pMin->anLt;
    tRowcnt *anDLt = pMin->anDLt;
    sampleClear(p->db, pMin);
    memmove(pMin, &pMin[1], ਆਕਾਰ(p->a[0])*(p->nSample-p->iMin-1));
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
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nSample==0 
       || pNew->anLt[p->n_ਖਾਨਾ-1] > p->a[p->nSample-1].anLt[p->n_ਖਾਨਾ-1] );

  /* Insert the new sample */
  pSample = &p->a[p->nSample];
  sampleCopy(p, pSample, pNew);
  p->nSample++;

  /* Zero the first nEqZero entries in the anEq[] array. */
  memset(pSample->anEq, 0, ਆਕਾਰ(tRowcnt)*nEqZero);

ਲੱਭੋ_new_min:
  ਜੇ( p->nSample>=p->mxSample ){
    ਪੂਰਨ iMin = -1;
    ਲਈ(i=0; i<p->mxSample; i++){
      ਜੇ( p->a[i].isPSample ) ਜਾਰੀ;
      ਜੇ( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( iMin>=0 );
    p->iMin = iMin;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
ਸਥਿਰ ਖਾਲੀ sample_ਧੱਕੋ_ਪਿਛਲਾ(StatAccum *p, ਪੂਰਨ iChng){
  ਪੂਰਨ i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  ਲਈ(i=(p->n_ਖਾਨਾ-2); i>=iChng; i--){
    StatSample *p_ਉੱਤਮ = &p->a_ਉੱਤਮ[i];
    p_ਉੱਤਮ->anEq[i] = p->current.anEq[i];
    ਜੇ( p->nSample<p->mxSample || sampleIsBetter(p, p_ਉੱਤਮ, &p->a[p->iMin]) ){
      sample_ਪਾਓ(p, p_ਉੱਤਮ, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  ਲਈ(i=p->nSample-1; i>=0; i--){
    ਪੂਰਨ j;
    ਲਈ(j=p->nMaxEqZero; j<p->n_ਖਾਨਾ; j++) ਪ੍ਰਤਿਜ੍ਞਾ( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  ਜੇ( iChng<p->nMaxEqZero ){
    ਲਈ(i=p->nSample-1; i>=0; i--){
      ਪੂਰਨ j;
      ਲਈ(j=iChng; j<p->n_ਖਾਨਾ; j++){
        ਜੇ( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

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
ਸਥਿਰ ਖਾਲੀ stat_ਧੱਕੋ(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_ਮੁੱਲ_blob(argv[0]);
  ਪੂਰਨ iChng = sqlite3_ਮੁੱਲ_int(argv[1]);

  ਅਣਵਰਤਿਆ_PARAMETER( argc );
  ਅਣਵਰਤਿਆ_PARAMETER( context );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਖਾਨਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iChng<p->n_ਖਾਨਾ );

  ਜੇ( p->n_ਪੰਕਤੀ==0 ){
    /* This is the first call to this function. Do initialization. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++) p->current.anEq[i] = 1;
#ਜੇ_ਅੰਤ
  }ਨਹੀਂਤਾਂ{
    /* Second and subsequent calls get processed here */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਜੇ( p->mxSample ) sample_ਧੱਕੋ_ਪਿਛਲਾ(p, iChng);
#ਜੇ_ਅੰਤ

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਲਈ(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#ਜੇ_ਅੰਤ
    ਲਈ(i=iChng; i<p->n_ਖਾਨਾ; i++){
      p->current.anDLt[i]++;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
      ਜੇ( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#ਜੇ_ਅੰਤ
    }
  }

  p->n_ਪੰਕਤੀ++;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( p->mxSample ){
    tRowcnt nLt;
    ਜੇ( sqlite3_ਮੁੱਲ_type(argv[2])==SQLITE_INTEGER ){
      sample_ਸੈੱਟ_Rowid_Int64(p->db, &p->current, sqlite3_ਮੁੱਲ_int64(argv[2]));
    }ਨਹੀਂਤਾਂ{
      sample_ਸੈੱਟ_Rowid(p->db, &p->current, sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[2]),
                                         sqlite3_ਮੁੱਲ_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->n_ਖਾਨਾ-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    ਜੇ( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.i_ਖਾਨਾ = 0;
      sample_ਪਾਓ(p, &p->current, p->n_ਖਾਨਾ-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    ਲਈ(i=0; i<(p->n_ਖਾਨਾ-1); i++){
      p->current.i_ਖਾਨਾ = i;
      ਜੇ( i>=iChng || sampleIsBetterPost(p, &p->current, &p->a_ਉੱਤਮ[i]) ){
        sampleCopy(p, &p->a_ਉੱਤਮ[i], &p->current);
      }
    }
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  ਜੇ( p->n_ਹੱਦਬੰਦੀ && p->n_ਪੰਕਤੀ>(tRowcnt)p->n_ਹੱਦਬੰਦੀ*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_ਨਤੀਜਾ_int(context, p->current.anDLt[0]>0);
  }
}

ਸਥਿਰ ਅਚਲ FuncDef stat_ਧੱਕੋ_Funcdef = {
  2+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  stat_ਧੱਕੋ,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_push",     /* zName */
  {0}
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

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
ਸਥਿਰ ਖਾਲੀ statGet(
  sqlite3_context *context,
  ਪੂਰਨ argc,
  sqlite3_ਮੁੱਲ **argv
){
  StatAccum *p = (StatAccum*)sqlite3_ਮੁੱਲ_blob(argv[0]);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  ਪੂਰਨ eCall = sqlite3_ਮੁੱਲ_int(argv[1]);
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  ਪ੍ਰਤਿਜ੍ਞਾ( eCall==STAT_GET_STAT1 || p->mxSample );
  ਜੇ( eCall==STAT_GET_STAT1 )
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( argc==1 );
#ਜੇ_ਅੰਤ
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
    ਪੂਰਨ i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_ਕੁੰਜੀ_ਖਾਨਾ+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->n_ਪੰਕਤੀ);
    ਲਈ(i=0; i<p->n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->n_ਪੰਕਤੀ + nDistinct - 1) / nDistinct;
      ਜੇ( iVal==2 && p->n_ਪੰਕਤੀ*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
      ਪ੍ਰਤਿਜ੍ਞਾ( p->current.anEq[i] || p->n_ਪੰਕਤੀ==0 );
#ਜੇ_ਅੰਤ
    }
    sqlite3_ਨਤੀਜਾ_str(context, &sStat, SQLITE_XFER);
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਨਹੀਂਤਾਂ ਜੇ( eCall==STAT_GET_ROWID ){
    ਜੇ( p->iGet<0 ){
      sample_ਧੱਕੋ_ਪਿਛਲਾ(p, 0);
      p->iGet = 0;
    }
    ਜੇ( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      ਜੇ( pS->nRowid==0 ){
        sqlite3_ਨਤੀਜਾ_int64(context, pS->u.iRowid);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }ਨਹੀਂਤਾਂ{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    ਪੂਰਨ i;

    ਪ੍ਰਤਿਜ੍ਞਾ( p->iGet<p->nSample );
    ਚੋਣ( eCall ){
      ਹਾਲਤ STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; ਰੋਕੋ;
      ਹਾਲਤ STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; ਰੋਕੋ;
      ਮੂਲ: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        ਰੋਕੋ;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->n_ਖਾਨਾ*100);
    ਲਈ(i=0; i<p->n_ਖਾਨਾ; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    ਜੇ( sStat.nChar ) sStat.nChar--;
    sqlite3_ਨਤੀਜਾ_str(context, &sStat, SQLITE_XFER);
  }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਅਣਵਰਤਿਆ_PARAMETER( argc );
#ਜੇ_ਅੰਤ
}
ਸਥਿਰ ਅਚਲ FuncDef statGetFuncdef = {
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

ਸਥਿਰ ਖਾਲੀ callStatGet(Parse *pParse, ਪੂਰਨ regStat, ਪੂਰਨ iParam, ਪੂਰਨ reg_ਬਾਹਰਲਾ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  sqlite3_Vdbe_ਜੋੜੋ_Op2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#ਜਾਂਜੇ SQLITE_DEBUG
  ਪ੍ਰਤਿਜ੍ਞਾ( iParam==STAT_GET_STAT1 );
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER( iParam );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( reg_ਬਾਹਰਲਾ!=regStat && reg_ਬਾਹਰਲਾ!=regStat+1 );
  sqlite3_Vdbe_ਜੋੜੋ_Function_Call(pParse, 0, regStat, reg_ਬਾਹਰਲਾ, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
ਸਥਿਰ ਖਾਲੀ analyze_Vdbe_Comment_ਸੂਚਕਾਂਕ_With_ਕਾਲਮ_Name(
  Vdbe *v,         /* Prepared statement under construction */
  ਸੂਚਕਾਂਕ *pIdx,     /* Index whose column is being loaded */
  ਪੂਰਨ k            /* Which column index */
){
  ਪੂਰਨ i;           /* Index of column in the table */
  ਪ੍ਰਤਿਜ੍ਞਾ( k>=0 && k<pIdx->n_ਕਾਲਮ );
  i = pIdx->ai_ਕਾਲਮ[k];
  ਜੇ( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }ਨਹੀਂਤਾਂ ਜੇ( i==XN_ਵਿਅੰਜਕ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->b_Has_ਵਿਅੰਜਕ );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }ਨਹੀਂਤਾਂ{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[i].zCnName));
  }
}
#ਨਹੀਂਤਾਂ
# define analyze_Vdbe_Comment_ਸੂਚਕਾਂਕ_With_ਕਾਲਮ_Name(a,b,c)
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
ਸਥਿਰ ਖਾਲੀ analyze_One_ਸਾਰਣੀ(
  Parse *pParse,   /* Parser context */
  ਸਾਰਣੀ *pTab,     /* Table whose indices are to be analyzed */
  ਸੂਚਕਾਂਕ *pOnlyIdx, /* If not NULL, only analyze this one index */
  ਪੂਰਨ iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  ਪੂਰਨ iMem,        /* Available memory locations begin here */
  ਪੂਰਨ iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  ਸੂਚਕਾਂਕ *pIdx;                 /* An index to being analyzed */
  ਪੂਰਨ iIdxCur;                 /* Cursor open on index being analyzed */
  ਪੂਰਨ iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  ਪੂਰਨ i;                       /* Loop counter */
  ਪੂਰਨ j_Zero_ਪੰਕਤੀਆਂ = -1;          /* Jump from here if number of rows is zero */
  ਪੂਰਨ iDb;                     /* Index of database containing pTab */
  u8 need_ਸਾਰਣੀ_Cnt = 1;         /* True to count the table */
  ਪੂਰਨ regNewRowid = iMem++;    /* Rowid for the inserted record */
  ਪੂਰਨ regStat = iMem++;        /* Register to hold StatAccum object */
  ਪੂਰਨ regChng = iMem++;        /* Index of changed index field */
  ਪੂਰਨ regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  ਪੂਰਨ reg_ਆਰਜ਼ੀ = iMem++;        /* Temporary use register */
  ਪੂਰਨ regTemp2 = iMem++;       /* Second temporary use register */
  ਪੂਰਨ regTabname = iMem++;     /* Register containing table name */
  ਪੂਰਨ regIdxname = iMem++;     /* Register containing index name */
  ਪੂਰਨ regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  ਪੂਰਨ regPrev = iMem;          /* MUST BE LAST (see below) */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਪੂਰਨ doOnce = 1;              /* Flag for a one-time computation */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
  ਸਾਰਣੀ *pStat1 = 0;
#ਜੇ_ਅੰਤ

  sqlite3_Touch_ਰਜਿਸਟਰ(pParse, iMem);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_No_Temps_ਵਿੱਚ_Range(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  ਜੇ( v==0 || NEVER(pTab==0) ){
    ਵਾਪਸ;
  }
  ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    ਵਾਪਸ;
  }
  ਜੇ( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
  ਜੇ( db->xPreUpdateCallback ){
    pStat1 = (ਸਾਰਣੀ*)sqlite3DbMallocZero(db, ਆਕਾਰ(ਸਾਰਣੀ) + 13);
    ਜੇ( pStat1==0 ) ਵਾਪਸ;
    pStat1->zName = (ਅੱਖਰ*)&pStat1[1];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pStat1->zName, "sqlite_stat1", 13);
    pStat1->n_ਖਾਨਾ = 3;
    pStat1->iPKey = -1;
    sqlite3_Vdbe_ਜੋੜੋ_Op4(pParse->pVdbe, OP_Noop, 0, 0, 0,(ਅੱਖਰ*)pStat1,P4_DYNAMIC);
  }
#ਜੇ_ਅੰਤ

  /* Establish a read-lock on the table at the shared-cache level. 
  ** Open a read-only cursor on the table. Also allocate a cursor number
  ** to use for scanning indexes (iIdxCur). No index cursor is opened at
  ** this time though.  */
  sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = MAX(pParse->nTab, iTab);
  sqlite3_Open_ਸਾਰਣੀ(pParse, iTabCur, iDb, pTab, OP_OpenRead);
  sqlite3_Vdbe_ਲੱਦੋ_String(v, regTabname, pTab->zName);

  ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
    ਪੂਰਨ n_ਖਾਨਾ;                     /* Number of columns in pIdx. "N" */
    ਪੂਰਨ addr_Goto_ਸਿਰਾ;               /* Address of "OP_Rewind iIdxCur" */
    ਪੂਰਨ addr_ਅਗਲਾ_ਪੰਕਤੀ;              /* Address of "next_row:" */
    ਅਚਲ ਅੱਖਰ *zIdxName;         /* Name of the index */
    ਪੂਰਨ n_ਖਾਨਾ_Test;                 /* Number of columns to test for changes */

    ਜੇ( pOnlyIdx && pOnlyIdx!=pIdx ) ਜਾਰੀ;
    ਜੇ( pIdx->pPartIdxWhere==0 ) need_ਸਾਰਣੀ_Cnt = 0;
    ਜੇ( !HasRowid(pTab) && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ){
      n_ਖਾਨਾ = pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ;
      zIdxName = pTab->zName;
      n_ਖਾਨਾ_Test = n_ਖਾਨਾ - 1;
    }ਨਹੀਂਤਾਂ{
      n_ਖਾਨਾ = pIdx->n_ਕਾਲਮ;
      zIdxName = pIdx->zName;
      n_ਖਾਨਾ_Test = pIdx->uniqNotNull ? pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ-1 : n_ਖਾਨਾ-1;
    }

    /* Populate the register containing the index name. */
    sqlite3_Vdbe_ਲੱਦੋ_String(v, regIdxname, zIdxName);
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
    sqlite3_Touch_ਰਜਿਸਟਰ(pParse, regPrev+n_ਖਾਨਾ_Test);

    /* Open a read-only cursor on the index being analyzed. */
    ਪ੍ਰਤਿਜ੍ਞਾ( iDb==sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pIdx->pSchema) );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pIdx);
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
    ਪ੍ਰਤਿਜ੍ਞਾ( regTemp2==regStat+4 );
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, db->n_Analysis_ਹੱਦਬੰਦੀ, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, n_ਖਾਨਾ, regStat+1);
    ਪ੍ਰਤਿਜ੍ਞਾ( regRowid==regStat+2 );
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ, regRowid);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਗਿਣਤੀ, iIdxCur, reg_ਆਰਜ਼ੀ,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3_Vdbe_ਜੋੜੋ_Function_Call(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addr_Goto_ਸਿਰਾ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regChng);
    addr_ਅਗਲਾ_ਪੰਕਤੀ = sqlite3VdbeCurrentAddr(v);

    ਜੇ( n_ਖਾਨਾ_Test>0 ){
      ਪੂਰਨ ਸਿਰਾ_Distinct_Test = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, ਆਕਾਰ(ਪੂਰਨ)*n_ਖਾਨਾ_Test);
      ਜੇ( aGotoChng==0 ) ਜਾਰੀ;

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
      sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
      addr_ਅਗਲਾ_ਪੰਕਤੀ = sqlite3VdbeCurrentAddr(v);
      ਜੇ( n_ਖਾਨਾ_Test==1 && pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ==1 && Is_Unique_ਸੂਚਕਾਂਕ(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NotNull, regPrev, ਸਿਰਾ_Distinct_Test);
        VdbeCoverage(v);
      }
      ਲਈ(i=0; i<n_ਖਾਨਾ_Test; i++){
        ਅੱਖਰ *pColl = (ਅੱਖਰ*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, i, regChng);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iIdxCur, i, reg_ਆਰਜ਼ੀ);
        analyze_Vdbe_Comment_ਸੂਚਕਾਂਕ_With_ਕਾਲਮ_Name(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Ne, reg_ਆਰਜ਼ੀ, 0, regPrev+i, pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, n_ਖਾਨਾ_Test, regChng);
      sqlite3VdbeGoto(v, ਸਿਰਾ_Distinct_Test);
  
  
      /*
      **  chng_addr_0:
      **   regPrev(0) = idx(0)
      **  chng_addr_1:
      **   regPrev(1) = idx(1)
      **  ...
      */
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਅਗਲਾ_ਪੰਕਤੀ-1);
      ਲਈ(i=0; i<n_ਖਾਨਾ_Test; i++){
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, aGotoChng[i]);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iIdxCur, i, regPrev+i);
        analyze_Vdbe_Comment_ਸੂਚਕਾਂਕ_With_ਕਾਲਮ_Name(v,pIdx,i);
      }
      sqlite3VdbeResolveLabel(v, ਸਿਰਾ_Distinct_Test);
      sqlite3DbFree(db, aGotoChng);
    }
  
    /*
    **  chng_addr_N:
    **   regRowid = idx(rowid)            // STAT4 only
    **   stat_push(P, regChng, regRowid)  // 3rd parameter STAT4 only
    **   Next csr
    **   if !eof(csr) goto next_row;
    */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਜੇ( OptimizationEnabled(db, SQLITE_Stat4) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( regRowid==(regStat+2) );
      ਜੇ( HasRowid(pTab) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IdxRowid, iIdxCur, regRowid);
      }ਨਹੀਂਤਾਂ{
        ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx->p_ਸਾਰਣੀ);
        ਪੂਰਨ j, k, reg_ਕੁੰਜੀ;
        reg_ਕੁੰਜੀ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
        ਲਈ(j=0; j<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; j++){
          k = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pIdx, pPk->ai_ਕਾਲਮ[j]);
          ਪ੍ਰਤਿਜ੍ਞਾ( k>=0 && k<pIdx->n_ਕਾਲਮ );
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iIdxCur, k, reg_ਕੁੰਜੀ+j);
          analyze_Vdbe_Comment_ਸੂਚਕਾਂਕ_With_ਕਾਲਮ_Name(v,pIdx,k);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਕੁੰਜੀ, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ, regRowid);
        sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਕੁੰਜੀ, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
      }
    }
#ਜੇ_ਅੰਤ
    ਪ੍ਰਤਿਜ੍ਞਾ( regChng==(regStat+1) );
    {
      sqlite3_Vdbe_ਜੋੜੋ_Function_Call(pParse, 1, regStat, reg_ਆਰਜ਼ੀ, 2+IsStat4,
                                 &stat_ਧੱਕੋ_Funcdef, 0);
      ਜੇ( db->n_Analysis_ਹੱਦਬੰਦੀ ){
        ਪੂਰਨ j1, j2, j3;
        j1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, reg_ਆਰਜ਼ੀ); VdbeCoverage(v);
        j2 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_If, reg_ਆਰਜ਼ੀ); VdbeCoverage(v);
        j3 = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_SeekGT, iIdxCur, 0, regPrev, 1);
        VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, j1);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iIdxCur, addr_ਅਗਲਾ_ਪੰਕਤੀ); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, j2);
        sqlite3_Vdbe_Jump_ਇੱਥੇ(v, j3);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iIdxCur, addr_ਅਗਲਾ_ਪੰਕਤੀ); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    ਜੇ( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_Goto_ਸਿਰਾ);
      addr_Goto_ਸਿਰਾ = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    ਪ੍ਰਤਿਜ੍ਞਾ( "BBB"[0]==SQLITE_AFF_ਪਾਠ );
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, regTabname, 3, reg_ਆਰਜ਼ੀ, "BBB", 0);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iStatCur, reg_ਆਰਜ਼ੀ, regNewRowid);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (ਅੱਖਰ*)pStat1, P4_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਜੇ( OptimizationEnabled(db, SQLITE_Stat4) && db->n_Analysis_ਹੱਦਬੰਦੀ==0 ){
      ਪੂਰਨ regEq = regStat1;
      ਪੂਰਨ regLt = regStat1+1;
      ਪੂਰਨ regDLt = regStat1+2;
      ਪੂਰਨ regSample = regStat1+3;
      ਪੂਰਨ reg_ਖਾਨਾ = regStat1+4;
      ਪੂਰਨ regSampleRowid = reg_ਖਾਨਾ + n_ਖਾਨਾ;
      ਪੂਰਨ addr_ਅਗਲਾ;
      ਪੂਰਨ addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_Not_ਮਿਲਿਆ;

      /* No STAT4 data is generated if the number of rows is zero */
      ਜੇ( addr_Goto_ਸਿਰਾ==0 ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addr_Goto_ਸਿਰਾ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      ਜੇ( doOnce ){
        ਪੂਰਨ mx_ਖਾਨਾ = n_ਖਾਨਾ;
        ਸੂਚਕਾਂਕ *pX;

        /* Compute the maximum number of columns in any index */
        ਲਈ(pX=pTab->p_ਸੂਚਕਾਂਕ; pX; pX=pX->p_ਅਗਲਾ){
          ਪੂਰਨ n_ਖਾਨਾ_X;                     /* Number of columns in pX */
          ਜੇ( !HasRowid(pTab) && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pX) ){
            n_ਖਾਨਾ_X = pX->n_ਕੁੰਜੀ_ਖਾਨਾ;
          }ਨਹੀਂਤਾਂ{
            n_ਖਾਨਾ_X = pX->n_ਕਾਲਮ;
          }
          ਜੇ( n_ਖਾਨਾ_X>mx_ਖਾਨਾ ) mx_ਖਾਨਾ = n_ਖਾਨਾ_X;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3_Touch_ਰਜਿਸਟਰ(pParse, reg_ਖਾਨਾ+mx_ਖਾਨਾ);
        doOnce = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3_No_Temps_ਵਿੱਚ_Range(pParse, regEq, reg_ਖਾਨਾ+mx_ਖਾਨਾ) );
#ਜੇ_ਅੰਤ
        sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(pParse);  /* tag-20230325-1 */
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_No_Temps_ਵਿੱਚ_Range(pParse, regEq, reg_ਖਾਨਾ+mx_ਖਾਨਾ) );
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_No_Temps_ਵਿੱਚ_Range(pParse, regEq, reg_ਖਾਨਾ+n_ਖਾਨਾ) );

      addr_ਅਗਲਾ = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, seekOp, iTabCur, addr_ਅਗਲਾ, regSampleRowid, 0);
      VdbeCoverage(v);
      ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੱਦੋ_ਸੂਚਕਾਂਕ_ਕਾਲਮ(pParse, pIdx, iTabCur, i, reg_ਖਾਨਾ+i);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਖਾਨਾ, n_ਖਾਨਾ, regSample);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, regTabname, 6, reg_ਆਰਜ਼ੀ);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iStatCur+1, reg_ਆਰਜ਼ੀ, regNewRowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 1, addr_ਅਗਲਾ); /* P1==1 for end-of-loop */
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrIsNull);
    }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    ਜੇ( addr_Goto_ਸਿਰਾ ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_Goto_ਸਿਰਾ);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  ਜੇ( pOnlyIdx==0 && need_ਸਾਰਣੀ_Cnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਗਿਣਤੀ, iTabCur, regStat1);
    j_Zero_ਪੰਕਤੀਆਂ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, regIdxname);
    ਪ੍ਰਤਿਜ੍ਞਾ( "BBB"[0]==SQLITE_AFF_ਪਾਠ );
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, regTabname, 3, reg_ਆਰਜ਼ੀ, "BBB", 0);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iStatCur, reg_ਆਰਜ਼ੀ, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (ਅੱਖਰ*)pStat1, P4_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, j_Zero_ਪੰਕਤੀਆਂ);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Analysis(Parse *pParse, ਪੂਰਨ iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  ਜੇ( v ){
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ਲੱਦੋ_Analysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
ਸਥਿਰ ਖਾਲੀ analyzeDatabase(Parse *pParse, ਪੂਰਨ iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  ਪੂਰਨ iStatCur;
  ਪੂਰਨ iMem;
  ਪੂਰਨ iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  open_Stat_ਸਾਰਣੀ(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ਲਈ(k=sqlite_Hash_ਪਹਿਲਾ(&pSchema->tblHash); k; k=sqlite_Hash_ਅਗਲਾ(k)){
    ਸਾਰਣੀ *pTab = (ਸਾਰਣੀ*)sqlite_Hash_ਅੰਕੜੇ(k);
    analyze_One_ਸਾਰਣੀ(pParse, pTab, 0, iStatCur, iMem, iTab);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    iMem = sqlite3_ਪਹਿਲਾ_ਉਪਲਬਧ_ਰਜਿਸਟਰ(pParse, iMem);
#ਨਹੀਂਤਾਂ
    ਪ੍ਰਤਿਜ੍ਞਾ( iMem==sqlite3_ਪਹਿਲਾ_ਉਪਲਬਧ_ਰਜਿਸਟਰ(pParse,iMem) );
#ਜੇ_ਅੰਤ
  }
  ਲੱਦੋ_Analysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
ਸਥਿਰ ਖਾਲੀ analyze_ਸਾਰਣੀ(Parse *pParse, ਸਾਰਣੀ *pTab, ਸੂਚਕਾਂਕ *pOnlyIdx){
  ਪੂਰਨ iDb;
  ਪੂਰਨ iStatCur;

  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  ਜੇ( pOnlyIdx ){
    open_Stat_ਸਾਰਣੀ(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }ਨਹੀਂਤਾਂ{
    open_Stat_ਸਾਰਣੀ(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyze_One_ਸਾਰਣੀ(pParse, pTab, pOnlyIdx, iStatCur,pParse->nMem+1,pParse->nTab);
  ਲੱਦੋ_Analysis(pParse, iDb);
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
ਖਾਲੀ sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  ਪੂਰਨ iDb;
  ਪੂਰਨ i;
  ਅੱਖਰ *z, *zDb;
  ਸਾਰਣੀ *pTab;
  ਸੂਚਕਾਂਕ *pIdx;
  Token *p_ਸਾਰਣੀ_Name;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  ਜੇ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    ਵਾਪਸ;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pName2!=0 || pName1==0 );
  ਜੇ( pName1==0 ){
    /* Form 1:  Analyze everything */
    ਲਈ(i=0; i<db->nDb; i++){
      ਜੇ( i==1 ) ਜਾਰੀ;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pName2->n==0 && (iDb = sqlite3_ਲੱਭੋ_Db(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }ਨਹੀਂਤਾਂ{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &p_ਸਾਰਣੀ_Name);
    ਜੇ( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, p_ਸਾਰਣੀ_Name);
      ਜੇ( z ){
        ਜੇ( (pIdx = sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(db, z, zDb))!=0 ){
          analyze_ਸਾਰਣੀ(pParse, pIdx->p_ਸਾਰਣੀ, pIdx);
        }ਨਹੀਂਤਾਂ ਜੇ( (pTab = sqlite3_Locate_ਸਾਰਣੀ(pParse, 0, z, zDb))!=0 ){
          analyze_ਸਾਰਣੀ(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  ਜੇ( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ analysisInfo analysisInfo;
ਬਣਤਰ analysisInfo {
  sqlite3 *db;
  ਅਚਲ ਅੱਖਰ *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
ਸਥਿਰ ਖਾਲੀ decodeIntArray(
  ਅੱਖਰ *zIntArray,       /* String containing int array to decode */
  ਪੂਰਨ n_ਬਾਹਰਲਾ,              /* Number of slots in aOut[] */
  tRowcnt *a_ਬਾਹਰਲਾ,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ          /* Handle extra flags for this index, if not NULL */
){
  ਅੱਖਰ *z = zIntArray;
  ਪੂਰਨ c;
  ਪੂਰਨ i;
  tRowcnt v;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( z==0 ) z = "";
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( z!=0 );
#ਜੇ_ਅੰਤ
  ਲਈ(i=0; *z && i<n_ਬਾਹਰਲਾ; i++){
    v = 0;
    ਜਦੋਂ( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    ਜੇ( a_ਬਾਹਰਲਾ ) a_ਬਾਹਰਲਾ[i] = v;
    ਜੇ( aLog ) aLog[i] = sqlite3LogEst(v);
#ਨਹੀਂਤਾਂ
    ਪ੍ਰਤਿਜ੍ਞਾ( a_ਬਾਹਰਲਾ==0 );
    ਅਣਵਰਤਿਆ_PARAMETER(a_ਬਾਹਰਲਾ);
    ਪ੍ਰਤਿਜ੍ਞਾ( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#ਜੇ_ਅੰਤ
    ਜੇ( *z==' ' ) z++;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੂਚਕਾਂਕ!=0 ); {
#ਨਹੀਂਤਾਂ
  ਜੇ( p_ਸੂਚਕਾਂਕ ){
#ਜੇ_ਅੰਤ
    p_ਸੂਚਕਾਂਕ->bUnordered = 0;
    p_ਸੂਚਕਾਂਕ->no_Skip_ਜਾਂਚ = 0;
    ਜਦੋਂ( z[0] ){
      ਜੇ( sqlite3_strglob("unordered*", z)==0 ){
        p_ਸੂਚਕਾਂਕ->bUnordered = 1;
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        ਪੂਰਨ sz = sqlite3Atoi(z+3);
        ਜੇ( sz<2 ) sz = 2;
        p_ਸੂਚਕਾਂਕ->sz_Idx_ਪੰਕਤੀ = sqlite3LogEst(sz);
      }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_strglob("noskipscan*", z)==0 ){
        p_ਸੂਚਕਾਂਕ->no_Skip_ਜਾਂਚ = 1;
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_COSTMULT
      ਨਹੀਂਤਾਂ ਜੇ( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        p_ਸੂਚਕਾਂਕ->p_ਸਾਰਣੀ->ਲਾਗਤ_Mult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#ਜੇ_ਅੰਤ
      ਜਦੋਂ( z[0]!=0 && z[0]!=' ' ) z++;
      ਜਦੋਂ( z[0]==' ' ) z++;
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
ਸਥਿਰ ਪੂਰਨ analysisLoader(ਖਾਲੀ *p_ਅੰਕੜੇ, ਪੂਰਨ argc, ਅੱਖਰ **argv, ਅੱਖਰ **Not_ਵਰਤਿਆ){
  analysisInfo *pInfo = (analysisInfo*)p_ਅੰਕੜੇ;
  ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ;
  ਸਾਰਣੀ *p_ਸਾਰਣੀ;
  ਅਚਲ ਅੱਖਰ *z;

  ਪ੍ਰਤਿਜ੍ਞਾ( argc==3 );
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, argc);

  ਜੇ( argv==0 || argv[0]==0 || argv[2]==0 ){
    ਵਾਪਸ 0;
  }
  p_ਸਾਰਣੀ = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(pInfo->db, argv[0], pInfo->zDatabase);
  ਜੇ( p_ਸਾਰਣੀ==0 ){
    ਵਾਪਸ 0;
  }
  ਜੇ( argv[1]==0 ){
    p_ਸੂਚਕਾਂਕ = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_stricmp(argv[0],argv[1])==0 ){
    p_ਸੂਚਕਾਂਕ = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(p_ਸਾਰਣੀ);
  }ਨਹੀਂਤਾਂ{
    p_ਸੂਚਕਾਂਕ = sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  ਜੇ( p_ਸੂਚਕਾਂਕ ){
    tRowcnt *ai_ਪੰਕਤੀ_Est = 0;
    ਪੂਰਨ n_ਖਾਨਾ = p_ਸੂਚਕਾਂਕ->n_ਕੁੰਜੀ_ਖਾਨਾ+1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    ਜੇ( p_ਸੂਚਕਾਂਕ->ai_ਪੰਕਤੀ_Est==0 ){
      p_ਸੂਚਕਾਂਕ->ai_ਪੰਕਤੀ_Est = (tRowcnt*)sqlite3MallocZero(ਆਕਾਰ(tRowcnt) * n_ਖਾਨਾ);
      ਜੇ( p_ਸੂਚਕਾਂਕ->ai_ਪੰਕਤੀ_Est==0 ) sqlite3OomFault(pInfo->db);
    }
    ai_ਪੰਕਤੀ_Est = p_ਸੂਚਕਾਂਕ->ai_ਪੰਕਤੀ_Est;
#ਜੇ_ਅੰਤ
    p_ਸੂਚਕਾਂਕ->bUnordered = 0;
    decodeIntArray((ਅੱਖਰ*)z, n_ਖਾਨਾ, ai_ਪੰਕਤੀ_Est, p_ਸੂਚਕਾਂਕ->ai_ਪੰਕਤੀ_Log_Est, p_ਸੂਚਕਾਂਕ);
    p_ਸੂਚਕਾਂਕ->hasStat1 = 1;
    ਜੇ( p_ਸੂਚਕਾਂਕ->pPartIdxWhere==0 ){
      p_ਸਾਰਣੀ->n_ਪੰਕਤੀ_Log_Est = p_ਸੂਚਕਾਂਕ->ai_ਪੰਕਤੀ_Log_Est[0];
      p_ਸਾਰਣੀ->tabFlags |= TF_HasStat1;
    }
  }ਨਹੀਂਤਾਂ{
    ਸੂਚਕਾਂਕ fakeIdx;
    fakeIdx.sz_Idx_ਪੰਕਤੀ = p_ਸਾਰਣੀ->sz_Tab_ਪੰਕਤੀ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_COSTMULT
    fakeIdx.p_ਸਾਰਣੀ = p_ਸਾਰਣੀ;
#ਜੇ_ਅੰਤ
    decodeIntArray((ਅੱਖਰ*)z, 1, 0, &p_ਸਾਰਣੀ->n_ਪੰਕਤੀ_Log_Est, &fakeIdx);
    p_ਸਾਰਣੀ->sz_Tab_ਪੰਕਤੀ = fakeIdx.sz_Idx_ਪੰਕਤੀ;
    p_ਸਾਰਣੀ->tabFlags |= TF_HasStat1;
  }

  ਵਾਪਸ 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
ਖਾਲੀ sqlite3_Delete_ਸੂਚਕਾਂਕ_Samples(sqlite3 *db, ਸੂਚਕਾਂਕ *pIdx){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( pIdx->aSample ){
    ਪੂਰਨ j;
    ਲਈ(j=0; j<pIdx->nSample; j++){
      ਸੂਚਕਾਂਕ_Sample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  ਜੇ( db->pn_ਬਾਈਟਾਂ_Freed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(db);
  ਅਣਵਰਤਿਆ_PARAMETER(pIdx);
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
ਸਥਿਰ ਖਾਲੀ initAvgEq(ਸੂਚਕਾਂਕ *pIdx){
  ਜੇ( pIdx ){
    ਸੂਚਕਾਂਕ_Sample *aSample = pIdx->aSample;
    ਸੂਚਕਾਂਕ_Sample *pFinal = &aSample[pIdx->nSample-1];
    ਪੂਰਨ i_ਖਾਨਾ;
    ਪੂਰਨ n_ਖਾਨਾ = 1;
    ਜੇ( pIdx->n_Sample_ਖਾਨਾ>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      n_ਖਾਨਾ = pIdx->n_Sample_ਖਾਨਾ-1;
      pIdx->aAvgEq[n_ਖਾਨਾ] = 1;
    }
    ਲਈ(i_ਖਾਨਾ=0; i_ਖਾਨਾ<n_ਖਾਨਾ; i_ਖਾਨਾ++){
      ਪੂਰਨ nSample = pIdx->nSample;
      ਪੂਰਨ i;                    /* Used to iterate through samples */
      tRowcnt ਜੋੜ_Eq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt n_ਪੰਕਤੀ;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      ਜੇ( !pIdx->ai_ਪੰਕਤੀ_Est || i_ਖਾਨਾ>=pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ || pIdx->ai_ਪੰਕਤੀ_Est[i_ਖਾਨਾ+1]==0 ){
        n_ਪੰਕਤੀ = pFinal->anLt[i_ਖਾਨਾ];
        nDist100 = (i64)100 * pFinal->anDLt[i_ਖਾਨਾ];
        nSample--;
      }ਨਹੀਂਤਾਂ{
        n_ਪੰਕਤੀ = pIdx->ai_ਪੰਕਤੀ_Est[0];
        nDist100 = ((i64)100 * pIdx->ai_ਪੰਕਤੀ_Est[0]) / pIdx->ai_ਪੰਕਤੀ_Est[i_ਖਾਨਾ+1];
      }
      pIdx->n_ਪੰਕਤੀ_Est0 = n_ਪੰਕਤੀ;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      ਲਈ(i=0; i<nSample; i++){
        ਜੇ( i==(pIdx->nSample-1)
         || aSample[i].anDLt[i_ਖਾਨਾ]!=aSample[i+1].anDLt[i_ਖਾਨਾ] 
        ){
          ਜੋੜ_Eq += aSample[i].anEq[i_ਖਾਨਾ];
          nSum100 += 100;
        }
      }

      ਜੇ( nDist100>nSum100 && ਜੋੜ_Eq<n_ਪੰਕਤੀ ){
        avgEq = ((i64)100 * (n_ਪੰਕਤੀ - ਜੋੜ_Eq))/(nDist100 - nSum100);
      }
      ਜੇ( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[i_ਖਾਨਾ] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
ਸਥਿਰ ਸੂਚਕਾਂਕ *ਲੱਭੋ_ਸੂਚਕਾਂਕ_Or_Primary_ਕੁੰਜੀ(
  sqlite3 *db,
  ਅਚਲ ਅੱਖਰ *zName,
  ਅਚਲ ਅੱਖਰ *zDb
){
  ਸੂਚਕਾਂਕ *pIdx = sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(db, zName, zDb);
  ਜੇ( pIdx==0 ){
    ਸਾਰਣੀ *pTab = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, zName, zDb);
    ਜੇ( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
  }
  ਵਾਪਸ pIdx;
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
ਸਥਿਰ ਪੂਰਨ ਲੱਦੋ_Stat_Tbl(
  sqlite3 *db,                  /* Database handle */
  ਅਚਲ ਅੱਖਰ *zSql1,            /* SQL statement 1 (see above) */
  ਅਚਲ ਅੱਖਰ *zSql2,            /* SQL statement 2 (see above) */
  ਅਚਲ ਅੱਖਰ *zDb               /* Database name (e.g. "main") */
){
  ਪੂਰਨ rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  ਅੱਖਰ *zSql;                   /* Text of the SQL statement */
  ਸੂਚਕਾਂਕ *pPrevIdx = 0;          /* Previous index in the loop */
  ਸੂਚਕਾਂਕ_Sample *pSample;         /* A slot in pIdx->aSample[] */

  ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  ਜੇ( !zSql ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ਜੇ( rc ) ਵਾਪਸ rc;

  ਜਦੋਂ( sqlite3_ਕਦਮ(pStmt)==SQLITE_ਪੰਕਤੀ ){
    ਪੂਰਨ n_Idx_ਖਾਨਾ = 1;              /* Number of columns in stat4 records */

    ਅੱਖਰ *z_ਸੂਚਕਾਂਕ;    /* Index name */
    ਸੂਚਕਾਂਕ *pIdx;     /* Pointer to the index object */
    ਪੂਰਨ nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    z_ਸੂਚਕਾਂਕ = (ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 0);
    ਜੇ( z_ਸੂਚਕਾਂਕ==0 ) ਜਾਰੀ;
    nSample = sqlite3_ਕਾਲਮ_int(pStmt, 1);
    pIdx = ਲੱਭੋ_ਸੂਚਕਾਂਕ_Or_Primary_ਕੁੰਜੀ(db, z_ਸੂਚਕਾਂਕ, zDb);
    ਪ੍ਰਤਿਜ੍ਞਾ( pIdx==0 || pIdx->nSample==0 );
    ਜੇ( pIdx==0 ) ਜਾਰੀ;
    ਜੇ( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      ਜਾਰੀ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( !HasRowid(pIdx->p_ਸਾਰਣੀ) || pIdx->n_ਕਾਲਮ==pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ+1 );
    ਜੇ( !HasRowid(pIdx->p_ਸਾਰਣੀ) && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ){
      n_Idx_ਖਾਨਾ = pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ;
    }ਨਹੀਂਤਾਂ{
      n_Idx_ਖਾਨਾ = pIdx->n_ਕਾਲਮ;
    }
    pIdx->n_Sample_ਖਾਨਾ = n_Idx_ਖਾਨਾ;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(ਸੂਚਕਾਂਕ_Sample) * nSample);
    nByte += sizeof64(tRowcnt) * n_Idx_ਖਾਨਾ * 3 * nSample;
    nByte += n_Idx_ਖਾਨਾ * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    ਜੇ( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += n_Idx_ਖਾਨਾ;
    pIdx->p_ਸਾਰਣੀ->tabFlags |= TF_HasStat4;
    ਲਈ(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += n_Idx_ਖਾਨਾ;
      pIdx->aSample[i].anLt = pSpace; pSpace += n_Idx_ਖਾਨਾ;
      pIdx->aSample[i].anDLt = pSpace; pSpace += n_Idx_ਖਾਨਾ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  ਜੇ( rc ) ਵਾਪਸ rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  ਜੇ( !zSql ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ਜੇ( rc ) ਵਾਪਸ rc;

  ਜਦੋਂ( sqlite3_ਕਦਮ(pStmt)==SQLITE_ਪੰਕਤੀ ){
    ਅੱਖਰ *z_ਸੂਚਕਾਂਕ;                 /* Index name */
    ਸੂਚਕਾਂਕ *pIdx;                  /* Pointer to the index object */
    ਪੂਰਨ n_ਖਾਨਾ = 1;                 /* Number of columns in index */

    z_ਸੂਚਕਾਂਕ = (ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 0);
    ਜੇ( z_ਸੂਚਕਾਂਕ==0 ) ਜਾਰੀ;
    pIdx = ਲੱਭੋ_ਸੂਚਕਾਂਕ_Or_Primary_ਕੁੰਜੀ(db, z_ਸੂਚਕਾਂਕ, zDb);
    ਜੇ( pIdx==0 ) ਜਾਰੀ;
    ਜੇ( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      ਜਾਰੀ;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    n_ਖਾਨਾ = pIdx->n_Sample_ਖਾਨਾ;
    ਜੇ( pIdx!=pPrevIdx ){
      initAvgEq(pPrevIdx);
      pPrevIdx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt,1),n_ਖਾਨਾ,pSample->anEq,0,0);
    decodeIntArray((ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt,2),n_ਖਾਨਾ,pSample->anLt,0,0);
    decodeIntArray((ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt,3),n_ਖਾਨਾ,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_ਕਾਲਮ_ਬਾਈਟਾਂ(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    ਜੇ( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    ਜੇ( pSample->n ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pSample->p, sqlite3_ਕਾਲਮ_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  ਜੇ( rc==SQLITE_OK ) initAvgEq(pPrevIdx);
  ਵਾਪਸ rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
ਸਥਿਰ ਪੂਰਨ ਲੱਦੋ_Stat4(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zDb){
  ਪੂਰਨ rc = SQLITE_OK;             /* Result codes from subroutines */
  ਅਚਲ ਸਾਰਣੀ *pStat4;

  ਪ੍ਰਤਿਜ੍ਞਾ( db->lookaside.bDisable );
  ਜੇ( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, "sqlite_stat4", zDb))!=0
   && Is_Ordinary_ਸਾਰਣੀ(pStat4)
  ){
    rc = ਲੱਦੋ_Stat_Tbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_STAT4 */

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
ਪੂਰਨ sqlite3_Analysis_ਲੱਦੋ(sqlite3 *db, ਪੂਰਨ iDb){
  analysisInfo sInfo;
  HashElem *i;
  ਅੱਖਰ *zSql;
  ਪੂਰਨ rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  ਅਚਲ ਸਾਰਣੀ *pStat1;

  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<db->nDb );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&pSchema->tblHash); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    ਸਾਰਣੀ *pTab = sqlite_Hash_ਅੰਕੜੇ(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&pSchema->idxHash); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    ਸੂਚਕਾਂਕ *pIdx = sqlite_Hash_ਅੰਕੜੇ(i);
    pIdx->hasStat1 = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
    sqlite3_Delete_ਸੂਚਕਾਂਕ_Samples(db, pIdx);
    pIdx->aSample = 0;
#ਜੇ_ਅੰਤ
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  ਜੇ( (pStat1 = sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(db, "sqlite_stat1", sInfo.zDatabase))
   && Is_Ordinary_ਸਾਰਣੀ(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    ਜੇ( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ਨਹੀਂਤਾਂ{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3SchemaMutexHeld(db, iDb, 0) );
  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&pSchema->idxHash); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    ਸੂਚਕਾਂਕ *pIdx = sqlite_Hash_ਅੰਕੜੇ(i);
    ਜੇ( !pIdx->hasStat1 ) sqlite3_Default_ਪੰਕਤੀ_Est(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਜੇ( rc==SQLITE_OK ){
    DisableLookaside;
    rc = ਲੱਦੋ_Stat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  ਲਈ(i=sqlite_Hash_ਪਹਿਲਾ(&pSchema->idxHash); i; i=sqlite_Hash_ਅਗਲਾ(i)){
    ਸੂਚਕਾਂਕ *pIdx = sqlite_Hash_ਅੰਕੜੇ(i);
    sqlite3_free(pIdx->ai_ਪੰਕਤੀ_Est);
    pIdx->ai_ਪੰਕਤੀ_Est = 0;
  }
#ਜੇ_ਅੰਤ

  ਜੇ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  ਵਾਪਸ rc;
}


#ਜੇ_ਅੰਤ /* SQLITE_OMIT_ANALYZE */
