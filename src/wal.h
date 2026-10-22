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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WAL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_WAL_H

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/* Macros for extracting appropriate sync flags for either transaction
** commits (WAL_SYNC_FLAGS(X)) or for checkpoint ops (CKPT_SYNC_FLAGS(X)):
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAL_SYNC_FLAGS(X)   ((X)&0x03)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CKPT_SYNC_FLAGS(X)  (((X)>>2)&0x03)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
# define sqlite3WalOpen(x,y,z)                   0
# define sqlite3_Wal_ਹੱਦਬੰਦੀ(x,y)
# define sqlite3WalClose(v,w,x,y,z)              0
# define sqlite3WalBeginReadTransaction(y,z)     0
# define sqlite3_Wal_ਸਿਰਾ_Read_Transaction(z)
# define sqlite3WalDbsize(y)                     0
# define sqlite3WalBeginWriteTransaction(y)      0
# define sqlite3_Wal_ਸਿਰਾ_Write_Transaction(x)        0
# define sqlite3WalUndo(x,y,z)                   0
# define sqlite3WalSavepoint(y,z)
# define sqlite3WalSavepointUndo(y,z)            0
# define sqlite3_Wal_ਚੌਖਟੇ(u,v,w,x,y,z)           0
# define sqlite3WalCheckpoint(q,r,s,t,u,v,w,x,y,z) 0
# define sqlite3WalCallback(z)                   0
# define sqlite3_Wal_Exclusive_ਬਹੁਲਕ(y,z)            0
# define sqlite3_Wal_ਢੇਰ_ਸਮ੍ਰਿਤੀ(z)                 0
# define sqlite3WalFramesize(z)                  0
# define sqlite3_Wal_ਲੱਭੋ_ਚੌਖਟਾ(x,y,z)              0
# define sqlite3WalFile(x)                       0
# undef SQLITE_USE_SEH
#ਨਹੀਂਤਾਂ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAL_SAVEPOINT_NDATA 4

/* Connection to a write-ahead log (WAL) file. 
** There is one object of this type for each pager. 
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Wal Wal;

/* Open and close a connection to a write-ahead log. */
ਪੂਰਨ sqlite3WalOpen(sqlite3_vfs*, sqlite3_file*, ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ, i64, Wal**);
ਪੂਰਨ sqlite3WalClose(Wal *pWal, sqlite3*, ਪੂਰਨ sync_flags, ਪੂਰਨ, u8 *);

/* Set the limiting size of a WAL file. */
ਖਾਲੀ sqlite3_Wal_ਹੱਦਬੰਦੀ(Wal*, i64);

/* Used by readers to open (lock) and close (unlock) a snapshot.  A 
** snapshot is like a read-transaction.  It is the state of the database
** at an instant in time.  sqlite3WalOpenSnapshot gets a read lock and
** preserves the current state even if the other threads or processes
** write to or checkpoint the WAL.  sqlite3WalCloseSnapshot() closes the
** transaction and releases the lock.
*/
ਪੂਰਨ sqlite3WalBeginReadTransaction(Wal *pWal, ਪੂਰਨ *);
ਖਾਲੀ sqlite3_Wal_ਸਿਰਾ_Read_Transaction(Wal *pWal);

/* Read a page from the write-ahead log, if it is present. */
ਪੂਰਨ sqlite3_Wal_ਲੱਭੋ_ਚੌਖਟਾ(Wal *, Pgno, u32 *);
ਪੂਰਨ sqlite3_Wal_Read_ਚੌਖਟਾ(Wal *, u32, ਪੂਰਨ, u8 *);

/* If the WAL is not empty, return the size of the database. */
Pgno sqlite3WalDbsize(Wal *pWal);

/* Obtain or release the WRITER lock. */
ਪੂਰਨ sqlite3WalBeginWriteTransaction(Wal *pWal);
ਪੂਰਨ sqlite3_Wal_ਸਿਰਾ_Write_Transaction(Wal *pWal);

/* Undo any frames written (but not committed) to the log */
ਪੂਰਨ sqlite3WalUndo(Wal *pWal, ਪੂਰਨ (*xUndo)(ਖਾਲੀ *, Pgno), ਖਾਲੀ *pUndoCtx);

