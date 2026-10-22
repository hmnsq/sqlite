/*
** 2010 February 1
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This header file defines the interface to the write-ahead logging 
** system. Refer to the comments below and the header comment attached to 
** the implementation of each function in log.c for further details.
*/

#నిర్వచితం_కాకపోతే SQLITE_WAL_H
#నిర్వచించు SQLITE_WAL_H

#అన్తర్భావ "sqliteInt.h"

/* Macros for extracting appropriate sync flags for either transaction
** commits (WAL_SYNC_FLAGS(X)) or for checkpoint ops (CKPT_SYNC_FLAGS(X)):
*/
#నిర్వచించు WAL_SYNC_FLAGS(X)   ((X)&0x03)
#నిర్వచించు CKPT_SYNC_FLAGS(X)  (((X)>>2)&0x03)

#నిర్వచితమైతే SQLITE_OMIT_WAL
# define sqlite3WalOpen(x,y,z)                   0
# define sqlite3_Wal_హద్దు(x,y)
# define sqlite3WalClose(v,w,x,y,z)              0
# define sqlite3WalBeginReadTransaction(y,z)     0
# define sqlite3_Wal_చివర_Read_Transaction(z)
# define sqlite3WalDbsize(y)                     0
# define sqlite3WalBeginWriteTransaction(y)      0
# define sqlite3_Wal_చివర_Write_Transaction(x)        0
# define sqlite3WalUndo(x,y,z)                   0
# define sqlite3WalSavepoint(y,z)
# define sqlite3WalSavepointUndo(y,z)            0
# define sqlite3_Wal_చట్రాలు(u,v,w,x,y,z)           0
# define sqlite3WalCheckpoint(q,r,s,t,u,v,w,x,y,z) 0
# define sqlite3WalCallback(z)                   0
# define sqlite3_Wal_Exclusive_బహుళకం(y,z)            0
# define sqlite3_Wal_కుప్ప_జ్ఞాపకం(z)                 0
# define sqlite3WalFramesize(z)                  0
# define sqlite3_Wal_కనుగొను_చట్రం(x,y,z)              0
# define sqlite3WalFile(x)                       0
# undef SQLITE_USE_SEH
#లేకపోతే

#నిర్వచించు WAL_SAVEPOINT_NDATA 4

/* Connection to a write-ahead log (WAL) file. 
** There is one object of this type for each pager. 
*/
రకనిర్వచనం నిర్మాణం Wal Wal;

/* Open and close a connection to a write-ahead log. */
పూర్ణాంకం sqlite3WalOpen(sqlite3_vfs*, sqlite3_file*, స్థిరరాశి అక్షరం *, పూర్ణాంకం, i64, Wal**);
పూర్ణాంకం sqlite3WalClose(Wal *pWal, sqlite3*, పూర్ణాంకం sync_flags, పూర్ణాంకం, u8 *);

/* Set the limiting size of a WAL file. */
శూన్య sqlite3_Wal_హద్దు(Wal*, i64);

/* Used by readers to open (lock) and close (unlock) a snapshot.  A 
** snapshot is like a read-transaction.  It is the state of the database
** at an instant in time.  sqlite3WalOpenSnapshot gets a read lock and
** preserves the current state even if the other threads or processes
** write to or checkpoint the WAL.  sqlite3WalCloseSnapshot() closes the
** transaction and releases the lock.
*/
పూర్ణాంకం sqlite3WalBeginReadTransaction(Wal *pWal, పూర్ణాంకం *);
శూన్య sqlite3_Wal_చివర_Read_Transaction(Wal *pWal);

/* Read a page from the write-ahead log, if it is present. */
పూర్ణాంకం sqlite3_Wal_కనుగొను_చట్రం(Wal *, Pgno, u32 *);
పూర్ణాంకం sqlite3_Wal_Read_చట్రం(Wal *, u32, పూర్ణాంకం, u8 *);

/* If the WAL is not empty, return the size of the database. */
Pgno sqlite3WalDbsize(Wal *pWal);

/* Obtain or release the WRITER lock. */
పూర్ణాంకం sqlite3WalBeginWriteTransaction(Wal *pWal);
పూర్ణాంకం sqlite3_Wal_చివర_Write_Transaction(Wal *pWal);

