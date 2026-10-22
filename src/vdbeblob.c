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

#شامل "sqliteInt.h"
#شامل "vdbeInt.h"

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB

/*
** Valid sqlite3_blob* handles point to Incrblob structures.
*/
قسمتعریف ساخت Incrblob Incrblob;
ساخت Incrblob {
  صحیح nByte;              /* Size of open blob, in bytes */
  صحیح i_انحراف;            /* Byte offset of blob in cursor data */
  u16 i_ستون;               /* Table column this handle is open on */
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
ساکن صحیح blob_Seek_مقصد_صف(Incrblob *p, sqlite3_int64 i_صف, حرف **pzErr){
  صحیح rc;                         /* Error code */
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
  اگر( v->pc>4 ){
    v->pc = 4;
    أكد( v->aOp[v->pc].opcode==OP_NotExists );
    rc = sqlite3VdbeExec(v);
  }ورنہ{
    rc = sqlite3_قدم(p->pStmt);
  }
  اگر( rc==SQLITE_صف ){
    VdbeCursor *pC = v->apCsr[0];
    u32 type;
    أكد( pC!=0 );
    أكد( pC->eCurType==CURTYPE_BTREE );
    type = pC->nHdrParsed>p->i_ستون ? pC->aType[p->i_ستون] : 0;
    testcase( pC->nHdrParsed==p->i_ستون );
    testcase( pC->nHdrParsed==p->i_ستون+1 );
    اگر( type<12 ){
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s",
          type==0?"null": type==7?"real": "integer"
      );
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    }ورنہ{
      p->i_انحراف = pC->aType[p->i_ستون + pC->n_میدان];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr =  pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  اگر( rc==SQLITE_صف ){
    rc = SQLITE_OK;
  }ورنہ اگر( p->pStmt ){
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    اگر( rc==SQLITE_OK ){
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", i_صف);
      rc = SQLITE_ERROR;
    }ورنہ{
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  أكد( rc!=SQLITE_OK || zErr==0 );
  أكد( rc!=SQLITE_صف && rc!=SQLITE_مکمل );

  *pzErr = zErr;
  واپس rc;
}

/*
** Open a blob handle.
*/
صحیح sqlite3_blob_open(
  sqlite3* db,            /* The database connection */
  ثابت حرف *zDb,        /* The attached database containing the blob */
  ثابت حرف *z_جدول,     /* The table containing the blob */
  ثابت حرف *z_عمود,    /* The column containing the blob */
  sqlite_int64 i_صف,      /* The row containing the glob */
  صحیح wrFlag,             /* True -> read/write access, false -> read-only */
  sqlite3_blob **ppBlob   /* Handle for accessing the blob returned here */
){
  صحیح nAttempt = 0;
  صحیح i_ستون;               /* Index of zColumn in row-record */
  صحیح rc = SQLITE_OK;
  حرف *zErr = 0;
  جدول *pTab;
  Incrblob *pBlob = 0;
  صحیح iDb;
  Parse sParse;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( ppBlob==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  *ppBlob = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) || z_جدول==0 || z_عمود==0 ){
    واپس SQLITE_MISUSE_BKPT;
  }
#اگر_ختم
  wrFlag = !!wrFlag;                /* wrFlag = (wrFlag ? 1 : 0); */

  sqlite3_mutex_enter(db->mutex);

  pBlob = (Incrblob *)sqlite3DbMallocZero(db, حجم(Incrblob));
  جب(1){
    sqlite3ParseObjectInit(&sParse,db);
    اگر( !pBlob ) جاؤ blob_open_برآمد;
    sqlite3DbFree(db, zErr);
    zErr = 0;

    sqlite3BtreeEnterAll(db);
    pTab = sqlite3_Locate_جدول(&sParse, 0, z_جدول, zDb);
    اگر( pTab && IsVirtual(pTab) ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open virtual table: %s", z_جدول);
    }
    اگر( pTab && !HasRowid(pTab) ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open table without rowid: %s", z_جدول);
    }
    اگر( pTab && (pTab->tabFlags&TF_HasGenerated)!=0 ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open table with generated columns: %s",
                      z_جدول);
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIEW
    اگر( pTab && IsView(pTab) ){
      pTab = 0;
      sqlite3_Error_پیغام(&sParse, "cannot open view: %s", z_جدول);
    }
#اگر_ختم
    اگر( pTab==0
     || ((iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema))==1 &&
         sqlite3_Open_عارضی_Database(&sParse))
    ){
      اگر( sParse.z_Err_پیغام ){
        sqlite3DbFree(db, zErr);
        zErr = sParse.z_Err_پیغام;
        sParse.z_Err_پیغام = 0;
      }
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      جاؤ blob_open_برآمد;
    }
    pBlob->pTab = pTab;
    pBlob->zDb = db->aDb[iDb].zDbSName;

    /* Now search pTab for the exact column. */
    i_ستون = sqlite3_عمود_اشاریہ(pTab, z_عمود);
    اگر( i_ستون<0 ){
      sqlite3DbFree(db, zErr);
      zErr = sqlite3MPrintf(db, "no such column: \"%s\"", z_عمود);
      rc = SQLITE_ERROR;
      sqlite3BtreeLeaveAll(db);
      جاؤ blob_open_برآمد;
    }

    /* If the value is being opened for writing, check that the
    ** column is not indexed, and that it is not part of a foreign key. 
    */
    اگر( wrFlag ){
      ثابت حرف *zFault = 0;
      اشاریہ *pIdx;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FOREIGN_کلید
      اگر( db->flags&SQLITE_Foreign_کلیدیں ){
        /* Check that the column is not part of an FK child key definition. It
        ** is not necessary to check if it is part of a parent key, as parent
        ** key columns must be indexed. The check below will pick up this 
        ** case.  */
        FKey *pFKey;
        أكد( Is_Ordinary_جدول(pTab) );
        برائے(pFKey=pTab->u.tab.pFKey; pFKey; pFKey=pFKey->p_اگلا_From){
          صحیح j;
          برائے(j=0; j<pFKey->n_ستون; j++){
            اگر( pFKey->a_ستون[j].iFrom==i_ستون ){
              zFault = "foreign key";
            }
          }
        }
      }
#اگر_ختم
      برائے(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
        صحیح j;
        برائے(j=0; j<pIdx->n_کلید_ستون; j++){
          /* FIXME: Be smarter about indexes that use expressions */
          اگر( pIdx->ai_عمود[j]==i_ستون || pIdx->ai_عمود[j]==XN_اظہاریہ ){
            zFault = "indexed";
          }
        }
      }
      اگر( zFault ){
        sqlite3DbFree(db, zErr);
        zErr = sqlite3MPrintf(db, "cannot open %s column for writing", zFault);
        rc = SQLITE_ERROR;
        sqlite3BtreeLeaveAll(db);
        جاؤ blob_open_برآمد;
      }
    }

    pBlob->pStmt = (sqlite3_stmt *)sqlite3VdbeCreate(&sParse);
    أكد( pBlob->pStmt || db->mallocFailed );
    اگر( pBlob->pStmt ){
      
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
      ساکن ثابت صحیح iLn = VDBE_انحراف_LINENO(2);
      ساکن ثابت VdbeOpList openBlob[] = {
        {OP_جدول_تالا,      0, 0, 0},  /* 0: Acquire a read or write lock */
        {OP_OpenRead,       0, 0, 0},  /* 1: Open a cursor */
        /* blobSeekToRow() will initialize r[1] to the desired rowid */
        {OP_NotExists,      0, 5, 1},  /* 2: Seek the cursor to rowid=r[1] */
        {OP_عمود,         0, 0, 1},  /* 3  */
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
      aOp = sqlite3_Vdbe_جوڑو_Op_List(v, Array_سائز(openBlob), openBlob, iLn);

      /* Make sure a mutex is held on the table to be accessed */
      sqlite3_Vdbe_استعمال_کرتا_Btree(v, iDb); 

      اگر( db->mallocFailed==0 ){
        أكد( aOp!=0 );
        /* Configure the OP_TableLock instruction */
#اگر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
        aOp[0].opcode = OP_Noop;
#ورنہ
        aOp[0].p1 = iDb;
        aOp[0].p2 = pTab->tnum;
        aOp[0].p3 = wrFlag;
        sqlite3VdbeChangeP4(v, 2, pTab->zName, P4_TRANSIENT);
      }
      اگر( db->mallocFailed==0 ){
#اگر_ختم

        /* Remove either the OP_OpenWrite or OpenRead. Set the P2 
        ** parameter of the other to pTab->tnum.  */
        اگر( wrFlag ) aOp[1].opcode = OP_OpenWrite;
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
        aOp[1].p4.i = pTab->n_ستون+1;
        aOp[3].p2 = pTab->n_ستون;

        sParse.nVar = 0;
        sParse.nMem = 1;
        sParse.nTab = 1;
        sqlite3VdbeMakeReady(v, &sParse);
      }
    }
   
    pBlob->i_ستون = i_ستون;
    pBlob->db = db;
    sqlite3BtreeLeaveAll(db);
    اگر( db->mallocFailed ){
      جاؤ blob_open_برآمد;
    }
    rc = blob_Seek_مقصد_صف(pBlob, i_صف, &zErr);
    اگر( (++nAttempt)>=SQLITE_MAX_SCHEMA_RETRY || rc!=SQLITE_SCHEMA ) توڑو;
    sqlite3ParseObjectReset(&sParse);
  }

blob_open_برآمد:
  اگر( rc==SQLITE_OK && db->mallocFailed==0 ){
    *ppBlob = (sqlite3_blob *)pBlob;
  }ورنہ{
    اگر( pBlob && pBlob->pStmt ) sqlite3VdbeFinalize((Vdbe *)pBlob->pStmt);
    sqlite3DbFree(db, pBlob);
  }
  sqlite3_Error_With_پیغام(db, rc, (zErr ? "%s" : (حرف*)0), zErr);
  sqlite3DbFree(db, zErr);
  sqlite3ParseObjectReset(&sParse);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Close a blob handle that was previously created using
** sqlite3_blob_open().
*/
صحیح sqlite3_blob_close(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  صحیح rc;
  sqlite3 *db;

  اگر( p ){
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  }ورنہ{
    rc = SQLITE_OK;
  }
  واپس rc;
}

/*
** Perform a read or write operation on a blob
*/
ساکن صحیح blobReadWrite(
  sqlite3_blob *pBlob, 
  خالی *z, 
  صحیح n, 
  صحیح i_انحراف, 
  صحیح (*xCall)(BtCursor*, u32, u32, خالی*)
){
  صحیح rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  اگر( p==0 ) واپس SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe*)p->pStmt;

  اگر( n<0 || i_انحراف<0 || ((sqlite3_int64)i_انحراف+n)>p->nByte ){
    /* Request is out of range. Return a transient error. */
    rc = SQLITE_ERROR;
  }ورنہ اگر( v==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }ورنہ{
    /* Call either BtreeData() or BtreePutData(). If SQLITE_ABORT is
    ** returned, clean-up the statement handle.
    */
    أكد( db == v->db );
    sqlite3BtreeEnterCursor(p->pCsr);

#اگر_تعریف_شدہ SQLITE_ENABLE_PREUPDATE_HOOK
    اگر( xCall==sqlite3_Btree_Put_ڈیٹا && db->xPreUpdateCallback ){
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
      اگر( sqlite3BtreeCursorIsValidNN(p->pCsr)==0 ){
        /* If the cursor is not currently valid, try to reseek it. This 
        ** always either fails or finds the correct row - the cursor will
        ** have been marked permanently CURSOR_INVALID if the open row has
        ** been deleted.  */
        صحیح bDiff = 0;
        rc = sqlite3BtreeCursorRestore(p->pCsr, &bDiff);
        أكد( bDiff==0 || sqlite3BtreeCursorIsValidNN(p->pCsr)==0 );
      }
      اگر( sqlite3BtreeCursorIsValidNN(p->pCsr) ){
        sqlite3_int64 i_کلید;
        i_کلید = sqlite3_Btree_Integer_کلید(p->pCsr);
        أكد( v->apCsr[0]!=0 );
        أكد( v->apCsr[0]->eCurType==CURTYPE_BTREE );
        sqlite3VdbePreUpdateHook(
            v, v->apCsr[0], SQLITE_DELETE, p->zDb, p->pTab, i_کلید, -1, p->i_ستون
        );
      }
    }
    اگر( rc==SQLITE_OK ){
      rc = xCall(p->pCsr, i_انحراف+p->i_انحراف, n, z);
    }
#ورنہ
    rc = xCall(p->pCsr, i_انحراف+p->i_انحراف, n, z);
#اگر_ختم

    sqlite3BtreeLeaveCursor(p->pCsr);
    اگر( rc==SQLITE_ABORT ){
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    }ورنہ{
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Read data from a blob handle.
*/
صحیح sqlite3_blob_read(sqlite3_blob *pBlob, خالی *z, صحیح n, صحیح i_انحراف){
  واپس blobReadWrite(pBlob, z, n, i_انحراف, sqlite3BtreePayloadChecked);
}

/*
** Write data to a blob handle.
*/
صحیح sqlite3_blob_write(sqlite3_blob *pBlob, ثابت خالی *z, صحیح n, صحیح i_انحراف){
  واپس blobReadWrite(pBlob, (خالی *)z, n, i_انحراف, sqlite3_Btree_Put_ڈیٹا);
}

/*
** Query a blob handle for the size of the data.
**
** The Incrblob.nByte field is fixed for the lifetime of the Incrblob
** so no mutex is required for access.
*/
صحیح sqlite3_blob_بائٹیں(sqlite3_blob *pBlob){
  Incrblob *p = (Incrblob *)pBlob;
  واپس (p && p->pStmt) ? p->nByte : 0;
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
صحیح sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 i_صف){
  صحیح rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  اگر( p==0 ) واپس SQLITE_MISUSE_BKPT;
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  اگر( p->pStmt==0 ){
    /* If there is no statement handle, then the blob-handle has
    ** already been invalidated. Return SQLITE_ABORT in this case.
    */
    rc = SQLITE_ABORT;
  }ورنہ{
    حرف *zErr;
    ((Vdbe*)p->pStmt)->rc = SQLITE_OK;
    rc = blob_Seek_مقصد_صف(p, i_صف, &zErr);
    اگر( rc!=SQLITE_OK ){
      sqlite3_Error_With_پیغام(db, rc, (zErr ? "%s" : (حرف*)0), zErr);
      sqlite3DbFree(db, zErr);
    }
    أكد( rc!=SQLITE_SCHEMA );
  }

  rc = sqlite3ApiExit(db, rc);
  أكد( rc==SQLITE_OK || p->pStmt==0 );
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

#اگر_ختم /* #ifndef SQLITE_OMIT_INCRBLOB */
