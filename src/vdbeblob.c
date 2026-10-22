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

#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

#جے_بنا_تعریف SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
تعريفنوع بنية Incrblob Incrblob;
بنية Incrblob {
  صحيح nByte;              /* Size of open blob, in bytes */
  صحيح i_وتھ;            /* Byte offset of blob in cursor data */
  u16 i_خانہ;               /* Table column this handle is open on */
  BtCursor *pCsr;         /* Cursor pointing at blob row */
  sqlite3_stmt *pStmt;    /* Statement holding cursor open */
  sqlite3 *db;            /* The associated database */
  حرف *zDb;              /* Database name */
  جدول *pTab;            /* Table object */
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
ساكن صحيح blob_Seek_منزل_صف(Incrblob *p, sqlite3_int64 i_صف, حرف **pzErr){
  صحيح rc;                         /* Error code */
  حرف *zErr = 0;                 /* Error message */
  Vdbe *v = (Vdbe *)p->pStmt;

  /* Set the value of register r[1] in the SQL statement to integer iRow. 
  ** This is done directly as a performance optimization
  */
  sqlite3_Vdbe_Mem_سیٹ_Int64(&v->aMem[1], i_صف);

  /* If the statement has been run before (and is paused at the OP_ResultRow)
  ** then back it up to the point where it does the OP_NotExists.  This could
  ** have been down with an extra OP_Goto, but simply setting the program
  ** counter is faster. */
  إذا( v->pc>4 ){
    v->pc = 4;
    أكد( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }وإلا{
    rc = sqlite3_قدم(p->pStmt);
  }
  إذا( rc==SQLITE_صف ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    أكد( pC!=0 );
    أكد( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->i_خانہ ? pC->aType[p->i_خانہ] : 0;
    testcase( pC->nHdrParsed==p->i_خانہ );
    testcase( pC->nHdrParsed==p->i_خانہ+1 );
    إذا( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }وإلا{
      p->i_وتھ = pC->aType[p->i_خانہ + pC->n_میدان];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  إذا( rc==SQLITE_صف ){
    rc = SQLITE_OK;
  }وإلا إذا( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    إذا( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", i_صف);
      rc = SQLITE_ERROR;
    }وإلا{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  أكد( rc!=SQLITE_OK || zErr==0 );
  أكد( rc!=SQLITE_صف && rc!=SQLITE_پورا );

  *pzErr = zErr;
  أرجع rc;
}

/*
** Open a blob handle.
*/
صحيح sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  ثابت حرف *zDb,        /* The attached database containing the blob */
  ثابت حرف *z_جدول,     /* The table containing the blob */
  ثابت حرف *z_کالم,    /* The column containing the blob */
  sqlite_int64 i_صف,      /* The row containing the glob */
  صحيح wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  صحيح nAttempt = 0;
  صحيح i_خانہ;               /* Index of zColumn in row-record */
  صحيح rc = SQLITE_OK;
  حرف *zErr = 0;
  جدول *pTab;
  Incrblob *pBlob = 0;
  صحيح iDb;
  Parse sParse;

#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( ppBlob==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#جے_انت
  *ppBlob = 0;
#جے_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) || z_جدول==0 || z_کالم==0 ){
    أرجع SQLITE_MISUSE_BKPT;
  }
#جے_انت
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, حجم(Incrblob));
  طالما(1){
    sqlite3ParseObjectInit(&sParse,db);
    إذا( !pBlob ) اذهب blob_open_باہرلا;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3_Locate_جدول(&sParse, 0, z_جدول, zDb);
    إذا( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open virtual table: %s", z_جدول);
    }
    إذا( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open table without rowid: %s", z_جدول);
    }
    إذا( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open table with generated columns: %s",
                      z_جدول);
    }
#جے_بنا_تعریف SQLITE_OMIT_VIEW
    إذا( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open view: %s", z_جدول);
    }
#جے_انت
    إذا( pTab==0
     || ((iDb = sqlite3_Schema_منزل_اشاریہ(db, pTab->pSchema))==1 &&
         sqlite3_Open_عارضی_Database(&sParse))
    ){
      إذا( sParse.z_Err_پیغام ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_پیغام;
        sParse.z_Err_پیغام = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      اذهب blob_open_باہرلا;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    i_خانہ = sqlite3_کالم_اشاریہ(pTab, z_کالم);
    إذا( i_خانہ<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", z_کالم);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      اذهب blob_open_باہرلا;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    إذا( wrFlag ){
      ثابت حرف *zFault = 0;
      اشاریہ *pIdx;
#جے_بنا_تعریف SQLITE_OMIT_FOREIGN_کلید
      إذا( db->flags&SQLITE_Foreign_کلیدیں ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        أكد( Is_Ordinary_جدول(pTab) );
        لكل(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_اگلا_From){
          صحيح j;
          لكل(j=0; j<pFKey->n_خانہ; j++){
            إذا( pFKey->a_خانہ[j].iFrom==i_خانہ ){
              zFault = "foreign key";
            }
          }
        }
      }
#جے_انت
      لكل(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
        صحيح j;
        لكل(j=0; j<pIdx->n_کلید_خانہ; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          إذا( pIdx->ai_کالم[j]==i_خانہ || pIdx->ai_کالم[j]==XN_اظہاریہ ){
            zFault = "indexed";
          }
        }
      }
      إذا( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        اذهب blob_open_باہرلا;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    أكد( pBlob->pStmt || db->mallocFailed );
    إذا( pBlob->pStmt ){
      
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
      ساكن ثابت صحيح iLn = VDBE_وتھ_LINENO(2);
      ساكن ثابت VdbeOpList openBlob[] = {
        {OP_جدول_جندرا,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_کالم,         0, 0, 1},  /* 3  */
        {OP_نتیجہ_صف,      1, 0, 0},  /* 4  */
        {OP_Halt,           0, 0, 0},  /* 5  */
      };
      Vdbe *v = (Vdbe *)pBlob->pStmt;
      VdbeOp *aOp;

      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Transaction, iDb, wrFlag, 
                           pTab->pSchema->schema_cookie,
                           pTab->pSchema->iGeneration);
      sqlite3VdbeChangeP5(v, 1);
      أكد( sqlite3VdbeCurrentAddr(v)==2 || db->mallocFailed );
      aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_ناپ(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3_Vdbe_ورتدا_Btree(v, iDb); 

      إذا( db->mallocFailed==0 ){
        أكد( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#جے_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#وإلا
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      إذا( db->mallocFailed==0 ){
#جے_انت

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        إذا( wrFlag ) aOp[1].opcode = OP_OpenWrite;
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
        aOp[1].p4.i = pTab->n_خانہ+1;
        aOp[3].p2 = pTab->n_خانہ;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }
   
    pBlob->i_خانہ = i_خانہ;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    إذا( db->mallocFailed ){
      اذهب blob_open_باہرلا;
    }
    rc = blob_Seek_منزل_صف(pBlob, i_صف, &zErr);
    إذا( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) اكسر;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_باہرلا:
  إذا( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }وإلا{
    إذا( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_پیغام(db, rc, (zErr ? "%s" : (حرف*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
صحيح sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  صحيح rc;
  sqlite3 *db;

  إذا( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }وإلا{
    rc = SQLITE_OK;
  }
  أرجع rc;
}

/*
** Perform a read or write operation on a blob
*/
ساكن صحيح blobReadWrite(
  sqlite3_blob *pBlob, 
  فراغ *z, 
  صحيح n, 
  صحيح i_وتھ, 
  صحيح (*xCall)(BtCursor*, u32, u32, فراغ*)
){
  صحيح rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  إذا( p==0 ) أرجع SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  إذا( n<0 || i_وتھ<0 || ((sqlite3_int64)i_وتھ+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }وإلا إذا( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }وإلا{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    أكد( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#جے_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    إذا( xCall==sqlite3_Btree_Put_انکڑے && db->xPreUpdateCallback ){
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
      إذا( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        صحيح bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        أكد( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      إذا( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_کلید;
        i_کلید = sqlite3_Btree_Integer_کلید(p->pCsr);
        أكد( v->apCsr[0]!=0 );
        أكد( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_کلید, -1, p->i_خانہ
        );
      }
    }
    إذا( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, i_وتھ+p->i_وتھ, n, z);
    }
#وإلا
    rc = xCall(p->pCsr, i_وتھ+p->i_وتھ, n, z);
#جے_انت

    sqlite3BtreeLeaveCursor(p->pCsr);
    إذا( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }وإلا{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

/*
** Read data from a blob handle.
*/
صحيح sqlite3_blob_read(sqlite3_blob *pBlob, فراغ *z, صحيح n, صحيح i_وتھ){
  أرجع blobReadWrite(pBlob, z, n, i_وتھ, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
صحيح sqlite3_blob_write(sqlite3_blob *pBlob, ثابت فراغ *z, صحيح n, صحيح i_وتھ){
  أرجع blobReadWrite(pBlob, (فراغ *)z, n, i_وتھ, sqlite3_Btree_Put_انکڑے);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
صحيح sqlite3_blob_بائٹاں(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  أرجع (p && p->pStmt) ? p->nByte : 0;
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
صحيح sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 i_صف){
  صحيح rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  إذا( p==0 ) أرجع SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  إذا( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }وإلا{
    حرف *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blob_Seek_منزل_صف(p, i_صف, &zErr);
    إذا( rc!=SQLITE_OK ){
      sqlite3_Error_With_پیغام(db, rc, (zErr ? "%s" : (حرف*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    أكد( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  أكد( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

#جے_انت /* #ifndef SQLITE_OMIT_INCRBLOB */
