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

#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "vdbeInt.h"

#ifndef SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
வகைவரை கட்டமைப்பு Incrblob Incrblob;
கட்டமைப்பு Incrblob {
  முழு nByte;              /* Size of open blob, in bytes */
  முழு iOffset;            /* Byte offset of blob in cursor data */
  u16 iCol;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
  எழுத்து *zDb;              /* Database name */
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
நிலையான முழு blobSeekToRow(Incrblob *p, sqlite3_int64 iRow, எழுத்து **pzErr){
  முழு rc;                         /* Error code */
  எழுத்து *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of register r[1] in the SQL statement to integer iRow. 
  ** This is done directly as a performance optimization
  */
  sqlite3VdbeMemSetInt64(&v->aMem[1], iRow);

  /* If the statement has been run before (and is paused at the OP_ResultRow)
  ** then back it up to the point where it does the OP_NotExists.  This could
  ** have been down with an extra OP_Goto, but simply setting the program
  ** counter is faster. */
  எனில்( v->pc>4 ){
    v->pc = 4;
    ப்ரதிஜ்ஞா( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }இல்லையெனில்{
    rc = sqlite3_step(p->pStmt);
  }
  எனில்( rc==SQLITE_ROW ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    ப்ரதிஜ்ஞா( pC!=0 );
    ப்ரதிஜ்ஞா( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->iCol ? pC->aType[p->iCol] : 0;
    testcase( pC->nHdrParsed==p->iCol );
    testcase( pC->nHdrParsed==p->iCol+1 );
    எனில்( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }இல்லையெனில்{
      p->iOffset = pC->aType[p->iCol + pC->nField];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  எனில்( rc==SQLITE_ROW ){
    rc = SQLITE_OK;
  }இல்லையெனில் எனில்( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    எனில்( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", iRow);
      rc = SQLITE_ERROR;
    }இல்லையெனில்{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  ப்ரதிஜ்ஞா( rc!=SQLITE_OK || zErr==0 );
  ப்ரதிஜ்ஞா( rc!=SQLITE_ROW && rc!=SQLITE_DONE );

  *pzErr = zErr;
  திரும்பு rc;
}

/*
** Open a blob handle.
*/
முழு sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  மாறிலி எழுத்து *zDb,        /* The attached database containing the blob */
  மாறிலி எழுத்து *zTable,     /* The table containing the blob */
  மாறிலி எழுத்து *zColumn,    /* The column containing the blob */
  sqlite_int64 iRow,      /* The row containing the glob */
  முழு wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  முழு nAttempt = 0;
  முழு iCol;               /* Index of zColumn in row-record */
  முழு rc = SQLITE_OK;
  எழுத்து *zErr = 0;
  Table *pTab;
  Incrblob *pBlob = 0;
  முழு iDb;
  Parse sParse;

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( ppBlob==0 ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
#endif
  *ppBlob = 0;
#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) || zTable==0 || zColumn==0 ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
#endif
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, அளவு(Incrblob));
  போது(1){
    sqlite3ParseObjectInit(&sParse,db);
    எனில்( !pBlob ) செல் blob_open_out;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3LocateTable(&sParse, 0, zTable, zDb);
    எனில்( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_செய்தி(&sParse, "cannot open virtual table: %s", zTable);
    }
    எனில்( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_செய்தி(&sParse, "cannot open table without rowid: %s", zTable);
    }
    எனில்( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_செய்தி(&sParse, "cannot open table with generated columns: %s",
                      zTable);
    }
#ifndef SQLITE_OMIT_VIEW
    எனில்( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_செய்தி(&sParse, "cannot open view: %s", zTable);
    }
#endif
    எனில்( pTab==0
     || ((iDb = sqlite3SchemaToIndex(db, pTab->pSchema))==1 &&
         sqlite3OpenTempDatabase(&sParse))
    ){
      எனில்( sParse.z_Err_செய்தி ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_செய்தி;
        sParse.z_Err_செய்தி = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      செல் blob_open_out;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    iCol = sqlite3ColumnIndex(pTab, zColumn);
    எனில்( iCol<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", zColumn);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      செல் blob_open_out;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    எனில்( wrFlag ){
      மாறிலி எழுத்து *zFault = 0;
      Index *pIdx;
#ifndef SQLITE_OMIT_FOREIGN_சாவி
      எனில்( db->flags&SQLITE_Foreign_சாவிகள் ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        ப்ரதிஜ்ஞா( IsOrdinaryTable(pTab) );
        க்கு(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->pNextFrom){
          முழு j;
          க்கு(j=0; j<pFKey->nCol; j++){
            எனில்( pFKey->aCol[j].iFrom==iCol ){
              zFault = "foreign key";
            }
          }
        }
      }
#endif
      க்கு(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        முழு j;
        க்கு(j=0; j<pIdx->n_சாவி_Col; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          எனில்( pIdx->aiColumn[j]==iCol || pIdx->aiColumn[j]==XN_கோவை ){
            zFault = "indexed";
          }
        }
      }
      எனில்( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        செல் blob_open_out;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    ப்ரதிஜ்ஞா( pBlob->pStmt || db->mallocFailed );
    எனில்( pBlob->pStmt ){
      
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
      நிலையான மாறிலி முழு iLn = VDBE_OFFSET_LINENO(2);
      நிலையான மாறிலி VdbeOpList openBlob[] = {
        {OP_TableLock,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_Column,         0, 0, 1},  /* 3  */
        {OP_ResultRow,      1, 0, 0},  /* 4  */
        {OP_Halt,           0, 0, 0},  /* 5  */
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3VdbeAddOp4Int(v, OP_Transaction, iDb, wrFlag, 
                           pTab->pSchema->schema_cookie,
                           pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);
      ப்ரதிஜ்ஞா( sqlite3VdbeCurrentAddr(v)==2 || db->mallocFailed );
      aOp = sqlite3VdbeAddOpList(v, ArraySize(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3VdbeUsesBtree(v, iDb); 

      எனில்( db->mallocFailed==0 ){
        ப்ரதிஜ்ஞா( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#ifdef SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#இல்லையெனில்
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      எனில்( db->mallocFailed==0 ){
#endif

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        எனில்( wrFlag ) aOp[1].opcode = OP_OpenWrite;
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
    எனில்( db->mallocFailed ){
      செல் blob_open_out;
    }
    rc = blobSeekToRow(pBlob, iRow, &zErr);
    எனில்( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) நிறுத்து;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_out:
  எனில்( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }இல்லையெனில்{
    எனில்( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_செய்தி(db, rc, (zErr ? "%s" : (எழுத்து*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
முழு sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  முழு rc;
  sqlite3 *db;

  எனில்( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }இல்லையெனில்{
    rc = SQLITE_OK;
  }
  திரும்பு rc;
}

/*
** Perform a read or write operation on a blob
*/
நிலையான முழு blobReadWrite(
  sqlite3_blob *pBlob, 
  வெறுமை *z, 
  முழு n, 
  முழு iOffset, 
  முழு (*xCall)(BtCursor*, u32, u32, வெறுமை*)
){
  முழு rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  எனில்( p==0 ) திரும்பு SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  எனில்( n<0 || iOffset<0 || ((sqlite3_int64)iOffset+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }இல்லையெனில் எனில்( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }இல்லையெனில்{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    ப்ரதிஜ்ஞா( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
    எனில்( xCall==sqlite3BtreePutData && db->xPreUpdateCallback ){
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
      எனில்( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        முழு bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        ப்ரதிஜ்ஞா( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      எனில்( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_சாவி;
        i_சாவி = sqlite3_Btree_Integer_சாவி(p->pCsr);
        ப்ரதிஜ்ஞா( v->apCsr[0]!=0 );
        ப்ரதிஜ்ஞா( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_சாவி, -1, p->iCol
        );
      }
    }
    எனில்( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, iOffset+p->iOffset, n, z);
    }
#இல்லையெனில்
    rc = xCall(p->pCsr, iOffset+p->iOffset, n, z);
#endif

    sqlite3BtreeLeaveCursor(p->pCsr);
    எனில்( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }இல்லையெனில்{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

/*
** Read data from a blob handle.
*/
முழு sqlite3_blob_read(sqlite3_blob *pBlob, வெறுமை *z, முழு n, முழு iOffset){
  திரும்பு blobReadWrite(pBlob, z, n, iOffset, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
முழு sqlite3_blob_write(sqlite3_blob *pBlob, மாறிலி வெறுமை *z, முழு n, முழு iOffset){
  திரும்பு blobReadWrite(pBlob, (வெறுமை *)z, n, iOffset, sqlite3BtreePutData);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
முழு sqlite3_blob_bytes(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  திரும்பு (p && p->pStmt) ? p->nByte : 0;
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
முழு sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 iRow){
  முழு rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  எனில்( p==0 ) திரும்பு SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  எனில்( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }இல்லையெனில்{
    எழுத்து *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blobSeekToRow(p, iRow, &zErr);
    எனில்( rc!=SQLITE_OK ){
      sqlite3_Error_With_செய்தி(db, rc, (zErr ? "%s" : (எழுத்து*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    ப்ரதிஜ்ஞா( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  ப்ரதிஜ்ஞா( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

#endif /* #ifndef SQLITE_OMIT_INCRBLOB */
