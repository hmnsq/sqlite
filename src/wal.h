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

#اگر_غیر_تعریف_شدہ SQLITE_WAL_H
#تعریف_کرو SQLITE_WAL_H

#شامل "sqliteInt.h"

/* Macros for extracting appropriate sync flags for either transaction
** commits (WAL_SYNC_FLAGS(X)) or for checkpoint ops (CKPT_SYNC_FLAGS(X)):
*/
#تعریف_کرو WAL_SYNC_FLAGS(X)   ((X)&0x03)
#تعریف_کرو CKPT_SYNC_FLAGS(X)  (((X)>>2)&0x03)

#اگر_تعریف_شدہ SQLITE_OMIT_WAL
# define sqlite3WalOpen(x,y,z)                   0
# define sqlite3_Wal_حد_بندی(x,y)
# define sqlite3WalClose(v,w,x,y,z)              0
# define sqlite3WalBeginReadTransaction(y,z)     0
# define sqlite3_Wal_انتہا_Read_Transaction(z)
# define sqlite3WalDbsize(y)                     0
# define sqlite3WalBeginWriteTransaction(y)      0
# define sqlite3_Wal_انتہا_Write_Transaction(x)        0
# define sqlite3WalUndo(x,y,z)                   0
# define sqlite3WalSavepoint(y,z)
# define sqlite3WalSavepointUndo(y,z)            0
# define sqlite3_Wal_چوکھٹے(u,v,w,x,y,z)           0
# define sqlite3WalCheckpoint(q,r,s,t,u,v,w,x,y,z) 0
# define sqlite3WalCallback(z)                   0
# define sqlite3_Wal_Exclusive_منوال(y,z)            0
# define sqlite3_Wal_ڈھیر_حافظہ(z)                 0
# define sqlite3WalFramesize(z)                  0
# define sqlite3_Wal_ڈھونڈو_چوکھٹا(x,y,z)              0
# define sqlite3WalFile(x)                       0
# undef SQLITE_USE_SEH
#ورنہ

#تعریف_کرو WAL_SAVEPOINT_NDATA 4

/* Connection to a write-ahead log (WAL) file. 
** There is one object of this type for each pager. 
*/
قسمتعریف ساخت Wal Wal;

/* Open and close a connection to a write-ahead log. */
صحیح sqlite3WalOpen(sqlite3_vfs*, sqlite3_file*, ثابت حرف *, صحیح, i64, Wal**);
صحیح sqlite3WalClose(Wal *pWal, sqlite3*, صحیح sync_flags, صحیح, u8 *);

/* Set the limiting size of a WAL file. */
خالی sqlite3_Wal_حد_بندی(Wal*, i64);

/* Used by readers to open (lock) and close (unlock) a snapshot.  A 
** snapshot is like a read-transaction.  It is the state of the database
** at an instant in time.  sqlite3WalOpenSnapshot gets a read lock and
** preserves the current state even if the other threads or processes
** write to or checkpoint the WAL.  sqlite3WalCloseSnapshot() closes the
** transaction and releases the lock.
*/
صحیح sqlite3WalBeginReadTransaction(Wal *pWal, صحیح *);
خالی sqlite3_Wal_انتہا_Read_Transaction(Wal *pWal);

/* Read a page from the write-ahead log, if it is present. */
صحیح sqlite3_Wal_ڈھونڈو_چوکھٹا(Wal *, Pgno, u32 *);
صحیح sqlite3_Wal_Read_چوکھٹا(Wal *, u32, صحیح, u8 *);

/* If the WAL is not empty, return the size of the database. */
Pgno sqlite3WalDbsize(Wal *pWal);

/* Obtain or release the WRITER lock. */
صحیح sqlite3WalBeginWriteTransaction(Wal *pWal);
صحیح sqlite3_Wal_انتہا_Write_Transaction(Wal *pWal);

/* Undo any frames written (but not committed) to the log */
صحیح sqlite3WalUndo(Wal *pWal, صحیح (*xUndo)(خالی *, Pgno), خالی *pUndoCtx);

/* Return an integer that records the current (uncommitted) write
** position in the WAL */
خالی sqlite3WalSavepoint(Wal *pWal, u32 *a_Wal_ڈیٹا);

/* Move the write position of the WAL back to iFrame.  Called in
** response to a ROLLBACK TO command. */
صحیح sqlite3WalSavepointUndo(Wal *pWal, u32 *a_Wal_ڈیٹا);

/* Write a frame or frames to the log. */
صحیح sqlite3_Wal_چوکھٹے(Wal *pWal, صحیح, PgHdr *, Pgno, صحیح, صحیح);

/* Copy pages from the log to the database file */ 
صحیح sqlite3WalCheckpoint(
  Wal *pWal,                      /* Write-ahead log connection */
  sqlite3 *db,                    /* Check this handle's interrupt flag */
  صحیح e_منوال,                      /* One of PASSIVE, FULL and RESTART */
  صحیح (*xBusy)(خالی*),            /* Function to call when busy */
  خالی *pBusyArg,                 /* Context argument for xBusyHandler */
  صحیح sync_flags,                 /* Flags to sync db file with (or 0) */
  صحیح nBuf,                       /* Size of buffer nBuf */
  u8 *zBuf,                       /* Temporary buffer to use */
  صحیح *pnLog,                     /* OUT: Number of frames in WAL */
  صحیح *pnCkpt                     /* OUT: Number of backfilled frames in WAL */
);

/* Return the value to pass to a sqlite3_wal_hook callback, the
** number of frames in the WAL at the point of the last commit since
** sqlite3WalCallback() was called.  If no commits have occurred since
** the last call, then return 0.
*/
صحیح sqlite3WalCallback(Wal *pWal);

/* Tell the wal layer that an EXCLUSIVE lock has been obtained (or released)
** by the pager layer on the database file.
*/
صحیح sqlite3_Wal_Exclusive_منوال(Wal *pWal, صحیح op);

/* Return true if the argument is non-NULL and the WAL module is using
** heap-memory for the wal-index. Otherwise, if the argument is NULL or the
** WAL module is using shared-memory, return false. 
*/
صحیح sqlite3_Wal_ڈھیر_حافظہ(Wal *pWal);

#اگر_تعریف_شدہ SQLITE_ENABLE_SNAPSHOT
صحیح sqlite3WalSnapshotGet(Wal *pWal, sqlite3_snapshot **ppSnapshot);
خالی sqlite3WalSnapshotOpen(Wal *pWal, sqlite3_snapshot *pSnapshot);
صحیح sqlite3WalSnapshotRecover(Wal *pWal);
صحیح sqlite3WalSnapshotCheck(Wal *pWal, sqlite3_snapshot *pSnapshot);
خالی sqlite3WalSnapshotUnlock(Wal *pWal);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ZIPVFS
/* If the WAL file is not empty, return the number of bytes of content
** stored in each frame (i.e. the db page-size when the WAL was created).
*/
صحیح sqlite3WalFramesize(Wal *pWal);
#اگر_ختم

/* Return the sqlite3_file object for the WAL file */
sqlite3_file *sqlite3WalFile(Wal *pWal);

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
صحیح sqlite3_Wal_Write_تالا(Wal *pWal, صحیح b_تالا);
خالی sqlite3WalDb(Wal *pWal, sqlite3 *db);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_USE_SEH
صحیح sqlite3WalSystemErrno(Wal*);
#اگر_ختم

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  خالی sqlite3WalStat(Wal*,sqlite3_str*);
#اگر_ختم

#اگر_ختم /* ifndef SQLITE_OMIT_WAL */
#اگر_ختم /* SQLITE_WAL_H */
