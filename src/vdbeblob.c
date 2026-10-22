/*
** 2007 May 1
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains code used to implement incremental BLOB I/O.
*/

#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
రకనిర్వచనం నిర్మాణం Incrblob Incrblob;
నిర్మాణం Incrblob {
  పూర్ణాంకం nByte;              /* Size of open blob, in bytes */
  పూర్ణాంకం i_స్థానభ్రంశం;            /* Byte offset of blob in cursor data */
  u16 i_నిలువు;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
  అక్షరం *zDb;              /* Database name */
  పట్టిక *pTab;            /* Table object */
};


/*
** This function is used by both blob_open() and blob_reopen(). It seeks
** the b-tree cursor associated with blob handle p to point to row iRow.
** If successful, SQLITE_OK is returned and subsequent calls to
** sqlite3_blob_read() or sqlite3_blob_write() access the specified row.
**
** If an error occurs, or if the specified row does not exist or does not
** contain a value of type TEXT or BLOB in the column nominated when the
** blob handle was opened, then an error code is returned and *pzErr may
** be set to point to a buffer containing an error message. It is the
** responsibility of the caller to free the error message buffer using
** sqlite3DbFree().
**
** If an error does occur, then the b-tree cursor is closed. All subsequent
** calls to sqlite3_blob_read(), blob_write() or blob_reopen() will 
** immediately return SQLITE_ABORT.
*/
స్థిర పూర్ణాంకం blob_Seek_గమ్యం_అడ్డువరుస(Incrblob *p, sqlite3_int64 i_అడ్డువరుస, అక్షరం **pzErr){
  పూర్ణాంకం rc;                         /* Error code */
  అక్షరం *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of register r[1] in the SQL statement to integer iRow. 
  ** This is done directly as a performance optimization
  */
  sqlite3_Vdbe_Mem_సమితి_Int64(&v->aMem[1], i_అడ్డువరుస);

  /* If the statement has been run before (and is paused at the OP_ResultRow)
  ** then back it up to the point where it does the OP_NotExists.  This could
  ** have been down with an extra OP_Goto, but simply setting the program
  ** counter is faster. */
  ఒకవేళ( v->pc>4 ){
    v->pc = 4;
    ప్రతిజ్ఞా( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }లేకపోతే{
    rc = sqlite3_అడుగు(p->pStmt);
  }
  ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    ప్రతిజ్ఞా( pC!=0 );
    ప్రతిజ్ఞా( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->i_నిలువు ? pC->aType[p->i_నిలువు] : 0;
    testcase( pC->nHdrParsed==p->i_నిలువు );
    testcase( pC->nHdrParsed==p->i_నిలువు+1 );
    ఒకవేళ( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }లేకపోతే{
      p->i_స్థానభ్రంశం = pC->aType[p->i_నిలువు + pC->n_క్షేత్రం];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
    rc = SQLITE_OK;
  }లేకపోతే ఒకవేళ( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    ఒకవేళ( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", i_అడ్డువరుస);
      rc = SQLITE_ERROR;
    }లేకపోతే{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  ప్రతిజ్ఞా( rc!=SQLITE_OK || zErr==0 );
  ప్రతిజ్ఞా( rc!=SQLITE_అడ్డువరుస && rc!=SQLITE_పూర్తి );

  *pzErr = zErr;
  తిరిగి rc;
}

/*
** Open a blob handle.
*/
పూర్ణాంకం sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  స్థిరరాశి అక్షరం *zDb,        /* The attached database containing the blob */
  స్థిరరాశి అక్షరం *z_పట్టిక,     /* The table containing the blob */
  స్థిరరాశి అక్షరం *z_స్తంభం,    /* The column containing the blob */
  sqlite_int64 i_అడ్డువరుస,      /* The row containing the glob */
  పూర్ణాంకం wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  పూర్ణాంకం nAttempt = 0;
  పూర్ణాంకం i_నిలువు;               /* Index of zColumn in row-record */
  పూర్ణాంకం rc = SQLITE_OK;
  అక్షరం *zErr = 0;
  పట్టిక *pTab;
  Incrblob *pBlob = 0;
  పూర్ణాంకం iDb;
  Parse sParse;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( ppBlob==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  *ppBlob = 0;
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) || z_పట్టిక==0 || z_స్తంభం==0 ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
#అయితే_ముగింపు
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, పరిమాణం(Incrblob));
  అయితే(1){
    sqlite3ParseObjectInit(&sParse,db);
    ఒకవేళ( !pBlob ) వెళ్ళు blob_open_బహిర్గతం;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3_Locate_పట్టిక(&sParse, 0, z_పట్టిక, zDb);
    ఒకవేళ( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_సందేశం(&sParse, "cannot open virtual table: %s", z_పట్టిక);
    }
    ఒకవేళ( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_సందేశం(&sParse, "cannot open table without rowid: %s", z_పట్టిక);
    }
    ఒకవేళ( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_సందేశం(&sParse, "cannot open table with generated columns: %s",
                      z_పట్టిక);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIEW
    ఒకవేళ( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_సందేశం(&sParse, "cannot open view: %s", z_పట్టిక);
    }
#అయితే_ముగింపు
    ఒకవేళ( pTab==0
     || ((iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema))==1 &&
         sqlite3_Open_తాత్కాలిక_Database(&sParse))
    ){
      ఒకవేళ( sParse.z_Err_సందేశం ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_సందేశం;
        sParse.z_Err_సందేశం = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      వెళ్ళు blob_open_బహిర్గతం;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    i_నిలువు = sqlite3_స్తంభం_సూచీ(pTab, z_స్తంభం);
    ఒకవేళ( i_నిలువు<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", z_స్తంభం);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      వెళ్ళు blob_open_బహిర్గతం;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    ఒకవేళ( wrFlag ){
      స్థిరరాశి అక్షరం *zFault = 0;
      సూచీ *pIdx;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
      ఒకవేళ( db->flags&SQLITE_Foreign_కీలకాలు ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        ప్రతిజ్ఞా( Is_Ordinary_పట్టిక(pTab) );
        కొరకు(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_తదుపరి_From){
          పూర్ణాంకం j;
          కొరకు(j=0; j<pFKey->n_నిలువు; j++){
            ఒకవేళ( pFKey->a_నిలువు[j].iFrom==i_నిలువు ){
              zFault = "foreign key";
            }
          }
        }
      }
#అయితే_ముగింపు
      కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
        పూర్ణాంకం j;
        కొరకు(j=0; j<pIdx->n_కీలకం_నిలువు; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          ఒకవేళ( pIdx->ai_స్తంభం[j]==i_నిలువు || pIdx->ai_స్తంభం[j]==XN_సమాసం ){
            zFault = "indexed";
          }
        }
      }
      ఒకవేళ( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        వెళ్ళు blob_open_బహిర్గతం;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    ప్రతిజ్ఞా( pBlob->pStmt || db->mallocFailed );
    ఒకవేళ( pBlob->pStmt ){
      
      /* This VDBE program seeks a btree cursor to the identified 
      ** db/table/row entry. The reason for using a vdbe program instead
      ** of writing code to use the b-tree layer directly is that the
      ** vdbe program will take advantage of the various transaction,
      ** locking and error handling infrastructure built into the vdbe.
      **
      ** After seeking the cursor, the vdbe executes an OP_ResultRow.
      ** Code external to the Vdbe then "borrows" the b-tree cursor and
      ** uses it to implement the blob_read(), blob_write() and 
      ** blob_bytes() functions.
      **
      ** The sqlite3_blob_close() function finalizes the vdbe program,
      ** which closes the b-tree cursor and (possibly) commits the 
      ** transaction.
      */
      స్థిర స్థిరరాశి పూర్ణాంకం iLn = VDBE_స్థానభ్రంశం_LINENO(2);
      స్థిర స్థిరరాశి VdbeOpList openBlob[] = {
        {OP_పట్టిక_తాళం,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_స్తంభం,         0, 0, 1},  /* 3  */
        {OP_ఫలితం_అడ్డువరుస,      1, 0, 0},  /* 4  */
        {OP_Halt,           0, 0, 0},  /* 5  */
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Transaction, iDb, wrFlag, 
                           pTab->pSchema->schema_cookie,
                           pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);
      ప్రతిజ్ఞా( sqlite3VdbeCurrentAddr(v)==2 || db->mallocFailed );
      aOp = sqlite3_Vdbe_కూడు_Op_List(v, Array_కొలత(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3_Vdbe_ఉపయోగిస్తుంది_Btree(v, iDb); 

      ఒకవేళ( db->mallocFailed==0 ){
        ప్రతిజ్ఞా( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#నిర్వచితమైతే SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#లేకపోతే
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      ఒకవేళ( db->mallocFailed==0 ){
#అయితే_ముగింపు

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        ఒకవేళ( wrFlag ) aOp[1].opcode = OP_OpenWrite;
        aOp[1].p2 = pTab->tnum;
        aOp[1].p3 = iDb;   

        /* Configure the number of columns. Configure the cursor to
        ** think that the table has one more column than it really
        ** does. An OP_Column to retrieve this imaginary column will
        ** always return an SQL NULL. This is useful because it means
        ** we can invoke OP_Column to fill in the vdbe cursors type 
        ** and offset cache without causing any IO.
        */
        aOp[1].p4type = P4_INT32;
        aOp[1].p4.i = pTab->n_నిలువు+1;
        aOp[3].p2 = pTab->n_నిలువు;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }
   
    pBlob->i_నిలువు = i_నిలువు;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    ఒకవేళ( db->mallocFailed ){
      వెళ్ళు blob_open_బహిర్గతం;
    }
    rc = blob_Seek_గమ్యం_అడ్డువరుస(pBlob, i_అడ్డువరుస, &zErr);
    ఒకవేళ( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) ఆపు;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_బహిర్గతం:
  ఒకవేళ( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }లేకపోతే{
    ఒకవేళ( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_సందేశం(db, rc, (zErr ? "%s" : (అక్షరం*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
పూర్ణాంకం sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  పూర్ణాంకం rc;
  sqlite3 *db;

  ఒకవేళ( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }లేకపోతే{
    rc = SQLITE_OK;
  }
  తిరిగి rc;
}

/*
** Perform a read or write operation on a blob
*/
స్థిర పూర్ణాంకం blobReadWrite(
  sqlite3_blob *pBlob, 
  శూన్య *z, 
  పూర్ణాంకం n, 
  పూర్ణాంకం i_స్థానభ్రంశం, 
  పూర్ణాంకం (*xCall)(BtCursor*, u32, u32, శూన్య*)
){
  పూర్ణాంకం rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  ఒకవేళ( p==0 ) తిరిగి SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  ఒకవేళ( n<0 || i_స్థానభ్రంశం<0 || ((sqlite3_int64)i_స్థానభ్రంశం+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }లేకపోతే ఒకవేళ( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }లేకపోతే{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    ప్రతిజ్ఞా( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
    ఒకవేళ( xCall==sqlite3_Btree_Put_దత్తాంశం && db->xPreUpdateCallback ){
      /* If a pre-update hook is registered and this is a write cursor, 
      ** invoke it here. 
      ** 
      ** TODO: The preupdate-hook is passed SQLITE_DELETE, even though this
      ** operation should really be an SQLITE_UPDATE. This is probably
      ** incorrect, but is convenient because at this point the new.* values 
      ** are not easily obtainable. And for the sessions module, an 
      ** SQLITE_UPDATE where the PK columns do not change is handled in the 
      ** same way as an SQLITE_DELETE (the SQLITE_DELETE code is actually
      ** slightly more efficient). Since you cannot write to a PK column
      ** using the incremental-blob API, this works. For the sessions module
      ** anyhow.
      */
      ఒకవేళ( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        పూర్ణాంకం bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        ప్రతిజ్ఞా( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      ఒకవేళ( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_కీలకం;
        i_కీలకం = sqlite3_Btree_Integer_కీలకం(p->pCsr);
        ప్రతిజ్ఞా( v->apCsr[0]!=0 );
        ప్రతిజ్ఞా( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_కీలకం, -1, p->i_నిలువు
        );
      }
    }
    ఒకవేళ( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, i_స్థానభ్రంశం+p->i_స్థానభ్రంశం, n, z);
    }
#లేకపోతే
    rc = xCall(p->pCsr, i_స్థానభ్రంశం+p->i_స్థానభ్రంశం, n, z);
#అయితే_ముగింపు

    sqlite3BtreeLeaveCursor(p->pCsr);
    ఒకవేళ( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }లేకపోతే{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Read data from a blob handle.
*/
పూర్ణాంకం sqlite3_blob_read(sqlite3_blob *pBlob, శూన్య *z, పూర్ణాంకం n, పూర్ణాంకం i_స్థానభ్రంశం){
  తిరిగి blobReadWrite(pBlob, z, n, i_స్థానభ్రంశం, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
పూర్ణాంకం sqlite3_blob_write(sqlite3_blob *pBlob, స్థిరరాశి శూన్య *z, పూర్ణాంకం n, పూర్ణాంకం i_స్థానభ్రంశం){
  తిరిగి blobReadWrite(pBlob, (శూన్య *)z, n, i_స్థానభ్రంశం, sqlite3_Btree_Put_దత్తాంశం);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
పూర్ణాంకం sqlite3_blob_బైట్లు(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  తిరిగి (p && p->pStmt) ? p->nByte : 0;
}

/*
** Move an existing blob handle to point to a different row of the same
** database table.
**
** If an error occurs, or if the specified row does not exist or does not
** contain a blob or text value, then an error code is returned and the
** database handle error code and message set. If this happens, then all 
** subsequent calls to sqlite3_blob_xxx() functions (except blob_close()) 
** immediately return SQLITE_ABORT.
*/
పూర్ణాంకం sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 i_అడ్డువరుస){
  పూర్ణాంకం rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  ఒకవేళ( p==0 ) తిరిగి SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  ఒకవేళ( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }లేకపోతే{
    అక్షరం *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blob_Seek_గమ్యం_అడ్డువరుస(p, i_అడ్డువరుస, &zErr);
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3_Error_With_సందేశం(db, rc, (zErr ? "%s" : (అక్షరం*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    ప్రతిజ్ఞా( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  ప్రతిజ్ఞా( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_INCRBLOB */