/* Undo any frames written (but not committed) to the log */
పూర్ణాంకం sqlite3WalUndo(Wal *pWal, పూర్ణాంకం (*xUndo)(శూన్య *, Pgno), శూన్య *pUndoCtx);

/* Return an integer that records the current (uncommitted) write
** position in the WAL */
శూన్య sqlite3WalSavepoint(Wal *pWal, u32 *a_Wal_దత్తాంశం);

/* Move the write position of the WAL back to iFrame.  Called in
** response to a ROLLBACK TO command. */
పూర్ణాంకం sqlite3WalSavepointUndo(Wal *pWal, u32 *a_Wal_దత్తాంశం);

/* Write a frame or frames to the log. */
పూర్ణాంకం sqlite3_Wal_చట్రాలు(Wal *pWal, పూర్ణాంకం, PgHdr *, Pgno, పూర్ణాంకం, పూర్ణాంకం);

/* Copy pages from the log to the database file */ 
పూర్ణాంకం sqlite3WalCheckpoint(
  Wal *pWal,                      /* Write-ahead log connection */
  sqlite3 *db,                    /* Check this handle's interrupt flag */
  పూర్ణాంకం e_బహుళకం,                      /* One of PASSIVE, FULL and RESTART */
  పూర్ణాంకం (*xBusy)(శూన్య*),            /* Function to call when busy */
  శూన్య *pBusyArg,                 /* Context argument for xBusyHandler */
  పూర్ణాంకం sync_flags,                 /* Flags to sync db file with (or 0) */
  పూర్ణాంకం nBuf,                       /* Size of buffer nBuf */
  u8 *zBuf,                       /* Temporary buffer to use */
  పూర్ణాంకం *pnLog,                     /* OUT: Number of frames in WAL */
  పూర్ణాంకం *pnCkpt                     /* OUT: Number of backfilled frames in WAL */
);

/* Return the value to pass to a sqlite3_wal_hook callback, the
** number of frames in the WAL at the point of the last commit since
** sqlite3WalCallback() was called.  If no commits have occurred since
** the last call, then return 0.
*/
పూర్ణాంకం sqlite3WalCallback(Wal *pWal);

/* Tell the wal layer that an EXCLUSIVE lock has been obtained (or released)
** by the pager layer on the database file.
*/
పూర్ణాంకం sqlite3_Wal_Exclusive_బహుళకం(Wal *pWal, పూర్ణాంకం op);

/* Return true if the argument is non-NULL and the WAL module is using
** heap-memory for the wal-index. Otherwise, if the argument is NULL or the
** WAL module is using shared-memory, return false. 
*/
పూర్ణాంకం sqlite3_Wal_కుప్ప_జ్ఞాపకం(Wal *pWal);

#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
పూర్ణాంకం sqlite3WalSnapshotGet(Wal *pWal, sqlite3_snapshot **ppSnapshot);
శూన్య sqlite3WalSnapshotOpen(Wal *pWal, sqlite3_snapshot *pSnapshot);
పూర్ణాంకం sqlite3WalSnapshotRecover(Wal *pWal);
పూర్ణాంకం sqlite3WalSnapshotCheck(Wal *pWal, sqlite3_snapshot *pSnapshot);
శూన్య sqlite3WalSnapshotUnlock(Wal *pWal);
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_ZIPVFS
/* If the WAL file is not empty, return the number of bytes of content
** stored in each frame (i.e. the db page-size when the WAL was created).
*/
పూర్ణాంకం sqlite3WalFramesize(Wal *pWal);
#అయితే_ముగింపు

/* Return the sqlite3_file object for the WAL file */
sqlite3_file *sqlite3WalFile(Wal *pWal);

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
పూర్ణాంకం sqlite3_Wal_Write_తాళం(Wal *pWal, పూర్ణాంకం b_తాళం);
శూన్య sqlite3WalDb(Wal *pWal, sqlite3 *db);
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_USE_SEH
పూర్ణాంకం sqlite3WalSystemErrno(Wal*);
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  శూన్య sqlite3WalStat(Wal*,sqlite3_str*);
#అయితే_ముగింపు

#అయితే_ముగింపు /* ifndef SQLITE_OMIT_WAL */
#అయితే_ముగింపు /* SQLITE_WAL_H */
