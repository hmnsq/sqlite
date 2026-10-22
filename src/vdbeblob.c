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

#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "vdbeInt.h"

#ifndef SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
প্রকারলেখ কাঠামো Incrblob Incrblob;
কাঠামো Incrblob {
  পূর্ণসংখ্যা nByte;              /* Size of open blob, in bytes */
  পূর্ণসংখ্যা iOffset;            /* Byte offset of blob in cursor data */
  u16 iCol;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
  অক্ষর *zDb;              /* Database name */
  Table *pTab;            /* Table object */
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
স্থির পূর্ণসংখ্যা blobSeekToRow(Incrblob *p, sqlite3_int64 iRow, অক্ষর **pzErr){
  পূর্ণসংখ্যা rc;                         /* Error code */
  অক্ষর *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of register r[1] in the SQL statement to integer iRow. 
  ** This is done directly as a performance optimization
  */
  sqlite3VdbeMemSetInt64(&v->aMem[1], iRow);

  /* If the statement has been run before (and is paused at the OP_ResultRow)
  ** then back it up to the point where it does the OP_NotExists.  This could
  ** have been down with an extra OP_Goto, but simply setting the program
  ** counter is faster. */
  যদি( v->pc>4 ){
    v->pc = 4;
    প্রতিজ্ঞা( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }অন্যথা{
    rc = sqlite3_step(p->pStmt);
  }
  যদি( rc==SQLITE_ROW ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    প্রতিজ্ঞা( pC!=0 );
    প্রতিজ্ঞা( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->iCol ? pC->aType[p->iCol] : 0;
    testcase( pC->nHdrParsed==p->iCol );
    testcase( pC->nHdrParsed==p->iCol+1 );
    যদি( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }অন্যথা{
      p->iOffset = pC->aType[p->iCol + pC->nField];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  যদি( rc==SQLITE_ROW ){
    rc = SQLITE_OK;
  }অন্যথা যদি( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    যদি( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", iRow);
      rc = SQLITE_ERROR;
    }অন্যথা{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  প্রতিজ্ঞা( rc!=SQLITE_OK || zErr==0 );
  প্রতিজ্ঞা( rc!=SQLITE_ROW && rc!=SQLITE_DONE );

  *pzErr = zErr;
  ফেরত rc;
}

/*
** Open a blob handle.
*/
পূর্ণসংখ্যা sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  ধ্রুবক অক্ষর *zDb,        /* The attached database containing the blob */
  ধ্রুবক অক্ষর *zTable,     /* The table containing the blob */
  ধ্রুবক অক্ষর *zColumn,    /* The column containing the blob */
  sqlite_int64 iRow,      /* The row containing the glob */
  পূর্ণসংখ্যা wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  পূর্ণসংখ্যা nAttempt = 0;
  পূর্ণসংখ্যা iCol;               /* Index of zColumn in row-record */
  পূর্ণসংখ্যা rc = SQLITE_OK;
  অক্ষর *zErr = 0;
  Table *pTab;
  Incrblob *pBlob = 0;
  পূর্ণসংখ্যা iDb;
  Parse sParse;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( ppBlob==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
  *ppBlob = 0;
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) || zTable==0 || zColumn==0 ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
#endif
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, আকার(Incrblob));
  যখন(1){
    sqlite3ParseObjectInit(&sParse,db);
    যদি( !pBlob ) যাও blob_open_out;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3LocateTable(&sParse, 0, zTable, zDb);
    যদি( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_বার্তা(&sParse, "cannot open virtual table: %s", zTable);
    }
    যদি( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_বার্তা(&sParse, "cannot open table without rowid: %s", zTable);
    }
    যদি( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_বার্তা(&sParse, "cannot open table with generated columns: %s",
                      zTable);
    }
#ifndef SQLITE_OMIT_VIEW
    যদি( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_বার্তা(&sParse, "cannot open view: %s", zTable);
    }
#endif
    যদি( pTab==0
     || ((iDb = sqlite3_Schema_To_সূচক(db, pTab->pSchema))==1 &&
         sqlite3_Open_অস্থায়ী_Database(&sParse))
    ){
      যদি( sParse.z_Err_বার্তা ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_বার্তা;
        sParse.z_Err_বার্তা = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      যাও blob_open_out;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    iCol = sqlite3_Column_সূচক(pTab, zColumn);
    যদি( iCol<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", zColumn);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      যাও blob_open_out;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    যদি( wrFlag ){
      ধ্রুবক অক্ষর *zFault = 0;
      সূচক *pIdx;
#ifndef SQLITE_OMIT_FOREIGN_চাবি
      যদি( db->flags&SQLITE_Foreign_চাবিগুলি ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        প্রতিজ্ঞা( IsOrdinaryTable(pTab) );
        জন্য(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_পরবর্তী_From){
          পূর্ণসংখ্যা j;
          জন্য(j=0; j<pFKey->nCol; j++){
            যদি( pFKey->aCol[j].iFrom==iCol ){
              zFault = "foreign key";
            }
          }
        }
      }
#endif
      জন্য(pIdx=pTab->p_সূচক; pIdx; pIdx=pIdx->p_পরবর্তী){
        পূর্ণসংখ্যা j;
        জন্য(j=0; j<pIdx->n_চাবি_Col; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          যদি( pIdx->aiColumn[j]==iCol || pIdx->aiColumn[j]==XN_রাশিমালা ){
            zFault = "indexed";
          }
        }
      }
      যদি( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        যাও blob_open_out;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    প্রতিজ্ঞা( pBlob->pStmt || db->mallocFailed );
    যদি( pBlob->pStmt ){
      
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
      স্থির ধ্রুবক পূর্ণসংখ্যা iLn = VDBE_OFFSET_LINENO(2);
      স্থির ধ্রুবক VdbeOpList openBlob[] = {
        {OP_TableLock,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_Column,         0, 0, 1},  /* 3  */
        {OP_ফলাফল_Row,      1, 0, 0},  /* 4  */
        {OP_Halt,           0, 0, 0},  /* 5  */
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3VdbeAddOp4Int(v, OP_Transaction, iDb, wrFlag, 
                           pTab->pSchema->schema_cookie,
                           pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);
      প্রতিজ্ঞা( sqlite3VdbeCurrentAddr(v)==2 || db->mallocFailed );
      aOp = sqlite3VdbeAddOpList(v, Array_আকার(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3VdbeUsesBtree(v, iDb); 

      যদি( db->mallocFailed==0 ){
        প্রতিজ্ঞা( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#ifdef SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#অন্যথা
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      যদি( db->mallocFailed==0 ){
#endif

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        যদি( wrFlag ) aOp[1].opcode = OP_OpenWrite;
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
        aOp[1].p4.i = pTab->nCol+1;
        aOp[3].p2 = pTab->nCol;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }
   
    pBlob->iCol = iCol;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    যদি( db->mallocFailed ){
      যাও blob_open_out;
    }
    rc = blobSeekToRow(pBlob, iRow, &zErr);
    যদি( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) বিরতি;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_out:
  যদি( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }অন্যথা{
    যদি( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_বার্তা(db, rc, (zErr ? "%s" : (অক্ষর*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
পূর্ণসংখ্যা sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  পূর্ণসংখ্যা rc;
  sqlite3 *db;

  যদি( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }অন্যথা{
    rc = SQLITE_OK;
  }
  ফেরত rc;
}

/*
** Perform a read or write operation on a blob
*/
স্থির পূর্ণসংখ্যা blobReadWrite(
  sqlite3_blob *pBlob, 
  শূন্য *z, 
  পূর্ণসংখ্যা n, 
  পূর্ণসংখ্যা iOffset, 
  পূর্ণসংখ্যা (*xCall)(BtCursor*, u32, u32, শূন্য*)
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  যদি( p==0 ) ফেরত SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  যদি( n<0 || iOffset<0 || ((sqlite3_int64)iOffset+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }অন্যথা যদি( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }অন্যথা{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    প্রতিজ্ঞা( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    যদি( xCall==sqlite3_Btree_Put_তথ্য && db->xPreUpdateCallback ){
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
      যদি( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        পূর্ণসংখ্যা bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        প্রতিজ্ঞা( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      যদি( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_চাবি;
        i_চাবি = sqlite3_Btree_Integer_চাবি(p->pCsr);
        প্রতিজ্ঞা( v->apCsr[0]!=0 );
        প্রতিজ্ঞা( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_চাবি, -1, p->iCol
        );
      }
    }
    যদি( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, iOffset+p->iOffset, n, z);
    }
#অন্যথা
    rc = xCall(p->pCsr, iOffset+p->iOffset, n, z);
#endif

    sqlite3BtreeLeaveCursor(p->pCsr);
    যদি( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }অন্যথা{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}

/*
** Read data from a blob handle.
*/
পূর্ণসংখ্যা sqlite3_blob_read(sqlite3_blob *pBlob, শূন্য *z, পূর্ণসংখ্যা n, পূর্ণসংখ্যা iOffset){
  ফেরত blobReadWrite(pBlob, z, n, iOffset, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
পূর্ণসংখ্যা sqlite3_blob_write(sqlite3_blob *pBlob, ধ্রুবক শূন্য *z, পূর্ণসংখ্যা n, পূর্ণসংখ্যা iOffset){
  ফেরত blobReadWrite(pBlob, (শূন্য *)z, n, iOffset, sqlite3_Btree_Put_তথ্য);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
পূর্ণসংখ্যা sqlite3_blob_bytes(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  ফেরত (p && p->pStmt) ? p->nByte : 0;
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
পূর্ণসংখ্যা sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 iRow){
  পূর্ণসংখ্যা rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  যদি( p==0 ) ফেরত SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  যদি( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }অন্যথা{
    অক্ষর *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blobSeekToRow(p, iRow, &zErr);
    যদি( rc!=SQLITE_OK ){
      sqlite3_Error_With_বার্তা(db, rc, (zErr ? "%s" : (অক্ষর*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    প্রতিজ্ঞা( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  প্রতিজ্ঞা( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}

#endif /* #ifndef SQLITE_OMIT_INCRBLOB */
