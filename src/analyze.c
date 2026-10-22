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
#अगर_अघोषित SQLITE_OMIT_ANALYZE
#समावेश "sqliteInt.h"

#अगर defined(SQLITE_ENABLE_STAT4)
# define IsStat4     1
#अन्यथा
# define IsStat4     0
# undef SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 1
#पूर्ण_अगर

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
अटल व्योम openStatTable(
  Parse *pParse,          /* Parsing context */
  पूर्णांक iDb,                /* The database we are looking in */
  पूर्णांक iStatCur,           /* Open the sqlite_stat1 table on this cursor */
  स्थिर अक्षर *zWhere,     /* Delete entries for this table or index */
  स्थिर अक्षर *zWhereType  /* Either "tbl" or "idx" */
){
  अटल स्थिर काष्ठा {
    स्थिर अक्षर *zName;
    स्थिर अक्षर *zCols;
  } aTable[] = {
    { "sqlite_stat1", "tbl,idx,stat" },
#अगर defined(SQLITE_ENABLE_STAT4)
    { "sqlite_stat4", "tbl,idx,neq,nlt,ndlt,sample" },
#अन्यथा
    { "sqlite_stat4", 0 },
#पूर्ण_अगर
    { "sqlite_stat3", 0 },
  };
  पूर्णांक i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 a_मूल[Array_आकार(aTable)];
  u8 aCreateTbl[Array_आकार(aTable)];
#अगर_घोषित SQLITE_ENABLE_STAT4
  स्थिर पूर्णांक nToOpen = OptimizationEnabled(db,SQLITE_Stat4) ? 2 : 1;
#अन्यथा
  स्थिर पूर्णांक nToOpen = 1;
#पूर्ण_अगर

  अगर( v==0 ) वापस;
  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  निश्चित( sqlite3VdbeDb(v)==db );
  pDb = &db->aDb[iDb];

  /* Create new statistic tables if they do not exist, or clear them
  ** if they do already exist.
  */
  क्रम(i=0; i<Array_आकार(aTable); i++){
    स्थिर अक्षर *zTab = aTable[i].zName;
    Table *pStat;
    aCreateTbl[i] = 0;
    अगर( (pStat = sqlite3FindTable(db, zTab, pDb->zDbSName))==0 ){
      अगर( i<nToOpen ){
        /* The sqlite_statN table does not exist. Create it. Note that a 
        ** side-effect of the CREATE TABLE statement is to leave the rootpage 
        ** of the new table in register pParse->regRoot. This is important 
        ** because the OpenWrite opcode below will be needing it. */
        sqlite3NestedParse(pParse,
            "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, aTable[i].zCols
        );
        निश्चित( pParse->isCreate || pParse->nErr );
        a_मूल[i] = (u32)pParse->u1.cr.reg_मूल;
        aCreateTbl[i] = OPFLAG_P2ISREG;
      }
    }अन्यथा{
      /* The table already exists. If zWhere is not NULL, delete all entries 
      ** associated with the table zWhere. If zWhere is NULL, delete the
      ** entire contents of the table. */
      a_मूल[i] = pStat->tnum;
      sqlite3TableLock(pParse, iDb, a_मूल[i], 1, zTab);
      अगर( zWhere ){
        sqlite3NestedParse(pParse,
           "DELETE FROM %Q.%s WHERE %s=%Q",
           pDb->zDbSName, zTab, zWhereType, zWhere
        );
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
      }अन्यथा अगर( db->xPreUpdateCallback ){
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s", pDb->zDbSName, zTab);
#पूर्ण_अगर
      }अन्यथा{
        /* The sqlite_stat[134] table already exists.  Delete all rows. */
        sqlite3VdbeAddOp2(v, OP_Clear, (पूर्णांक)a_मूल[i], iDb);
      }
    }
  }

  /* Open the sqlite_stat[134] tables for writing. */
  क्रम(i=0; i<nToOpen; i++){
    निश्चित( i<Array_आकार(aTable) );
    sqlite3VdbeAddOp4Int(v, OP_OpenWrite, iStatCur+i, (पूर्णांक)a_मूल[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
    VdbeComment((v, aTable[i].zName));
  }
}

/*
** Recommended number of samples for sqlite_stat4
*/
#अगर_अघोषित SQLITE_STAT4_SAMPLES
# define SQLITE_STAT4_SAMPLES 24
#पूर्ण_अगर

/*
** Three SQL functions - stat_init(), stat_push(), and stat_get() -
** share an instance of the following structure to hold their state
** information.
*/
प्रकार काष्ठा StatAccum StatAccum;
प्रकार काष्ठा StatSample StatSample;
काष्ठा StatSample {
  tRowcnt *anDLt;                 /* sqlite_stat4.nDLt */
#अगर_घोषित SQLITE_ENABLE_STAT4
  tRowcnt *anEq;                  /* sqlite_stat4.nEq */
  tRowcnt *anLt;                  /* sqlite_stat4.nLt */
  जोड़ {
    i64 iRowid;                     /* Rowid in main table of the key */
    u8 *aRowid;                     /* Key for WITHOUT ROWID tables */
  } u;
  u32 nRowid;                     /* Sizeof aRowid[] */
  u8 isPSample;                   /* True if a periodic sample */
  पूर्णांक iCol;                       /* If !isPSample, the reason for inclusion */
  u32 iHash;                      /* Tiebreaker hash */
#पूर्ण_अगर
};                                                    
काष्ठा StatAccum {
  sqlite3 *db;              /* Database connection, for malloc() */
  tRowcnt nEst;             /* Estimated number of rows */
  tRowcnt nRow;             /* Number of rows visited so far */
  पूर्णांक nLimit;               /* Analysis row-scan limit */
  पूर्णांक nCol;                 /* Number of columns in index + pk/rowid */
  पूर्णांक n_कुंजी_Col;              /* Number of index columns w/o the pk/rowid */
  u8 nSkipAhead;            /* Number of times of skip-ahead */
  StatSample current;       /* Current row as a StatSample */
#अगर_घोषित SQLITE_ENABLE_STAT4
  tRowcnt nPSample;         /* How often to do a periodic sample */
  पूर्णांक mxSample;             /* Maximum number of samples to accumulate */
  u32 iPrn;                 /* Pseudo-random number used for sampling */
  StatSample *aBest;        /* Array of nCol best samples */
  पूर्णांक iMin;                 /* Index in a[] of entry with minimum score */
  पूर्णांक nSample;              /* Current number of samples */
  पूर्णांक nMaxEqZero;           /* Max leading 0 in anEq[] for any a[] entry */
  पूर्णांक iGet;                 /* Index of current sample accessed by stat_get() */
  StatSample *a;            /* Array of mxSample StatSample objects */
#पूर्ण_अगर
};

/* Reclaim memory used by a StatSample
*/
#अगर_घोषित SQLITE_ENABLE_STAT4
अटल व्योम sampleClear(sqlite3 *db, StatSample *p){
  निश्चित( db!=0 );
  अगर( p->nRowid ){
    sqlite3DbFree(db, p->u.aRowid);
    p->nRowid = 0;
  }
}
#पूर्ण_अगर

/* Initialize the BLOB value of a ROWID
*/
#अगर_घोषित SQLITE_ENABLE_STAT4
अटल व्योम sampleSetRowid(sqlite3 *db, StatSample *p, पूर्णांक n, स्थिर u8 *p_तथ्य){
  निश्चित( db!=0 );
  अगर( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->u.aRowid = sqlite3DbMallocRawNN(db, n);
  अगर( p->u.aRowid ){
    p->nRowid = n;
    स_नकल(p->u.aRowid, p_तथ्य, n);
  }अन्यथा{
    p->nRowid = 0;
  }
}
#पूर्ण_अगर

/* Initialize the INTEGER value of a ROWID.
*/
#अगर_घोषित SQLITE_ENABLE_STAT4
अटल व्योम sampleSetRowidInt64(sqlite3 *db, StatSample *p, i64 iRowid){
  निश्चित( db!=0 );
  अगर( p->nRowid ) sqlite3DbFree(db, p->u.aRowid);
  p->nRowid = 0;
  p->u.iRowid = iRowid;
}
#पूर्ण_अगर


/*
** Copy the contents of object (*pFrom) into (*pTo).
*/
#अगर_घोषित SQLITE_ENABLE_STAT4
अटल व्योम sampleCopy(StatAccum *p, StatSample *pTo, StatSample *pFrom){
  pTo->isPSample = pFrom->isPSample;
  pTo->iCol = pFrom->iCol;
  pTo->iHash = pFrom->iHash;
  स_नकल(pTo->anEq, pFrom->anEq, माप(tRowcnt)*p->nCol);
  स_नकल(pTo->anLt, pFrom->anLt, माप(tRowcnt)*p->nCol);
  स_नकल(pTo->anDLt, pFrom->anDLt, माप(tRowcnt)*p->nCol);
  अगर( pFrom->nRowid ){
    sampleSetRowid(p->db, pTo, pFrom->nRowid, pFrom->u.aRowid);
  }अन्यथा{
    sampleSetRowidInt64(p->db, pTo, pFrom->u.iRowid);
  }
}
#पूर्ण_अगर

/*
** Reclaim all memory of a StatAccum structure.
*/
अटल व्योम statAccumDestructor(व्योम *pOld){
  StatAccum *p = (StatAccum*)pOld;
#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( p->mxSample ){
    पूर्णांक i;
    क्रम(i=0; i<p->nCol; i++) sampleClear(p->db, p->aBest+i);
    क्रम(i=0; i<p->mxSample; i++) sampleClear(p->db, p->a+i);
    sampleClear(p->db, &p->current);
  }
#पूर्ण_अगर
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
अटल व्योम statInit(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  StatAccum *p;
  पूर्णांक nCol;                       /* Number of columns in index being sampled */
  पूर्णांक n_कुंजी_Col;                    /* Number of key columns */
  पूर्णांक nColUp;                     /* nCol rounded up for alignment */
  i64 n;                          /* Bytes of space to allocate */
  sqlite3 *db = sqlite3_context_db_handle(context);   /* Database connection */
#अगर_घोषित SQLITE_ENABLE_STAT4
  /* Maximum number of samples.  0 if STAT4 data is not collected */
  पूर्णांक mxSample = OptimizationEnabled(db,SQLITE_Stat4) ?SQLITE_STAT4_SAMPLES :0;
#पूर्ण_अगर

  /* Decode the three function arguments */
  UNUSED_PARAMETER(argc);
  nCol = sqlite3_मूल्य_int(argv[0]);
  निश्चित( nCol>0 );
  nColUp = माप(tRowcnt)<8 ? (nCol+1)&~1 : nCol;
  n_कुंजी_Col = sqlite3_मूल्य_int(argv[1]);
  निश्चित( n_कुंजी_Col<=nCol );
  निश्चित( n_कुंजी_Col>0 );

  /* Allocate the space required for the StatAccum object */
  n = माप(*p) 
    + माप(tRowcnt)*nColUp;                    /* StatAccum.anDLt */
#अगर_घोषित SQLITE_ENABLE_STAT4
  n += माप(tRowcnt)*nColUp;                   /* StatAccum.anEq */
  अगर( mxSample ){
    n += माप(tRowcnt)*nColUp                  /* StatAccum.anLt */
      + माप(StatSample)*(nCol+mxSample)       /* StatAccum.aBest[], a[] */
      + माप(tRowcnt)*3*nColUp*(nCol+mxSample);
  }
#पूर्ण_अगर
  p = sqlite3DbMallocZero(db, n);
  अगर( p==0 ){
    sqlite3_परिणाम_error_nomem(context);
    वापस;
  }

  p->db = db;
  p->nEst = sqlite3_मूल्य_int64(argv[2]);
  p->nRow = 0;
  p->nLimit = sqlite3_मूल्य_int(argv[3]);
  p->nCol = nCol;
  p->n_कुंजी_Col = n_कुंजी_Col;
  p->nSkipAhead = 0;
  p->current.anDLt = (tRowcnt*)&p[1];

#अगर_घोषित SQLITE_ENABLE_STAT4
  p->current.anEq = &p->current.anDLt[nColUp];
  p->mxSample = p->nLimit==0 ? mxSample : 0;
  अगर( mxSample ){
    u8 *pSpace;                     /* Allocated space not yet assigned */
    पूर्णांक i;                          /* Used to iterate through p->aSample[] */

    p->iGet = -1;
    p->nPSample = (tRowcnt)(p->nEst/(mxSample/3+1) + 1);
    p->current.anLt = &p->current.anEq[nColUp];
    p->iPrn = 0x689e962d*(u32)nCol ^ 0xd0944565*(u32)sqlite3_मूल्य_int(argv[2]);
  
    /* Set up the StatAccum.a[] and aBest[] arrays */
    p->a = (काष्ठा StatSample*)&p->current.anLt[nColUp];
    p->aBest = &p->a[mxSample];
    pSpace = (u8*)(&p->a[mxSample+nCol]);
    क्रम(i=0; i<(mxSample+nCol); i++){
      p->a[i].anEq = (tRowcnt *)pSpace; pSpace += (माप(tRowcnt) * nColUp);
      p->a[i].anLt = (tRowcnt *)pSpace; pSpace += (माप(tRowcnt) * nColUp);
      p->a[i].anDLt = (tRowcnt *)pSpace; pSpace += (माप(tRowcnt) * nColUp);
    }
    निश्चित( (pSpace - (u8*)p)==n );
  
    क्रम(i=0; i<nCol; i++){
      p->aBest[i].iCol = i;
    }
  }
#पूर्ण_अगर

  /* Return a pointer to the allocated object to the caller.  Note that
  ** only the pointer (the 2nd parameter) matters.  The size of the object
  ** (given by the 3rd parameter) is never used and can be any positive
  ** value. */
  sqlite3_परिणाम_blob(context, p, माप(*p), statAccumDestructor);
}
अटल स्थिर FuncDef statInitFuncdef = {
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

#अगर_घोषित SQLITE_ENABLE_STAT4
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
अटल पूर्णांक sampleIsBetterPost(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  पूर्णांक nCol = pAccum->nCol;
  पूर्णांक i;
  निश्चित( pNew->iCol==pOld->iCol );
  क्रम(i=pNew->iCol+1; i<nCol; i++){
    अगर( pNew->anEq[i]>pOld->anEq[i] ) वापस 1;
    अगर( pNew->anEq[i]<pOld->anEq[i] ) वापस 0;
  }
  अगर( pNew->iHash>pOld->iHash ) वापस 1;
  वापस 0;
}
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_STAT4
/*
** Return true if pNew is to be preferred over pOld.
**
** This function assumes that for each argument sample, the contents of
** the anEq[] array from pSample->anEq[pSample->iCol] onwards are valid. 
*/
अटल पूर्णांक sampleIsBetter(
  StatAccum *pAccum, 
  StatSample *pNew, 
  StatSample *pOld
){
  tRowcnt nEqNew = pNew->anEq[pNew->iCol];
  tRowcnt nEqOld = pOld->anEq[pOld->iCol];

  निश्चित( pOld->isPSample==0 && pNew->isPSample==0 );
  निश्चित( IsStat4 || (pNew->iCol==0 && pOld->iCol==0) );

  अगर( (nEqNew>nEqOld) ) वापस 1;
  अगर( nEqNew==nEqOld ){
    अगर( pNew->iCol<pOld->iCol ) वापस 1;
    वापस (pNew->iCol==pOld->iCol && sampleIsBetterPost(pAccum, pNew, pOld));
  }
  वापस 0;
}

/*
** Copy the contents of sample *pNew into the p->a[] array. If necessary,
** remove the least desirable sample from p->a[] to make room.
*/
अटल व्योम sample_जोड़ो(StatAccum *p, StatSample *pNew, पूर्णांक nEqZero){
  StatSample *pSample = 0;
  पूर्णांक i;

  निश्चित( IsStat4 || nEqZero==0 );

  /* StatAccum.nMaxEqZero is set to the maximum number of leading 0
  ** values in the anEq[] array of any sample in StatAccum.a[]. In
  ** other words, if nMaxEqZero is n, then it is guaranteed that there
  ** are no samples with StatSample.anEq[m]==0 for (m>=n). */
  अगर( nEqZero>p->nMaxEqZero ){
    p->nMaxEqZero = nEqZero;
  }
  अगर( pNew->isPSample==0 ){
    StatSample *pUpgrade = 0;
    निश्चित( pNew->anEq[pNew->iCol]>0 );

    /* This sample is being added because the prefix that ends in column 
    ** iCol occurs many times in the table. However, if we have already
    ** added a sample that shares this prefix, there is no need to add
    ** this one. Instead, upgrade the priority of the highest priority
    ** existing sample that shares this prefix.  */
    क्रम(i=p->nSample-1; i>=0; i--){
      StatSample *pOld = &p->a[i];
      अगर( pOld->anEq[pNew->iCol]==0 ){
        अगर( pOld->isPSample ) वापस;
        निश्चित( pOld->iCol>pNew->iCol );
        निश्चित( sampleIsBetter(p, pNew, pOld) );
        अगर( pUpgrade==0 || sampleIsBetter(p, pOld, pUpgrade) ){
          pUpgrade = pOld;
        }
      }
    }
    अगर( pUpgrade ){
      pUpgrade->iCol = pNew->iCol;
      pUpgrade->anEq[pUpgrade->iCol] = pNew->anEq[pUpgrade->iCol];
      जाओ find_new_min;
    }
  }

  /* If necessary, remove sample iMin to make room for the new sample. */
  अगर( p->nSample>=p->mxSample ){
    StatSample *pMin = &p->a[p->iMin];
    tRowcnt *anEq = pMin->anEq;
    tRowcnt *anLt = pMin->anLt;
    tRowcnt *anDLt = pMin->anDLt;
    sampleClear(p->db, pMin);
    स_हटाओ(pMin, &pMin[1], माप(p->a[0])*(p->nSample-p->iMin-1));
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
  निश्चित( p->nSample==0 
       || pNew->anLt[p->nCol-1] > p->a[p->nSample-1].anLt[p->nCol-1] );

  /* Insert the new sample */
  pSample = &p->a[p->nSample];
  sampleCopy(p, pSample, pNew);
  p->nSample++;

  /* Zero the first nEqZero entries in the anEq[] array. */
  स_रखो(pSample->anEq, 0, माप(tRowcnt)*nEqZero);

find_new_min:
  अगर( p->nSample>=p->mxSample ){
    पूर्णांक iMin = -1;
    क्रम(i=0; i<p->mxSample; i++){
      अगर( p->a[i].isPSample ) जारी;
      अगर( iMin<0 || sampleIsBetter(p, &p->a[iMin], &p->a[i]) ){
        iMin = i;
      }
    }
    निश्चित( iMin>=0 );
    p->iMin = iMin;
  }
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

#अगर_घोषित SQLITE_ENABLE_STAT4
/*
** Field iChng of the index being scanned has changed. So at this point
** p->current contains a sample that reflects the previous row of the
** index. The value of anEq[iChng] and subsequent anEq[] elements are
** correct at this point.
*/
अटल व्योम sample_धकेलो_Previous(StatAccum *p, पूर्णांक iChng){
  पूर्णांक i;

  /* Check if any samples from the aBest[] array should be pushed
  ** into IndexSample.a[] at this point.  */
  क्रम(i=(p->nCol-2); i>=iChng; i--){
    StatSample *pBest = &p->aBest[i];
    pBest->anEq[i] = p->current.anEq[i];
    अगर( p->nSample<p->mxSample || sampleIsBetter(p, pBest, &p->a[p->iMin]) ){
      sample_जोड़ो(p, pBest, i);
    }
  }

  /* Check that no sample contains an anEq[] entry with an index of
  ** p->nMaxEqZero or greater set to zero. */
  क्रम(i=p->nSample-1; i>=0; i--){
    पूर्णांक j;
    क्रम(j=p->nMaxEqZero; j<p->nCol; j++) निश्चित( p->a[i].anEq[j]>0 );
  }

  /* Update the anEq[] fields of any samples already collected. */
  अगर( iChng<p->nMaxEqZero ){
    क्रम(i=p->nSample-1; i>=0; i--){
      पूर्णांक j;
      क्रम(j=iChng; j<p->nCol; j++){
        अगर( p->a[i].anEq[j]==0 ) p->a[i].anEq[j] = p->current.anEq[j];
      }
    }
    p->nMaxEqZero = iChng;
  }
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

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
अटल व्योम stat_धकेलो(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  पूर्णांक i;

  /* The three function arguments */
  StatAccum *p = (StatAccum*)sqlite3_मूल्य_blob(argv[0]);
  पूर्णांक iChng = sqlite3_मूल्य_int(argv[1]);

  UNUSED_PARAMETER( argc );
  UNUSED_PARAMETER( context );
  निश्चित( p->nCol>0 );
  निश्चित( iChng<p->nCol );

  अगर( p->nRow==0 ){
    /* This is the first call to this function. Do initialization. */
#अगर_घोषित SQLITE_ENABLE_STAT4
    क्रम(i=0; i<p->nCol; i++) p->current.anEq[i] = 1;
#पूर्ण_अगर
  }अन्यथा{
    /* Second and subsequent calls get processed here */
#अगर_घोषित SQLITE_ENABLE_STAT4
    अगर( p->mxSample ) sample_धकेलो_Previous(p, iChng);
#पूर्ण_अगर

    /* Update anDLt[], anLt[] and anEq[] to reflect the values that apply
    ** to the current row of the index. */
#अगर_घोषित SQLITE_ENABLE_STAT4
    क्रम(i=0; i<iChng; i++){
      p->current.anEq[i]++;
    }
#पूर्ण_अगर
    क्रम(i=iChng; i<p->nCol; i++){
      p->current.anDLt[i]++;
#अगर_घोषित SQLITE_ENABLE_STAT4
      अगर( p->mxSample ) p->current.anLt[i] += p->current.anEq[i];
      p->current.anEq[i] = 1;
#पूर्ण_अगर
    }
  }

  p->nRow++;
#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( p->mxSample ){
    tRowcnt nLt;
    अगर( sqlite3_मूल्य_type(argv[2])==SQLITE_INTEGER ){
      sampleSetRowidInt64(p->db, &p->current, sqlite3_मूल्य_int64(argv[2]));
    }अन्यथा{
      sampleSetRowid(p->db, &p->current, sqlite3_मूल्य_bytes(argv[2]),
                                         sqlite3_मूल्य_blob(argv[2]));
    }
    p->current.iHash = p->iPrn = p->iPrn*1103515245 + 12345;

    nLt = p->current.anLt[p->nCol-1];
    /* Check if this is to be a periodic sample. If so, add it. */
    अगर( (nLt/p->nPSample)!=(nLt+1)/p->nPSample ){
      p->current.isPSample = 1;
      p->current.iCol = 0;
      sample_जोड़ो(p, &p->current, p->nCol-1);
      p->current.isPSample = 0;
    }

    /* Update the aBest[] array. */
    क्रम(i=0; i<(p->nCol-1); i++){
      p->current.iCol = i;
      अगर( i>=iChng || sampleIsBetterPost(p, &p->current, &p->aBest[i]) ){
        sampleCopy(p, &p->aBest[i], &p->current);
      }
    }
  }अन्यथा
#पूर्ण_अगर
  अगर( p->nLimit && p->nRow>(tRowcnt)p->nLimit*(p->nSkipAhead+1) ){
    p->nSkipAhead++;
    sqlite3_परिणाम_int(context, p->current.anDLt[0]>0);
  }
}

अटल स्थिर FuncDef stat_धकेलो_Funcdef = {
  2+IsStat4,       /* nArg */
  SQLITE_UTF8,     /* funcFlags */
  0,               /* pUserData */
  0,               /* pNext */
  stat_धकेलो,        /* xSFunc */
  0,               /* xFinalize */
  0, 0,            /* xValue, xInverse */
  "stat_push",     /* zName */
  {0}
};

#घोषणा STAT_GET_STAT1 0          /* "stat" column of stat1 table */
#घोषणा STAT_GET_ROWID 1          /* "rowid" column of stat[34] entry */
#घोषणा STAT_GET_NEQ   2          /* "neq" column of stat[34] entry */
#घोषणा STAT_GET_NLT   3          /* "nlt" column of stat[34] entry */
#घोषणा STAT_GET_NDLT  4          /* "ndlt" column of stat[34] entry */

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
अटल व्योम statGet(
  sqlite3_context *context,
  पूर्णांक argc,
  sqlite3_मूल्य **argv
){
  StatAccum *p = (StatAccum*)sqlite3_मूल्य_blob(argv[0]);
#अगर_घोषित SQLITE_ENABLE_STAT4
  /* STAT4 has a parameter on this routine. */
  पूर्णांक eCall = sqlite3_मूल्य_int(argv[1]);
  निश्चित( argc==2 );
  निश्चित( eCall==STAT_GET_STAT1 || eCall==STAT_GET_NEQ 
       || eCall==STAT_GET_ROWID || eCall==STAT_GET_NLT
       || eCall==STAT_GET_NDLT 
  );
  निश्चित( eCall==STAT_GET_STAT1 || p->mxSample );
  अगर( eCall==STAT_GET_STAT1 )
#अन्यथा
  निश्चित( argc==1 );
#पूर्ण_अगर
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
    पूर्णांक i;               /* Loop counter */

    sqlite3StrAccumInit(&sStat, 0, 0, 0, (p->n_कुंजी_Col+1)*100);
    sqlite3_str_appendf(&sStat, "%llu", 
        p->nSkipAhead ? (u64)p->nEst : (u64)p->nRow);
    क्रम(i=0; i<p->n_कुंजी_Col; i++){
      u64 nDistinct = p->current.anDLt[i] + 1;
      u64 iVal = (p->nRow + nDistinct - 1) / nDistinct;
      अगर( iVal==2 && p->nRow*10 <= nDistinct*11 ) iVal = 1;
      sqlite3_str_appendf(&sStat, " %llu", iVal);
#अगर_घोषित SQLITE_ENABLE_STAT4
      निश्चित( p->current.anEq[i] || p->nRow==0 );
#पूर्ण_अगर
    }
    sqlite3_परिणाम_str(context, &sStat, SQLITE_XFER);
  }
#अगर_घोषित SQLITE_ENABLE_STAT4
  अन्यथा अगर( eCall==STAT_GET_ROWID ){
    अगर( p->iGet<0 ){
      sample_धकेलो_Previous(p, 0);
      p->iGet = 0;
    }
    अगर( p->iGet<p->nSample ){
      StatSample *pS = p->a + p->iGet;
      अगर( pS->nRowid==0 ){
        sqlite3_परिणाम_int64(context, pS->u.iRowid);
      }अन्यथा{
        sqlite3_परिणाम_blob(context, pS->u.aRowid, pS->nRowid,
                            SQLITE_TRANSIENT);
      }
    }
  }अन्यथा{
    tRowcnt *aCnt = 0;
    sqlite3_str sStat;
    पूर्णांक i;

    निश्चित( p->iGet<p->nSample );
    चयन( eCall ){
      हाल STAT_GET_NEQ:  aCnt = p->a[p->iGet].anEq; अवरोध;
      हाल STAT_GET_NLT:  aCnt = p->a[p->iGet].anLt; अवरोध;
      शेष: {
        aCnt = p->a[p->iGet].anDLt; 
        p->iGet++;
        अवरोध;
      }
    }
    sqlite3StrAccumInit(&sStat, 0, 0, 0, p->nCol*100);
    क्रम(i=0; i<p->nCol; i++){
      sqlite3_str_appendf(&sStat, "%llu ", (u64)aCnt[i]);
    }
    अगर( sStat.nChar ) sStat.nChar--;
    sqlite3_परिणाम_str(context, &sStat, SQLITE_XFER);
  }
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */
#अगर_अघोषित SQLITE_DEBUG
  UNUSED_PARAMETER( argc );
#पूर्ण_अगर
}
अटल स्थिर FuncDef statGetFuncdef = {
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

अटल व्योम callStatGet(Parse *pParse, पूर्णांक regStat, पूर्णांक iParam, पूर्णांक regOut){
#अगर_घोषित SQLITE_ENABLE_STAT4
  sqlite3VdbeAddOp2(pParse->pVdbe, OP_Integer, iParam, regStat+1);
#या_अगर SQLITE_DEBUG
  निश्चित( iParam==STAT_GET_STAT1 );
#अन्यथा
  UNUSED_PARAMETER( iParam );
#पूर्ण_अगर
  निश्चित( regOut!=regStat && regOut!=regStat+1 );
  sqlite3VdbeAddFunctionCall(pParse, 0, regStat, regOut, 1+IsStat4,
                             &statGetFuncdef, 0);
}

#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
/* Add a comment to the most recent VDBE opcode that is the name
** of the k-th column of the pIdx index.
*/
अटल व्योम analyze_Vdbe_Comment_सूचकांक_With_Column_Name(
  Vdbe *v,         /* Prepared statement under construction */
  सूचकांक *pIdx,     /* Index whose column is being loaded */
  पूर्णांक k            /* Which column index */
){
  पूर्णांक i;           /* Index of column in the table */
  निश्चित( k>=0 && k<pIdx->nColumn );
  i = pIdx->aiColumn[k];
  अगर( NEVER(i==XN_ROWID) ){
    VdbeComment((v,"%s.rowid",pIdx->zName));
  }अन्यथा अगर( i==XN_व्यंजक ){
    निश्चित( pIdx->b_Has_व्यंजक );
    VdbeComment((v,"%s.expr(%d)",pIdx->zName, k));
  }अन्यथा{
    VdbeComment((v,"%s.%s", pIdx->zName, pIdx->pTable->aCol[i].zCnName));
  }
}
#अन्यथा
# define analyze_Vdbe_Comment_सूचकांक_With_Column_Name(a,b,c)
#पूर्ण_अगर /* SQLITE_DEBUG */

/*
** Generate code to do an analysis of all indices associated with
** a single table.
*/
अटल व्योम analyzeOneTable(
  Parse *pParse,   /* Parser context */
  Table *pTab,     /* Table whose indices are to be analyzed */
  सूचकांक *pOnlyIdx, /* If not NULL, only analyze this one index */
  पूर्णांक iStatCur,    /* Index of VdbeCursor that writes the sqlite_stat1 table */
  पूर्णांक iMem,        /* Available memory locations begin here */
  पूर्णांक iTab         /* Next available cursor */
){
  sqlite3 *db = pParse->db;    /* Database handle */
  सूचकांक *pIdx;                 /* An index to being analyzed */
  पूर्णांक iIdxCur;                 /* Cursor open on index being analyzed */
  पूर्णांक iTabCur;                 /* Table cursor */
  Vdbe *v;                     /* The virtual machine being built up */
  पूर्णांक i;                       /* Loop counter */
  पूर्णांक jZeroRows = -1;          /* Jump from here if number of rows is zero */
  पूर्णांक iDb;                     /* Index of database containing pTab */
  u8 needTableCnt = 1;         /* True to count the table */
  पूर्णांक regNewRowid = iMem++;    /* Rowid for the inserted record */
  पूर्णांक regStat = iMem++;        /* Register to hold StatAccum object */
  पूर्णांक regChng = iMem++;        /* Index of changed index field */
  पूर्णांक regRowid = iMem++;       /* Rowid argument passed to stat_push() */
  पूर्णांक reg_अस्थायी = iMem++;        /* Temporary use register */
  पूर्णांक regTemp2 = iMem++;       /* Second temporary use register */
  पूर्णांक regTabname = iMem++;     /* Register containing table name */
  पूर्णांक regIdxname = iMem++;     /* Register containing index name */
  पूर्णांक regStat1 = iMem++;       /* Value for the stat column of sqlite_stat1 */
  पूर्णांक reg_पूर्व = iMem;          /* MUST BE LAST (see below) */
#अगर_घोषित SQLITE_ENABLE_STAT4
  पूर्णांक doOnce = 1;              /* Flag for a one-time computation */
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
  Table *pStat1 = 0;
#पूर्ण_अगर

  sqlite3TouchRegister(pParse, iMem);
  निश्चित( sqlite3NoTempsInRange(pParse, regNewRowid, iMem) );
  v = sqlite3GetVdbe(pParse);
  अगर( v==0 || NEVER(pTab==0) ){
    वापस;
  }
  अगर( !IsOrdinaryTable(pTab) ){
    /* Do not gather statistics on views or virtual tables */
    वापस;
  }
  अगर( sqlite3_strlike("sqlite\\_%", pTab->zName, '\\')==0 ){
    /* Do not gather statistics on system tables */
    वापस;
  }
  निश्चित( sqlite3BtreeHoldsAllMutexes(db) );
  iDb = sqlite3_Schema_To_सूचकांक(db, pTab->pSchema);
  निश्चित( iDb>=0 );
  निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  अगर( sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0,
      db->aDb[iDb].zDbSName ) ){
    वापस;
  }
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
  अगर( db->xPreUpdateCallback ){
    pStat1 = (Table*)sqlite3DbMallocZero(db, माप(Table) + 13);
    अगर( pStat1==0 ) वापस;
    pStat1->zName = (अक्षर*)&pStat1[1];
    स_नकल(pStat1->zName, "sqlite_stat1", 13);
    pStat1->nCol = 3;
    pStat1->iPKey = -1;
    sqlite3VdbeAddOp4(pParse->pVdbe, OP_Noop, 0, 0, 0,(अक्षर*)pStat1,P4_DYNAMIC);
  }
#पूर्ण_अगर

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

  क्रम(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
    पूर्णांक nCol;                     /* Number of columns in pIdx. "N" */
    पूर्णांक addrGotoEnd;               /* Address of "OP_Rewind iIdxCur" */
    पूर्णांक addr_अग्र_Row;              /* Address of "next_row:" */
    स्थिर अक्षर *zIdxName;         /* Name of the index */
    पूर्णांक nColTest;                 /* Number of columns to test for changes */

    अगर( pOnlyIdx && pOnlyIdx!=pIdx ) जारी;
    अगर( pIdx->pPartIdxWhere==0 ) needTableCnt = 0;
    अगर( !HasRowid(pTab) && Is_Primary_कुंजी_सूचकांक(pIdx) ){
      nCol = pIdx->n_कुंजी_Col;
      zIdxName = pTab->zName;
      nColTest = nCol - 1;
    }अन्यथा{
      nCol = pIdx->nColumn;
      zIdxName = pIdx->zName;
      nColTest = pIdx->uniqNotNull ? pIdx->n_कुंजी_Col-1 : nCol-1;
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
    sqlite3TouchRegister(pParse, reg_पूर्व+nColTest);

    /* Open a read-only cursor on the index being analyzed. */
    निश्चित( iDb==sqlite3_Schema_To_सूचकांक(db, pIdx->pSchema) );
    sqlite3VdbeAddOp3(v, OP_OpenRead, iIdxCur, pIdx->tnum, iDb);
    sqlite3_Vdbe_Set_P4_कुंजी_Info(pParse, pIdx);
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
    निश्चित( regTemp2==regStat+4 );
    sqlite3VdbeAddOp2(v, OP_Integer, db->nAnalysisLimit, regTemp2);

    /* Arguments to stat_init(): 
    **    (1) the number of columns in the index including the rowid
    **        (or for a WITHOUT ROWID table, the number of PK columns),
    **    (2) the number of columns in the key without the rowid/pk
    **    (3) estimated number of rows in the index. */
    sqlite3VdbeAddOp2(v, OP_Integer, nCol, regStat+1);
    निश्चित( regRowid==regStat+2 );
    sqlite3VdbeAddOp2(v, OP_Integer, pIdx->n_कुंजी_Col, regRowid);
    sqlite3VdbeAddOp3(v, OP_गिनती, iIdxCur, reg_अस्थायी,
                      OptimizationDisabled(db, SQLITE_Stat4));
    sqlite3VdbeAddFunctionCall(pParse, 0, regStat+1, regStat, 4,
                               &statInitFuncdef, 0);
    addrGotoEnd = sqlite3VdbeAddOp1(v, OP_Rewind, iIdxCur);
    VdbeCoverage(v);

    sqlite3VdbeAddOp2(v, OP_Integer, 0, regChng);
    addr_अग्र_Row = sqlite3VdbeCurrentAddr(v);

    अगर( nColTest>0 ){
      पूर्णांक endDistinctTest = sqlite3VdbeMakeLabel(pParse);
      पूर्णांक *aGotoChng;               /* Array of jump instruction addresses */
      aGotoChng = sqlite3DbMallocRawNN(db, माप(पूर्णांक)*nColTest);
      अगर( aGotoChng==0 ) जारी;

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
      addr_अग्र_Row = sqlite3VdbeCurrentAddr(v);
      अगर( nColTest==1 && pIdx->n_कुंजी_Col==1 && Is_Unique_सूचकांक(pIdx) ){
        /* For a single-column UNIQUE index, once we have found a non-NULL
        ** row, we know that all the rest will be distinct, so skip 
        ** subsequent distinctness tests. */
        sqlite3VdbeAddOp2(v, OP_NotNull, reg_पूर्व, endDistinctTest);
        VdbeCoverage(v);
      }
      क्रम(i=0; i<nColTest; i++){
        अक्षर *pColl = (अक्षर*)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3VdbeAddOp2(v, OP_Integer, i, regChng);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, i, reg_अस्थायी);
        analyze_Vdbe_Comment_सूचकांक_With_Column_Name(v,pIdx,i);
        aGotoChng[i] = 
        sqlite3VdbeAddOp4(v, OP_Ne, reg_अस्थायी, 0, reg_पूर्व+i, pColl, P4_COLLSEQ);
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
      sqlite3VdbeJumpHere(v, addr_अग्र_Row-1);
      क्रम(i=0; i<nColTest; i++){
        sqlite3VdbeJumpHere(v, aGotoChng[i]);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, i, reg_पूर्व+i);
        analyze_Vdbe_Comment_सूचकांक_With_Column_Name(v,pIdx,i);
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
#अगर_घोषित SQLITE_ENABLE_STAT4
    अगर( OptimizationEnabled(db, SQLITE_Stat4) ){
      निश्चित( regRowid==(regStat+2) );
      अगर( HasRowid(pTab) ){
        sqlite3VdbeAddOp2(v, OP_IdxRowid, iIdxCur, regRowid);
      }अन्यथा{
        सूचकांक *pPk = sqlite3_Primary_कुंजी_सूचकांक(pIdx->pTable);
        पूर्णांक j, k, reg_कुंजी;
        reg_कुंजी = sqlite3_Get_अस्थायी_Range(pParse, pPk->n_कुंजी_Col);
        क्रम(j=0; j<pPk->n_कुंजी_Col; j++){
          k = sqlite3_Table_Column_To_सूचकांक(pIdx, pPk->aiColumn[j]);
          निश्चित( k>=0 && k<pIdx->nColumn );
          sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, k, reg_कुंजी+j);
          analyze_Vdbe_Comment_सूचकांक_With_Column_Name(v,pIdx,k);
        }
        sqlite3VdbeAddOp3(v, OP_MakeRecord, reg_कुंजी, pPk->n_कुंजी_Col, regRowid);
        sqlite3_Release_अस्थायी_Range(pParse, reg_कुंजी, pPk->n_कुंजी_Col);
      }
    }
#पूर्ण_अगर
    निश्चित( regChng==(regStat+1) );
    {
      sqlite3VdbeAddFunctionCall(pParse, 1, regStat, reg_अस्थायी, 2+IsStat4,
                                 &stat_धकेलो_Funcdef, 0);
      अगर( db->nAnalysisLimit ){
        पूर्णांक j1, j2, j3;
        j1 = sqlite3VdbeAddOp1(v, OP_IsNull, reg_अस्थायी); VdbeCoverage(v);
        j2 = sqlite3VdbeAddOp1(v, OP_If, reg_अस्थायी); VdbeCoverage(v);
        j3 = sqlite3VdbeAddOp4Int(v, OP_SeekGT, iIdxCur, 0, reg_पूर्व, 1);
        VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, j1);
        sqlite3VdbeAddOp2(v, OP_अग्र, iIdxCur, addr_अग्र_Row); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, j2);
        sqlite3VdbeJumpHere(v, j3);
      }अन्यथा{
        sqlite3VdbeAddOp2(v, OP_अग्र, iIdxCur, addr_अग्र_Row); VdbeCoverage(v);
      }
    }

    /* Add the entry to the stat1 table. */
    अगर( pIdx->pPartIdxWhere ){
      /* Partial indexes might get a zero-entry in sqlite_stat1.  But
      ** an empty table is omitted from sqlite_stat1. */
      sqlite3VdbeJumpHere(v, addrGotoEnd);
      addrGotoEnd = 0;
    }
    callStatGet(pParse, regStat, STAT_GET_STAT1, regStat1);
    निश्चित( "BBB"[0]==SQLITE_AFF_TEXT );
    sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 3, reg_अस्थायी, "BBB", 0);
    sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3VdbeAddOp3(v, OP_जोड़ो, iStatCur, reg_अस्थायी, regNewRowid);
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (अक्षर*)pStat1, P4_TABLE);
#पूर्ण_अगर
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);

    /* Add the entries to the stat4 table. */
#अगर_घोषित SQLITE_ENABLE_STAT4
    अगर( OptimizationEnabled(db, SQLITE_Stat4) && db->nAnalysisLimit==0 ){
      पूर्णांक regEq = regStat1;
      पूर्णांक regLt = regStat1+1;
      पूर्णांक regDLt = regStat1+2;
      पूर्णांक regSample = regStat1+3;
      पूर्णांक regCol = regStat1+4;
      पूर्णांक regSampleRowid = regCol + nCol;
      पूर्णांक addr_अग्र;
      पूर्णांक addrIsNull;
      u8 seekOp = HasRowid(pTab) ? OP_NotExists : OP_NotFound;

      /* No STAT4 data is generated if the number of rows is zero */
      अगर( addrGotoEnd==0 ){
        sqlite3VdbeAddOp2(v, OP_Cast, regStat1, SQLITE_AFF_INTEGER);
        addrGotoEnd = sqlite3VdbeAddOp1(v, OP_IfNot, regStat1);
        VdbeCoverage(v);
      }

      अगर( doOnce ){
        पूर्णांक mxCol = nCol;
        सूचकांक *pX;

        /* Compute the maximum number of columns in any index */
        क्रम(pX=pTab->p_सूचकांक; pX; pX=pX->p_अग्र){
          पूर्णांक nColX;                     /* Number of columns in pX */
          अगर( !HasRowid(pTab) && Is_Primary_कुंजी_सूचकांक(pX) ){
            nColX = pX->n_कुंजी_Col;
          }अन्यथा{
            nColX = pX->nColumn;
          }
          अगर( nColX>mxCol ) mxCol = nColX;
        }

        /* Allocate space to compute results for the largest index */
        sqlite3TouchRegister(pParse, regCol+mxCol);
        doOnce = 0;
#अगर_घोषित SQLITE_DEBUG
        /* Verify that the call to sqlite3ClearTempRegCache() below
        ** really is needed.
        ** https://sqlite.org/forum/forumpost/83cb4a95a0 (2023-03-25)
        */
        testcase( !sqlite3NoTempsInRange(pParse, regEq, regCol+mxCol) );
#पूर्ण_अगर
        sqlite3_Clear_अस्थायी_Reg_Cache(pParse);  /* tag-20230325-1 */
        निश्चित( sqlite3NoTempsInRange(pParse, regEq, regCol+mxCol) );
      }
      निश्चित( sqlite3NoTempsInRange(pParse, regEq, regCol+nCol) );

      addr_अग्र = sqlite3VdbeCurrentAddr(v);
      callStatGet(pParse, regStat, STAT_GET_ROWID, regSampleRowid);
      addrIsNull = sqlite3VdbeAddOp1(v, OP_IsNull, regSampleRowid);
      VdbeCoverage(v);
      callStatGet(pParse, regStat, STAT_GET_NEQ, regEq);
      callStatGet(pParse, regStat, STAT_GET_NLT, regLt);
      callStatGet(pParse, regStat, STAT_GET_NDLT, regDLt);
      sqlite3VdbeAddOp4Int(v, seekOp, iTabCur, addr_अग्र, regSampleRowid, 0);
      VdbeCoverage(v);
      क्रम(i=0; i<nCol; i++){
        sqlite3_व्यंजक_Code_Load_सूचकांक_Column(pParse, pIdx, iTabCur, i, regCol+i);
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regCol, nCol, regSample);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regTabname, 6, reg_अस्थायी);
      sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur+1, regNewRowid);
      sqlite3VdbeAddOp3(v, OP_जोड़ो, iStatCur+1, reg_अस्थायी, regNewRowid);
      sqlite3VdbeAddOp2(v, OP_Goto, 1, addr_अग्र); /* P1==1 for end-of-loop */
      sqlite3VdbeJumpHere(v, addrIsNull);
    }
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

    /* End of analysis */
    अगर( addrGotoEnd ) sqlite3VdbeJumpHere(v, addrGotoEnd);
  }


  /* Create a single sqlite_stat1 entry containing NULL as the index
  ** name and the row count as the content.
  */
  अगर( pOnlyIdx==0 && needTableCnt ){
    VdbeComment((v, "%s", pTab->zName));
    sqlite3VdbeAddOp2(v, OP_गिनती, iTabCur, regStat1);
    jZeroRows = sqlite3VdbeAddOp1(v, OP_IfNot, regStat1); VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Null, 0, regIdxname);
    निश्चित( "BBB"[0]==SQLITE_AFF_TEXT );
    sqlite3VdbeAddOp4(v, OP_MakeRecord, regTabname, 3, reg_अस्थायी, "BBB", 0);
    sqlite3VdbeAddOp2(v, OP_NewRowid, iStatCur, regNewRowid);
    sqlite3VdbeAddOp3(v, OP_जोड़ो, iStatCur, reg_अस्थायी, regNewRowid);
    sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
    sqlite3VdbeChangeP4(v, -1, (अक्षर*)pStat1, P4_TABLE);
