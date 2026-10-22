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

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Incrblob Incrblob;
ರಚನೆ Incrblob {
  ಪೂರ್ಣಾಂಕ nByte;              /* Size of open blob, in bytes */
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;            /* Byte offset of blob in cursor data */
  u16 i_ಲಂಬಸಾಲು;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
  ಅಕ್ಷರ *zDb;              /* Database name */
  ಕೋಷ್ಟಕ *pTab;            /* Table object */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ blob_Seek_ಗಮ್ಯ_ಅಡ್ಡಸಾಲು(Incrblob *p, sqlite3_int64 i_ಅಡ್ಡಸಾಲು, ಅಕ್ಷರ **pzErr){
  ಪೂರ್ಣಾಂಕ rc;                         /* Error code */
  ಅಕ್ಷರ *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of register r[1] in the SQL statement to integer iRow. 
  ** This is done directly as a performance optimization
  */
  sqlite3_Vdbe_Mem_ಗಣ_Int64(&v->aMem[1], i_ಅಡ್ಡಸಾಲು);

  /* If the statement has been run before (and is paused at the OP_ResultRow)
  ** then back it up to the point where it does the OP_NotExists.  This could
  ** have been down with an extra OP_Goto, but simply setting the program
  ** counter is faster. */
  ಆದರೆ( v->pc>4 ){
    v->pc = 4;
    ಪ್ರತಿಜ್ಞಾ( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3_ಹೆಜ್ಜೆ(p->pStmt);
  }
  ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    ಪ್ರತಿಜ್ಞಾ( pC!=0 );
    ಪ್ರತಿಜ್ಞಾ( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->i_ಲಂಬಸಾಲು ? pC->aType[p->i_ಲಂಬಸಾಲು] : 0;
    testcase( pC->nHdrParsed==p->i_ಲಂಬಸಾಲು );
    testcase( pC->nHdrParsed==p->i_ಲಂಬಸಾಲು+1 );
    ಆದರೆ( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->i_ಸ್ಥಾನಾಂತರ = pC->aType[p->i_ಲಂಬಸಾಲು + pC->n_ಕ್ಷೇತ್ರ];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    ಆದರೆ( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", i_ಅಡ್ಡಸಾಲು);
      rc = SQLITE_ERROR;
    }ಇಲ್ಲದಿದ್ದರೆ{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || zErr==0 );
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_ಅಡ್ಡಸಾಲು && rc!=SQLITE_ಮುಗಿದ );

  *pzErr = zErr;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Open a blob handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,        /* The attached database containing the blob */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ,     /* The table containing the blob */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸ್ತಂಭ,    /* The column containing the blob */
  sqlite_int64 i_ಅಡ್ಡಸಾಲು,      /* The row containing the glob */
  ಪೂರ್ಣಾಂಕ wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  ಪೂರ್ಣಾಂಕ nAttempt = 0;
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;               /* Index of zColumn in row-record */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಅಕ್ಷರ *zErr = 0;
  ಕೋಷ್ಟಕ *pTab;
  Incrblob *pBlob = 0;
  ಪೂರ್ಣಾಂಕ iDb;
  Parse sParse;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( ppBlob==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  *ppBlob = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) || z_ಕೋಷ್ಟಕ==0 || z_ಸ್ತಂಭ==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, ಗಾತ್ರ(Incrblob));
  ಆಗ(1){
    sqlite3ParseObjectInit(&sParse,db);
    ಆದರೆ( !pBlob ) ಹೋಗು blob_open_ಹೊರಹೋಗು;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3_Locate_ಕೋಷ್ಟಕ(&sParse, 0, z_ಕೋಷ್ಟಕ, zDb);
    ಆದರೆ( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_ಸಂದೇಶ(&sParse, "cannot open virtual table: %s", z_ಕೋಷ್ಟಕ);
    }
    ಆದರೆ( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_ಸಂದೇಶ(&sParse, "cannot open table without rowid: %s", z_ಕೋಷ್ಟಕ);
    }
    ಆದರೆ( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_ಸಂದೇಶ(&sParse, "cannot open table with generated columns: %s",
                      z_ಕೋಷ್ಟಕ);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIEW
    ಆದರೆ( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_ಸಂದೇಶ(&sParse, "cannot open view: %s", z_ಕೋಷ್ಟಕ);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( pTab==0
     || ((iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema))==1 &&
         sqlite3_Open_ತಾತ್ಕಾಲಿಕ_Database(&sParse))
    ){
      ಆದರೆ( sParse.z_Err_ಸಂದೇಶ ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_ಸಂದೇಶ;
        sParse.z_Err_ಸಂದೇಶ = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      ಹೋಗು blob_open_ಹೊರಹೋಗು;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, z_ಸ್ತಂಭ);
    ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", z_ಸ್ತಂಭ);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      ಹೋಗು blob_open_ಹೊರಹೋಗು;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    ಆದರೆ( wrFlag ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFault = 0;
      ಸೂಚ್ಯಂಕ *pIdx;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
      ಆದರೆ( db->flags&SQLITE_Foreign_ಕೀಲಿಗಳು ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        ಪ್ರತಿಜ್ಞಾ( Is_Ordinary_ಕೋಷ್ಟಕ(pTab) );
        ಗಾಗಿ(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_ಮುಂದಿನ_From){
          ಪೂರ್ಣಾಂಕ j;
          ಗಾಗಿ(j=0; j<pFKey->n_ಲಂಬಸಾಲು; j++){
            ಆದರೆ( pFKey->a_ಲಂಬಸಾಲು[j].iFrom==i_ಲಂಬಸಾಲು ){
              zFault = "foreign key";
            }
          }
        }
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
        ಪೂರ್ಣಾಂಕ j;
        ಗಾಗಿ(j=0; j<pIdx->n_ಕೀಲಿ_ಲಂಬಸಾಲು; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          ಆದರೆ( pIdx->ai_ಸ್ತಂಭ[j]==i_ಲಂಬಸಾಲು || pIdx->ai_ಸ್ತಂಭ[j]==XN_ಅಭಿವ್ಯಕ್ತಿ ){
            zFault = "indexed";
          }
        }
      }
      ಆದರೆ( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        ಹೋಗು blob_open_ಹೊರಹೋಗು;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    ಪ್ರತಿಜ್ಞಾ( pBlob->pStmt || db->mallocFailed );
    ಆದರೆ( pBlob->pStmt ){
      
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
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iLn = VDBE_ಸ್ಥಾನಾಂತರ_LINENO(2);
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ VdbeOpList openBlob[] = {
        {OP_ಕೋಷ್ಟಕ_ಬೀಗ,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_ಸ್ತಂಭ,         0, 0, 1},  /* 3  */
        {OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು,      1, 0, 0},  /* 4  */
        {OP_Halt,           0, 0, 0},  /* 5  */
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Transaction, iDb, wrFlag, 
                           pTab->pSchema->schema_cookie,
                           pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCurrentAddr(v)==2 || db->mallocFailed );
      aOp = sqlite3_Vdbe_ಕೂಡಿಸು_Op_List(v, Array_ಗಾತ್ರ(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Btree(v, iDb); 

      ಆದರೆ( db->mallocFailed==0 ){
        ಪ್ರತಿಜ್ಞಾ( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#ಇಲ್ಲದಿದ್ದರೆ
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      ಆದರೆ( db->mallocFailed==0 ){
#ಆದರೆ_ಅಂತ್ಯ

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        ಆದರೆ( wrFlag ) aOp[1].opcode = OP_OpenWrite;
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
        aOp[1].p4.i = pTab->n_ಲಂಬಸಾಲು+1;
        aOp[3].p2 = pTab->n_ಲಂಬಸಾಲು;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }
   
    pBlob->i_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    ಆದರೆ( db->mallocFailed ){
      ಹೋಗು blob_open_ಹೊರಹೋಗು;
    }
    rc = blob_Seek_ಗಮ್ಯ_ಅಡ್ಡಸಾಲು(pBlob, i_ಅಡ್ಡಸಾಲು, &zErr);
    ಆದರೆ( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) ನಿಲ್ಲಿಸು;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_ಹೊರಹೋಗು:
  ಆದರೆ( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_ಸಂದೇಶ(db, rc, (zErr ? "%s" : (ಅಕ್ಷರ*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
ಪೂರ್ಣಾಂಕ sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3 *db;

  ಆದರೆ( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Perform a read or write operation on a blob
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ blobReadWrite(
  sqlite3_blob *pBlob, 
  ಶೂನ್ಯ *z, 
  ಪೂರ್ಣಾಂಕ n, 
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ, 
  ಪೂರ್ಣಾಂಕ (*xCall)(BtCursor*, u32, u32, ಶೂನ್ಯ*)
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  ಆದರೆ( n<0 || i_ಸ್ಥಾನಾಂತರ<0 || ((sqlite3_int64)i_ಸ್ಥಾನಾಂತರ+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    ಪ್ರತಿಜ್ಞಾ( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
    ಆದರೆ( xCall==sqlite3_Btree_Put_ದತ್ತಾಂಶ && db->xPreUpdateCallback ){
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
      ಆದರೆ( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        ಪೂರ್ಣಾಂಕ bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        ಪ್ರತಿಜ್ಞಾ( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      ಆದರೆ( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_ಕೀಲಿ;
        i_ಕೀಲಿ = sqlite3_Btree_Integer_ಕೀಲಿ(p->pCsr);
        ಪ್ರತಿಜ್ಞಾ( v->apCsr[0]!=0 );
        ಪ್ರತಿಜ್ಞಾ( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_ಕೀಲಿ, -1, p->i_ಲಂಬಸಾಲು
        );
      }
    }
    ಆದರೆ( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, i_ಸ್ಥಾನಾಂತರ+p->i_ಸ್ಥಾನಾಂತರ, n, z);
    }
#ಇಲ್ಲದಿದ್ದರೆ
    rc = xCall(p->pCsr, i_ಸ್ಥಾನಾಂತರ+p->i_ಸ್ಥಾನಾಂತರ, n, z);
#ಆದರೆ_ಅಂತ್ಯ

    sqlite3BtreeLeaveCursor(p->pCsr);
    ಆದರೆ( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Read data from a blob handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3_blob_read(sqlite3_blob *pBlob, ಶೂನ್ಯ *z, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ){
  ಹಿಂತಿರುಗಿ blobReadWrite(pBlob, z, n, i_ಸ್ಥಾನಾಂತರ, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
ಪೂರ್ಣಾಂಕ sqlite3_blob_write(sqlite3_blob *pBlob, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ){
  ಹಿಂತಿರುಗಿ blobReadWrite(pBlob, (ಶೂನ್ಯ *)z, n, i_ಸ್ಥಾನಾಂತರ, sqlite3_Btree_Put_ದತ್ತಾಂಶ);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
ಪೂರ್ಣಾಂಕ sqlite3_blob_ಬೈಟುಗಳು(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  ಹಿಂತಿರುಗಿ (p && p->pStmt) ? p->nByte : 0;
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
ಪೂರ್ಣಾಂಕ sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 i_ಅಡ್ಡಸಾಲು){
  ಪೂರ್ಣಾಂಕ rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  ಆದರೆ( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಕ್ಷರ *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blob_Seek_ಗಮ್ಯ_ಅಡ್ಡಸಾಲು(p, i_ಅಡ್ಡಸಾಲು, &zErr);
    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3_Error_With_ಸಂದೇಶ(db, rc, (zErr ? "%s" : (ಅಕ್ಷರ*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_INCRBLOB */
