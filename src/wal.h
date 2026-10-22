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

#ifndef SQLITE_WAL_H
#define SQLITE_WAL_H

#include "sqliteInt.h"

/* Macros for extracting appropriate sync flags for either transaction
** commits (WAL_SYNC_FLAGS(X)) or for checkpoint ops (CKPT_SYNC_FLAGS(X)):
*/
#define WAL_SYNC_FLAGS(X)   ((X)&0x03)
#define CKPT_SYNC_FLAGS(X)  (((X)>>2)&0x03)

#ifdef SQLITE_OMIT_WAL
# define sqlite3WalOpen(x,y,z)                   0
# define sqlite3WalLimit(x,y)
# define sqlite3WalClose(v,w,x,y,z)              0
# define sqlite3WalBeginReadTransaction(y,z)     0
# define sqlite3WalEndReadTransaction(z)
# define sqlite3WalDbsize(y)                     0
# define sqlite3WalBeginWriteTransaction(y)      0
# define sqlite3WalEndWriteTransaction(x)        0
# define sqlite3WalUndo(x,y,z)                   0
# define sqlite3WalSavepoint(y,z)
# define sqlite3WalSavepointUndo(y,z)            0
# define sqlite3WalFrames(u,v,w,x,y,z)           0
# define sqlite3WalCheckpoint(q,r,s,t,u,v,w,x,y,z) 0
# define sqlite3WalCallback(z)                   0
# define sqlite3WalExclusiveMode(y,z)            0
# define sqlite3WalHeapMemory(z)                 0
# define sqlite3WalFramesize(z)                  0
# define sqlite3WalFindFrame(x,y,z)              0
# define sqlite3WalFile(x)                       0
# undef SQLITE_USE_SEH
#아니면

#define WAL_SAVEPOINT_NDATA 4

/* Connection to a write-ahead log (WAL) file. 
** There is one object of this type for each pager. 
*/
typedef struct Wal Wal;

/* Open and close a connection to a write-ahead log. */
정수 sqlite3WalOpen(sqlite3_vfs*, sqlite3_file*, const char *, 정수, i64, Wal**);
정수 sqlite3WalClose(Wal *pWal, sqlite3*, 정수 sync_flags, 정수, u8 *);

/* Set the limiting size of a WAL file. */
void sqlite3WalLimit(Wal*, i64);

/* Used by readers to open (lock) and close (unlock) a snapshot.  A 
** snapshot is like a read-transaction.  It is the state of the database
** at an instant in time.  sqlite3WalOpenSnapshot gets a read lock and
** preserves the current state even if the other threads or processes
** write to or checkpoint the WAL.  sqlite3WalCloseSnapshot() closes the
** transaction and releases the lock.
*/
정수 sqlite3WalBeginReadTransaction(Wal *pWal, 정수 *);
void sqlite3WalEndReadTransaction(Wal *pWal);

/* Read a page from the write-ahead log, if it is present. */
정수 sqlite3WalFindFrame(Wal *, Pgno, u32 *);
정수 sqlite3WalReadFrame(Wal *, u32, 정수, u8 *);

/* If the WAL is not empty, return the size of the database. */
Pgno sqlite3WalDbsize(Wal *pWal);

/* Obtain or release the WRITER lock. */
정수 sqlite3WalBeginWriteTransaction(Wal *pWal);
정수 sqlite3WalEndWriteTransaction(Wal *pWal);

/* Undo any frames written (but not committed) to the log */
정수 sqlite3WalUndo(Wal *pWal, 정수 (*xUndo)(void *, Pgno), void *pUndoCtx);

/* Return an integer that records the current (uncommitted) write
** position in the WAL */
void sqlite3WalSavepoint(Wal *pWal, u32 *aWalData);

/* Move the write position of the WAL back to iFrame.  Called in
** response to a ROLLBACK TO command. */
정수 sqlite3WalSavepointUndo(Wal *pWal, u32 *aWalData);

/* Write a frame or frames to the log. */
정수 sqlite3WalFrames(Wal *pWal, 정수, PgHdr *, Pgno, 정수, 정수);

/* Copy pages from the log to the database file */ 
정수 sqlite3WalCheckpoint(
  Wal *pWal,                      /* Write-ahead log connection */
  sqlite3 *db,                    /* Check this handle's interrupt flag */
  정수 eMode,                      /* One of PASSIVE, FULL and RESTART */
  정수 (*xBusy)(void*),            /* Function to call when busy */
  void *pBusyArg,                 /* Context argument for xBusyHandler */
  정수 sync_flags,                 /* Flags to sync db file with (or 0) */
  정수 nBuf,                       /* Size of buffer nBuf */
  u8 *zBuf,                       /* Temporary buffer to use */
  정수 *pnLog,                     /* OUT: Number of frames in WAL */
  정수 *pnCkpt                     /* OUT: Number of backfilled frames in WAL */
);

/* Return the value to pass to a sqlite3_wal_hook callback, the
** number of frames in the WAL at the point of the last commit since
** sqlite3WalCallback() was called.  If no commits have occurred since
** the last call, then return 0.
*/
정수 sqlite3WalCallback(Wal *pWal);

/* Tell the wal layer that an EXCLUSIVE lock has been obtained (or released)
** by the pager layer on the database file.
*/
정수 sqlite3WalExclusiveMode(Wal *pWal, 정수 op);

/* Return true if the argument is non-NULL and the WAL module is using
** heap-memory for the wal-index. Otherwise, if the argument is NULL or the
** WAL module is using shared-memory, return false. 
*/
정수 sqlite3WalHeapMemory(Wal *pWal);

#ifdef SQLITE_ENABLE_SNAPSHOT
정수 sqlite3WalSnapshotGet(Wal *pWal, sqlite3_snapshot **ppSnapshot);
void sqlite3WalSnapshotOpen(Wal *pWal, sqlite3_snapshot *pSnapshot);
정수 sqlite3WalSnapshotRecover(Wal *pWal);
정수 sqlite3WalSnapshotCheck(Wal *pWal, sqlite3_snapshot *pSnapshot);
void sqlite3WalSnapshotUnlock(Wal *pWal);
#endif

#ifdef SQLITE_ENABLE_ZIPVFS
/* If the WAL file is not empty, return the number of bytes of content
** stored in each frame (i.e. the db page-size when the WAL was created).
*/
정수 sqlite3WalFramesize(Wal *pWal);
#endif

/* Return the sqlite3_file object for the WAL file */
sqlite3_file *sqlite3WalFile(Wal *pWal);

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
정수 sqlite3WalWriteLock(Wal *pWal, 정수 bLock);
void sqlite3WalDb(Wal *pWal, sqlite3 *db);
#endif

#ifdef SQLITE_USE_SEH
정수 sqlite3WalSystemErrno(Wal*);
#endif

#만약 defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  void sqlite3WalStat(Wal*,sqlite3_str*);
#endif

#endif /* ifndef SQLITE_OMIT_WAL */
#endif /* SQLITE_WAL_H */
