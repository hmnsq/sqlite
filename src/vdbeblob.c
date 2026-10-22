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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Incrblob Incrblob;
ਬਣਤਰ Incrblob {
  ਪੂਰਨ nByte;              /* Size of open blob, in bytes */
  ਪੂਰਨ i_ਵਿੱਥ;            /* Byte offset of blob in cursor data */
  u16 i_ਖਾਨਾ;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
  ਅੱਖਰ *zDb;              /* Database name */
  ਸਾਰਣੀ *pTab;            /* Table object */
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
ਸਥਿਰ ਪੂਰਨ blob_Seek_ਮੰਜ਼ਿਲ_ਪੰਕਤੀ(Incrblob *p, sqlite3_int64 i_ਪੰਕਤੀ, ਅੱਖਰ **pzErr){
  ਪੂਰਨ rc;                         /* Error code */
  ਅੱਖਰ *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of register r[1] in the SQL statement to integer iRow. 
  ** This is done directly as a performance optimization
  */
  sqlite3_Vdbe_Mem_ਸੈੱਟ_Int64(&v->aMem[1], i_ਪੰਕਤੀ);

  /* If the statement has been run before (and is paused at the OP_ResultRow)
  ** then back it up to the point where it does the OP_NotExists.  This could
  ** have been down with an extra OP_Goto, but simply setting the program
  ** counter is faster. */
  ਜੇ( v->pc>4 ){
    v->pc = 4;
    ਪ੍ਰਤਿਜ੍ਞਾ( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3_ਕਦਮ(p->pStmt);
  }
  ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    ਪ੍ਰਤਿਜ੍ਞਾ( pC!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->i_ਖਾਨਾ ? pC->aType[p->i_ਖਾਨਾ] : 0;
    testcase( pC->nHdrParsed==p->i_ਖਾਨਾ );
    testcase( pC->nHdrParsed==p->i_ਖਾਨਾ+1 );
    ਜੇ( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }ਨਹੀਂਤਾਂ{
      p->i_ਵਿੱਥ = pC->aType[p->i_ਖਾਨਾ + pC->n_ਖੇਤਰ];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ ਜੇ( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    ਜੇ( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", i_ਪੰਕਤੀ);
      rc = SQLITE_ERROR;
    }ਨਹੀਂਤਾਂ{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || zErr==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_ਪੰਕਤੀ && rc!=SQLITE_ਪੂਰਾ );

  *pzErr = zErr;
  ਵਾਪਸ rc;
}

/*
** Open a blob handle.
*/
ਪੂਰਨ sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  ਅਚਲ ਅੱਖਰ *zDb,        /* The attached database containing the blob */
  ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ,     /* The table containing the blob */
  ਅਚਲ ਅੱਖਰ *z_ਕਾਲਮ,    /* The column containing the blob */
  sqlite_int64 i_ਪੰਕਤੀ,      /* The row containing the glob */
  ਪੂਰਨ wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  ਪੂਰਨ nAttempt = 0;
  ਪੂਰਨ i_ਖਾਨਾ;               /* Index of zColumn in row-record */
  ਪੂਰਨ rc = SQLITE_OK;
  ਅੱਖਰ *zErr = 0;
  ਸਾਰਣੀ *pTab;
  Incrblob *pBlob = 0;
  ਪੂਰਨ iDb;
  Parse sParse;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( ppBlob==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  *ppBlob = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) || z_ਸਾਰਣੀ==0 || z_ਕਾਲਮ==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, ਆਕਾਰ(Incrblob));
  ਜਦੋਂ(1){
    sqlite3ParseObjectInit(&sParse,db);
    ਜੇ( !pBlob ) ਜਾਓ blob_open_ਬਾਹਰਲਾ;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3_Locate_ਸਾਰਣੀ(&sParse, 0, z_ਸਾਰਣੀ, zDb);
    ਜੇ( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_ਸੁਨੇਹਾ(&sParse, "cannot open virtual table: %s", z_ਸਾਰਣੀ);
    }
    ਜੇ( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_ਸੁਨੇਹਾ(&sParse, "cannot open table without rowid: %s", z_ਸਾਰਣੀ);
    }
    ਜੇ( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_ਸੁਨੇਹਾ(&sParse, "cannot open table with generated columns: %s",
                      z_ਸਾਰਣੀ);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIEW
    ਜੇ( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_ਸੁਨੇਹਾ(&sParse, "cannot open view: %s", z_ਸਾਰਣੀ);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( pTab==0
     || ((iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema))==1 &&
         sqlite3_Open_ਆਰਜ਼ੀ_Database(&sParse))
    ){
      ਜੇ( sParse.z_Err_ਸੁਨੇਹਾ ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_ਸੁਨੇਹਾ;
        sParse.z_Err_ਸੁਨੇਹਾ = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      ਜਾਓ blob_open_ਬਾਹਰਲਾ;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, z_ਕਾਲਮ);
    ਜੇ( i_ਖਾਨਾ<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", z_ਕਾਲਮ);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      ਜਾਓ blob_open_ਬਾਹਰਲਾ;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    ਜੇ( wrFlag ){
      ਅਚਲ ਅੱਖਰ *zFault = 0;
      ਸੂਚਕਾਂਕ *pIdx;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
      ਜੇ( db->flags&SQLITE_Foreign_ਕੁੰਜੀਆਂ ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        ਪ੍ਰਤਿਜ੍ਞਾ( Is_Ordinary_ਸਾਰਣੀ(pTab) );
        ਲਈ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ਅਗਲਾ_From){
          ਪੂਰਨ j;
          ਲਈ(j=0; j<pFKey->n_ਖਾਨਾ; j++){
            ਜੇ( pFKey->a_ਖਾਨਾ[j].iFrom==i_ਖਾਨਾ ){
              zFault = "foreign key";
            }
          }
        }
      }
#ਜੇ_ਅੰਤ
      ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
        ਪੂਰਨ j;
        ਲਈ(j=0; j<pIdx->n_ਕੁੰਜੀ_ਖਾਨਾ; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          ਜੇ( pIdx->ai_ਕਾਲਮ[j]==i_ਖਾਨਾ || pIdx->ai_ਕਾਲਮ[j]==XN_ਵਿਅੰਜਕ ){
            zFault = "indexed";
          }
        }
      }
      ਜੇ( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        ਜਾਓ blob_open_ਬਾਹਰਲਾ;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    ਪ੍ਰਤਿਜ੍ਞਾ( pBlob->pStmt || db->mallocFailed );
    ਜੇ( pBlob->pStmt ){
      
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
      ਸਥਿਰ ਅਚਲ ਪੂਰਨ iLn = VDBE_ਵਿੱਥ_LINENO(2);
      ਸਥਿਰ ਅਚਲ VdbeOpList openBlob[] = {
        {OP_ਸਾਰਣੀ_ਤਾਲਾ,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_ਕਾਲਮ,         0, 0, 1},  /* 3  */
        {OP_ਨਤੀਜਾ_ਪੰਕਤੀ,      1, 0, 0},  /* 4  */
        {OP_Halt,           0, 0, 0},  /* 5  */
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Transaction, iDb, wrFlag, 
                           pTab->pSchema->schema_cookie,
                           pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeCurrentAddr(v)==2 || db->mallocFailed );
      aOp = sqlite3_Vdbe_ਜੋੜੋ_Op_List(v, Array_ਮਾਪ(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3_Vdbe_ਵਰਤਦਾ_Btree(v, iDb); 

      ਜੇ( db->mallocFailed==0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#ਨਹੀਂਤਾਂ
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      ਜੇ( db->mallocFailed==0 ){
#ਜੇ_ਅੰਤ

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        ਜੇ( wrFlag ) aOp[1].opcode = OP_OpenWrite;
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
        aOp[1].p4.i = pTab->n_ਖਾਨਾ+1;
        aOp[3].p2 = pTab->n_ਖਾਨਾ;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }
   
    pBlob->i_ਖਾਨਾ = i_ਖਾਨਾ;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    ਜੇ( db->mallocFailed ){
      ਜਾਓ blob_open_ਬਾਹਰਲਾ;
    }
    rc = blob_Seek_ਮੰਜ਼ਿਲ_ਪੰਕਤੀ(pBlob, i_ਪੰਕਤੀ, &zErr);
    ਜੇ( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) ਰੋਕੋ;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_ਬਾਹਰਲਾ:
  ਜੇ( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc, (zErr ? "%s" : (ਅੱਖਰ*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
ਪੂਰਨ sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  ਪੂਰਨ rc;
  sqlite3 *db;

  ਜੇ( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_OK;
  }
  ਵਾਪਸ rc;
}

/*
** Perform a read or write operation on a blob
*/
ਸਥਿਰ ਪੂਰਨ blobReadWrite(
  sqlite3_blob *pBlob, 
  ਖਾਲੀ *z, 
  ਪੂਰਨ n, 
  ਪੂਰਨ i_ਵਿੱਥ, 
  ਪੂਰਨ (*xCall)(BtCursor*, u32, u32, ਖਾਲੀ*)
){
  ਪੂਰਨ rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  ਜੇ( n<0 || i_ਵਿੱਥ<0 || ((sqlite3_int64)i_ਵਿੱਥ+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ ਜੇ( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }ਨਹੀਂਤਾਂ{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
    ਜੇ( xCall==sqlite3_Btree_Put_ਅੰਕੜੇ && db->xPreUpdateCallback ){
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
      ਜੇ( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        ਪੂਰਨ bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        ਪ੍ਰਤਿਜ੍ਞਾ( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      ਜੇ( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_ਕੁੰਜੀ;
        i_ਕੁੰਜੀ = sqlite3_Btree_Integer_ਕੁੰਜੀ(p->pCsr);
        ਪ੍ਰਤਿਜ੍ਞਾ( v->apCsr[0]!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_ਕੁੰਜੀ, -1, p->i_ਖਾਨਾ
        );
      }
    }
    ਜੇ( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, i_ਵਿੱਥ+p->i_ਵਿੱਥ, n, z);
    }
#ਨਹੀਂਤਾਂ
    rc = xCall(p->pCsr, i_ਵਿੱਥ+p->i_ਵਿੱਥ, n, z);
#ਜੇ_ਅੰਤ

    sqlite3BtreeLeaveCursor(p->pCsr);
    ਜੇ( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }ਨਹੀਂਤਾਂ{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Read data from a blob handle.
*/
ਪੂਰਨ sqlite3_blob_read(sqlite3_blob *pBlob, ਖਾਲੀ *z, ਪੂਰਨ n, ਪੂਰਨ i_ਵਿੱਥ){
  ਵਾਪਸ blobReadWrite(pBlob, z, n, i_ਵਿੱਥ, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
ਪੂਰਨ sqlite3_blob_write(sqlite3_blob *pBlob, ਅਚਲ ਖਾਲੀ *z, ਪੂਰਨ n, ਪੂਰਨ i_ਵਿੱਥ){
  ਵਾਪਸ blobReadWrite(pBlob, (ਖਾਲੀ *)z, n, i_ਵਿੱਥ, sqlite3_Btree_Put_ਅੰਕੜੇ);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
ਪੂਰਨ sqlite3_blob_ਬਾਈਟਾਂ(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  ਵਾਪਸ (p && p->pStmt) ? p->nByte : 0;
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
ਪੂਰਨ sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 i_ਪੰਕਤੀ){
  ਪੂਰਨ rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  ਜੇ( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blob_Seek_ਮੰਜ਼ਿਲ_ਪੰਕਤੀ(p, i_ਪੰਕਤੀ, &zErr);
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3_Error_With_ਸੁਨੇਹਾ(db, rc, (zErr ? "%s" : (ਅੱਖਰ*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_INCRBLOB */