#पूर्ण_अगर
    sqlite3VdbeJumpHere(v, jZeroRows);
  }
}


/*
** Generate code that will cause the most recent index analysis to
** be loaded into internal hash tables where is can be used.
*/
अटल व्योम loadAnalysis(Parse *pParse, पूर्णांक iDb){
  Vdbe *v = sqlite3GetVdbe(pParse);
  अगर( v ){
    sqlite3VdbeAddOp1(v, OP_LoadAnalysis, iDb);
  }
}

/*
** Generate code that will do an analysis of an entire database
*/
अटल व्योम analyzeDatabase(Parse *pParse, पूर्णांक iDb){
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;    /* Schema of database iDb */
  HashElem *k;
  पूर्णांक iStatCur;
  पूर्णांक iMem;
  पूर्णांक iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  openStatTable(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem+1;
  iTab = pParse->nTab;
  निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
  क्रम(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqlite_Hash_अग्र(k)){
    Table *pTab = (Table*)sqlite_Hash_तथ्य(k);
    analyzeOneTable(pParse, pTab, 0, iStatCur, iMem, iTab);
#अगर_घोषित SQLITE_ENABLE_STAT4
    iMem = sqlite3FirstAvailableRegister(pParse, iMem);
#अन्यथा
    निश्चित( iMem==sqlite3FirstAvailableRegister(pParse,iMem) );
#पूर्ण_अगर
  }
  loadAnalysis(pParse, iDb);
}

/*
** Generate code that will do an analysis of a single table in
** a database.  If pOnlyIdx is not NULL then it is a single index
** in pTab that should be analyzed.
*/
अटल व्योम analyzeTable(Parse *pParse, Table *pTab, सूचकांक *pOnlyIdx){
  पूर्णांक iDb;
  पूर्णांक iStatCur;

  निश्चित( pTab!=0 );
  निश्चित( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  iDb = sqlite3_Schema_To_सूचकांक(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  अगर( pOnlyIdx ){
    openStatTable(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  }अन्यथा{
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
व्योम sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2){
  sqlite3 *db = pParse->db;
  पूर्णांक iDb;
  पूर्णांक i;
  अक्षर *z, *zDb;
  Table *pTab;
  सूचकांक *pIdx;
  Token *pTableName;
  Vdbe *v;

  /* Read the database schema. If an error occurs, leave an error message
  ** and code in pParse and return NULL. */
  निश्चित( sqlite3BtreeHoldsAllMutexes(pParse->db) );
  अगर( SQLITE_OK!=sqlite3ReadSchema(pParse) ){
    वापस;
  }

  निश्चित( pName2!=0 || pName1==0 );
  अगर( pName1==0 ){
    /* Form 1:  Analyze everything */
    क्रम(i=0; i<db->nDb; i++){
      अगर( i==1 ) जारी;  /* Do not analyze the TEMP database */
      analyzeDatabase(pParse, i);
    }
  }अन्यथा अगर( pName2->n==0 && (iDb = sqlite3FindDb(db, pName1))>=0 ){
    /* Analyze the schema named as the argument */
    analyzeDatabase(pParse, iDb);
  }अन्यथा{
    /* Form 3: Analyze the table or index named as an argument */
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pTableName);
    अगर( iDb>=0 ){
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, pTableName);
      अगर( z ){
        अगर( (pIdx = sqlite3_Find_सूचकांक(db, z, zDb))!=0 ){
          analyzeTable(pParse, pIdx->pTable, pIdx);
        }अन्यथा अगर( (pTab = sqlite3LocateTable(pParse, 0, z, zDb))!=0 ){
          analyzeTable(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  अगर( db->nSqlExec==0 && (v = sqlite3GetVdbe(pParse))!=0 ){
    sqlite3VdbeAddOp0(v, OP_Expire);
  }
}

/*
** Used to pass information from the analyzer reader through to the
** callback routine.
*/
प्रकार काष्ठा analysisInfo analysisInfo;
काष्ठा analysisInfo {
  sqlite3 *db;
  स्थिर अक्षर *zDatabase;
};

/*
** The first argument points to a nul-terminated string containing a
** list of space separated integers. Read the first nOut of these into
** the array aOut[].
*/
अटल व्योम decodeIntArray(
  अक्षर *zIntArray,       /* String containing int array to decode */
  पूर्णांक nOut,              /* Number of slots in aOut[] */
  tRowcnt *aOut,         /* Store integers here */
  LogEst *aLog,          /* Or, if aOut==0, here */
  सूचकांक *p_सूचकांक          /* Handle extra flags for this index, if not NULL */
){
  अक्षर *z = zIntArray;
  पूर्णांक c;
  पूर्णांक i;
  tRowcnt v;

#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( z==0 ) z = "";
#अन्यथा
  निश्चित( z!=0 );
#पूर्ण_अगर
  क्रम(i=0; *z && i<nOut; i++){
    v = 0;
    जबतक( (c=z[0])>='0' && c<='9' ){
      v = v*10 + c - '0';
      z++;
    }
#अगर_घोषित SQLITE_ENABLE_STAT4
    अगर( aOut ) aOut[i] = v;
    अगर( aLog ) aLog[i] = sqlite3LogEst(v);
#अन्यथा
    निश्चित( aOut==0 );
    UNUSED_PARAMETER(aOut);
    निश्चित( aLog!=0 );
    aLog[i] = sqlite3LogEst(v);
#पूर्ण_अगर
    अगर( *z==' ' ) z++;
  }
#अगर_अघोषित SQLITE_ENABLE_STAT4
  निश्चित( p_सूचकांक!=0 ); {
#अन्यथा
  अगर( p_सूचकांक ){
#पूर्ण_अगर
    p_सूचकांक->bUnordered = 0;
    p_सूचकांक->noSkipScan = 0;
    जबतक( z[0] ){
      अगर( sqlite3_strglob("unordered*", z)==0 ){
        p_सूचकांक->bUnordered = 1;
      }अन्यथा अगर( sqlite3_strglob("sz=[0-9]*", z)==0 ){
        पूर्णांक sz = sqlite3Atoi(z+3);
        अगर( sz<2 ) sz = 2;
        p_सूचकांक->szIdxRow = sqlite3LogEst(sz);
      }अन्यथा अगर( sqlite3_strglob("noskipscan*", z)==0 ){
        p_सूचकांक->noSkipScan = 1;
      }
#अगर_घोषित SQLITE_ENABLE_COSTMULT
      अन्यथा अगर( sqlite3_strglob("costmult=[0-9]*",z)==0 ){
        p_सूचकांक->pTable->costMult = sqlite3LogEst(sqlite3Atoi(z+9));
      }
#पूर्ण_अगर
      जबतक( z[0]!=0 && z[0]!=' ' ) z++;
      जबतक( z[0]==' ' ) z++;
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
अटल पूर्णांक analysisLoader(व्योम *p_तथ्य, पूर्णांक argc, अक्षर **argv, अक्षर **NotUsed){
  analysisInfo *pInfo = (analysisInfo*)p_तथ्य;
  सूचकांक *p_सूचकांक;
  Table *pTable;
  स्थिर अक्षर *z;

  निश्चित( argc==3 );
  UNUSED_PARAMETER2(NotUsed, argc);

  अगर( argv==0 || argv[0]==0 || argv[2]==0 ){
    वापस 0;
  }
  pTable = sqlite3FindTable(pInfo->db, argv[0], pInfo->zDatabase);
  अगर( pTable==0 ){
    वापस 0;
  }
  अगर( argv[1]==0 ){
    p_सूचकांक = 0;
  }अन्यथा अगर( sqlite3_stricmp(argv[0],argv[1])==0 ){
    p_सूचकांक = sqlite3_Primary_कुंजी_सूचकांक(pTable);
  }अन्यथा{
    p_सूचकांक = sqlite3_Find_सूचकांक(pInfo->db, argv[1], pInfo->zDatabase);
  }
  z = argv[2];

  अगर( p_सूचकांक ){
    tRowcnt *aiRowEst = 0;
    पूर्णांक nCol = p_सूचकांक->n_कुंजी_Col+1;
#अगर_घोषित SQLITE_ENABLE_STAT4
    /* Index.aiRowEst may already be set here if there are duplicate 
    ** sqlite_stat1 entries for this index. In that case just clobber
    ** the old data with the new instead of allocating a new array.  */
    अगर( p_सूचकांक->aiRowEst==0 ){
      p_सूचकांक->aiRowEst = (tRowcnt*)sqlite3MallocZero(माप(tRowcnt) * nCol);
      अगर( p_सूचकांक->aiRowEst==0 ) sqlite3OomFault(pInfo->db);
    }
    aiRowEst = p_सूचकांक->aiRowEst;
#पूर्ण_अगर
    p_सूचकांक->bUnordered = 0;
    decodeIntArray((अक्षर*)z, nCol, aiRowEst, p_सूचकांक->aiRowLogEst, p_सूचकांक);
    p_सूचकांक->hasStat1 = 1;
    अगर( p_सूचकांक->pPartIdxWhere==0 ){
      pTable->nRowLogEst = p_सूचकांक->aiRowLogEst[0];
      pTable->tabFlags |= TF_HasStat1;
    }
  }अन्यथा{
    सूचकांक fakeIdx;
    fakeIdx.szIdxRow = pTable->szTabRow;
#अगर_घोषित SQLITE_ENABLE_COSTMULT
    fakeIdx.pTable = pTable;
#पूर्ण_अगर
    decodeIntArray((अक्षर*)z, 1, 0, &pTable->nRowLogEst, &fakeIdx);
    pTable->szTabRow = fakeIdx.szIdxRow;
    pTable->tabFlags |= TF_HasStat1;
  }

  वापस 0;
}

/*
** If the Index.aSample variable is not NULL, delete the aSample[] array
** and its contents.
*/
व्योम sqlite3_Delete_सूचकांक_Samples(sqlite3 *db, सूचकांक *pIdx){
  निश्चित( db!=0 );
  निश्चित( pIdx!=0 );
#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( pIdx->aSample ){
    पूर्णांक j;
    क्रम(j=0; j<pIdx->nSample; j++){
      सूचकांक_Sample *p = &pIdx->aSample[j];
      sqlite3DbFree(db, p->p);
    }
    sqlite3DbFree(db, pIdx->aSample);
  }
  अगर( db->pnBytesFreed==0 ){
    pIdx->nSample = 0;
    pIdx->aSample = 0;
  }
#अन्यथा
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(pIdx);
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */
}

#अगर_घोषित SQLITE_ENABLE_STAT4
/*
** Populate the pIdx->aAvgEq[] array based on the samples currently
** stored in pIdx->aSample[]. 
*/
अटल व्योम initAvgEq(सूचकांक *pIdx){
  अगर( pIdx ){
    सूचकांक_Sample *aSample = pIdx->aSample;
    सूचकांक_Sample *pFinal = &aSample[pIdx->nSample-1];
    पूर्णांक iCol;
    पूर्णांक nCol = 1;
    अगर( pIdx->nSampleCol>1 ){
      /* If this is stat4 data, then calculate aAvgEq[] values for all
      ** sample columns except the last. The last is always set to 1, as
      ** once the trailing PK fields are considered all index keys are
      ** unique.  */
      nCol = pIdx->nSampleCol-1;
      pIdx->aAvgEq[nCol] = 1;
    }
    क्रम(iCol=0; iCol<nCol; iCol++){
      पूर्णांक nSample = pIdx->nSample;
      पूर्णांक i;                    /* Used to iterate through samples */
      tRowcnt योग_Eq = 0;        /* Sum of the nEq values */
      tRowcnt avgEq = 0;
      tRowcnt nRow;             /* Number of rows in index */
      i64 nSum100 = 0;          /* Number of terms contributing to sumEq */
      i64 nDist100;             /* Number of distinct values in index */

      अगर( !pIdx->aiRowEst || iCol>=pIdx->n_कुंजी_Col || pIdx->aiRowEst[iCol+1]==0 ){
        nRow = pFinal->anLt[iCol];
        nDist100 = (i64)100 * pFinal->anDLt[iCol];
        nSample--;
      }अन्यथा{
        nRow = pIdx->aiRowEst[0];
        nDist100 = ((i64)100 * pIdx->aiRowEst[0]) / pIdx->aiRowEst[iCol+1];
      }
      pIdx->nRowEst0 = nRow;

      /* Set nSum to the number of distinct (iCol+1) field prefixes that
      ** occur in the stat4 table for this index. Set sumEq to the sum of 
      ** the nEq values for column iCol for the same set (adding the value 
      ** only once where there exist duplicate prefixes).  */
      क्रम(i=0; i<nSample; i++){
        अगर( i==(pIdx->nSample-1)
         || aSample[i].anDLt[iCol]!=aSample[i+1].anDLt[iCol] 
        ){
          योग_Eq += aSample[i].anEq[iCol];
          nSum100 += 100;
        }
      }

      अगर( nDist100>nSum100 && योग_Eq<nRow ){
        avgEq = ((i64)100 * (nRow - योग_Eq))/(nDist100 - nSum100);
      }
      अगर( avgEq==0 ) avgEq = 1;
      pIdx->aAvgEq[iCol] = avgEq;
    }
  }
}

/*
** Look up an index by name.  Or, if the name of a WITHOUT ROWID table
** is supplied instead, find the PRIMARY KEY index for that table.
*/
अटल सूचकांक *find_सूचकांक_Or_Primary_कुंजी(
  sqlite3 *db,
  स्थिर अक्षर *zName,
  स्थिर अक्षर *zDb
){
  सूचकांक *pIdx = sqlite3_Find_सूचकांक(db, zName, zDb);
  अगर( pIdx==0 ){
    Table *pTab = sqlite3FindTable(db, zName, zDb);
    अगर( pTab && !HasRowid(pTab) ) pIdx = sqlite3_Primary_कुंजी_सूचकांक(pTab);
  }
  वापस pIdx;
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
अटल पूर्णांक loadStatTbl(
  sqlite3 *db,                  /* Database handle */
  स्थिर अक्षर *zSql1,            /* SQL statement 1 (see above) */
  स्थिर अक्षर *zSql2,            /* SQL statement 2 (see above) */
  स्थिर अक्षर *zDb               /* Database name (e.g. "main") */
){
  पूर्णांक rc;                       /* Result codes from subroutines */
  sqlite3_stmt *pStmt = 0;      /* An SQL statement being run */
  अक्षर *zSql;                   /* Text of the SQL statement */
  सूचकांक *p_पूर्व_Idx = 0;          /* Previous index in the loop */
  सूचकांक_Sample *pSample;         /* A slot in pIdx->aSample[] */

  निश्चित( db->lookaside.bDisable );
  zSql = sqlite3MPrintf(db, zSql1, zDb);
  अगर( !zSql ){
    वापस SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  अगर( rc ) वापस rc;

  जबतक( sqlite3_step(pStmt)==SQLITE_ROW ){
    पूर्णांक nIdxCol = 1;              /* Number of columns in stat4 records */

    अक्षर *z_सूचकांक;    /* Index name */
    सूचकांक *pIdx;     /* Pointer to the index object */
    पूर्णांक nSample;     /* Number of samples */
    i64 nByte;       /* Bytes of space required */
    i64 i;           /* Bytes of space required */
    tRowcnt *pSpace; /* Available allocated memory space */
    u8 *pPtr;        /* Available memory as a u8 for easier manipulation */

    z_सूचकांक = (अक्षर *)sqlite3_column_text(pStmt, 0);
    अगर( z_सूचकांक==0 ) जारी;
    nSample = sqlite3_column_int(pStmt, 1);
    pIdx = find_सूचकांक_Or_Primary_कुंजी(db, z_सूचकांक, zDb);
    निश्चित( pIdx==0 || pIdx->nSample==0 );
    अगर( pIdx==0 ) जारी;
    अगर( pIdx->aSample!=0 ){
      /* The same index appears in sqlite_stat4 under multiple names */
      जारी;
    }
    निश्चित( !HasRowid(pIdx->pTable) || pIdx->nColumn==pIdx->n_कुंजी_Col+1 );
    अगर( !HasRowid(pIdx->pTable) && Is_Primary_कुंजी_सूचकांक(pIdx) ){
      nIdxCol = pIdx->n_कुंजी_Col;
    }अन्यथा{
      nIdxCol = pIdx->nColumn;
    }
    pIdx->nSampleCol = nIdxCol;
    pIdx->mxSample = nSample;
    nByte = ROUND8(sizeof64(सूचकांक_Sample) * nSample);
    nByte += sizeof64(tRowcnt) * nIdxCol * 3 * nSample;
    nByte += nIdxCol * sizeof64(tRowcnt);   /* Space for Index.aAvgEq[] */

    pIdx->aSample = sqlite3DbMallocZero(db, nByte);
    अगर( pIdx->aSample==0 ){
      sqlite3_finalize(pStmt);
      वापस SQLITE_NOMEM_BKPT;
    }
    pPtr = (u8*)pIdx->aSample;
    pPtr += ROUND8(nSample*sizeof64(pIdx->aSample[0]));
    pSpace = (tRowcnt*)pPtr;
    निश्चित( EIGHT_BYTE_ALIGNMENT( pSpace ) );
    pIdx->aAvgEq = pSpace; pSpace += nIdxCol;
    pIdx->pTable->tabFlags |= TF_HasStat4;
    क्रम(i=0; i<nSample; i++){
      pIdx->aSample[i].anEq = pSpace; pSpace += nIdxCol;
      pIdx->aSample[i].anLt = pSpace; pSpace += nIdxCol;
      pIdx->aSample[i].anDLt = pSpace; pSpace += nIdxCol;
    }
    निश्चित( ((u8*)pSpace)-nByte==(u8*)(pIdx->aSample) );
  }
  rc = sqlite3_finalize(pStmt);
  अगर( rc ) वापस rc;

  zSql = sqlite3MPrintf(db, zSql2, zDb);
  अगर( !zSql ){
    वापस SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
  sqlite3DbFree(db, zSql);
  अगर( rc ) वापस rc;

  जबतक( sqlite3_step(pStmt)==SQLITE_ROW ){
    अक्षर *z_सूचकांक;                 /* Index name */
    सूचकांक *pIdx;                  /* Pointer to the index object */
    पूर्णांक nCol = 1;                 /* Number of columns in index */

    z_सूचकांक = (अक्षर *)sqlite3_column_text(pStmt, 0);
    अगर( z_सूचकांक==0 ) जारी;
    pIdx = find_सूचकांक_Or_Primary_कुंजी(db, z_सूचकांक, zDb);
    अगर( pIdx==0 ) जारी;
    अगर( pIdx->nSample>=pIdx->mxSample ){
      /* Too many slots used because the same index appears in
      ** sqlite_stat4 using multiple names */
      जारी;
    }
    /* This next condition is true if data has already been loaded from 
    ** the sqlite_stat4 table. */
    nCol = pIdx->nSampleCol;
    अगर( pIdx!=p_पूर्व_Idx ){
      initAvgEq(p_पूर्व_Idx);
      p_पूर्व_Idx = pIdx;
    }
    pSample = &pIdx->aSample[pIdx->nSample];
    decodeIntArray((अक्षर*)sqlite3_column_text(pStmt,1),nCol,pSample->anEq,0,0);
    decodeIntArray((अक्षर*)sqlite3_column_text(pStmt,2),nCol,pSample->anLt,0,0);
    decodeIntArray((अक्षर*)sqlite3_column_text(pStmt,3),nCol,pSample->anDLt,0,0);

    /* Take a copy of the sample. Add 8 extra 0x00 bytes the end of the buffer.
    ** This is in case the sample record is corrupted. In that case, the
    ** sqlite3VdbeRecordCompare() may read up to two varints past the
    ** end of the allocated buffer before it realizes it is dealing with
    ** a corrupt record.  Or it might try to read a large integer from the
    ** buffer.  In any case, eight 0x00 bytes prevents this from causing
    ** a buffer overread.  */
    pSample->n = sqlite3_column_bytes(pStmt, 4);
    pSample->p = sqlite3DbMallocZero(db, pSample->n + 8);
    अगर( pSample->p==0 ){
      sqlite3_finalize(pStmt);
      वापस SQLITE_NOMEM_BKPT;
    }
    अगर( pSample->n ){
      स_नकल(pSample->p, sqlite3_column_blob(pStmt, 4), pSample->n);
    }
    pIdx->nSample++;
  }
  rc = sqlite3_finalize(pStmt);
  अगर( rc==SQLITE_OK ) initAvgEq(p_पूर्व_Idx);
  वापस rc;
}

/*
** Load content from the sqlite_stat4 table into 
** the Index.aSample[] arrays of all indices.
*/
अटल पूर्णांक loadStat4(sqlite3 *db, स्थिर अक्षर *zDb){
  पूर्णांक rc = SQLITE_OK;             /* Result codes from subroutines */
  स्थिर Table *pStat4;

  निश्चित( db->lookaside.bDisable );
  अगर( OptimizationEnabled(db, SQLITE_Stat4)
   && (pStat4 = sqlite3FindTable(db, "sqlite_stat4", zDb))!=0
   && IsOrdinaryTable(pStat4)
  ){
    rc = loadStatTbl(db,
      "SELECT idx,count(*) FROM %Q.sqlite_stat4 GROUP BY idx COLLATE nocase", 
      "SELECT idx,neq,nlt,ndlt,sample FROM %Q.sqlite_stat4",
      zDb
    );
  }
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_ENABLE_STAT4 */

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
पूर्णांक sqlite3AnalysisLoad(sqlite3 *db, पूर्णांक iDb){
  analysisInfo sInfo;
  HashElem *i;
  अक्षर *zSql;
  पूर्णांक rc = SQLITE_OK;
  Schema *pSchema = db->aDb[iDb].pSchema;
  स्थिर Table *pStat1;

  निश्चित( iDb>=0 && iDb<db->nDb );
  निश्चित( db->aDb[iDb].pBt!=0 );

  /* Clear any prior statistics */
  निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
  क्रम(i=sqliteHashFirst(&pSchema->tblHash); i; i=sqlite_Hash_अग्र(i)){
    Table *pTab = sqlite_Hash_तथ्य(i);
    pTab->tabFlags &= ~TF_HasStat1;
  }
  क्रम(i=sqliteHashFirst(&pSchema->idxHash); i; i=sqlite_Hash_अग्र(i)){
    सूचकांक *pIdx = sqlite_Hash_तथ्य(i);
    pIdx->hasStat1 = 0;
#अगर_घोषित SQLITE_ENABLE_STAT4
    sqlite3_Delete_सूचकांक_Samples(db, pIdx);
    pIdx->aSample = 0;
#पूर्ण_अगर
  }

  /* Load new statistics out of the sqlite_stat1 table */
  sInfo.db = db;
  sInfo.zDatabase = db->aDb[iDb].zDbSName;
  अगर( (pStat1 = sqlite3FindTable(db, "sqlite_stat1", sInfo.zDatabase))
   && IsOrdinaryTable(pStat1)
  ){
    zSql = sqlite3MPrintf(db, 
        "SELECT tbl,idx,stat FROM %Q.sqlite_stat1", sInfo.zDatabase);
    अगर( zSql==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }अन्यथा{
      rc = sqlite3_exec(db, zSql, analysisLoader, &sInfo, 0);
      sqlite3DbFree(db, zSql);
    }
  }

  /* Set appropriate defaults on all indexes not in the sqlite_stat1 table */
  निश्चित( sqlite3SchemaMutexHeld(db, iDb, 0) );
  क्रम(i=sqliteHashFirst(&pSchema->idxHash); i; i=sqlite_Hash_अग्र(i)){
    सूचकांक *pIdx = sqlite_Hash_तथ्य(i);
    अगर( !pIdx->hasStat1 ) sqlite3DefaultRowEst(pIdx);
  }

  /* Load the statistics from the sqlite_stat4 table. */
#अगर_घोषित SQLITE_ENABLE_STAT4
  अगर( rc==SQLITE_OK ){
    DisableLookaside;
    rc = loadStat4(db, sInfo.zDatabase);
    EnableLookaside;
  }
  क्रम(i=sqliteHashFirst(&pSchema->idxHash); i; i=sqlite_Hash_अग्र(i)){
    सूचकांक *pIdx = sqlite_Hash_तथ्य(i);
    sqlite3_free(pIdx->aiRowEst);
    pIdx->aiRowEst = 0;
  }
#पूर्ण_अगर

  अगर( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
  वापस rc;
}


#पूर्ण_अगर /* SQLITE_OMIT_ANALYZE */
