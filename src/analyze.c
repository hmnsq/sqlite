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
#که_نه_تعریف_شوی SQLITE_OMIT_ANALYZE
#ضمن "sqliteInt.h"

#إذا defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#وإلا
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#د_که_پای

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
ساكن فراغ open_Stat_جدول(
  Parse *pParse,          /* Parsing context */
  صحيح iDb,                /* The database we are looking in */
  صحيح iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  ثابت حرف *zWhere,     /* Delete entries for this table or index */
  ثابت حرف *zWhereType  /* Either "tbl" or "idx" */
){
  ساكن ثابت بنية {
    ثابت حرف *zName;
    ثابت حرف *z_ستون_شمېر;
  } a_جدول[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#إذا defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#وإلا
    { "sqlite_stat4", 0 },
#د_که_پای
    { "sqlite_stat3", 0 },
  };
  صحيح i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_ریښه[Array_اندازه(a_جدول)];
  u8 aCreateTbl[Array_اندازه(a_جدول)];
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  ثابت صحيح n_منزل_Open = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#وإلا
  ثابت صحيح n_منزل_Open = 1;
#د_که_پای

  إذا( v==0 ) أرجع;
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  أكد( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  لكل(i=0; i<Array_اندازه(a_جدول); i++){
    ثابت حرف *zTab = a_جدول[i].zName;
    جدول *pStat;
    aCreateTbl[i] = 0;
    إذا( (pStat = sqlite3_پیدا_کړه_جدول(db, zTab, pDb->zDbSName))==0 ){
      إذا( i<n_منزل_Open ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, a_جدول[i].z_ستون_شمېر
        );
        أكد( pParse->isCreate || pParse->nErr );
        a_ریښه[i] = (u32)pParse->u1.cr.reg_ریښه;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }وإلا{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_ریښه[i] = pStat->tnum;
      sqlite3_جدول_قلف(pParse, iDb, a_ریښه[i], 1, zTab);
      إذا( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
      }وإلا إذا( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#د_که_پای
      }وإلا{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Clear, (صحيح)a_ریښه[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  لكل(i=0; i<n_منزل_Open; i++){
    أكد( i<Array_اندازه(a_جدول) );
    sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_OpenWrite, iStatCur+i, (صحيح)a_ریښه[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, a_جدول[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#که_نه_تعریف_شوی SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#د_که_پای

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
تعريفنوع بنية StatAccum StatAccum;
تعريفنوع بنية StatSample StatSample;
بنية StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  اتحاد {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  صحيح i_ستون;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#د_که_پای
};                                                    
بنية StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt n_لیکه;             /* Number of rows visited so far */
  صحيح n_حد;               /* Analysis row-scan limit */
  صحيح n_ستون;                 /* Number of columns in index + pk/rowid */
  صحيح n_کلی_ستون;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  صحيح mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *a_غوره;        /* Array of nCol best samples */
  صحيح iMin;                 /* Index in a[] of entry with minimum score */
  صحيح nSample;              /* Current number of samples */
  صحيح nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  صحيح iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#د_که_پای
};

/* Reclaim memory used by a StatSample
*/
#که_تعریف_شوی SQLITE_ENABLE_STAT4
ساكن فراغ sampleClear(sqlite3 *db, StatSample *p){
  أكد( db!=0 );
  إذا( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#د_که_پای

/* Initialize the BLOB value of a ROWID
*/
#که_تعریف_شوی SQLITE_ENABLE_STAT4
ساكن فراغ sample_سېټ_Rowid(sqlite3 *db, StatSample *p, صحيح n, ثابت u8 *p_معلومات){
  أكد( db!=0 );
  إذا( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  إذا( p->u.aRowid ){
    p->nRowid = n;
    حافظه_نقل(p->u.aRowid, p_معلومات, n);
  }وإلا{
    p->nRowid = 0;
  }
}
#د_که_پای

/* Initialize the INTEGER value of a ROWID.
*/
#که_تعریف_شوی SQLITE_ENABLE_STAT4
ساكن فراغ sample_سېټ_Rowid_Int64(sqlite3 *db, StatSample *p, i64 iRowid){
  أكد( db!=0 );
  إذا( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#د_که_پای


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#که_تعریف_شوی SQLITE_ENABLE_STAT4
ساكن فراغ sampleCopy(StatAccum *p, StatSample *p_منزل, StatSample *pFrom){
  p_منزل->isPSample = pFrom->isPSample;
  p_منزل->i_ستون = pFrom->i_ستون;
  p_منزل->iHash = pFrom->iHash;
  حافظه_نقل(p_منزل->anEq, pFrom->anEq, حجم(tRowcnt)*p->n_ستون);
  حافظه_نقل(p_منزل->anLt, pFrom->anLt, حجم(tRowcnt)*p->n_ستون);
  حافظه_نقل(p_منزل->anDLt, pFrom->anDLt, حجم(tRowcnt)*p->n_ستون);
  إذا( pFrom->nRowid ){
    sample_سېټ_Rowid(p->db, p_منزل, pFrom->nRowid, pFrom->u.aRowid);
  }وإلا{
    sample_سېټ_Rowid_Int64(p->db, p_منزل, pFrom->u.iRowid);
  }
}
#د_که_پای

/*
** Reclaim all memory of a StatAccum structure.
*/
ساكن فراغ statAccumDestructor(فراغ *pOld){
  StatAccum *p = (StatAccum*)pOld;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( p->mxSample ){
    صحيح i;
    لكل(i=0; i<p->n_ستون; i++) sampleClear(p->db, p->a_غوره+i);
    لكل(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#د_که_پای
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
ساكن فراغ statInit(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  StatAccum *p;
  صحيح n_ستون;                       /* Number of columns in index being sampled */
  صحيح n_کلی_ستون;                    /* Number of key columns */
  صحيح n_ستون_پورته;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  صحيح mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#د_که_پای

  /* Decode the three function arguments */
  ناکارول_شوی_PARAMETER(argc);
  n_ستون = sqlite3_ارزښت_int(argv[0]);
  أكد( n_ستون>0 );
  n_ستون_پورته = حجم(tRowcnt)<8 ? (n_ستون+1)&~1 : n_ستون;
  n_کلی_ستون = sqlite3_ارزښت_int(argv[1]);
  أكد( n_کلی_ستون<=n_ستون );
  أكد( n_کلی_ستون>0 );

  /* Allocate the space required for the StatAccum object */
  n = حجم(*p) 
    + حجم(tRowcnt)*n_ستون_پورته;                    /* StatAccum.anDLt */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  n += حجم(tRowcnt)*n_ستون_پورته;                   /* StatAccum.anEq */
  إذا( mxSample ){
    n += حجم(tRowcnt)*n_ستون_پورته                  /* StatAccum.anLt */
      + حجم(StatSample)*(n_ستون+mxSample)       /* StatAccum.aBest[], a[] */
      + حجم(tRowcnt)*3*n_ستون_پورته*(n_ستون+mxSample);
  }
#د_که_پای
  p = sqlite3DbMallocZero(db, n);
  إذا( p==0 ){
    sqlite3_پایله_error_nomem(context);
    أرجع;
  }

  p->db = db;
  p->nEst = sqlite3_ارزښت_int64(argv[2]);
  p->n_لیکه = 0;
  p->n_حد = sqlite3_ارزښت_int(argv[3]);
  p->n_ستون = n_ستون;
  p->n_کلی_ستون = n_کلی_ستون;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#که_تعریف_شوی SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[n_ستون_پورته];
  p->mxSample = p->n_حد==0 ? mxSample : 0;
  إذا( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    صحيح i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[n_ستون_پورته];
    p->iPrn = 0x689e962d*(u32)n_ستون ^ 0xd0944565*(u32)sqlite3_ارزښت_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (بنية StatSample*)&p->current.anLt[n_ستون_پورته];
    p->a_غوره = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+n_ستون]);
    لكل(i=0; i<(mxSample+n_ستون); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (حجم(tRowcnt) * n_ستون_پورته);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (حجم(tRowcnt) * n_ستون_پورته);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (حجم(tRowcnt) * n_ستون_پورته);
    }
    أكد( (pSpace - (u8*)p)==n );
  
    لكل(i=0; i<n_ستون; i++){
      p->a_غوره[i].i_ستون = i;
    }
  }
#د_که_پای

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_پایله_blob(context, p, حجم(*p), statAccumDestructor);
}
ساكن ثابت FuncDef statInitFuncdef = {
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

#که_تعریف_شوی SQLITE_ENABLE_STAT4
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
ساكن صحيح sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  صحيح n_ستون = pAccum->n_ستون;
  صحيح i;
  أكد( pNew->i_ستون==pOld->i_ستون );
  لكل(i=pNew->i_ستون+1; i<n_ستون; i++){
    إذا( pNew->anEq[i]>pOld->anEq[i] ) أرجع 1;
    إذا( pNew->anEq[i]<pOld->anEq[i] ) أرجع 0;
  }
  إذا( pNew->iHash>pOld->iHash ) أرجع 1;
  أرجع 0;
}
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
ساكن صحيح sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->i_ستون];
  tRowcnt nEqOld = pOld->anEq[pOld->i_ستون];

  أكد( pOld->isPSample==0 && pNew->isPSample==0 );
  أكد( IsStat4 || (pNew->i_ستون==0 && pOld->i_ستون==0) );

  إذا( (nEqNew>nEqOld) ) أرجع 1;
  إذا( nEqNew==nEqOld ){
    إذا( pNew->i_ستون<pOld->i_ستون ) أرجع 1;
    أرجع (pNew->i_ستون==pOld->i_ستون && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  أرجع 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
ساكن فراغ sample_ورننباسه(StatAccum *p, StatSample *pNew, صحيح nEqZero){
  StatSample *pSample = 0;
  صحيح i;

  أكد( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  إذا( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  إذا( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    أكد( pNew->anEq[pNew->i_ستون]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    لكل(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      إذا( pOld->anEq[pNew->i_ستون]==0 ){
        إذا( pOld->isPSample ) أرجع;
        أكد( pOld->i_ستون>pNew->i_ستون );
        أكد( sampleIsBetter(p, pNew, pOld) );
        إذا( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    إذا( pUpgrade ){
      pUpgrade->i_ستون = pNew->i_ستون;
      pUpgrade->anEq[pUpgrade->i_ستون] = pNew->anEq[pUpgrade->i_ستون];
      اذهب پیدا_کړه_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  إذا( p->nSample>=p->mxSample ){
    StatSample *pMin = &p->a[p->iMin];
    tRowcnt *anEq = pMin->anEq;
    tRowcnt *anLt = pMin->anLt;
    tRowcnt *anDLt = pMin->anDLt;
    sampleClear(p->db, pMin);
    memmove(pMin, &pMin[1], حجم(p->a[0])*(p->nSample-p->iMin-1));
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
  أكد( p->nSample==0 
       || pNew->anLt[p->n_ستون-1] > p->a[p->nSample-1].anLt[p->n_ستون-1] );

  /* Insert the new sample */
  pSample = &p->a[p->nSample];
  sampleCopy(p, pSample, pNew);
  p->nSample++;

  /* Zero the first nEqZero entries in the anEq[] array. */
  memset(pSample->anEq, 0, حجم(tRowcnt)*nEqZero);

پیدا_کړه_new_min:
  إذا( p->nSample>=p->mxSample ){
    صحيح iMin = -1;
    لكل(i=0; i<p->mxSample; i++){
      إذا( p->a[i].isPSample ) استمر;
      إذا( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    أكد( iMin>=0 );
    p->iMin = iMin;
  }
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */

#که_تعریف_شوی SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
ساكن فراغ sample_ټېل_پخوانی(StatAccum *p, صحيح iChng){
  صحيح i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  لكل(i=(p->n_ستون-2); i>=iChng; i--){
    StatSample *p_غوره = &p->a_غوره[i];
    p_غوره->anEq[i] = p->current.anEq[i];
    إذا( p->nSample<p->mxSample || sampleIsBetter(p, p_غوره, &p->a[p->iMin]) ){
      sample_ورننباسه(p, p_غوره, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  لكل(i=p->nSample-1; i>=0; i--){
    صحيح j;
    لكل(j=p->nMaxEqZero; j<p->n_ستون; j++) أكد( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  إذا( iChng<p->nMaxEqZero ){
    لكل(i=p->nSample-1; i>=0; i--){
      صحيح j;
      لكل(j=iChng; j<p->n_ستون; j++){
        إذا( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */

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
ساكن فراغ stat_ټېل(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  صحيح i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_ارزښت_blob(argv[0]);
  صحيح iChng = sqlite3_ارزښت_int(argv[1]);

  ناکارول_شوی_PARAMETER( argc );
  ناکارول_شوی_PARAMETER( context );
  أكد( p->n_ستون>0 );
  أكد( iChng<p->n_ستون );

  إذا( p->n_لیکه==0 ){
    /* This is the first call to this function. Do initialization. */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    لكل(i=0; i<p->n_ستون; i++) p->current.anEq[i] = 1;
#د_که_پای
  }وإلا{
    /* Second and subsequent calls get processed here */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    إذا( p->mxSample ) sample_ټېل_پخوانی(p, iChng);
#د_که_پای

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    لكل(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#د_که_پای
    لكل(i=iChng; i<p->n_ستون; i++){
      p->current.anDLt[i]++;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
      إذا( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#د_که_پای
    }
  }

  p->n_لیکه++;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( p->mxSample ){
    tRowcnt nLt;
    إذا( sqlite3_ارزښت_type(argv[2])==SQLITE_INTEGER ){
      sample_سېټ_Rowid_Int64(p->db, &p->current, sqlite3_ارزښت_int64(argv[2]));
    }وإلا{
      sample_سېټ_Rowid(p->db, &p->current, sqlite3_ارزښت_بایټونه(argv[2]),
                                         sqlite3_ارزښت_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->n_ستون-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    إذا( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.i_ستون = 0;
      sample_ورننباسه(p, &p->current, p->n_ستون-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    لكل(i=0; i<(p->n_ستون-1); i++){
      p->current.i_ستون = i;
      إذا( i>=iChng || sampleIsBetterPost(p, &p->current, &p->a_غوره[i]) ){
        sampleCopy(p, &p->a_غوره[i], &p->current);
      }
    }
  }وإلا
#د_که_پای
  إذا( p->n_حد && p->n_لیکه>(tRowcnt)p->n_حد*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_پایله_int(context, p->current.anDLt[0]>0);
  }
}

ساكن ثابت FuncDef stat_ټېل_Funcdef = {
  2+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  stat_ټېل,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_push",     /* zName */
  {0}
};

#تعریف_کړه STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#تعریف_کړه STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#تعریف_کړه STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#تعریف_کړه STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#تعریف_کړه STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

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
ساكن فراغ statGet(
  sqlite3_context *context,
  صحيح argc,
  sqlite3_ارزښت **argv
){
  StatAccum *p = (StatAccum*)sqlite3_ارزښت_blob(argv[0]);
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  صحيح eCall = sqlite3_ارزښت_int(argv[1]);
  أكد( argc==2 );
  أكد( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  أكد( eCall==STAT_GET_STAT1 || p->mxSample );
  إذا( eCall==STAT_GET_STAT1 )
#وإلا
  أكد( argc==1 );
#د_که_پای
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
    صحيح i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_کلی_ستون+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->n_لیکه);
    لكل(i=0; i<p->n_کلی_ستون; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->n_لیکه + nDistinct - 1) / nDistinct;
      إذا( iVal==2 && p->n_لیکه*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#که_تعریف_شوی SQLITE_ENABLE_STAT4
      أكد( p->current.anEq[i] || p->n_لیکه==0 );
#د_که_پای
    }
    sqlite3_پایله_str(context, &sStat, SQLITE_XFER);
  }
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  وإلا إذا( eCall==STAT_GET_ROWID ){
    إذا( p->iGet<0 ){
      sample_ټېل_پخوانی(p, 0);
      p->iGet = 0;
    }
    إذا( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      إذا( pS->nRowid==0 ){
        sqlite3_پایله_int64(context, pS->u.iRowid);
      }وإلا{
        sqlite3_پایله_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }وإلا{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    صحيح i;

    أكد( p->iGet<p->nSample );
    بدل( eCall ){
      حالة STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; اكسر;
      حالة STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; اكسر;
      افتراضي: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        اكسر;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->n_ستون*100);
    لكل(i=0; i<p->n_ستون; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    إذا( sStat.nChar ) sStat.nChar--;
    sqlite3_پایله_str(context, &sStat, SQLITE_XFER);
  }
#د_که_پای /* SQLITE_ENABLE_STAT4 */
#که_نه_تعریف_شوی SQLITE_DEBUG
  ناکارول_شوی_PARAMETER( argc );
#د_که_پای
}
ساكن ثابت FuncDef statGetFuncdef = {
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

ساكن فراغ callStatGet(Parse *pParse, صحيح regStat, صحيح iParam, صحيح reg_بهر){
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  sqlite3_Vdbe_جمع_کړه_Op2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#وإلاإذا SQLITE_DEBUG
  أكد( iParam==STAT_GET_STAT1 );
#وإلا
  ناکارول_شوی_PARAMETER( iParam );
#د_که_پای
  أكد( reg_بهر!=regStat && reg_بهر!=regStat+1 );
  sqlite3_Vdbe_جمع_کړه_Function_Call(pParse, 0, regStat, reg_بهر, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
ساكن فراغ analyze_Vdbe_Comment_شاخص_With_ستنه_Name(
  Vdbe *v,         /* Prepared statement under construction */
  شاخص *pIdx,     /* Index whose column is being loaded */
  صحيح k            /* Which column index */
){
  صحيح i;           /* Index of column in the table */
  أكد( k>=0 && k<pIdx->n_ستنه );
  i = pIdx->ai_ستنه[k];
  إذا( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }وإلا إذا( i==XN_بیان ){
    أكد( pIdx->b_Has_بیان );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }وإلا{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->p_جدول->a_ستون[i].zCnName));
  }
}
#وإلا
# define analyze_Vdbe_Comment_شاخص_With_ستنه_Name(a,b,c)
#د_که_پای /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
ساكن فراغ analyze_One_جدول(
  Parse *pParse,   /* Parser context */
  جدول *pTab,     /* Table whose indices are to be analyzed */
  شاخص *pOnlyIdx, /* If not NULL, only analyze this one index */
  صحيح iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  صحيح iMem,        /* Available memory locations begin here */
  صحيح iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  شاخص *pIdx;                 /* An index to being analyzed */
  صحيح iIdxCur;                 /* Cursor open on index being analyzed */
  صحيح iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  صحيح i;                       /* Loop counter */
  صحيح j_Zero_لیکې = -1;          /* Jump from here if number of rows is zero */
  صحيح iDb;                     /* Index of database containing pTab */
  u8 need_جدول_Cnt = 1;         /* True to count the table */
  صحيح regNewRowid = iMem++;    /* Rowid for the inserted record */
  صحيح regStat = iMem++;        /* Register to hold StatAccum object */
  صحيح regChng = iMem++;        /* Index of changed index field */
  صحيح regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  صحيح reg_لنډمهاله = iMem++;        /* Temporary use register */
  صحيح regTemp2 = iMem++;       /* Second temporary use register */
  صحيح regTabname = iMem++;     /* Register containing table name */
  صحيح regIdxname = iMem++;     /* Register containing index name */
  صحيح regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  صحيح regPrev = iMem;          /* MUST BE LAST (see below) */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  صحيح doOnce = 1;              /* Flag for a one-time computation */
#د_که_پای
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
  جدول *pStat1 = 0;
#د_که_پای

  sqlite3_Touch_ثبتوونکی(pParse, iMem);
  أكد( sqlite3_No_Temps_دننه_Range(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  إذا( v==0 || NEVER(pTab==0) ){
    أرجع;
  }
  إذا( !Is_Ordinary_جدول(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    أرجع;
  }
  إذا( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    أرجع;
  }
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_منزل_شاخص(db, pTab->pSchema);
  أكد( iDb>=0 );
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
  إذا( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    أرجع;
  }
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
  إذا( db->xPreUpdateCallback ){
    pStat1 = (جدول*)sqlite3DbMallocZero(db, حجم(جدول) + 13);
    إذا( pStat1==0 ) أرجع;
    pStat1->zName = (حرف*)&pStat1[1];
    حافظه_نقل(pStat1->zName, "sqlite_stat1", 13);
    pStat1->n_ستون = 3;
    pStat1->iPKey = -1;
    sqlite3_Vdbe_جمع_کړه_Op4(pParse->pVdbe, OP_Noop, 0, 0, 0,(حرف*)pStat1,P4_DYNAMIC);
  }
#د_که_پای

  /* Establish a read-lock on the table at the shared-cache level. 
  ** Open a read-only cursor on the table. Also allocate a cursor number
  ** to use for scanning indexes (iIdxCur). No index cursor is opened at
  ** this time though.  */
  sqlite3_جدول_قلف(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = MAX(pParse->nTab, iTab);
  sqlite3_Open_جدول(pParse, iTabCur, iDb, pTab, OP_OpenRead);
  sqlite3_Vdbe_بارول_String(v, regTabname, pTab->zName);

  لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
    صحيح n_ستون;                     /* Number of columns in pIdx. "N" */
    صحيح addr_Goto_پای_ټکی;               /* Address of "OP_Rewind iIdxCur" */
    صحيح addr_بل_لیکه;              /* Address of "next_row:" */
    ثابت حرف *zIdxName;         /* Name of the index */
    صحيح n_ستون_Test;                 /* Number of columns to test for changes */

    إذا( pOnlyIdx && pOnlyIdx!=pIdx ) استمر;
    إذا( pIdx->pPartIdxWhere==0 ) need_جدول_Cnt = 0;
    إذا( !HasRowid(pTab) && Is_Primary_کلی_شاخص(pIdx) ){
      n_ستون = pIdx->n_کلی_ستون;
      zIdxName = pTab->zName;
      n_ستون_Test = n_ستون - 1;
    }وإلا{
      n_ستون = pIdx->n_ستنه;
      zIdxName = pIdx->zName;
      n_ستون_Test = pIdx->uniqNotNull ? pIdx->n_کلی_ستون-1 : n_ستون-1;
    }

    /* Populate the register containing the index name. */
    sqlite3_Vdbe_بارول_String(v, regIdxname, zIdxName);
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
    sqlite3_Touch_ثبتوونکی(pParse, regPrev+n_ستون_Test);

    /* Open a read-only cursor on the index being analyzed. */
    أكد( iDb==sqlite3_Schema_منزل_شاخص(db, pIdx->pSchema) );
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_سېټ_P4_کلی_Info(pParse, pIdx);
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
    أكد( regTemp2==regStat+4 );
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, db->n_Analysis_حد, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, n_ستون, regStat+1);
    أكد( regRowid==regStat+2 );
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, pIdx->n_کلی_ستون, regRowid);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_شمېر, iIdxCur, reg_لنډمهاله,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3_Vdbe_جمع_کړه_Function_Call(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addr_Goto_پای_ټکی = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, regChng);
    addr_بل_لیکه = sqlite3VdbeCurrentAddr(v);

    إذا( n_ستون_Test>0 ){
      صحيح پای_ټکی_Distinct_Test = sqlite3VdbeMakeLabel(pParse);
      صحيح *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, حجم(صحيح)*n_ستون_Test);
      إذا( aGotoChng==0 ) استمر;

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
      sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Goto);
      addr_بل_لیکه = sqlite3VdbeCurrentAddr(v);
      إذا( n_ستون_Test==1 && pIdx->n_کلی_ستون==1 && Is_Unique_شاخص(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NotNull, regPrev, پای_ټکی_Distinct_Test);
        VdbeCoverage(v);
      }
      لكل(i=0; i<n_ستون_Test; i++){
        حرف *pColl = (حرف*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, i, regChng);
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, iIdxCur, i, reg_لنډمهاله);
        analyze_Vdbe_Comment_شاخص_With_ستنه_Name(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Ne, reg_لنډمهاله, 0, regPrev+i, pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, n_ستون_Test, regChng);
      sqlite3VdbeGoto(v, پای_ټکی_Distinct_Test);
  
  
      /*
      **  chng_addr_0:
      **   regPrev(0) = idx(0)
      **  chng_addr_1:
      **   regPrev(1) = idx(1)
      **  ...
      */
      sqlite3_Vdbe_Jump_دلته(v, addr_بل_لیکه-1);
      لكل(i=0; i<n_ستون_Test; i++){
        sqlite3_Vdbe_Jump_دلته(v, aGotoChng[i]);
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, iIdxCur, i, regPrev+i);
        analyze_Vdbe_Comment_شاخص_With_ستنه_Name(v,pIdx,i);
      }
      sqlite3VdbeResolveLabel(v, پای_ټکی_Distinct_Test);
      sqlite3DbFree(db, aGotoChng);
    }
  
    /*
    **  chng_addr_N:
    **   regRowid = idx(rowid)            // STAT4 only
    **   stat_push(P, regChng, regRowid)  // 3rd parameter STAT4 only
    **   Next csr
    **   if !eof(csr) goto next_row;
    */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    إذا( OptimizationEnabled(db, SQLITE_Stat4) ){
      أكد( regRowid==(regStat+2) );
      إذا( HasRowid(pTab) ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IdxRowid, iIdxCur, regRowid);
      }وإلا{
        شاخص *pPk = sqlite3_Primary_کلی_شاخص(pIdx->p_جدول);
        صحيح j, k, reg_کلی;
        reg_کلی = sqlite3_Get_لنډمهاله_Range(pParse, pPk->n_کلی_ستون);
        لكل(j=0; j<pPk->n_کلی_ستون; j++){
          k = sqlite3_جدول_ستنه_منزل_شاخص(pIdx, pPk->ai_ستنه[j]);
          أكد( k>=0 && k<pIdx->n_ستنه );
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, iIdxCur, k, reg_کلی+j);
          analyze_Vdbe_Comment_شاخص_With_ستنه_Name(v,pIdx,k);
        }
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_کلی, pPk->n_کلی_ستون, regRowid);
        sqlite3_Release_لنډمهاله_Range(pParse, reg_کلی, pPk->n_کلی_ستون);
      }
    }
#د_که_پای
    أكد( regChng==(regStat+1) );
    {
      sqlite3_Vdbe_جمع_کړه_Function_Call(pParse, 1, regStat, reg_لنډمهاله, 2+IsStat4,
                                 &stat_ټېل_Funcdef, 0);
      إذا( db->n_Analysis_حد ){
        صحيح j1, j2, j3;
        j1 = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IsNull, reg_لنډمهاله); VdbeCoverage(v);
        j2 = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_If, reg_لنډمهاله); VdbeCoverage(v);
        j3 = sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_SeekGT, iIdxCur, 0, regPrev, 1);
        VdbeCoverage(v);
        sqlite3_Vdbe_Jump_دلته(v, j1);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, iIdxCur, addr_بل_لیکه); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_دلته(v, j2);
        sqlite3_Vdbe_Jump_دلته(v, j3);
      }وإلا{
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, iIdxCur, addr_بل_لیکه); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    إذا( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3_Vdbe_Jump_دلته(v, addr_Goto_پای_ټکی);
      addr_Goto_پای_ټکی = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    أكد( "BBB"[0]==SQLITE_AFF_متن );
    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Make_ریکارډ, regTabname, 3, reg_لنډمهاله, "BBB", 0);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iStatCur, reg_لنډمهاله, regNewRowid);
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (حرف*)pStat1, P4_جدول);
#د_که_پای
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    إذا( OptimizationEnabled(db, SQLITE_Stat4) && db->n_Analysis_حد==0 ){
      صحيح regEq = regStat1;
      صحيح regLt = regStat1+1;
      صحيح regDLt = regStat1+2;
      صحيح regSample = regStat1+3;
      صحيح reg_ستون = regStat1+4;
      صحيح regSampleRowid = reg_ستون + n_ستون;
      صحيح addr_بل;
      صحيح addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_Not_وموندل;

      /* No STAT4 data is generated if the number of rows is zero */
      إذا( addr_Goto_پای_ټکی==0 ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addr_Goto_پای_ټکی = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      إذا( doOnce ){
        صحيح mx_ستون = n_ستون;
        شاخص *pX;

        /* Compute the maximum number of columns in any index */
        لكل(pX=pTab->p_شاخص; pX; pX=pX->p_بل){
          صحيح n_ستون_X;                     /* Number of columns in pX */
          إذا( !HasRowid(pTab) && Is_Primary_کلی_شاخص(pX) ){
            n_ستون_X = pX->n_کلی_ستون;
          }وإلا{
            n_ستون_X = pX->n_ستنه;
          }
          إذا( n_ستون_X>mx_ستون ) mx_ستون = n_ستون_X;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3_Touch_ثبتوونکی(pParse, reg_ستون+mx_ستون);
        doOnce = 0;
#که_تعریف_شوی SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3_No_Temps_دننه_Range(pParse, regEq, reg_ستون+mx_ستون) );
#د_که_پای
        sqlite3_Clear_لنډمهاله_Reg_Cache(pParse);  /* tag-20230325-1 */
        أكد( sqlite3_No_Temps_دننه_Range(pParse, regEq, reg_ستون+mx_ستون) );
      }
      أكد( sqlite3_No_Temps_دننه_Range(pParse, regEq, reg_ستون+n_ستون) );

      addr_بل = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, seekOp, iTabCur, addr_بل, regSampleRowid, 0);
      VdbeCoverage(v);
      لكل(i=0; i<n_ستون; i++){
        sqlite3_بیان_کوډ_بارول_شاخص_ستنه(pParse, pIdx, iTabCur, i, reg_ستون+i);
      }
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_ستون, n_ستون, regSample);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, regTabname, 6, reg_لنډمهاله);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iStatCur+1, reg_لنډمهاله, regNewRowid);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Goto, 1, addr_بل); /* P1==1 for end-of-loop */
      sqlite3_Vdbe_Jump_دلته(v, addrIsNull);
    }
#د_که_پای /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    إذا( addr_Goto_پای_ټکی ) sqlite3_Vdbe_Jump_دلته(v, addr_Goto_پای_ټکی);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  إذا( pOnlyIdx==0 && need_جدول_Cnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_شمېر, iTabCur, regStat1);
    j_Zero_لیکې = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Null, 0, regIdxname);
    أكد( "BBB"[0]==SQLITE_AFF_متن );
    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Make_ریکارډ, regTabname, 3, reg_لنډمهاله, "BBB", 0);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iStatCur, reg_لنډمهاله, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (حرف*)pStat1, P4_جدول);
#د_که_پای
    sqlite3_Vdbe_Jump_دلته(v, j_Zero_لیکې);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
ساكن فراغ بارول_Analysis(Parse *pParse, صحيح iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  إذا( v ){
    sqlite3_Vdbe_جمع_کړه_Op1(v, OP_بارول_Analysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
ساكن فراغ analyzeDatabase(Parse *pParse, صحيح iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  صحيح iStatCur;
  صحيح iMem;
  صحيح iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  open_Stat_جدول(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  لكل(k=sqlite_Hash_لومړی(&pSchema->tblHash); k; k=sqlite_Hash_بل(k)){
    جدول *pTab = (جدول*)sqlite_Hash_معلومات(k);
    analyze_One_جدول(pParse, pTab, 0, iStatCur, iMem, iTab);
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    iMem = sqlite3_لومړی_شته_ثبتوونکی(pParse, iMem);
#وإلا
    أكد( iMem==sqlite3_لومړی_شته_ثبتوونکی(pParse,iMem) );
#د_که_پای
  }
  بارول_Analysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
ساكن فراغ analyze_جدول(Parse *pParse, جدول *pTab, شاخص *pOnlyIdx){
  صحيح iDb;
  صحيح iStatCur;

  أكد( pTab!=0 );
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3_Schema_منزل_شاخص(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  إذا( pOnlyIdx ){
    open_Stat_جدول(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }وإلا{
    open_Stat_جدول(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyze_One_جدول(pParse, pTab, pOnlyIdx, iStatCur,pParse->nMem+1,pParse->nTab);
  بارول_Analysis(pParse, iDb);
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
فراغ sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  صحيح iDb;
  صحيح i;
  حرف *z, *zDb;
  جدول *pTab;
  شاخص *pIdx;
  Token *p_جدول_Name;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  إذا( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    أرجع;
  }

  أكد( pName2!=0 || pName1==0 );
  إذا( pName1==0 ){
    /* Form 1:  Analyze everything */
    لكل(i=0; i<db->nDb; i++){
      إذا( i==1 ) استمر;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }وإلا إذا( pName2->n==0 && (iDb = sqlite3_پیدا_کړه_Db(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }وإلا{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &p_جدول_Name);
    إذا( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, p_جدول_Name);
      إذا( z ){
        إذا( (pIdx = sqlite3_پیدا_کړه_شاخص(db, z, zDb))!=0 ){
          analyze_جدول(pParse, pIdx->p_جدول, pIdx);
        }وإلا إذا( (pTab = sqlite3_Locate_جدول(pParse, 0, z, zDb))!=0 ){
          analyze_جدول(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  إذا( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
تعريفنوع بنية analysisInfo analysisInfo;
بنية analysisInfo {
  sqlite3 *db;
  ثابت حرف *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
ساكن فراغ decodeIntArray(
  حرف *zIntArray,       /* String containing int array to decode */
  صحيح n_بهر,              /* Number of slots in aOut[] */
  tRowcnt *a_بهر,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  شاخص *p_شاخص          /* Handle extra flags for this index, if not NULL */
){
  حرف *z = zIntArray;
  صحيح c;
  صحيح i;
  tRowcnt v;

#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( z==0 ) z = "";
#وإلا
  أكد( z!=0 );
#د_که_پای
  لكل(i=0; *z && i<n_بهر; i++){
    v = 0;
    طالما( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    إذا( a_بهر ) a_بهر[i] = v;
    إذا( aLog ) aLog[i] = sqlite3LogEst(v);
#وإلا
    أكد( a_بهر==0 );
    ناکارول_شوی_PARAMETER(a_بهر);
    أكد( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#د_که_پای
    إذا( *z==' ' ) z++;
  }
#که_نه_تعریف_شوی SQLITE_ENABLE_STAT4
  أكد( p_شاخص!=0 ); {
#وإلا
  إذا( p_شاخص ){
#د_که_پای
    p_شاخص->bUnordered = 0;
    p_شاخص->no_Skip_لټون = 0;
    طالما( z[0] ){
      إذا( sqlite3_strglob("unordered*", z)==0 ){
        p_شاخص->bUnordered = 1;
      }وإلا إذا( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        صحيح sz = sqlite3Atoi(z+3);
        إذا( sz<2 ) sz = 2;
        p_شاخص->sz_Idx_لیکه = sqlite3LogEst(sz);
      }وإلا إذا( sqlite3_strglob("noskipscan*", z)==0 ){
        p_شاخص->no_Skip_لټون = 1;
      }
#که_تعریف_شوی SQLITE_ENABLE_COSTMULT
      وإلا إذا( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        p_شاخص->p_جدول->لګښت_Mult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#د_که_پای
      طالما( z[0]!=0 && z[0]!=' ' ) z++;
      طالما( z[0]==' ' ) z++;
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
ساكن صحيح analysisLoader(فراغ *p_معلومات, صحيح argc, حرف **argv, حرف **Not_کارول_شوی){
  analysisInfo *pInfo = (analysisInfo*)p_معلومات;
  شاخص *p_شاخص;
  جدول *p_جدول;
  ثابت حرف *z;

  أكد( argc==3 );
  ناکارول_شوی_PARAMETER2(Not_کارول_شوی, argc);

  إذا( argv==0 || argv[0]==0 || argv[2]==0 ){
    أرجع 0;
  }
  p_جدول = sqlite3_پیدا_کړه_جدول(pInfo->db, argv[0], pInfo->zDatabase);
  إذا( p_جدول==0 ){
    أرجع 0;
  }
  إذا( argv[1]==0 ){
    p_شاخص = 0;
  }وإلا إذا( sqlite3_stricmp(argv[0],argv[1])==0 ){
    p_شاخص = sqlite3_Primary_کلی_شاخص(p_جدول);
  }وإلا{
    p_شاخص = sqlite3_پیدا_کړه_شاخص(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  إذا( p_شاخص ){
    tRowcnt *ai_لیکه_Est = 0;
    صحيح n_ستون = p_شاخص->n_کلی_ستون+1;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    إذا( p_شاخص->ai_لیکه_Est==0 ){
      p_شاخص->ai_لیکه_Est = (tRowcnt*)sqlite3MallocZero(حجم(tRowcnt) * n_ستون);
      إذا( p_شاخص->ai_لیکه_Est==0 ) sqlite3OomFault(pInfo->db);
    }
    ai_لیکه_Est = p_شاخص->ai_لیکه_Est;
#د_که_پای
    p_شاخص->bUnordered = 0;
    decodeIntArray((حرف*)z, n_ستون, ai_لیکه_Est, p_شاخص->ai_لیکه_Log_Est, p_شاخص);
    p_شاخص->hasStat1 = 1;
    إذا( p_شاخص->pPartIdxWhere==0 ){
      p_جدول->n_لیکه_Log_Est = p_شاخص->ai_لیکه_Log_Est[0];
      p_جدول->tabFlags |= TF_HasStat1;
    }
  }وإلا{
    شاخص fakeIdx;
    fakeIdx.sz_Idx_لیکه = p_جدول->sz_Tab_لیکه;
#که_تعریف_شوی SQLITE_ENABLE_COSTMULT
    fakeIdx.p_جدول = p_جدول;
#د_که_پای
    decodeIntArray((حرف*)z, 1, 0, &p_جدول->n_لیکه_Log_Est, &fakeIdx);
    p_جدول->sz_Tab_لیکه = fakeIdx.sz_Idx_لیکه;
    p_جدول->tabFlags |= TF_HasStat1;
  }

  أرجع 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
فراغ sqlite3_Delete_شاخص_Samples(sqlite3 *db, شاخص *pIdx){
  أكد( db!=0 );
  أكد( pIdx!=0 );
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( pIdx->aSample ){
    صحيح j;
    لكل(j=0; j<pIdx->nSample; j++){
      شاخص_Sample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  إذا( db->pn_بایټونه_Freed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#وإلا
  ناکارول_شوی_PARAMETER(db);
  ناکارول_شوی_PARAMETER(pIdx);
#د_که_پای /* SQLITE_ENABLE_STAT4 */
}

#که_تعریف_شوی SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
ساكن فراغ initAvgEq(شاخص *pIdx){
  إذا( pIdx ){
    شاخص_Sample *aSample = pIdx->aSample;
    شاخص_Sample *pFinal = &aSample[pIdx->nSample-1];
    صحيح i_ستون;
    صحيح n_ستون = 1;
    إذا( pIdx->n_Sample_ستون>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      n_ستون = pIdx->n_Sample_ستون-1;
      pIdx->aAvgEq[n_ستون] = 1;
    }
    لكل(i_ستون=0; i_ستون<n_ستون; i_ستون++){
      صحيح nSample = pIdx->nSample;
      صحيح i;                    /* Used to iterate through samples */
      tRowcnt مجموعه_Eq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt n_لیکه;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      إذا( !pIdx->ai_لیکه_Est || i_ستون>=pIdx->n_کلی_ستون || pIdx->ai_لیکه_Est[i_ستون+1]==0 ){
        n_لیکه = pFinal->anLt[i_ستون];
        nDist100 = (i64)100 * pFinal->anDLt[i_ستون];
        nSample--;
      }وإلا{
        n_لیکه = pIdx->ai_لیکه_Est[0];
        nDist100 = ((i64)100 * pIdx->ai_لیکه_Est[0]) / pIdx->ai_لیکه_Est[i_ستون+1];
      }
      pIdx->n_لیکه_Est0 = n_لیکه;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      لكل(i=0; i<nSample; i++){
        إذا( i==(pIdx->nSample-1)
         || aSample[i].anDLt[i_ستون]!=aSample[i+1].anDLt[i_ستون] 
        ){
          مجموعه_Eq += aSample[i].anEq[i_ستون];
          nSum100 += 100;
        }
      }

      إذا( nDist100>nSum100 && مجموعه_Eq<n_لیکه ){
        avgEq = ((i64)100 * (n_لیکه - مجموعه_Eq))/(nDist100 - nSum100);
      }
      إذا( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[i_ستون] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
ساكن شاخص *پیدا_کړه_شاخص_Or_Primary_کلی(
  sqlite3 *db,
  ثابت حرف *zName,
  ثابت حرف *zDb
){
  شاخص *pIdx = sqlite3_پیدا_کړه_شاخص(db, zName, zDb);
  إذا( pIdx==0 ){
    جدول *pTab = sqlite3_پیدا_کړه_جدول(db, zName, zDb);
    إذا( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_کلی_شاخص(pTab);
  }
  أرجع pIdx;
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
ساكن صحيح بارول_Stat_Tbl(
  sqlite3 *db,                  /* Database handle */
  ثابت حرف *zSql1,            /* SQL statement 1 (see above) */
  ثابت حرف *zSql2,            /* SQL statement 2 (see above) */
  ثابت حرف *zDb               /* Database name (e.g. "main") */
){
  صحيح rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  حرف *zSql;                   /* Text of the SQL statement */
  شاخص *pPrevIdx = 0;          /* Previous index in the loop */
  شاخص_Sample *pSample;         /* A slot in pIdx->aSample[] */

  أكد( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  إذا( !zSql ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  إذا( rc ) أرجع rc;

  طالما( sqlite3_ګام(pStmt)==SQLITE_لیکه ){
    صحيح n_Idx_ستون = 1;              /* Number of columns in stat4 records */

    حرف *z_شاخص;    /* Index name */
    شاخص *pIdx;     /* Pointer to the index object */
    صحيح nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    z_شاخص = (حرف *)sqlite3_ستنه_متن(pStmt, 0);
    إذا( z_شاخص==0 ) استمر;
    nSample = sqlite3_ستنه_int(pStmt, 1);
    pIdx = پیدا_کړه_شاخص_Or_Primary_کلی(db, z_شاخص, zDb);
    أكد( pIdx==0 || pIdx->nSample==0 );
    إذا( pIdx==0 ) استمر;
    إذا( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      استمر;
    }
    أكد( !HasRowid(pIdx->p_جدول) || pIdx->n_ستنه==pIdx->n_کلی_ستون+1 );
    إذا( !HasRowid(pIdx->p_جدول) && Is_Primary_کلی_شاخص(pIdx) ){
      n_Idx_ستون = pIdx->n_کلی_ستون;
    }وإلا{
      n_Idx_ستون = pIdx->n_ستنه;
    }
    pIdx->n_Sample_ستون = n_Idx_ستون;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(شاخص_Sample) * nSample);
    nByte += sizeof64(tRowcnt) * n_Idx_ستون * 3 * nSample;
    nByte += n_Idx_ستون * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    إذا( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      أرجع SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    أكد( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += n_Idx_ستون;
    pIdx->p_جدول->tabFlags |= TF_HasStat4;
    لكل(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += n_Idx_ستون;
      pIdx->aSample[i].anLt = pSpace; pSpace += n_Idx_ستون;
      pIdx->aSample[i].anDLt = pSpace; pSpace += n_Idx_ستون;
    }
    أكد( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  إذا( rc ) أرجع rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  إذا( !zSql ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  إذا( rc ) أرجع rc;

  طالما( sqlite3_ګام(pStmt)==SQLITE_لیکه ){
    حرف *z_شاخص;                 /* Index name */
    شاخص *pIdx;                  /* Pointer to the index object */
    صحيح n_ستون = 1;                 /* Number of columns in index */

    z_شاخص = (حرف *)sqlite3_ستنه_متن(pStmt, 0);
    إذا( z_شاخص==0 ) استمر;
    pIdx = پیدا_کړه_شاخص_Or_Primary_کلی(db, z_شاخص, zDb);
    إذا( pIdx==0 ) استمر;
    إذا( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      استمر;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    n_ستون = pIdx->n_Sample_ستون;
    إذا( pIdx!=pPrevIdx ){
      initAvgEq(pPrevIdx);
      pPrevIdx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((حرف*)sqlite3_ستنه_متن(pStmt,1),n_ستون,pSample->anEq,0,0);
    decodeIntArray((حرف*)sqlite3_ستنه_متن(pStmt,2),n_ستون,pSample->anLt,0,0);
    decodeIntArray((حرف*)sqlite3_ستنه_متن(pStmt,3),n_ستون,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_ستنه_بایټونه(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    إذا( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      أرجع SQLITE_NOMEM_BKPT;
    }
    إذا( pSample->n ){
      حافظه_نقل(pSample->p, sqlite3_ستنه_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  إذا( rc==SQLITE_OK ) initAvgEq(pPrevIdx);
  أرجع rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
ساكن صحيح بارول_Stat4(sqlite3 *db, ثابت حرف *zDb){
  صحيح rc = SQLITE_OK;             /* Result codes from subroutines */
  ثابت جدول *pStat4;

  أكد( db->lookaside.bDisable );
  إذا( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3_پیدا_کړه_جدول(db, "sqlite_stat4", zDb))!=0
   && Is_Ordinary_جدول(pStat4)
  ){
    rc = بارول_Stat_Tbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  أرجع rc;
}
#د_که_پای /* SQLITE_ENABLE_STAT4 */

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
صحيح sqlite3_Analysis_بارول(sqlite3 *db, صحيح iDb){
  analysisInfo sInfo;
  HashElem *i;
  حرف *zSql;
  صحيح rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  ثابت جدول *pStat1;

  أكد( iDb>=0 && iDb<db->nDb );
  أكد( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  لكل(i=sqlite_Hash_لومړی(&pSchema->tblHash); i; i=sqlite_Hash_بل(i)){
    جدول *pTab = sqlite_Hash_معلومات(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  لكل(i=sqlite_Hash_لومړی(&pSchema->idxHash); i; i=sqlite_Hash_بل(i)){
    شاخص *pIdx = sqlite_Hash_معلومات(i);
    pIdx->hasStat1 = 0;
#که_تعریف_شوی SQLITE_ENABLE_STAT4
    sqlite3_Delete_شاخص_Samples(db, pIdx);
    pIdx->aSample = 0;
#د_که_پای
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  إذا( (pStat1 = sqlite3_پیدا_کړه_جدول(db, "sqlite_stat1", sInfo.zDatabase))
   && Is_Ordinary_جدول(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    إذا( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }وإلا{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  لكل(i=sqlite_Hash_لومړی(&pSchema->idxHash); i; i=sqlite_Hash_بل(i)){
    شاخص *pIdx = sqlite_Hash_معلومات(i);
    إذا( !pIdx->hasStat1 ) sqlite3_Default_لیکه_Est(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  إذا( rc==SQLITE_OK ){
    DisableLookaside;
    rc = بارول_Stat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  لكل(i=sqlite_Hash_لومړی(&pSchema->idxHash); i; i=sqlite_Hash_بل(i)){
    شاخص *pIdx = sqlite_Hash_معلومات(i);
    sqlite3_free(pIdx->ai_لیکه_Est);
    pIdx->ai_لیکه_Est = 0;
  }
#د_که_پای

  إذا( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  أرجع rc;
}


#د_که_پای /* SQLITE_OMIT_ANALYZE */