/* Return an integer that records the current (uncommitted) write
** position in the WAL */
ਖਾਲੀ sqlite3WalSavepoint(Wal *pWal, u32 *a_Wal_ਅੰਕੜੇ);

/* Move the write position of the WAL back to iFrame.  Called in
** response to a ROLLBACK TO command. */
ਪੂਰਨ sqlite3WalSavepointUndo(Wal *pWal, u32 *a_Wal_ਅੰਕੜੇ);

/* Write a frame or frames to the log. */
ਪੂਰਨ sqlite3_Wal_ਚੌਖਟੇ(Wal *pWal, ਪੂਰਨ, PgHdr *, Pgno, ਪੂਰਨ, ਪੂਰਨ);

/* Copy pages from the log to the database file */ 
ਪੂਰਨ sqlite3WalCheckpoint(
  Wal *pWal,                      /* Write-ahead log connection */
  sqlite3 *db,                    /* Check this handle's interrupt flag */
  ਪੂਰਨ e_ਬਹੁਲਕ,                      /* One of PASSIVE, FULL and RESTART */
  ਪੂਰਨ (*xBusy)(ਖਾਲੀ*),            /* Function to call when busy */
  ਖਾਲੀ *pBusyArg,                 /* Context argument for xBusyHandler */
  ਪੂਰਨ sync_flags,                 /* Flags to sync db file with (or 0) */
  ਪੂਰਨ nBuf,                       /* Size of buffer nBuf */
  u8 *zBuf,                       /* Temporary buffer to use */
  ਪੂਰਨ *pnLog,                     /* OUT: Number of frames in WAL */
  ਪੂਰਨ *pnCkpt                     /* OUT: Number of backfilled frames in WAL */
);

/* Return the value to pass to a sqlite3_wal_hook callback, the
** number of frames in the WAL at the point of the last commit since
** sqlite3WalCallback() was called.  If no commits have occurred since
** the last call, then return 0.
*/
ਪੂਰਨ sqlite3WalCallback(Wal *pWal);

/* Tell the wal layer that an EXCLUSIVE lock has been obtained (or released)
** by the pager layer on the database file.
*/
ਪੂਰਨ sqlite3_Wal_Exclusive_ਬਹੁਲਕ(Wal *pWal, ਪੂਰਨ op);

/* Return true if the argument is non-NULL and the WAL module is using
** heap-memory for the wal-index. Otherwise, if the argument is NULL or the
** WAL module is using shared-memory, return false. 
*/
ਪੂਰਨ sqlite3_Wal_ਢੇਰ_ਸਮ੍ਰਿਤੀ(Wal *pWal);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SNAPSHOT
ਪੂਰਨ sqlite3WalSnapshotGet(Wal *pWal, sqlite3_snapshot **ppSnapshot);
ਖਾਲੀ sqlite3WalSnapshotOpen(Wal *pWal, sqlite3_snapshot *pSnapshot);
ਪੂਰਨ sqlite3WalSnapshotRecover(Wal *pWal);
ਪੂਰਨ sqlite3WalSnapshotCheck(Wal *pWal, sqlite3_snapshot *pSnapshot);
ਖਾਲੀ sqlite3WalSnapshotUnlock(Wal *pWal);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ZIPVFS
/* If the WAL file is not empty, return the number of bytes of content
** stored in each frame (i.e. the db page-size when the WAL was created).
*/
ਪੂਰਨ sqlite3WalFramesize(Wal *pWal);
#ਜੇ_ਅੰਤ

/* Return the sqlite3_file object for the WAL file */
sqlite3_file *sqlite3WalFile(Wal *pWal);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
ਪੂਰਨ sqlite3_Wal_Write_ਤਾਲਾ(Wal *pWal, ਪੂਰਨ b_ਤਾਲਾ);
ਖਾਲੀ sqlite3WalDb(Wal *pWal, sqlite3 *db);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_USE_SEH
ਪੂਰਨ sqlite3WalSystemErrno(Wal*);
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  ਖਾਲੀ sqlite3WalStat(Wal*,sqlite3_str*);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_WAL */
#ਜੇ_ਅੰਤ /* SQLITE_WAL_H */
