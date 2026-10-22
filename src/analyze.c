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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_ANALYZE
#شامل "sqliteInt.h"

#اگر defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#ورنہ
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#اگر_ختم

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
ساکن خالی open_Stat_جدول(
  Parse *pParse,          /* Parsing context */
  صحیح iDb,                /* The database we are looking in */
  صحیح iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  ثابت حرف *zWhere,     /* Delete entries for this table or index */
  ثابت حرف *zWhereType  /* Either "tbl" or "idx" */
){
  ساکن ثابت ساخت {
    ثابت حرف *zName;
    ثابت حرف *z_ستون_تعداد;
  } a_جدول[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#اگر defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#ورنہ
    { "sqlite_stat4", 0 },
#اگر_ختم
    { "sqlite_stat3", 0 },
  };
  صحیح i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_جڑ[Array_سائز(a_جدول)];
  u8 aCreateTbl[Array_سائز(a_جدول)];
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  ثابت صحیح n_مقصد_Open = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#ورنہ
  ثابت صحیح n_مقصد_Open = 1;
#اگر_ختم

  اگر( v==0 ) واپس;
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  أكد( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  برائے(i=0; i<Array_سائز(a_جدول); i++){
    ثابت حرف *zTab = a_جدول[i].zName;
    جدول *pStat;
    aCreateTbl[i] = 0;
    اگر( (pStat = sqlite3_ڈھونڈو_جدول(db, zTab, pDb->zDbSName))==0 ){
      اگر( i<n_مقصد_Open ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, a_جدول[i].z_ستون_تعداد
        );
        أكد( pParse->isCreate || pParse->nErr );
        a_جڑ[i] = (u32)pParse->u1.cr.reg_جڑ;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }ورنہ{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_جڑ[i] = pStat->tnum;
      sqlite3_جدول_تالا(pParse, iDb, a_جڑ[i], 1, zTab);
      اگر( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
      }ورنہ اگر( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#اگر_ختم
      }ورنہ{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Clear, (صحیح)a_جڑ[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  برائے(i=0; i<n_مقصد_Open; i++){
    أكد( i<Array_سائز(a_جدول) );
    sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_OpenWrite, iStatCur+i, (صحیح)a_جڑ[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, a_جدول[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#اگر_غیر_تعریف_شدہ SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#اگر_ختم

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
قسمتعریف ساخت StatAccum StatAccum;
قسمتعریف ساخت StatSample StatSample;
ساخت StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  اتحاد {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  صحیح i_ستون;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#اگر_ختم
};                                                    
ساخت StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt n_صف;             /* Number of rows visited so far */
  صحیح n_حد_بندی;               /* Analysis row-scan limit */
  صحیح n_ستون;                 /* Number of columns in index + pk/rowid */
  صحیح n_کلید_ستون;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  صحیح mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *a_بہترین;        /* Array of nCol best samples */
  صحیح iMin;                 /* Index in a[] of entry with minimum score */
  صحیح nSample;              /* Current number of samples */
  صحیح nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  صحیح iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#اگر_ختم
};

/* Reclaim memory used by a StatSample
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
ساکن خالی sampleClear(sqlite3 *db, StatSample *p){
  أكد( db!=0 );
  اگر( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#اگر_ختم

/* Initialize the BLOB value of a ROWID
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
ساکن خالی sample_سیٹ_Rowid(sqlite3 *db, StatSample *p, صحیح n, ثابت u8 *p_ڈیٹا){
  أكد( db!=0 );
  اگر( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  اگر( p->u.aRowid ){
    p->nRowid = n;
    حافظہ_نقل(p->u.aRowid, p_ڈیٹا, n);
  }ورنہ{
    p->nRowid = 0;
  }
}
#اگر_ختم

/* Initialize the INTEGER value of a ROWID.
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
ساکن خالی sample_سیٹ_Rowid_Int64(sqlite3 *db, StatSample *p, i64 iRowid){
  أكد( db!=0 );
  اگر( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#اگر_ختم


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
ساکن خالی sampleCopy(StatAccum *p, StatSample *p_مقصد, StatSample *pFrom){
  p_مقصد->isPSample = pFrom->isPSample;
  p_مقصد->i_ستون = pFrom->i_ستون;
  p_مقصد->iHash = pFrom->iHash;
  حافظہ_نقل(p_مقصد->anEq, pFrom->anEq, حجم(tRowcnt)*p->n_ستون);
  حافظہ_نقل(p_مقصد->anLt, pFrom->anLt, حجم(tRowcnt)*p->n_ستون);
  حافظہ_نقل(p_مقصد->anDLt, pFrom->anDLt, حجم(tRowcnt)*p->n_ستون);
  اگر( pFrom->nRowid ){
    sample_سیٹ_Rowid(p->db, p_مقصد, pFrom->nRowid, pFrom->u.aRowid);
  }ورنہ{
    sample_سیٹ_Rowid_Int64(p->db, p_مقصد, pFrom->u.iRowid);
  }
}
#اگر_ختم

/*
** Reclaim all memory of a StatAccum structure.
*/
ساکن خالی statAccumDestructor(خالی *pOld){
  StatAccum *p = (StatAccum*)pOld;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( p->mxSample ){
    صحیح i;
    برائے(i=0; i<p->n_ستون; i++) sampleClear(p->db, p->a_بہترین+i);
    برائے(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#اگر_ختم
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
ساکن خالی statInit(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  StatAccum *p;
  صحیح n_ستون;                       /* Number of columns in index being sampled */
  صحیح n_کلید_ستون;                    /* Number of key columns */
  صحیح n_ستون_اوپر;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  صحیح mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#اگر_ختم

  /* Decode the three function arguments */
  غیر_مستعمل_PARAMETER(argc);
  n_ستون = sqlite3_قدر_int(argv[0]);
  أكد( n_ستون>0 );
  n_ستون_اوپر = حجم(tRowcnt)<8 ? (n_ستون+1)&~1 : n_ستون;
  n_کلید_ستون = sqlite3_قدر_int(argv[1]);
  أكد( n_کلید_ستون<=n_ستون );
  أكد( n_کلید_ستون>0 );

  /* Allocate the space required for the StatAccum object */
  n = حجم(*p) 
    + حجم(tRowcnt)*n_ستون_اوپر;                    /* StatAccum.anDLt */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  n += حجم(tRowcnt)*n_ستون_اوپر;                   /* StatAccum.anEq */
  اگر( mxSample ){
    n += حجم(tRowcnt)*n_ستون_اوپر                  /* StatAccum.anLt */
      + حجم(StatSample)*(n_ستون+mxSample)       /* StatAccum.aBest[], a[] */
      + حجم(tRowcnt)*3*n_ستون_اوپر*(n_ستون+mxSample);
  }
#اگر_ختم
  p = sqlite3DbMallocZero(db, n);
  اگر( p==0 ){
    sqlite3_نتیجہ_error_nomem(context);
    واپس;
  }

  p->db = db;
  p->nEst = sqlite3_قدر_int64(argv[2]);
  p->n_صف = 0;
  p->n_حد_بندی = sqlite3_قدر_int(argv[3]);
  p->n_ستون = n_ستون;
  p->n_کلید_ستون = n_کلید_ستون;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[n_ستون_اوپر];
  p->mxSample = p->n_حد_بندی==0 ? mxSample : 0;
  اگر( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    صحیح i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[n_ستون_اوپر];
    p->iPrn = 0x689e962d*(u32)n_ستون ^ 0xd0944565*(u32)sqlite3_قدر_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (ساخت StatSample*)&p->current.anLt[n_ستون_اوپر];
    p->a_بہترین = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+n_ستون]);
    برائے(i=0; i<(mxSample+n_ستون); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (حجم(tRowcnt) * n_ستون_اوپر);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (حجم(tRowcnt) * n_ستون_اوپر);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (حجم(tRowcnt) * n_ستون_اوپر);
    }
    أكد( (pSpace - (u8*)p)==n );
  
    برائے(i=0; i<n_ستون; i++){
      p->a_بہترین[i].i_ستون = i;
    }
  }
#اگر_ختم

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_نتیجہ_blob(context, p, حجم(*p), statAccumDestructor);
}
ساکن ثابت FuncDef statInitFuncdef = {
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

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
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
ساکن صحیح sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  صحیح n_ستون = pAccum->n_ستون;
  صحیح i;
  أكد( pNew->i_ستون==pOld->i_ستون );
  برائے(i=pNew->i_ستون+1; i<n_ستون; i++){
    اگر( pNew->anEq[i]>pOld->anEq[i] ) واپس 1;
    اگر( pNew->anEq[i]<pOld->anEq[i] ) واپس 0;
  }
  اگر( pNew->iHash>pOld->iHash ) واپس 1;
  واپس 0;
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
ساکن صحیح sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->i_ستون];
  tRowcnt nEqOld = pOld->anEq[pOld->i_ستون];

  أكد( pOld->isPSample==0 && pNew->isPSample==0 );
  أكد( IsStat4 || (pNew->i_ستون==0 && pOld->i_ستون==0) );

  اگر( (nEqNew>nEqOld) ) واپس 1;
  اگر( nEqNew==nEqOld ){
    اگر( pNew->i_ستون<pOld->i_ستون ) واپس 1;
    واپس (pNew->i_ستون==pOld->i_ستون && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  واپس 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
ساکن خالی sample_داخل_کرو(StatAccum *p, StatSample *pNew, صحیح nEqZero){
  StatSample *pSample = 0;
  صحیح i;

  أكد( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  اگر( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  اگر( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    أكد( pNew->anEq[pNew->i_ستون]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    برائے(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      اگر( pOld->anEq[pNew->i_ستون]==0 ){
        اگر( pOld->isPSample ) واپس;
        أكد( pOld->i_ستون>pNew->i_ستون );
        أكد( sampleIsBetter(p, pNew, pOld) );
        اگر( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    اگر( pUpgrade ){
      pUpgrade->i_ستون = pNew->i_ستون;
      pUpgrade->anEq[pUpgrade->i_ستون] = pNew->anEq[pUpgrade->i_ستون];
      جاؤ ڈھونڈو_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  اگر( p->nSample>=p->mxSample ){
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

ڈھونڈو_new_min:
  اگر( p->nSample>=p->mxSample ){
    صحیح iMin = -1;
    برائے(i=0; i<p->mxSample; i++){
      اگر( p->a[i].isPSample ) جاری;
      اگر( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    أكد( iMin>=0 );
    p->iMin = iMin;
  }
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
ساکن خالی sample_ڈالو_پچھلا(StatAccum *p, صحیح iChng){
  صحیح i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  برائے(i=(p->n_ستون-2); i>=iChng; i--){
    StatSample *p_بہترین = &p->a_بہترین[i];
    p_بہترین->anEq[i] = p->current.anEq[i];
    اگر( p->nSample<p->mxSample || sampleIsBetter(p, p_بہترین, &p->a[p->iMin]) ){
      sample_داخل_کرو(p, p_بہترین, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  برائے(i=p->nSample-1; i>=0; i--){
    صحیح j;
    برائے(j=p->nMaxEqZero; j<p->n_ستون; j++) أكد( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  اگر( iChng<p->nMaxEqZero ){
    برائے(i=p->nSample-1; i>=0; i--){
      صحیح j;
      برائے(j=iChng; j<p->n_ستون; j++){
        اگر( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

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
ساکن خالی stat_ڈالو(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  صحیح i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_قدر_blob(argv[0]);
  صحیح iChng = sqlite3_قدر_int(argv[1]);

  غیر_مستعمل_PARAMETER( argc );
  غیر_مستعمل_PARAMETER( context );
  أكد( p->n_ستون>0 );
  أكد( iChng<p->n_ستون );

  اگر( p->n_صف==0 ){
    /* This is the first call to this function. Do initialization. */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    برائے(i=0; i<p->n_ستون; i++) p->current.anEq[i] = 1;
#اگر_ختم
  }ورنہ{
    /* Second and subsequent calls get processed here */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    اگر( p->mxSample ) sample_ڈالو_پچھلا(p, iChng);
#اگر_ختم

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    برائے(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#اگر_ختم
    برائے(i=iChng; i<p->n_ستون; i++){
      p->current.anDLt[i]++;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
      اگر( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#اگر_ختم
    }
  }

  p->n_صف++;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( p->mxSample ){
    tRowcnt nLt;
    اگر( sqlite3_قدر_type(argv[2])==SQLITE_INTEGER ){
      sample_سیٹ_Rowid_Int64(p->db, &p->current, sqlite3_قدر_int64(argv[2]));
    }ورنہ{
      sample_سیٹ_Rowid(p->db, &p->current, sqlite3_قدر_بائٹیں(argv[2]),
                                         sqlite3_قدر_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->n_ستون-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    اگر( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.i_ستون = 0;
      sample_داخل_کرو(p, &p->current, p->n_ستون-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    برائے(i=0; i<(p->n_ستون-1); i++){
      p->current.i_ستون = i;
      اگر( i>=iChng || sampleIsBetterPost(p, &p->current, &p->a_بہترین[i]) ){
        sampleCopy(p, &p->a_بہترین[i], &p->current);
      }
    }
  }ورنہ
#اگر_ختم
  اگر( p->n_حد_بندی && p->n_صف>(tRowcnt)p->n_حد_بندی*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_نتیجہ_int(context, p->current.anDLt[0]>0);
  }
}

ساکن ثابت FuncDef stat_ڈالو_Funcdef = {
  2+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  stat_ڈالو,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_push",     /* zName */
  {0}
};

#تعریف_کرو STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#تعریف_کرو STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#تعریف_کرو STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#تعریف_کرو STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#تعریف_کرو STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

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
ساکن خالی statGet(
  sqlite3_context *context,
  صحیح argc,
  sqlite3_قدر **argv
){
  StatAccum *p = (StatAccum*)sqlite3_قدر_blob(argv[0]);
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  صحیح eCall = sqlite3_قدر_int(argv[1]);
  أكد( argc==2 );
  أكد( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  أكد( eCall==STAT_GET_STAT1 || p->mxSample );
  اگر( eCall==STAT_GET_STAT1 )
#ورنہ
  أكد( argc==1 );
#اگر_ختم
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
    صحیح i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_کلید_ستون+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->n_صف);
    برائے(i=0; i<p->n_کلید_ستون; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->n_صف + nDistinct - 1) / nDistinct;
      اگر( iVal==2 && p->n_صف*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
      أكد( p->current.anEq[i] || p->n_صف==0 );
#اگر_ختم
    }
    sqlite3_نتیجہ_str(context, &sStat, SQLITE_XFER);
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  ورنہ اگر( eCall==STAT_GET_ROWID ){
    اگر( p->iGet<0 ){
      sample_ڈالو_پچھلا(p, 0);
      p->iGet = 0;
    }
    اگر( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      اگر( pS->nRowid==0 ){
        sqlite3_نتیجہ_int64(context, pS->u.iRowid);
      }ورنہ{
        sqlite3_نتیجہ_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }ورنہ{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    صحیح i;

    أكد( p->iGet<p->nSample );
    انتخاب( eCall ){
      حالت STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; توڑو;
      حالت STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; توڑو;
      طےشدہ: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        توڑو;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->n_ستون*100);
    برائے(i=0; i<p->n_ستون; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    اگر( sStat.nChar ) sStat.nChar--;
    sqlite3_نتیجہ_str(context, &sStat, SQLITE_XFER);
  }
#اگر_ختم /* SQLITE_ENABLE_STAT4 */
#اگر_غیر_تعریف_شدہ SQLITE_DEBUG
  غیر_مستعمل_PARAMETER( argc );
#اگر_ختم
}
ساکن ثابت FuncDef statGetFuncdef = {
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

ساکن خالی callStatGet(Parse *pParse, صحیح regStat, صحیح iParam, صحیح reg_برآمد){
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  sqlite3_Vdbe_جوڑو_Op2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#وگرنہ_اگر SQLITE_DEBUG
  أكد( iParam==STAT_GET_STAT1 );
#ورنہ
  غیر_مستعمل_PARAMETER( iParam );
#اگر_ختم
  أكد( reg_برآمد!=regStat && reg_برآمد!=regStat+1 );
  sqlite3_Vdbe_جوڑو_Function_Call(pParse, 0, regStat, reg_برآمد, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#اگر_تعریف_شدہ SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
ساکن خالی analyze_Vdbe_Comment_اشاریہ_With_عمود_Name(
  Vdbe *v,         /* Prepared statement under construction */
  اشاریہ *pIdx,     /* Index whose column is being loaded */
  صحیح k            /* Which column index */
){
  صحیح i;           /* Index of column in the table */
  أكد( k>=0 && k<pIdx->n_عمود );
  i = pIdx->ai_عمود[k];
  اگر( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }ورنہ اگر( i==XN_اظہاریہ ){
    أكد( pIdx->b_Has_اظہاریہ );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }ورنہ{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->p_جدول->a_ستون[i].zCnName));
  }
}
#ورنہ
# define analyze_Vdbe_Comment_اشاریہ_With_عمود_Name(a,b,c)
#اگر_ختم /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
ساکن خالی analyze_One_جدول(
  Parse *pParse,   /* Parser context */
  جدول *pTab,     /* Table whose indices are to be analyzed */
  اشاریہ *pOnlyIdx, /* If not NULL, only analyze this one index */
  صحیح iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  صحیح iMem,        /* Available memory locations begin here */
  صحیح iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  اشاریہ *pIdx;                 /* An index to being analyzed */
  صحیح iIdxCur;                 /* Cursor open on index being analyzed */
  صحیح iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  صحیح i;                       /* Loop counter */
  صحیح j_Zero_صفیں = -1;          /* Jump from here if number of rows is zero */
  صحیح iDb;                     /* Index of database containing pTab */
  u8 need_جدول_Cnt = 1;         /* True to count the table */
  صحیح regNewRowid = iMem++;    /* Rowid for the inserted record */
  صحیح regStat = iMem++;        /* Register to hold StatAccum object */
  صحیح regChng = iMem++;        /* Index of changed index field */
  صحیح regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  صحیح reg_عارضی = iMem++;        /* Temporary use register */
  صحیح regTemp2 = iMem++;       /* Second temporary use register */
  صحیح regTabname = iMem++;     /* Register containing table name */
  صحیح regIdxname = iMem++;     /* Register containing index name */
  صحیح regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  صحیح reg_پچھلا = iMem;          /* MUST BE LAST (see below) */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  صحیح doOnce = 1;              /* Flag for a one-time computation */
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
  جدول *pStat1 = 0;
#اگر_ختم

  sqlite3_Touch_رجسٹر(pParse, iMem);
  أكد( sqlite3_No_Temps_اندر_Range(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  اگر( v==0 || NEVER(pTab==0) ){
    واپس;
  }
  اگر( !Is_Ordinary_جدول(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    واپس;
  }
  اگر( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    واپس;
  }
  أكد( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
  أكد( iDb>=0 );
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
  اگر( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    واپس;
  }
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
  اگر( db->xPreUpdateCallback ){
    pStat1 = (جدول*)sqlite3DbMallocZero(db, حجم(جدول) + 13);
    اگر( pStat1==0 ) واپس;
    pStat1->zName = (حرف*)&pStat1[1];
    حافظہ_نقل(pStat1->zName, "sqlite_stat1", 13);
    pStat1->n_ستون = 3;
    pStat1->iPKey = -1;
    sqlite3_Vdbe_جوڑو_Op4(pParse->pVdbe, OP_Noop, 0, 0, 0,(حرف*)pStat1,P4_DYNAMIC);
  }
#اگر_ختم

  /* Establish a read-lock on the table at the shared-cache level. 
  ** Open a read-only cursor on the table. Also allocate a cursor number
  ** to use for scanning indexes (iIdxCur). No index cursor is opened at
  ** this time though.  */
  sqlite3_جدول_تالا(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = MAX(pParse->nTab, iTab);
  sqlite3_Open_جدول(pParse, iTabCur, iDb, pTab, OP_OpenRead);
  sqlite3_Vdbe_بھرو_String(v, regTabname, pTab->zName);

  برائے(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
    صحیح n_ستون;                     /* Number of columns in pIdx. "N" */
    صحیح addr_Goto_انتہا;               /* Address of "OP_Rewind iIdxCur" */
    صحیح addr_اگلا_صف;              /* Address of "next_row:" */
    ثابت حرف *zIdxName;         /* Name of the index */
    صحیح n_ستون_Test;                 /* Number of columns to test for changes */

    اگر( pOnlyIdx && pOnlyIdx!=pIdx ) جاری;
    اگر( pIdx->pPartIdxWhere==0 ) need_جدول_Cnt = 0;
    اگر( !HasRowid(pTab) && Is_Primary_کلید_اشاریہ(pIdx) ){
      n_ستون = pIdx->n_کلید_ستون;
      zIdxName = pTab->zName;
      n_ستون_Test = n_ستون - 1;
    }ورنہ{
      n_ستون = pIdx->n_عمود;
      zIdxName = pIdx->zName;
      n_ستون_Test = pIdx->uniqNotNull ? pIdx->n_کلید_ستون-1 : n_ستون-1;
    }

    /* Populate the register containing the index name. */
    sqlite3_Vdbe_بھرو_String(v, regIdxname, zIdxName);
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
    sqlite3_Touch_رجسٹر(pParse, reg_پچھلا+n_ستون_Test);

    /* Open a read-only cursor on the index being analyzed. */
    أكد( iDb==sqlite3_Schema_مقصد_اشاریہ(db, pIdx->pSchema) );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_سیٹ_P4_کلید_Info(pParse, pIdx);
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
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, db->n_Analysis_حد_بندی, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, n_ستون, regStat+1);
    أكد( regRowid==regStat+2 );
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, pIdx->n_کلید_ستون, regRowid);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_گنتی, iIdxCur, reg_عارضی,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3_Vdbe_جوڑو_Function_Call(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addr_Goto_انتہا = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, regChng);
    addr_اگلا_صف = sqlite3VdbeCurrentAddr(v);

    اگر( n_ستون_Test>0 ){
      صحیح انتہا_Distinct_Test = sqlite3VdbeMakeLabel(pParse);
      صحیح *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, حجم(صحیح)*n_ستون_Test);
      اگر( aGotoChng==0 ) جاری;

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
      sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
      addr_اگلا_صف = sqlite3VdbeCurrentAddr(v);
      اگر( n_ستون_Test==1 && pIdx->n_کلید_ستون==1 && Is_Unique_اشاریہ(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_NotNull, reg_پچھلا, انتہا_Distinct_Test);
        VdbeCoverage(v);
      }
      برائے(i=0; i<n_ستون_Test; i++){
        حرف *pColl = (حرف*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, i, regChng);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iIdxCur, i, reg_عارضی);
        analyze_Vdbe_Comment_اشاریہ_With_عمود_Name(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3_Vdbe_جوڑو_Op4(v, OP_Ne, reg_عارضی, 0, reg_پچھلا+i, pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, n_ستون_Test, regChng);
      sqlite3VdbeGoto(v, انتہا_Distinct_Test);
  
  
      /*
      **  chng_addr_0:
      **   regPrev(0) = idx(0)
      **  chng_addr_1:
      **   regPrev(1) = idx(1)
      **  ...
      */
      sqlite3_Vdbe_Jump_یہاں(v, addr_اگلا_صف-1);
      برائے(i=0; i<n_ستون_Test; i++){
        sqlite3_Vdbe_Jump_یہاں(v, aGotoChng[i]);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iIdxCur, i, reg_پچھلا+i);
        analyze_Vdbe_Comment_اشاریہ_With_عمود_Name(v,pIdx,i);
      }
      sqlite3VdbeResolveLabel(v, انتہا_Distinct_Test);
      sqlite3DbFree(db, aGotoChng);
    }
  
    /*
    **  chng_addr_N:
    **   regRowid = idx(rowid)            // STAT4 only
    **   stat_push(P, regChng, regRowid)  // 3rd parameter STAT4 only
    **   Next csr
    **   if !eof(csr) goto next_row;
    */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    اگر( OptimizationEnabled(db, SQLITE_Stat4) ){
      أكد( regRowid==(regStat+2) );
      اگر( HasRowid(pTab) ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_IdxRowid, iIdxCur, regRowid);
      }ورنہ{
        اشاریہ *pPk = sqlite3_Primary_کلید_اشاریہ(pIdx->p_جدول);
        صحیح j, k, reg_کلید;
        reg_کلید = sqlite3_Get_عارضی_Range(pParse, pPk->n_کلید_ستون);
        برائے(j=0; j<pPk->n_کلید_ستون; j++){
          k = sqlite3_جدول_عمود_مقصد_اشاریہ(pIdx, pPk->ai_عمود[j]);
          أكد( k>=0 && k<pIdx->n_عمود );
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, iIdxCur, k, reg_کلید+j);
          analyze_Vdbe_Comment_اشاریہ_With_عمود_Name(v,pIdx,k);
        }
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_کلید, pPk->n_کلید_ستون, regRowid);
        sqlite3_Release_عارضی_Range(pParse, reg_کلید, pPk->n_کلید_ستون);
      }
    }
#اگر_ختم
    أكد( regChng==(regStat+1) );
    {
      sqlite3_Vdbe_جوڑو_Function_Call(pParse, 1, regStat, reg_عارضی, 2+IsStat4,
                                 &stat_ڈالو_Funcdef, 0);
      اگر( db->n_Analysis_حد_بندی ){
        صحیح j1, j2, j3;
        j1 = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, reg_عارضی); VdbeCoverage(v);
        j2 = sqlite3_Vdbe_جوڑو_Op1(v, OP_If, reg_عارضی); VdbeCoverage(v);
        j3 = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_SeekGT, iIdxCur, 0, reg_پچھلا, 1);
        VdbeCoverage(v);
        sqlite3_Vdbe_Jump_یہاں(v, j1);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iIdxCur, addr_اگلا_صف); VdbeCoverage(v);
        sqlite3_Vdbe_Jump_یہاں(v, j2);
        sqlite3_Vdbe_Jump_یہاں(v, j3);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iIdxCur, addr_اگلا_صف); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    اگر( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3_Vdbe_Jump_یہاں(v, addr_Goto_انتہا);
      addr_Goto_انتہا = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    أكد( "BBB"[0]==SQLITE_AFF_متن );
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, regTabname, 3, reg_عارضی, "BBB", 0);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iStatCur, reg_عارضی, regNewRowid);
#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (حرف*)pStat1, P4_جدول);
#اگر_ختم
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    اگر( OptimizationEnabled(db, SQLITE_Stat4) && db->n_Analysis_حد_بندی==0 ){
      صحیح regEq = regStat1;
      صحیح regLt = regStat1+1;
      صحیح regDLt = regStat1+2;
      صحیح regSample = regStat1+3;
      صحیح reg_ستون = regStat1+4;
      صحیح regSampleRowid = reg_ستون + n_ستون;
      صحیح addr_اگلا;
      صحیح addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_Not_ملا;

      /* No STAT4 data is generated if the number of rows is zero */
      اگر( addr_Goto_انتہا==0 ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addr_Goto_انتہا = sqlite3_Vdbe_جوڑو_Op1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      اگر( doOnce ){
        صحیح mx_ستون = n_ستون;
        اشاریہ *pX;

        /* Compute the maximum number of columns in any index */
        برائے(pX=pTab->p_اشاریہ; pX; pX=pX->p_اگلا){
          صحیح n_ستون_X;                     /* Number of columns in pX */
          اگر( !HasRowid(pTab) && Is_Primary_کلید_اشاریہ(pX) ){
            n_ستون_X = pX->n_کلید_ستون;
          }ورنہ{
            n_ستون_X = pX->n_عمود;
          }
          اگر( n_ستون_X>mx_ستون ) mx_ستون = n_ستون_X;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3_Touch_رجسٹر(pParse, reg_ستون+mx_ستون);
        doOnce = 0;
#اگر_تعریف_شدہ SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3_No_Temps_اندر_Range(pParse, regEq, reg_ستون+mx_ستون) );
#اگر_ختم
        sqlite3_Clear_عارضی_Reg_Cache(pParse);  /* tag-20230325-1 */
        أكد( sqlite3_No_Temps_اندر_Range(pParse, regEq, reg_ستون+mx_ستون) );
      }
      أكد( sqlite3_No_Temps_اندر_Range(pParse, regEq, reg_ستون+n_ستون) );

      addr_اگلا = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3_Vdbe_جوڑو_Op1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, seekOp, iTabCur, addr_اگلا, regSampleRowid, 0);
      VdbeCoverage(v);
      برائے(i=0; i<n_ستون; i++){
        sqlite3_اظہاریہ_رمز_بھرو_اشاریہ_عمود(pParse, pIdx, iTabCur, i, reg_ستون+i);
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_ستون, n_ستون, regSample);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regTabname, 6, reg_عارضی);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iStatCur+1, reg_عارضی, regNewRowid);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 1, addr_اگلا); /* P1==1 for end-of-loop */
      sqlite3_Vdbe_Jump_یہاں(v, addrIsNull);
    }
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    اگر( addr_Goto_انتہا ) sqlite3_Vdbe_Jump_یہاں(v, addr_Goto_انتہا);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  اگر( pOnlyIdx==0 && need_جدول_Cnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3_Vdbe_جوڑو_Op2(v, OP_گنتی, iTabCur, regStat1);
    j_Zero_صفیں = sqlite3_Vdbe_جوڑو_Op1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, regIdxname);
    أكد( "BBB"[0]==SQLITE_AFF_متن );
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, regTabname, 3, reg_عارضی, "BBB", 0);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iStatCur, reg_عارضی, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (حرف*)pStat1, P4_جدول);
#اگر_ختم
    sqlite3_Vdbe_Jump_یہاں(v, j_Zero_صفیں);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
ساکن خالی بھرو_Analysis(Parse *pParse, صحیح iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  اگر( v ){
    sqlite3_Vdbe_جوڑو_Op1(v, OP_بھرو_Analysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
ساکن خالی analyzeDatabase(Parse *pParse, صحیح iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  صحیح iStatCur;
  صحیح iMem;
  صحیح iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  open_Stat_جدول(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  برائے(k=sqlite_Hash_پہلا(&pSchema->tblHash); k; k=sqlite_Hash_اگلا(k)){
    جدول *pTab = (جدول*)sqlite_Hash_ڈیٹا(k);
    analyze_One_جدول(pParse, pTab, 0, iStatCur, iMem, iTab);
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    iMem = sqlite3_پہلا_دستیاب_رجسٹر(pParse, iMem);
#ورنہ
    أكد( iMem==sqlite3_پہلا_دستیاب_رجسٹر(pParse,iMem) );
#اگر_ختم
  }
  بھرو_Analysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
ساکن خالی analyze_جدول(Parse *pParse, جدول *pTab, اشاریہ *pOnlyIdx){
  صحیح iDb;
  صحیح iStatCur;

  أكد( pTab!=0 );
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  اگر( pOnlyIdx ){
    open_Stat_جدول(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }ورنہ{
    open_Stat_جدول(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyze_One_جدول(pParse, pTab, pOnlyIdx, iStatCur,pParse->nMem+1,pParse->nTab);
  بھرو_Analysis(pParse, iDb);
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
خالی sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  صحیح iDb;
  صحیح i;
  حرف *z, *zDb;
  جدول *pTab;
  اشاریہ *pIdx;
  Token *p_جدول_Name;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  أكد( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  اگر( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    واپس;
  }

  أكد( pName2!=0 || pName1==0 );
  اگر( pName1==0 ){
    /* Form 1:  Analyze everything */
    برائے(i=0; i<db->nDb; i++){
      اگر( i==1 ) جاری;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }ورنہ اگر( pName2->n==0 && (iDb = sqlite3_ڈھونڈو_Db(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }ورنہ{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &p_جدول_Name);
    اگر( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, p_جدول_Name);
      اگر( z ){
        اگر( (pIdx = sqlite3_ڈھونڈو_اشاریہ(db, z, zDb))!=0 ){
          analyze_جدول(pParse, pIdx->p_جدول, pIdx);
        }ورنہ اگر( (pTab = sqlite3_Locate_جدول(pParse, 0, z, zDb))!=0 ){
          analyze_جدول(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  اگر( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3_Vdbe_جوڑو_Op0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
قسمتعریف ساخت analysisInfo analysisInfo;
ساخت analysisInfo {
  sqlite3 *db;
  ثابت حرف *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
ساکن خالی decodeIntArray(
  حرف *zIntArray,       /* String containing int array to decode */
  صحیح n_برآمد,              /* Number of slots in aOut[] */
  tRowcnt *a_برآمد,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  اشاریہ *p_اشاریہ          /* Handle extra flags for this index, if not NULL */
){
  حرف *z = zIntArray;
  صحیح c;
  صحیح i;
  tRowcnt v;

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( z==0 ) z = "";
#ورنہ
  أكد( z!=0 );
#اگر_ختم
  برائے(i=0; *z && i<n_برآمد; i++){
    v = 0;
    جب( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    اگر( a_برآمد ) a_برآمد[i] = v;
    اگر( aLog ) aLog[i] = sqlite3LogEst(v);
#ورنہ
    أكد( a_برآمد==0 );
    غیر_مستعمل_PARAMETER(a_برآمد);
    أكد( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#اگر_ختم
    اگر( *z==' ' ) z++;
  }
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_STAT4
  أكد( p_اشاریہ!=0 ); {
#ورنہ
  اگر( p_اشاریہ ){
#اگر_ختم
    p_اشاریہ->bUnordered = 0;
    p_اشاریہ->no_Skip_جائزہ = 0;
    جب( z[0] ){
      اگر( sqlite3_strglob("unordered*", z)==0 ){
        p_اشاریہ->bUnordered = 1;
      }ورنہ اگر( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        صحیح sz = sqlite3Atoi(z+3);
        اگر( sz<2 ) sz = 2;
        p_اشاریہ->sz_Idx_صف = sqlite3LogEst(sz);
      }ورنہ اگر( sqlite3_strglob("noskipscan*", z)==0 ){
        p_اشاریہ->no_Skip_جائزہ = 1;
      }
#اگر_تعریف_شدہ SQLITE_ENABLE_COSTMULT
      ورنہ اگر( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        p_اشاریہ->p_جدول->لاگت_Mult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#اگر_ختم
      جب( z[0]!=0 && z[0]!=' ' ) z++;
      جب( z[0]==' ' ) z++;
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
ساکن صحیح analysisLoader(خالی *p_ڈیٹا, صحیح argc, حرف **argv, حرف **Not_مستعمل){
  analysisInfo *pInfo = (analysisInfo*)p_ڈیٹا;
  اشاریہ *p_اشاریہ;
  جدول *p_جدول;
  ثابت حرف *z;

  أكد( argc==3 );
  غیر_مستعمل_PARAMETER2(Not_مستعمل, argc);

  اگر( argv==0 || argv[0]==0 || argv[2]==0 ){
    واپس 0;
  }
  p_جدول = sqlite3_ڈھونڈو_جدول(pInfo->db, argv[0], pInfo->zDatabase);
  اگر( p_جدول==0 ){
    واپس 0;
  }
  اگر( argv[1]==0 ){
    p_اشاریہ = 0;
  }ورنہ اگر( sqlite3_stricmp(argv[0],argv[1])==0 ){
    p_اشاریہ = sqlite3_Primary_کلید_اشاریہ(p_جدول);
  }ورنہ{
    p_اشاریہ = sqlite3_ڈھونڈو_اشاریہ(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  اگر( p_اشاریہ ){
    tRowcnt *ai_صف_Est = 0;
    صحیح n_ستون = p_اشاریہ->n_کلید_ستون+1;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    اگر( p_اشاریہ->ai_صف_Est==0 ){
      p_اشاریہ->ai_صف_Est = (tRowcnt*)sqlite3MallocZero(حجم(tRowcnt) * n_ستون);
      اگر( p_اشاریہ->ai_صف_Est==0 ) sqlite3OomFault(pInfo->db);
    }
    ai_صف_Est = p_اشاریہ->ai_صف_Est;
#اگر_ختم
    p_اشاریہ->bUnordered = 0;
    decodeIntArray((حرف*)z, n_ستون, ai_صف_Est, p_اشاریہ->ai_صف_Log_Est, p_اشاریہ);
    p_اشاریہ->hasStat1 = 1;
    اگر( p_اشاریہ->pPartIdxWhere==0 ){
      p_جدول->n_صف_Log_Est = p_اشاریہ->ai_صف_Log_Est[0];
      p_جدول->tabFlags |= TF_HasStat1;
    }
  }ورنہ{
    اشاریہ fakeIdx;
    fakeIdx.sz_Idx_صف = p_جدول->sz_Tab_صف;
#اگر_تعریف_شدہ SQLITE_ENABLE_COSTMULT
    fakeIdx.p_جدول = p_جدول;
#اگر_ختم
    decodeIntArray((حرف*)z, 1, 0, &p_جدول->n_صف_Log_Est, &fakeIdx);
    p_جدول->sz_Tab_صف = fakeIdx.sz_Idx_صف;
    p_جدول->tabFlags |= TF_HasStat1;
  }

  واپس 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
خالی sqlite3_Delete_اشاریہ_Samples(sqlite3 *db, اشاریہ *pIdx){
  أكد( db!=0 );
  أكد( pIdx!=0 );
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( pIdx->aSample ){
    صحیح j;
    برائے(j=0; j<pIdx->nSample; j++){
      اشاریہ_Sample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  اگر( db->pn_بائٹیں_Freed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#ورنہ
  غیر_مستعمل_PARAMETER(db);
  غیر_مستعمل_PARAMETER(pIdx);
#اگر_ختم /* SQLITE_ENABLE_STAT4 */
}

#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
ساکن خالی initAvgEq(اشاریہ *pIdx){
  اگر( pIdx ){
    اشاریہ_Sample *aSample = pIdx->aSample;
    اشاریہ_Sample *pFinal = &aSample[pIdx->nSample-1];
    صحیح i_ستون;
    صحیح n_ستون = 1;
    اگر( pIdx->n_Sample_ستون>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      n_ستون = pIdx->n_Sample_ستون-1;
      pIdx->aAvgEq[n_ستون] = 1;
    }
    برائے(i_ستون=0; i_ستون<n_ستون; i_ستون++){
      صحیح nSample = pIdx->nSample;
      صحیح i;                    /* Used to iterate through samples */
      tRowcnt مجموع_Eq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt n_صف;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      اگر( !pIdx->ai_صف_Est || i_ستون>=pIdx->n_کلید_ستون || pIdx->ai_صف_Est[i_ستون+1]==0 ){
        n_صف = pFinal->anLt[i_ستون];
        nDist100 = (i64)100 * pFinal->anDLt[i_ستون];
        nSample--;
      }ورنہ{
        n_صف = pIdx->ai_صف_Est[0];
        nDist100 = ((i64)100 * pIdx->ai_صف_Est[0]) / pIdx->ai_صف_Est[i_ستون+1];
      }
      pIdx->n_صف_Est0 = n_صف;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      برائے(i=0; i<nSample; i++){
        اگر( i==(pIdx->nSample-1)
         || aSample[i].anDLt[i_ستون]!=aSample[i+1].anDLt[i_ستون] 
        ){
          مجموع_Eq += aSample[i].anEq[i_ستون];
          nSum100 += 100;
        }
      }

      اگر( nDist100>nSum100 && مجموع_Eq<n_صف ){
        avgEq = ((i64)100 * (n_صف - مجموع_Eq))/(nDist100 - nSum100);
      }
      اگر( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[i_ستون] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
ساکن اشاریہ *ڈھونڈو_اشاریہ_Or_Primary_کلید(
  sqlite3 *db,
  ثابت حرف *zName,
  ثابت حرف *zDb
){
  اشاریہ *pIdx = sqlite3_ڈھونڈو_اشاریہ(db, zName, zDb);
  اگر( pIdx==0 ){
    جدول *pTab = sqlite3_ڈھونڈو_جدول(db, zName, zDb);
    اگر( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_کلید_اشاریہ(pTab);
  }
  واپس pIdx;
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
ساکن صحیح بھرو_Stat_Tbl(
  sqlite3 *db,                  /* Database handle */
  ثابت حرف *zSql1,            /* SQL statement 1 (see above) */
  ثابت حرف *zSql2,            /* SQL statement 2 (see above) */
  ثابت حرف *zDb               /* Database name (e.g. "main") */
){
  صحیح rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  حرف *zSql;                   /* Text of the SQL statement */
  اشاریہ *p_پچھلا_Idx = 0;          /* Previous index in the loop */
  اشاریہ_Sample *pSample;         /* A slot in pIdx->aSample[] */

  أكد( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  اگر( !zSql ){
    واپس SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  اگر( rc ) واپس rc;

  جب( sqlite3_قدم(pStmt)==SQLITE_صف ){
    صحیح n_Idx_ستون = 1;              /* Number of columns in stat4 records */

    حرف *z_اشاریہ;    /* Index name */
    اشاریہ *pIdx;     /* Pointer to the index object */
    صحیح nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    z_اشاریہ = (حرف *)sqlite3_عمود_متن(pStmt, 0);
    اگر( z_اشاریہ==0 ) جاری;
    nSample = sqlite3_عمود_int(pStmt, 1);
    pIdx = ڈھونڈو_اشاریہ_Or_Primary_کلید(db, z_اشاریہ, zDb);
    أكد( pIdx==0 || pIdx->nSample==0 );
    اگر( pIdx==0 ) جاری;
    اگر( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      جاری;
    }
    أكد( !HasRowid(pIdx->p_جدول) || pIdx->n_عمود==pIdx->n_کلید_ستون+1 );
    اگر( !HasRowid(pIdx->p_جدول) && Is_Primary_کلید_اشاریہ(pIdx) ){
      n_Idx_ستون = pIdx->n_کلید_ستون;
    }ورنہ{
      n_Idx_ستون = pIdx->n_عمود;
    }
    pIdx->n_Sample_ستون = n_Idx_ستون;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(اشاریہ_Sample) * nSample);
    nByte += sizeof64(tRowcnt) * n_Idx_ستون * 3 * nSample;
    nByte += n_Idx_ستون * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    اگر( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      واپس SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    أكد( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += n_Idx_ستون;
    pIdx->p_جدول->tabFlags |= TF_HasStat4;
    برائے(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += n_Idx_ستون;
      pIdx->aSample[i].anLt = pSpace; pSpace += n_Idx_ستون;
      pIdx->aSample[i].anDLt = pSpace; pSpace += n_Idx_ستون;
    }
    أكد( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  اگر( rc ) واپس rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  اگر( !zSql ){
    واپس SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  اگر( rc ) واپس rc;

  جب( sqlite3_قدم(pStmt)==SQLITE_صف ){
    حرف *z_اشاریہ;                 /* Index name */
    اشاریہ *pIdx;                  /* Pointer to the index object */
    صحیح n_ستون = 1;                 /* Number of columns in index */

    z_اشاریہ = (حرف *)sqlite3_عمود_متن(pStmt, 0);
    اگر( z_اشاریہ==0 ) جاری;
    pIdx = ڈھونڈو_اشاریہ_Or_Primary_کلید(db, z_اشاریہ, zDb);
    اگر( pIdx==0 ) جاری;
    اگر( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      جاری;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    n_ستون = pIdx->n_Sample_ستون;
    اگر( pIdx!=p_پچھلا_Idx ){
      initAvgEq(p_پچھلا_Idx);
      p_پچھلا_Idx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((حرف*)sqlite3_عمود_متن(pStmt,1),n_ستون,pSample->anEq,0,0);
    decodeIntArray((حرف*)sqlite3_عمود_متن(pStmt,2),n_ستون,pSample->anLt,0,0);
    decodeIntArray((حرف*)sqlite3_عمود_متن(pStmt,3),n_ستون,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_عمود_بائٹیں(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    اگر( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      واپس SQLITE_NOMEM_BKPT;
    }
    اگر( pSample->n ){
      حافظہ_نقل(pSample->p, sqlite3_عمود_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  اگر( rc==SQLITE_OK ) initAvgEq(p_پچھلا_Idx);
  واپس rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
ساکن صحیح بھرو_Stat4(sqlite3 *db, ثابت حرف *zDb){
  صحیح rc = SQLITE_OK;             /* Result codes from subroutines */
  ثابت جدول *pStat4;

  أكد( db->lookaside.bDisable );
  اگر( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3_ڈھونڈو_جدول(db, "sqlite_stat4", zDb))!=0
   && Is_Ordinary_جدول(pStat4)
  ){
    rc = بھرو_Stat_Tbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  واپس rc;
}
#اگر_ختم /* SQLITE_ENABLE_STAT4 */

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
صحیح sqlite3_Analysis_بھرو(sqlite3 *db, صحیح iDb){
  analysisInfo sInfo;
  HashElem *i;
  حرف *zSql;
  صحیح rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  ثابت جدول *pStat1;

  أكد( iDb>=0 && iDb<db->nDb );
  أكد( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  برائے(i=sqlite_Hash_پہلا(&pSchema->tblHash); i; i=sqlite_Hash_اگلا(i)){
    جدول *pTab = sqlite_Hash_ڈیٹا(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  برائے(i=sqlite_Hash_پہلا(&pSchema->idxHash); i; i=sqlite_Hash_اگلا(i)){
    اشاریہ *pIdx = sqlite_Hash_ڈیٹا(i);
    pIdx->hasStat1 = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
    sqlite3_Delete_اشاریہ_Samples(db, pIdx);
    pIdx->aSample = 0;
#اگر_ختم
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  اگر( (pStat1 = sqlite3_ڈھونڈو_جدول(db, "sqlite_stat1", sInfo.zDatabase))
   && Is_Ordinary_جدول(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    اگر( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ورنہ{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  أكد( sqlite3SchemaMutexHeld(db, iDb, 0) );
  برائے(i=sqlite_Hash_پہلا(&pSchema->idxHash); i; i=sqlite_Hash_اگلا(i)){
    اشاریہ *pIdx = sqlite_Hash_ڈیٹا(i);
    اگر( !pIdx->hasStat1 ) sqlite3_Default_صف_Est(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#اگر_تعریف_شدہ SQLITE_ENABLE_STAT4
  اگر( rc==SQLITE_OK ){
    DisableLookaside;
    rc = بھرو_Stat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  برائے(i=sqlite_Hash_پہلا(&pSchema->idxHash); i; i=sqlite_Hash_اگلا(i)){
    اشاریہ *pIdx = sqlite_Hash_ڈیٹا(i);
    sqlite3_free(pIdx->ai_صف_Est);
    pIdx->ai_صف_Est = 0;
  }
#اگر_ختم

  اگر( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  واپس rc;
}


#اگر_ختم /* SQLITE_OMIT_ANALYZE */
