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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ANALYZE
#అన్తర్భావ "sqliteInt.h"

#ఒకవేళ defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#లేకపోతే
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#అయితే_ముగింపు

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
స్థిర శూన్య open_Stat_పట్టిక(
  Parse *pParse,          /* Parsing context */
  పూర్ణాంకం iDb,                /* The database we are looking in */
  పూర్ణాంకం iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  స్థిరరాశి అక్షరం *zWhere,     /* Delete entries for this table or index */
  స్థిరరాశి అక్షరం *zWhereType  /* Either "tbl" or "idx" */
){
  స్థిర స్థిరరాశి నిర్మాణం {
    స్థిరరాశి అక్షరం *zName;
    స్థిరరాశి అక్షరం *z_నిలువుల_సంఖ్య;
  } a_పట్టిక[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#ఒకవేళ defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#లేకపోతే
    { "sqlite_stat4", 0 },
#అయితే_ముగింపు
    { "sqlite_stat3", 0 },
  };
  పూర్ణాంకం i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_మూలం[Array_కొలత(a_పట్టిక)];
  u8 aCreateTbl[Array_కొలత(a_పట్టిక)];
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  స్థిరరాశి పూర్ణాంకం n_గమ్యం_Open = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#లేకపోతే
  స్థిరరాశి పూర్ణాంకం n_గమ్యం_Open = 1;
#అయితే_ముగింపు

  ఒకవేళ( v==0 ) తిరిగి;
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  ప్రతిజ్ఞా( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  కొరకు(i=0; i<Array_కొలత(a_పట్టిక); i++){
    స్థిరరాశి అక్షరం *zTab = a_పట్టిక[i].zName;
    పట్టిక *pStat;
    aCreateTbl[i] = 0;
    ఒకవేళ( (pStat = sqlite3_కనుగొను_పట్టిక(db, zTab, pDb->zDbSName))==0 ){
      ఒకవేళ( i<n_గమ్యం_Open ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, a_పట్టిక[i].z_నిలువుల_సంఖ్య
        );
        ప్రతిజ్ఞా( pParse->isCreate || pParse->nErr );
        a_మూలం[i] = (u32)pParse->u1.cr.reg_మూలం;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }లేకపోతే{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_మూలం[i] = pStat->tnum;
      sqlite3_పట్టిక_తాళం(pParse, iDb, a_మూలం[i], 1, zTab);
      ఒకవేళ( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
      }లేకపోతే ఒకవేళ( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#అయితే_ముగింపు
      }లేకపోతే{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3_Vdbe_కూడు_Op2(v, OP_Clear, (పూర్ణాంకం)a_మూలం[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  కొరకు(i=0; i<n_గమ్యం_Open; i++){
    ప్రతిజ్ఞా( i<Array_కొలత(a_పట్టిక) );
    sqlite3_Vdbe_కూడు_Op4_Int(v, OP_OpenWrite, iStatCur+i, (పూర్ణాంకం)a_మూలం[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, a_పట్టిక[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#నిర్వచితం_కాకపోతే SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#అయితే_ముగింపు

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
రకనిర్వచనం నిర్మాణం StatAccum StatAccum;
రకనిర్వచనం నిర్మాణం StatSample StatSample;
నిర్మాణం StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  సంఘం {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  పూర్ణాంకం i_నిలువు;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#అయితే_ముగింపు
};                                                    
నిర్మాణం StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt n_అడ్డువరుస;             /* Number of rows visited so far */
  పూర్ణాంకం n_హద్దు;               /* Analysis row-scan limit */
  పూర్ణాంకం n_నిలువు;                 /* Number of columns in index + pk/rowid */
  పూర్ణాంకం n_కీలకం_నిలువు;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  పూర్ణాంకం mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *a_ఉత్తమ;        /* Array of nCol best samples */
  పూర్ణాంకం iMin;                 /* Index in a[] of entry with minimum score */
  పూర్ణాంకం nSample;              /* Current number of samples */
  పూర్ణాంకం nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  పూర్ణాంకం iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#అయితే_ముగింపు
};

/* Reclaim memory used by a StatSample
*/
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
స్థిర శూన్య sampleClear(sqlite3 *db, StatSample *p){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#అయితే_ముగింపు

/* Initialize the BLOB value of a ROWID
*/
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
స్థిర శూన్య sample_సమితి_Rowid(sqlite3 *db, StatSample *p, పూర్ణాంకం n, స్థిరరాశి u8 *p_దత్తాంశం){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  ఒకవేళ( p->u.aRowid ){
    p->nRowid = n;
    జ్ఞాపకం_నకలు(p->u.aRowid, p_దత్తాంశం, n);
  }లేకపోతే{
    p->nRowid = 0;
  }
}
#అయితే_ముగింపు

/* Initialize the INTEGER value of a ROWID.
*/
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
స్థిర శూన్య sample_సమితి_Rowid_Int64(sqlite3 *db, StatSample *p, i64 iRowid){
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#అయితే_ముగింపు


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
స్థిర శూన్య sampleCopy(StatAccum *p, StatSample *p_గమ్యం, StatSample *pFrom){
  p_గమ్యం->isPSample = pFrom->isPSample;
  p_గమ్యం->i_నిలువు = pFrom->i_నిలువు;
  p_గమ్యం->iHash = pFrom->iHash;
  జ్ఞాపకం_నకలు(p_గమ్యం->anEq, pFrom->anEq, పరిమాణం(tRowcnt)*p->n_నిలువు);
  జ్ఞాపకం_నకలు(p_గమ్యం->anLt, pFrom->anLt, పరిమాణం(tRowcnt)*p->n_నిలువు);
  జ్ఞాపకం_నకలు(p_గమ్యం->anDLt, pFrom->anDLt, పరిమాణం(tRowcnt)*p->n_నిలువు);
  ఒకవేళ( pFrom->nRowid ){
    sample_సమితి_Rowid(p->db, p_గమ్యం, pFrom->nRowid, pFrom->u.aRowid);
  }లేకపోతే{
    sample_సమితి_Rowid_Int64(p->db, p_గమ్యం, pFrom->u.iRowid);
  }
}
#అయితే_ముగింపు

/*
** Reclaim all memory of a StatAccum structure.
*/
స్థిర శూన్య statAccumDestructor(శూన్య *pOld){
  StatAccum *p = (StatAccum*)pOld;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( p->mxSample ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->n_నిలువు; i++) sampleClear(p->db, p->a_ఉత్తమ+i);
    కొరకు(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#అయితే_ముగింపు
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
స్థిర శూన్య statInit(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  StatAccum *p;
  పూర్ణాంకం n_నిలువు;                       /* Number of columns in index being sampled */
  పూర్ణాంకం n_కీలకం_నిలువు;                    /* Number of key columns */
  పూర్ణాంకం n_నిలువు_పైకి;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  పూర్ణాంకం mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#అయితే_ముగింపు

  /* Decode the three function arguments */
  వాడని_PARAMETER(argc);
  n_నిలువు = sqlite3_విలువ_int(argv[0]);
  ప్రతిజ్ఞా( n_నిలువు>0 );
  n_నిలువు_పైకి = పరిమాణం(tRowcnt)<8 ? (n_నిలువు+1)&~1 : n_నిలువు;
  n_కీలకం_నిలువు = sqlite3_విలువ_int(argv[1]);
  ప్రతిజ్ఞా( n_కీలకం_నిలువు<=n_నిలువు );
  ప్రతిజ్ఞా( n_కీలకం_నిలువు>0 );

  /* Allocate the space required for the StatAccum object */
  n = పరిమాణం(*p) 
    + పరిమాణం(tRowcnt)*n_నిలువు_పైకి;                    /* StatAccum.anDLt */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  n += పరిమాణం(tRowcnt)*n_నిలువు_పైకి;                   /* StatAccum.anEq */
  ఒకవేళ( mxSample ){
    n += పరిమాణం(tRowcnt)*n_నిలువు_పైకి                  /* StatAccum.anLt */
      + పరిమాణం(StatSample)*(n_నిలువు+mxSample)       /* StatAccum.aBest[], a[] */
      + పరిమాణం(tRowcnt)*3*n_నిలువు_పైకి*(n_నిలువు+mxSample);
  }
#అయితే_ముగింపు
  p = sqlite3DbMallocZero(db, n);
  ఒకవేళ( p==0 ){
    sqlite3_ఫలితం_error_nomem(context);
    తిరిగి;
  }

  p->db = db;
  p->nEst = sqlite3_విలువ_int64(argv[2]);
  p->n_అడ్డువరుస = 0;
  p->n_హద్దు = sqlite3_విలువ_int(argv[3]);
  p->n_నిలువు = n_నిలువు;
  p->n_కీలకం_నిలువు = n_కీలకం_నిలువు;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[n_నిలువు_పైకి];
  p->mxSample = p->n_హద్దు==0 ? mxSample : 0;
  ఒకవేళ( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    పూర్ణాంకం i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[n_నిలువు_పైకి];
    p->iPrn = 0x689e962d*(u32)n_నిలువు ^ 0xd0944565*(u32)sqlite3_విలువ_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (నిర్మాణం StatSample*)&p->current.anLt[n_నిలువు_పైకి];
    p->a_ఉత్తమ = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+n_నిలువు]);
    కొరకు(i=0; i<(mxSample+n_నిలువు); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (పరిమాణం(tRowcnt) * n_నిలువు_పైకి);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (పరిమాణం(tRowcnt) * n_నిలువు_పైకి);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (పరిమాణం(tRowcnt) * n_నిలువు_పైకి);
    }
    ప్రతిజ్ఞా( (pSpace - (u8*)p)==n );
  
    కొరకు(i=0; i<n_నిలువు; i++){
      p->a_ఉత్తమ[i].i_నిలువు = i;
    }
  }
#అయితే_ముగింపు

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_ఫలితం_blob(context, p, పరిమాణం(*p), statAccumDestructor);
}
స్థిర స్థిరరాశి FuncDef statInitFuncdef = {
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

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
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
స్థిర పూర్ణాంకం sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  పూర్ణాంకం n_నిలువు = pAccum->n_నిలువు;
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( pNew->i_నిలువు==pOld->i_నిలువు );
  కొరకు(i=pNew->i_నిలువు+1; i<n_నిలువు; i++){
    ఒకవేళ( pNew->anEq[i]>pOld->anEq[i] ) తిరిగి 1;
    ఒకవేళ( pNew->anEq[i]<pOld->anEq[i] ) తిరిగి 0;
  }
  ఒకవేళ( pNew->iHash>pOld->iHash ) తిరిగి 1;
  తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
స్థిర పూర్ణాంకం sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->i_నిలువు];
  tRowcnt nEqOld = pOld->anEq[pOld->i_నిలువు];

  ప్రతిజ్ఞా( pOld->isPSample==0 && pNew->isPSample==0 );
  ప్రతిజ్ఞా( IsStat4 || (pNew->i_నిలువు==0 && pOld->i_నిలువు==0) );

  ఒకవేళ( (nEqNew>nEqOld) ) తిరిగి 1;
  ఒకవేళ( nEqNew==nEqOld ){
    ఒకవేళ( pNew->i_నిలువు<pOld->i_నిలువు ) తిరిగి 1;
    తిరిగి (pNew->i_నిలువు==pOld->i_నిలువు && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  తిరిగి 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
స్థిర శూన్య sample_చొప్పించు(StatAccum *p, StatSample *pNew, పూర్ణాంకం nEqZero){
  StatSample *pSample = 0;
  పూర్ణాంకం i;

  ప్రతిజ్ఞా( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  ఒకవేళ( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  ఒకవేళ( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    ప్రతిజ్ఞా( pNew->anEq[pNew->i_నిలువు]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    కొరకు(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      ఒకవేళ( pOld->anEq[pNew->i_నిలువు]==0 ){
        ఒకవేళ( pOld->isPSample ) తిరిగి;
        ప్రతిజ్ఞా( pOld->i_నిలువు>pNew->i_నిలువు );
        ప్రతిజ్ఞా( sampleIsBetter(p, pNew, pOld) );
        ఒకవేళ( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    ఒకవేళ( pUpgrade ){
      pUpgrade->i_నిలువు = pNew->i_నిలువు;
      pUpgrade->anEq[pUpgrade->i_నిలువు] = pNew->anEq[pUpgrade->i_నిలువు];
      వెళ్ళు కనుగొను_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  ఒకవేళ( p->nSample>=p->mxSample ){
    StatSample *pMin = &p->a[p->iMin];
    tRowcnt *anEq = pMin->anEq;
    tRowcnt *anLt = pMin->anLt;
    tRowcnt *anDLt = pMin->anDLt;
    sampleClear(p->db, pMin);
    memmove(pMin, &pMin[1], పరిమాణం(p->a[0])*(p->nSample-p->iMin-1));
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
  ప్రతిజ్ఞా( p->nSample==0 
       || pNew->anLt[p->n_నిలువు-1] > p->a[p->nSample-1].anLt[p->n_నిలువు-1] );

  /* Insert the new sample */
  pSample = &p->a[p->nSample];
  sampleCopy(p, pSample, pNew);
  p->nSample++;

  /* Zero the first nEqZero entries in the anEq[] array. */
  memset(pSample->anEq, 0, పరిమాణం(tRowcnt)*nEqZero);

కనుగొను_new_min:
  ఒకవేళ( p->nSample>=p->mxSample ){
    పూర్ణాంకం iMin = -1;
    కొరకు(i=0; i<p->mxSample; i++){
      ఒకవేళ( p->a[i].isPSample ) కొనసాగు;
      ఒకవేళ( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    ప్రతిజ్ఞా( iMin>=0 );
    p->iMin = iMin;
  }
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
స్థిర శూన్య sample_నెట్టు_మునుపటి(StatAccum *p, పూర్ణాంకం iChng){
  పూర్ణాంకం i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  కొరకు(i=(p->n_నిలువు-2); i>=iChng; i--){
    StatSample *p_ఉత్తమ = &p->a_ఉత్తమ[i];
    p_ఉత్తమ->anEq[i] = p->current.anEq[i];
    ఒకవేళ( p->nSample<p->mxSample || sampleIsBetter(p, p_ఉత్తమ, &p->a[p->iMin]) ){
      sample_చొప్పించు(p, p_ఉత్తమ, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  కొరకు(i=p->nSample-1; i>=0; i--){
    పూర్ణాంకం j;
    కొరకు(j=p->nMaxEqZero; j<p->n_నిలువు; j++) ప్రతిజ్ఞా( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  ఒకవేళ( iChng<p->nMaxEqZero ){
    కొరకు(i=p->nSample-1; i>=0; i--){
      పూర్ణాంకం j;
      కొరకు(j=iChng; j<p->n_నిలువు; j++){
        ఒకవేళ( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

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
స్థిర శూన్య stat_నెట్టు(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  పూర్ణాంకం i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_విలువ_blob(argv[0]);
  పూర్ణాంకం iChng = sqlite3_విలువ_int(argv[1]);

  వాడని_PARAMETER( argc );
  వాడని_PARAMETER( context );
  ప్రతిజ్ఞా( p->n_నిలువు>0 );
  ప్రతిజ్ఞా( iChng<p->n_నిలువు );

  ఒకవేళ( p->n_అడ్డువరుస==0 ){
    /* This is the first call to this function. Do initialization. */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    కొరకు(i=0; i<p->n_నిలువు; i++) p->current.anEq[i] = 1;
#అయితే_ముగింపు
  }లేకపోతే{
    /* Second and subsequent calls get processed here */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    ఒకవేళ( p->mxSample ) sample_నెట్టు_మునుపటి(p, iChng);
#అయితే_ముగింపు

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    కొరకు(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#అయితే_ముగింపు
    కొరకు(i=iChng; i<p->n_నిలువు; i++){
      p->current.anDLt[i]++;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
      ఒకవేళ( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#అయితే_ముగింపు
    }
  }

  p->n_అడ్డువరుస++;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( p->mxSample ){
    tRowcnt nLt;
    ఒకవేళ( sqlite3_విలువ_type(argv[2])==SQLITE_INTEGER ){
      sample_సమితి_Rowid_Int64(p->db, &p->current, sqlite3_విలువ_int64(argv[2]));
    }లేకపోతే{
      sample_సమితి_Rowid(p->db, &p->current, sqlite3_విలువ_బైట్లు(argv[2]),
                                         sqlite3_విలువ_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->n_నిలువు-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    ఒకవేళ( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.i_నిలువు = 0;
      sample_చొప్పించు(p, &p->current, p->n_నిలువు-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    కొరకు(i=0; i<(p->n_నిలువు-1); i++){
      p->current.i_నిలువు = i;
      ఒకవేళ( i>=iChng || sampleIsBetterPost(p, &p->current, &p->a_ఉత్తమ[i]) ){
        sampleCopy(p, &p->a_ఉత్తమ[i], &p->current);
      }
    }
  }లేకపోతే
#అయితే_ముగింపు
  ఒకవేళ( p->n_హద్దు && p->n_అడ్డువరుస>(tRowcnt)p->n_హద్దు*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_ఫలితం_int(context, p->current.anDLt[0]>0);
  }
}

స్థిర స్థిరరాశి FuncDef stat_నెట్టు_Funcdef = {
  2+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  stat_నెట్టు,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_push",     /* zName */
  {0}
};

#నిర్వచించు STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#నిర్వచించు STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#నిర్వచించు STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#నిర్వచించు STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#నిర్వచించు STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

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
స్థిర శూన్య statGet(
  sqlite3_context *context,
  పూర్ణాంకం argc,
  sqlite3_విలువ **argv
){
  StatAccum *p = (StatAccum*)sqlite3_విలువ_blob(argv[0]);
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  పూర్ణాంకం eCall = sqlite3_విలువ_int(argv[1]);
  ప్రతిజ్ఞా( argc==2 );
  ప్రతిజ్ఞా( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  ప్రతిజ్ఞా( eCall==STAT_GET_STAT1 || p->mxSample );
  ఒకవేళ( eCall==STAT_GET_STAT1 )
#లేకపోతే
  ప్రతిజ్ఞా( argc==1 );
#అయితే_ముగింపు
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
    పూర్ణాంకం i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_కీలకం_నిలువు+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->n_అడ్డువరుస);
    కొరకు(i=0; i<p->n_కీలకం_నిలువు; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->n_అడ్డువరుస + nDistinct - 1) / nDistinct;
      ఒకవేళ( iVal==2 && p->n_అడ్డువరుస*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
      ప్రతిజ్ఞా( p->current.anEq[i] || p->n_అడ్డువరుస==0 );
#అయితే_ముగింపు
    }
    sqlite3_ఫలితం_str(context, &sStat, SQLITE_XFER);
  }
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  లేకపోతే ఒకవేళ( eCall==STAT_GET_ROWID ){
    ఒకవేళ( p->iGet<0 ){
      sample_నెట్టు_మునుపటి(p, 0);
      p->iGet = 0;
    }
    ఒకవేళ( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      ఒకవేళ( pS->nRowid==0 ){
        sqlite3_ఫలితం_int64(context, pS->u.iRowid);
      }లేకపోతే{
        sqlite3_ఫలితం_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }లేకపోతే{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    పూర్ణాంకం i;

    ప్రతిజ్ఞా( p->iGet<p->nSample );
    ఎంపిక( eCall ){
      స్థితి STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; ఆపు;
      స్థితి STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; ఆపు;
      అప్రమేయ: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        ఆపు;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->n_నిలువు*100);
    కొరకు(i=0; i<p->n_నిలువు; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    ఒకవేళ( sStat.nChar ) sStat.nChar--;
    sqlite3_ఫలితం_str(context, &sStat, SQLITE_XFER);
  }
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */
#నిర్వచితం_కాకపోతే SQLITE_DEBUG
  వాడని_PARAMETER( argc );
#అయితే_ముగింపు
}
స్థిర స్థిరరాశి FuncDef statGetFuncdef = {
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

స్థిర శూన్య callStatGet(Parse *pParse, పూర్ణాంకం regStat, పూర్ణాంకం iParam, పూర్ణాంకం reg_బహిర్గతం){
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  sqlite3_Vdbe_కూడు_Op2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#లేదావేళ SQLITE_DEBUG
  ప్రతిజ్ఞా( iParam==STAT_GET_STAT1 );
#లేకపోతే
  వాడని_PARAMETER( iParam );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( reg_బహిర్గతం!=regStat && reg_బహిర్గతం!=regStat+1 );
  sqlite3_Vdbe_కూడు_Function_Call(pParse, 0, regStat, reg_బహిర్గతం, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
స్థిర శూన్య analyze_Vdbe_Comment_సూచీ_With_స్తంభం_Name(
  Vdbe *v,         /* Prepared statement under construction */
  సూచీ *pIdx,     /* Index whose column is being loaded */
  పూర్ణాంకం k            /* Which column index */
){
  పూర్ణాంకం i;           /* Index of column in the table */
  ప్రతిజ్ఞా( k>=0 && k<pIdx->n_స్తంభం );
  i = pIdx->ai_స్తంభం[k];
  ఒకవేళ( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }లేకపోతే ఒకవేళ( i==XN_సమాసం ){
    ప్రతిజ్ఞా( pIdx->b_Has_సమాసం );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }లేకపోతే{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->p_పట్టిక->a_నిలువు[i].zCnName));
  }
}
#లేకపోతే
# define analyze_Vdbe_Comment_సూచీ_With_స్తంభం_Name(a,b,c)
#అయితే_ముగింపు /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
స్థిర శూన్య analyze_One_పట్టిక(
  Parse *pParse,   /* Parser context */
  పట్టిక *pTab,     /* Table whose indices are to be analyzed */
  సూచీ *pOnlyIdx, /* If not NULL, only analyze this one index */
  పూర్ణాంకం iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  పూర్ణాంకం iMem,        /* Available memory locations begin here */
  పూర్ణాంకం iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  సూచీ *pIdx;                 /* An index to being analyzed */
  పూర్ణాంకం iIdxCur;                 /* Cursor open on index being analyzed */
  పూర్ణాంకం iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  పూర్ణాంకం i;                       /* Loop counter */
  పూర్ణాంకం j_Zero_అడ్డువరుసలు = -1;          /* Jump from here if number of rows is zero */
  పూర్ణాంకం iDb;                     /* Index of database containing pTab */
  u8 need_పట్టిక_Cnt = 1;         /* True to count the table */
  పూర్ణాంకం regNewRowid = iMem++;    /* Rowid for the inserted record */
  పూర్ణాంకం regStat = iMem++;        /* Register to hold StatAccum object */
  పూర్ణాంకం regChng = iMem++;        /* Index of changed index field */
  పూర్ణాంకం regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  పూర్ణాంకం reg_తాత్కాలిక = iMem++;        /* Temporary use register */
  పూర్ణాంకం regTemp2 = iMem++;       /* Second temporary use register */
  పూర్ణాంకం regTabname = iMem++;     /* Register containing table name */
  పూర్ణాంకం regIdxname = iMem++;     /* Register containing index name */
  పూర్ణాంకం regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  పూర్ణాంకం regPrev = iMem;          /* MUST BE LAST (see below) */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  పూర్ణాంకం doOnce = 1;              /* Flag for a one-time computation */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
  పట్టిక *pStat1 = 0;
#అయితే_ముగింపు

  sqlite3_Touch_రిజిస్టరు(pParse, iMem);
  ప్రతిజ్ఞా( sqlite3_No_Temps_లోపల_Range(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v==0 || NEVER(pTab==0) ){
    తిరిగి;
  }
  ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    తిరిగి;
  }
  ఒకవేళ( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    తిరిగి;
  }
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
  ప్రతిజ్ఞా( iDb>=0 );
  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    తిరిగి;
  }
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
  ఒకవేళ( db->xPreUpdateCallback ){
    pStat1 = (పట్టిక*)sqlite3DbMallocZero(db, పరిమాణం(పట్టిక) + 13);
    ఒకవేళ( pStat1==0 ) తిరిగి;
    pStat1->zName = (అక్షరం*)&pStat1[1];
    జ్ఞాపకం_నకలు(pStat1->zName, "sqlite_stat1", 13);
    pStat1->n_నిలువు = 3;
    pStat1->iPKey = -1;
    sqlite3_Vdbe_కూడు_Op4(pParse->pVdbe, OP_Noop, 0, 0, 0,(అక్షరం*)pStat1,P4_DYNAMIC);
  }
#అయితే_ముగింపు

  /* Establish a read-lock on the table at the shared-cache level. 
  ** Open a read-only cursor on the table. Also allocate a cursor number
  ** to use for scanning indexes (iIdxCur). No index cursor is opened at
  ** this time though.  */
  sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = MAX(pParse->nTab, iTab);
  sqlite3_Open_పట్టిక(pParse, iTabCur, iDb, pTab, OP_OpenRead);
  sqlite3_Vdbe_ఎక్కించు_String(v, regTabname, pTab->zName);

  కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
    పూర్ణాంకం n_నిలువు;                     /* Number of columns in pIdx. "N" */
    పూర్ణాంకం addr_Goto_చివర;               /* Address of "OP_Rewind iIdxCur" */
    పూర్ణాంకం addr_తదుపరి_అడ్డువరుస;              /* Address of "next_row:" */
    స్థిరరాశి అక్షరం *zIdxName;         /* Name of the index */
    పూర్ణాంకం n_నిలువు_Test;                 /* Number of columns to test for changes */

    ఒకవేళ( pOnlyIdx && pOnlyIdx!=pIdx ) కొనసాగు;
    ఒకవేళ( pIdx->pPartIdxWhere==0 ) need_పట్టిక_Cnt = 0;
    ఒకవేళ( !HasRowid(pTab) && Is_Primary_కీలకం_సూచీ(pIdx) ){
      n_నిలువు = pIdx->n_కీలకం_నిలువు;
      zIdxName = pTab->zName;
      n_నిలువు_Test = n_నిలువు - 1;
    }లేకపోతే{
      n_నిలువు = pIdx->n_స్తంభం;
      zIdxName = pIdx->zName;
      n_నిలువు_Test = pIdx->uniqNotNull ? pIdx->n_కీలకం_నిలువు-1 : n_నిలువు-1;
    }

    /* Populate the register containing the index name. */
    sqlite3_Vdbe_ఎక్కించు_String(v, regIdxname, zIdxName);
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
    sqlite3_Touch_రిజిస్టరు(pParse, regPrev+n_నిలువు_Test);

    /* Open a read-only cursor on the index being analyzed. */
    ప్రతిజ్ఞా( iDb==sqlite3_Schema_గమ్యం_సూచీ(db, pIdx->pSchema) );
    sqlite3_Vdbe_కూడు_Op3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pIdx);
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
    ప్రతిజ్ఞా( regTemp2==regStat+4 );
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, db->n_Analysis_హద్దు, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, n_నిలువు, regStat+1);
    ప్రతిజ్ఞా( regRowid==regStat+2 );
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, pIdx->n_కీలకం_నిలువు, regRowid);
    sqlite3_Vdbe_కూడు_Op3(v, OP_లెక్క, iIdxCur, reg_తాత్కాలిక,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3_Vdbe_కూడు_Function_Call(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addr_Goto_చివర = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regChng);
    addr_తదుపరి_అడ్డువరుస = sqlite3VdbeCurrentAddr(v);

    ఒకవేళ( n_నిలువు_Test>0 ){
      పూర్ణాంకం చివర_Distinct_Test = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, పరిమాణం(పూర్ణాంకం)*n_నిలువు_Test);
      ఒకవేళ( aGotoChng==0 ) కొనసాగు;

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
      sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
      addr_తదుపరి_అడ్డువరుస = sqlite3VdbeCurrentAddr(v);
      ఒకవేళ( n_నిలువు_Test==1 && pIdx->n_కీలకం_నిలువు==1 && Is_Unique_సూచీ(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3_Vdbe_కూడు_Op2(v, OP_NotNull, regPrev, చివర_Distinct_Test);
        VdbeCoverage(v);
      }
      కొరకు(i=0; i<n_నిలువు_Test; i++){
        అక్షరం *pColl = (అక్షరం*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, i, regChng);
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iIdxCur, i, reg_తాత్కాలిక);
        analyze_Vdbe_Comment_సూచీ_With_స్తంభం_Name(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3_Vdbe_కూడు_Op4(v, OP_Ne, reg_తాత్కాలిక, 0, regPrev+i, pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, n_నిలువు_Test, regChng);
      sqlite3VdbeGoto(v, చివర_Distinct_Test);
  
  
      /*
      **  chng_addr_0:
      **   regPrev(0) = idx(0)
      **  chng_addr_1:
      **   regPrev(1) = idx(1)
      **  ...
      */
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_తదుపరి_అడ్డువరుస-1);
      కొరకు(i=0; i<n_నిలువు_Test; i++){
        sqlite3_Vdbe_Jump_ఇక్కడ(v, aGotoChng[i]);
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iIdxCur, i, regPrev+i);
        analyze_Vdbe_Comment_సూచీ_With_స్తంభం_Name(v,pIdx,i);
      }
      sqlite3VdbeResolveLabel(v, చివర_Distinct_Test);
      sqlite3DbFree(db, aGotoChng);
    }
  
    /*
    **  chng_addr_N:
    **   regRowid = idx(rowid)            // STAT4 only
    **   stat_push(P, regChng, regRowid)  // 3rd parameter STAT4 only
    **   Next csr
    **   if !eof(csr) goto next_row;
    */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    ఒకవేళ( OptimizationEnabled(db, SQLITE_Stat4) ){
      ప్రతిజ్ఞా( regRowid==(regStat+2) );
      ఒకవేళ( HasRowid(pTab) ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_IdxRowid, iIdxCur, regRowid);
      }లేకపోతే{
        సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pIdx->p_పట్టిక);
        పూర్ణాంకం j, k, reg_కీలకం;
        reg_కీలకం = sqlite3_Get_తాత్కాలిక_Range(pParse, pPk->n_కీలకం_నిలువు);
        కొరకు(j=0; j<pPk->n_కీలకం_నిలువు; j++){
          k = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pIdx, pPk->ai_స్తంభం[j]);
          ప్రతిజ్ఞా( k>=0 && k<pIdx->n_స్తంభం );
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iIdxCur, k, reg_కీలకం+j);
          analyze_Vdbe_Comment_సూచీ_With_స్తంభం_Name(v,pIdx,k);
        }
        sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_కీలకం, pPk->n_కీలకం_నిలువు, regRowid);
        sqlite3_Release_తాత్కాలిక_Range(pParse, reg_కీలకం, pPk->n_కీలకం_నిలువు);
      }
    }
#అయితే_ముగింపు
    ప్రతిజ్ఞా( regChng==(regStat+1) );
    {
      sqlite3_Vdbe_కూడు_Function_Call(pParse, 1, regStat, reg_తాత్కాలిక, 2+IsStat4,
                                 &stat_నెట్టు_Funcdef, 0);
      ఒకవేళ( db->n_Analysis_హద్దు ){
        పూర్ణాంకం j1, j2, j3;
        j1 = sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, reg_తాత్కాలిక); VdbeCoverage(v);
        j2 = sqlite3_Vdbe_కూడు_Op1(v, OP_If, reg_తాత్కాలిక); VdbeCoverage(v);
        j3 = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_SeekGT, iIdxCur, 0, regPrev, 1);
        VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, j1);
        sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iIdxCur, addr_తదుపరి_అడ్డువరుస); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, j2);
        sqlite3_Vdbe_Jump_ఇక్కడ(v, j3);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iIdxCur, addr_తదుపరి_అడ్డువరుస); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    ఒకవేళ( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_Goto_చివర);
      addr_Goto_చివర = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    ప్రతిజ్ఞా( "BBB"[0]==SQLITE_AFF_పాఠ్యం );
    sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, regTabname, 3, reg_తాత్కాలిక, "BBB", 0);
    sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iStatCur, reg_తాత్కాలిక, regNewRowid);
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (అక్షరం*)pStat1, P4_పట్టిక);
#అయితే_ముగింపు
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    ఒకవేళ( OptimizationEnabled(db, SQLITE_Stat4) && db->n_Analysis_హద్దు==0 ){
      పూర్ణాంకం regEq = regStat1;
      పూర్ణాంకం regLt = regStat1+1;
      పూర్ణాంకం regDLt = regStat1+2;
      పూర్ణాంకం regSample = regStat1+3;
      పూర్ణాంకం reg_నిలువు = regStat1+4;
      పూర్ణాంకం regSampleRowid = reg_నిలువు + n_నిలువు;
      పూర్ణాంకం addr_తదుపరి;
      పూర్ణాంకం addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_Not_దొరికింది;

      /* No STAT4 data is generated if the number of rows is zero */
      ఒకవేళ( addr_Goto_చివర==0 ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addr_Goto_చివర = sqlite3_Vdbe_కూడు_Op1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      ఒకవేళ( doOnce ){
        పూర్ణాంకం mx_నిలువు = n_నిలువు;
        సూచీ *pX;

        /* Compute the maximum number of columns in any index */
        కొరకు(pX=pTab->p_సూచీ; pX; pX=pX->p_తదుపరి){
          పూర్ణాంకం n_నిలువు_X;                     /* Number of columns in pX */
          ఒకవేళ( !HasRowid(pTab) && Is_Primary_కీలకం_సూచీ(pX) ){
            n_నిలువు_X = pX->n_కీలకం_నిలువు;
          }లేకపోతే{
            n_నిలువు_X = pX->n_స్తంభం;
          }
          ఒకవేళ( n_నిలువు_X>mx_నిలువు ) mx_నిలువు = n_నిలువు_X;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3_Touch_రిజిస్టరు(pParse, reg_నిలువు+mx_నిలువు);
        doOnce = 0;
#నిర్వచితమైతే SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3_No_Temps_లోపల_Range(pParse, regEq, reg_నిలువు+mx_నిలువు) );
#అయితే_ముగింపు
        sqlite3_Clear_తాత్కాలిక_Reg_Cache(pParse);  /* tag-20230325-1 */
        ప్రతిజ్ఞా( sqlite3_No_Temps_లోపల_Range(pParse, regEq, reg_నిలువు+mx_నిలువు) );
      }
      ప్రతిజ్ఞా( sqlite3_No_Temps_లోపల_Range(pParse, regEq, reg_నిలువు+n_నిలువు) );

      addr_తదుపరి = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3_Vdbe_కూడు_Op4_Int(v, seekOp, iTabCur, addr_తదుపరి, regSampleRowid, 0);
      VdbeCoverage(v);
      కొరకు(i=0; i<n_నిలువు; i++){
        sqlite3_సమాసం_సంకేతం_ఎక్కించు_సూచీ_స్తంభం(pParse, pIdx, iTabCur, i, reg_నిలువు+i);
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_నిలువు, n_నిలువు, regSample);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, regTabname, 6, reg_తాత్కాలిక);
      sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iStatCur+1, reg_తాత్కాలిక, regNewRowid);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 1, addr_తదుపరి); /* P1==1 for end-of-loop */
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addrIsNull);
    }
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    ఒకవేళ( addr_Goto_చివర ) sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_Goto_చివర);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  ఒకవేళ( pOnlyIdx==0 && need_పట్టిక_Cnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3_Vdbe_కూడు_Op2(v, OP_లెక్క, iTabCur, regStat1);
    j_Zero_అడ్డువరుసలు = sqlite3_Vdbe_కూడు_Op1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, regIdxname);
    ప్రతిజ్ఞా( "BBB"[0]==SQLITE_AFF_పాఠ్యం );
    sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, regTabname, 3, reg_తాత్కాలిక, "BBB", 0);
    sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iStatCur, reg_తాత్కాలిక, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (అక్షరం*)pStat1, P4_పట్టిక);
#అయితే_ముగింపు
    sqlite3_Vdbe_Jump_ఇక్కడ(v, j_Zero_అడ్డువరుసలు);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
స్థిర శూన్య ఎక్కించు_Analysis(Parse *pParse, పూర్ణాంకం iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  ఒకవేళ( v ){
    sqlite3_Vdbe_కూడు_Op1(v, OP_ఎక్కించు_Analysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
స్థిర శూన్య analyzeDatabase(Parse *pParse, పూర్ణాంకం iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  పూర్ణాంకం iStatCur;
  పూర్ణాంకం iMem;
  పూర్ణాంకం iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  open_Stat_పట్టిక(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
  కొరకు(k=sqlite_Hash_మొదటి(&pSchema->tblHash); k; k=sqlite_Hash_తదుపరి(k)){
    పట్టిక *pTab = (పట్టిక*)sqlite_Hash_దత్తాంశం(k);
    analyze_One_పట్టిక(pParse, pTab, 0, iStatCur, iMem, iTab);
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    iMem = sqlite3_మొదటి_అందుబాటు_రిజిస్టరు(pParse, iMem);
#లేకపోతే
    ప్రతిజ్ఞా( iMem==sqlite3_మొదటి_అందుబాటు_రిజిస్టరు(pParse,iMem) );
#అయితే_ముగింపు
  }
  ఎక్కించు_Analysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
స్థిర శూన్య analyze_పట్టిక(Parse *pParse, పట్టిక *pTab, సూచీ *pOnlyIdx){
  పూర్ణాంకం iDb;
  పూర్ణాంకం iStatCur;

  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  ఒకవేళ( pOnlyIdx ){
    open_Stat_పట్టిక(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }లేకపోతే{
    open_Stat_పట్టిక(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyze_One_పట్టిక(pParse, pTab, pOnlyIdx, iStatCur,pParse->nMem+1,pParse->nTab);
  ఎక్కించు_Analysis(pParse, iDb);
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
శూన్య sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  పూర్ణాంకం iDb;
  పూర్ణాంకం i;
  అక్షరం *z, *zDb;
  పట్టిక *pTab;
  సూచీ *pIdx;
  Token *p_పట్టిక_Name;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  ప్రతిజ్ఞా( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  ఒకవేళ( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    తిరిగి;
  }

  ప్రతిజ్ఞా( pName2!=0 || pName1==0 );
  ఒకవేళ( pName1==0 ){
    /* Form 1:  Analyze everything */
    కొరకు(i=0; i<db->nDb; i++){
      ఒకవేళ( i==1 ) కొనసాగు;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }లేకపోతే ఒకవేళ( pName2->n==0 && (iDb = sqlite3_కనుగొను_Db(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }లేకపోతే{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &p_పట్టిక_Name);
    ఒకవేళ( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, p_పట్టిక_Name);
      ఒకవేళ( z ){
        ఒకవేళ( (pIdx = sqlite3_కనుగొను_సూచీ(db, z, zDb))!=0 ){
          analyze_పట్టిక(pParse, pIdx->p_పట్టిక, pIdx);
        }లేకపోతే ఒకవేళ( (pTab = sqlite3_Locate_పట్టిక(pParse, 0, z, zDb))!=0 ){
          analyze_పట్టిక(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  ఒకవేళ( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3_Vdbe_కూడు_Op0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
రకనిర్వచనం నిర్మాణం analysisInfo analysisInfo;
నిర్మాణం analysisInfo {
  sqlite3 *db;
  స్థిరరాశి అక్షరం *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
స్థిర శూన్య decodeIntArray(
  అక్షరం *zIntArray,       /* String containing int array to decode */
  పూర్ణాంకం n_బహిర్గతం,              /* Number of slots in aOut[] */
  tRowcnt *a_బహిర్గతం,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  సూచీ *p_సూచీ          /* Handle extra flags for this index, if not NULL */
){
  అక్షరం *z = zIntArray;
  పూర్ణాంకం c;
  పూర్ణాంకం i;
  tRowcnt v;

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( z==0 ) z = "";
#లేకపోతే
  ప్రతిజ్ఞా( z!=0 );
#అయితే_ముగింపు
  కొరకు(i=0; *z && i<n_బహిర్గతం; i++){
    v = 0;
    అయితే( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    ఒకవేళ( a_బహిర్గతం ) a_బహిర్గతం[i] = v;
    ఒకవేళ( aLog ) aLog[i] = sqlite3LogEst(v);
#లేకపోతే
    ప్రతిజ్ఞా( a_బహిర్గతం==0 );
    వాడని_PARAMETER(a_బహిర్గతం);
    ప్రతిజ్ఞా( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#అయితే_ముగింపు
    ఒకవేళ( *z==' ' ) z++;
  }
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_STAT4
  ప్రతిజ్ఞా( p_సూచీ!=0 ); {
#లేకపోతే
  ఒకవేళ( p_సూచీ ){
#అయితే_ముగింపు
    p_సూచీ->bUnordered = 0;
    p_సూచీ->no_Skip_పరిశీలన = 0;
    అయితే( z[0] ){
      ఒకవేళ( sqlite3_strglob("unordered*", z)==0 ){
        p_సూచీ->bUnordered = 1;
      }లేకపోతే ఒకవేళ( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        పూర్ణాంకం sz = sqlite3Atoi(z+3);
        ఒకవేళ( sz<2 ) sz = 2;
        p_సూచీ->sz_Idx_అడ్డువరుస = sqlite3LogEst(sz);
      }లేకపోతే ఒకవేళ( sqlite3_strglob("noskipscan*", z)==0 ){
        p_సూచీ->no_Skip_పరిశీలన = 1;
      }
#నిర్వచితమైతే SQLITE_ENABLE_COSTMULT
      లేకపోతే ఒకవేళ( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        p_సూచీ->p_పట్టిక->ఖర్చు_Mult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#అయితే_ముగింపు
      అయితే( z[0]!=0 && z[0]!=' ' ) z++;
      అయితే( z[0]==' ' ) z++;
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
స్థిర పూర్ణాంకం analysisLoader(శూన్య *p_దత్తాంశం, పూర్ణాంకం argc, అక్షరం **argv, అక్షరం **Not_వాడిన){
  analysisInfo *pInfo = (analysisInfo*)p_దత్తాంశం;
  సూచీ *p_సూచీ;
  పట్టిక *p_పట్టిక;
  స్థిరరాశి అక్షరం *z;

  ప్రతిజ్ఞా( argc==3 );
  వాడని_PARAMETER2(Not_వాడిన, argc);

  ఒకవేళ( argv==0 || argv[0]==0 || argv[2]==0 ){
    తిరిగి 0;
  }
  p_పట్టిక = sqlite3_కనుగొను_పట్టిక(pInfo->db, argv[0], pInfo->zDatabase);
  ఒకవేళ( p_పట్టిక==0 ){
    తిరిగి 0;
  }
  ఒకవేళ( argv[1]==0 ){
    p_సూచీ = 0;
  }లేకపోతే ఒకవేళ( sqlite3_stricmp(argv[0],argv[1])==0 ){
    p_సూచీ = sqlite3_Primary_కీలకం_సూచీ(p_పట్టిక);
  }లేకపోతే{
    p_సూచీ = sqlite3_కనుగొను_సూచీ(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  ఒకవేళ( p_సూచీ ){
    tRowcnt *ai_అడ్డువరుస_Est = 0;
    పూర్ణాంకం n_నిలువు = p_సూచీ->n_కీలకం_నిలువు+1;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    ఒకవేళ( p_సూచీ->ai_అడ్డువరుస_Est==0 ){
      p_సూచీ->ai_అడ్డువరుస_Est = (tRowcnt*)sqlite3MallocZero(పరిమాణం(tRowcnt) * n_నిలువు);
      ఒకవేళ( p_సూచీ->ai_అడ్డువరుస_Est==0 ) sqlite3OomFault(pInfo->db);
    }
    ai_అడ్డువరుస_Est = p_సూచీ->ai_అడ్డువరుస_Est;
#అయితే_ముగింపు
    p_సూచీ->bUnordered = 0;
    decodeIntArray((అక్షరం*)z, n_నిలువు, ai_అడ్డువరుస_Est, p_సూచీ->ai_అడ్డువరుస_Log_Est, p_సూచీ);
    p_సూచీ->hasStat1 = 1;
    ఒకవేళ( p_సూచీ->pPartIdxWhere==0 ){
      p_పట్టిక->n_అడ్డువరుస_Log_Est = p_సూచీ->ai_అడ్డువరుస_Log_Est[0];
      p_పట్టిక->tabFlags |= TF_HasStat1;
    }
  }లేకపోతే{
    సూచీ fakeIdx;
    fakeIdx.sz_Idx_అడ్డువరుస = p_పట్టిక->sz_Tab_అడ్డువరుస;
#నిర్వచితమైతే SQLITE_ENABLE_COSTMULT
    fakeIdx.p_పట్టిక = p_పట్టిక;
#అయితే_ముగింపు
    decodeIntArray((అక్షరం*)z, 1, 0, &p_పట్టిక->n_అడ్డువరుస_Log_Est, &fakeIdx);
    p_పట్టిక->sz_Tab_అడ్డువరుస = fakeIdx.sz_Idx_అడ్డువరుస;
    p_పట్టిక->tabFlags |= TF_HasStat1;
  }

  తిరిగి 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
శూన్య sqlite3_Delete_సూచీ_Samples(sqlite3 *db, సూచీ *pIdx){
  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( pIdx!=0 );
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( pIdx->aSample ){
    పూర్ణాంకం j;
    కొరకు(j=0; j<pIdx->nSample; j++){
      సూచీ_Sample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  ఒకవేళ( db->pn_బైట్లు_Freed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#లేకపోతే
  వాడని_PARAMETER(db);
  వాడని_PARAMETER(pIdx);
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */
}

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
స్థిర శూన్య initAvgEq(సూచీ *pIdx){
  ఒకవేళ( pIdx ){
    సూచీ_Sample *aSample = pIdx->aSample;
    సూచీ_Sample *pFinal = &aSample[pIdx->nSample-1];
    పూర్ణాంకం i_నిలువు;
    పూర్ణాంకం n_నిలువు = 1;
    ఒకవేళ( pIdx->n_Sample_నిలువు>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      n_నిలువు = pIdx->n_Sample_నిలువు-1;
      pIdx->aAvgEq[n_నిలువు] = 1;
    }
    కొరకు(i_నిలువు=0; i_నిలువు<n_నిలువు; i_నిలువు++){
      పూర్ణాంకం nSample = pIdx->nSample;
      పూర్ణాంకం i;                    /* Used to iterate through samples */
      tRowcnt కూడిక_Eq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt n_అడ్డువరుస;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      ఒకవేళ( !pIdx->ai_అడ్డువరుస_Est || i_నిలువు>=pIdx->n_కీలకం_నిలువు || pIdx->ai_అడ్డువరుస_Est[i_నిలువు+1]==0 ){
        n_అడ్డువరుస = pFinal->anLt[i_నిలువు];
        nDist100 = (i64)100 * pFinal->anDLt[i_నిలువు];
        nSample--;
      }లేకపోతే{
        n_అడ్డువరుస = pIdx->ai_అడ్డువరుస_Est[0];
        nDist100 = ((i64)100 * pIdx->ai_అడ్డువరుస_Est[0]) / pIdx->ai_అడ్డువరుస_Est[i_నిలువు+1];
      }
      pIdx->n_అడ్డువరుస_Est0 = n_అడ్డువరుస;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      కొరకు(i=0; i<nSample; i++){
        ఒకవేళ( i==(pIdx->nSample-1)
         || aSample[i].anDLt[i_నిలువు]!=aSample[i+1].anDLt[i_నిలువు] 
        ){
          కూడిక_Eq += aSample[i].anEq[i_నిలువు];
          nSum100 += 100;
        }
      }

      ఒకవేళ( nDist100>nSum100 && కూడిక_Eq<n_అడ్డువరుస ){
        avgEq = ((i64)100 * (n_అడ్డువరుస - కూడిక_Eq))/(nDist100 - nSum100);
      }
      ఒకవేళ( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[i_నిలువు] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
స్థిర సూచీ *కనుగొను_సూచీ_Or_Primary_కీలకం(
  sqlite3 *db,
  స్థిరరాశి అక్షరం *zName,
  స్థిరరాశి అక్షరం *zDb
){
  సూచీ *pIdx = sqlite3_కనుగొను_సూచీ(db, zName, zDb);
  ఒకవేళ( pIdx==0 ){
    పట్టిక *pTab = sqlite3_కనుగొను_పట్టిక(db, zName, zDb);
    ఒకవేళ( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_కీలకం_సూచీ(pTab);
  }
  తిరిగి pIdx;
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
స్థిర పూర్ణాంకం ఎక్కించు_Stat_Tbl(
  sqlite3 *db,                  /* Database handle */
  స్థిరరాశి అక్షరం *zSql1,            /* SQL statement 1 (see above) */
  స్థిరరాశి అక్షరం *zSql2,            /* SQL statement 2 (see above) */
  స్థిరరాశి అక్షరం *zDb               /* Database name (e.g. "main") */
){
  పూర్ణాంకం rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  అక్షరం *zSql;                   /* Text of the SQL statement */
  సూచీ *pPrevIdx = 0;          /* Previous index in the loop */
  సూచీ_Sample *pSample;         /* A slot in pIdx->aSample[] */

  ప్రతిజ్ఞా( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  ఒకవేళ( !zSql ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ఒకవేళ( rc ) తిరిగి rc;

  అయితే( sqlite3_అడుగు(pStmt)==SQLITE_అడ్డువరుస ){
    పూర్ణాంకం n_Idx_నిలువు = 1;              /* Number of columns in stat4 records */

    అక్షరం *z_సూచీ;    /* Index name */
    సూచీ *pIdx;     /* Pointer to the index object */
    పూర్ణాంకం nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    z_సూచీ = (అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pStmt, 0);
    ఒకవేళ( z_సూచీ==0 ) కొనసాగు;
    nSample = sqlite3_స్తంభం_int(pStmt, 1);
    pIdx = కనుగొను_సూచీ_Or_Primary_కీలకం(db, z_సూచీ, zDb);
    ప్రతిజ్ఞా( pIdx==0 || pIdx->nSample==0 );
    ఒకవేళ( pIdx==0 ) కొనసాగు;
    ఒకవేళ( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      కొనసాగు;
    }
    ప్రతిజ్ఞా( !HasRowid(pIdx->p_పట్టిక) || pIdx->n_స్తంభం==pIdx->n_కీలకం_నిలువు+1 );
    ఒకవేళ( !HasRowid(pIdx->p_పట్టిక) && Is_Primary_కీలకం_సూచీ(pIdx) ){
      n_Idx_నిలువు = pIdx->n_కీలకం_నిలువు;
    }లేకపోతే{
      n_Idx_నిలువు = pIdx->n_స్తంభం;
    }
    pIdx->n_Sample_నిలువు = n_Idx_నిలువు;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(సూచీ_Sample) * nSample);
    nByte += sizeof64(tRowcnt) * n_Idx_నిలువు * 3 * nSample;
    nByte += n_Idx_నిలువు * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    ఒకవేళ( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += n_Idx_నిలువు;
    pIdx->p_పట్టిక->tabFlags |= TF_HasStat4;
    కొరకు(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += n_Idx_నిలువు;
      pIdx->aSample[i].anLt = pSpace; pSpace += n_Idx_నిలువు;
      pIdx->aSample[i].anDLt = pSpace; pSpace += n_Idx_నిలువు;
    }
    ప్రతిజ్ఞా( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  ఒకవేళ( rc ) తిరిగి rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  ఒకవేళ( !zSql ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  ఒకవేళ( rc ) తిరిగి rc;

  అయితే( sqlite3_అడుగు(pStmt)==SQLITE_అడ్డువరుస ){
    అక్షరం *z_సూచీ;                 /* Index name */
    సూచీ *pIdx;                  /* Pointer to the index object */
    పూర్ణాంకం n_నిలువు = 1;                 /* Number of columns in index */

    z_సూచీ = (అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pStmt, 0);
    ఒకవేళ( z_సూచీ==0 ) కొనసాగు;
    pIdx = కనుగొను_సూచీ_Or_Primary_కీలకం(db, z_సూచీ, zDb);
    ఒకవేళ( pIdx==0 ) కొనసాగు;
    ఒకవేళ( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      కొనసాగు;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    n_నిలువు = pIdx->n_Sample_నిలువు;
    ఒకవేళ( pIdx!=pPrevIdx ){
      initAvgEq(pPrevIdx);
      pPrevIdx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt,1),n_నిలువు,pSample->anEq,0,0);
    decodeIntArray((అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt,2),n_నిలువు,pSample->anLt,0,0);
    decodeIntArray((అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pStmt,3),n_నిలువు,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_స్తంభం_బైట్లు(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    ఒకవేళ( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    ఒకవేళ( pSample->n ){
      జ్ఞాపకం_నకలు(pSample->p, sqlite3_స్తంభం_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  ఒకవేళ( rc==SQLITE_OK ) initAvgEq(pPrevIdx);
  తిరిగి rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
స్థిర పూర్ణాంకం ఎక్కించు_Stat4(sqlite3 *db, స్థిరరాశి అక్షరం *zDb){
  పూర్ణాంకం rc = SQLITE_OK;             /* Result codes from subroutines */
  స్థిరరాశి పట్టిక *pStat4;

  ప్రతిజ్ఞా( db->lookaside.bDisable );
  ఒకవేళ( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3_కనుగొను_పట్టిక(db, "sqlite_stat4", zDb))!=0
   && Is_Ordinary_పట్టిక(pStat4)
  ){
    rc = ఎక్కించు_Stat_Tbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_STAT4 */

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
పూర్ణాంకం sqlite3_Analysis_ఎక్కించు(sqlite3 *db, పూర్ణాంకం iDb){
  analysisInfo sInfo;
  HashElem *i;
  అక్షరం *zSql;
  పూర్ణాంకం rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  స్థిరరాశి పట్టిక *pStat1;

  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ప్రతిజ్ఞా( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
  కొరకు(i=sqlite_Hash_మొదటి(&pSchema->tblHash); i; i=sqlite_Hash_తదుపరి(i)){
    పట్టిక *pTab = sqlite_Hash_దత్తాంశం(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  కొరకు(i=sqlite_Hash_మొదటి(&pSchema->idxHash); i; i=sqlite_Hash_తదుపరి(i)){
    సూచీ *pIdx = sqlite_Hash_దత్తాంశం(i);
    pIdx->hasStat1 = 0;
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
    sqlite3_Delete_సూచీ_Samples(db, pIdx);
    pIdx->aSample = 0;
#అయితే_ముగింపు
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  ఒకవేళ( (pStat1 = sqlite3_కనుగొను_పట్టిక(db, "sqlite_stat1", sInfo.zDatabase))
   && Is_Ordinary_పట్టిక(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    ఒకవేళ( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }లేకపోతే{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  ప్రతిజ్ఞా( sqlite3SchemaMutexHeld(db, iDb, 0) );
  కొరకు(i=sqlite_Hash_మొదటి(&pSchema->idxHash); i; i=sqlite_Hash_తదుపరి(i)){
    సూచీ *pIdx = sqlite_Hash_దత్తాంశం(i);
    ఒకవేళ( !pIdx->hasStat1 ) sqlite3_Default_అడ్డువరుస_Est(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  ఒకవేళ( rc==SQLITE_OK ){
    DisableLookaside;
    rc = ఎక్కించు_Stat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  కొరకు(i=sqlite_Hash_మొదటి(&pSchema->idxHash); i; i=sqlite_Hash_తదుపరి(i)){
    సూచీ *pIdx = sqlite_Hash_దత్తాంశం(i);
    sqlite3_free(pIdx->ai_అడ్డువరుస_Est);
    pIdx->ai_అడ్డువరుస_Est = 0;
  }
#అయితే_ముగింపు

  ఒకవేళ( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  తిరిగి rc;
}


#అయితే_ముగింపు /* SQLITE_OMIT_ANALYZE */
