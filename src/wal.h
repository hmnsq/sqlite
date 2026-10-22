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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WAL_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_WAL_H

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/* Macros for extracting appropriate sync flags for either transaction
** commits (WAL_SYNC_FLAGS(X)) or for checkpoint ops (CKPT_SYNC_FLAGS(X)):
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WAL_SYNC_FLAGS(X)   ((X)&0x03)
#ವ್ಯಾಖ್ಯಾನಿಸು CKPT_SYNC_FLAGS(X)  (((X)>>2)&0x03)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WAL
# define sqlite3WalOpen(x,y,z)                   0
# define sqlite3_Wal_ಎಲ್ಲೆ(x,y)
# define sqlite3WalClose(v,w,x,y,z)              0
# define sqlite3WalBeginReadTransaction(y,z)     0
# define sqlite3_Wal_ಕೊನೆ_Read_Transaction(z)
# define sqlite3WalDbsize(y)                     0
# define sqlite3WalBeginWriteTransaction(y)      0
# define sqlite3_Wal_ಕೊನೆ_Write_Transaction(x)        0
# define sqlite3WalUndo(x,y,z)                   0
# define sqlite3WalSavepoint(y,z)
# define sqlite3WalSavepointUndo(y,z)            0
# define sqlite3_Wal_ಚೌಕಟ್ಟುಗಳು(u,v,w,x,y,z)           0
# define sqlite3WalCheckpoint(q,r,s,t,u,v,w,x,y,z) 0
# define sqlite3WalCallback(z)                   0
# define sqlite3_Wal_Exclusive_ಬಹುಲಕ(y,z)            0
# define sqlite3_Wal_ರಾಶಿ_ಸ್ಮರಣೆ(z)                 0
# define sqlite3WalFramesize(z)                  0
# define sqlite3_Wal_ಕಂಡುಹಿಡಿ_ಚೌಕಟ್ಟು(x,y,z)              0
# define sqlite3WalFile(x)                       0
# undef SQLITE_USE_SEH
#ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿಸು WAL_SAVEPOINT_NDATA 4

/* Connection to a write-ahead log (WAL) file. 
** There is one object of this type for each pager. 
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Wal Wal;

/* Open and close a connection to a write-ahead log. */
ಪೂರ್ಣಾಂಕ sqlite3WalOpen(sqlite3_vfs*, sqlite3_file*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ, i64, Wal**);
ಪೂರ್ಣಾಂಕ sqlite3WalClose(Wal *pWal, sqlite3*, ಪೂರ್ಣಾಂಕ sync_flags, ಪೂರ್ಣಾಂಕ, u8 *);

/* Set the limiting size of a WAL file. */
ಶೂನ್ಯ sqlite3_Wal_ಎಲ್ಲೆ(Wal*, i64);

/* Used by readers to open (lock) and close (unlock) a snapshot.  A 
** snapshot is like a read-transaction.  It is the state of the database
** at an instant in time.  sqlite3WalOpenSnapshot gets a read lock and
** preserves the current state even if the other threads or processes
** write to or checkpoint the WAL.  sqlite3WalCloseSnapshot() closes the
** transaction and releases the lock.
*/
ಪೂರ್ಣಾಂಕ sqlite3WalBeginReadTransaction(Wal *pWal, ಪೂರ್ಣಾಂಕ *);
ಶೂನ್ಯ sqlite3_Wal_ಕೊನೆ_Read_Transaction(Wal *pWal);

/* Read a page from the write-ahead log, if it is present. */
ಪೂರ್ಣಾಂಕ sqlite3_Wal_ಕಂಡುಹಿಡಿ_ಚೌಕಟ್ಟು(Wal *, Pgno, u32 *);
ಪೂರ್ಣಾಂಕ sqlite3_Wal_Read_ಚೌಕಟ್ಟು(Wal *, u32, ಪೂರ್ಣಾಂಕ, u8 *);

/* If the WAL is not empty, return the size of the database. */
Pgno sqlite3WalDbsize(Wal *pWal);

/* Obtain or release the WRITER lock. */
ಪೂರ್ಣಾಂಕ sqlite3WalBeginWriteTransaction(Wal *pWal);
ಪೂರ್ಣಾಂಕ sqlite3_Wal_ಕೊನೆ_Write_Transaction(Wal *pWal);

/* Undo any frames written (but not committed) to the log */
ಪೂರ್ಣಾಂಕ sqlite3WalUndo(Wal *pWal, ಪೂರ್ಣಾಂಕ (*xUndo)(ಶೂನ್ಯ *, Pgno), ಶೂನ್ಯ *pUndoCtx);

/* Return an integer that records the current (uncommitted) write
** position in the WAL */
ಶೂನ್ಯ sqlite3WalSavepoint(Wal *pWal, u32 *a_Wal_ದತ್ತಾಂಶ);

/* Move the write position of the WAL back to iFrame.  Called in
** response to a ROLLBACK TO command. */
ಪೂರ್ಣಾಂಕ sqlite3WalSavepointUndo(Wal *pWal, u32 *a_Wal_ದತ್ತಾಂಶ);

/* Write a frame or frames to the log. */
ಪೂರ್ಣಾಂಕ sqlite3_Wal_ಚೌಕಟ್ಟುಗಳು(Wal *pWal, ಪೂರ್ಣಾಂಕ, PgHdr *, Pgno, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);

/* Copy pages from the log to the database file */ 
ಪೂರ್ಣಾಂಕ sqlite3WalCheckpoint(
  Wal *pWal,                      /* Write-ahead log connection */
  sqlite3 *db,                    /* Check this handle's interrupt flag */
  ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ,                      /* One of PASSIVE, FULL and RESTART */
  ಪೂರ್ಣಾಂಕ (*xBusy)(ಶೂನ್ಯ*),            /* Function to call when busy */
  ಶೂನ್ಯ *pBusyArg,                 /* Context argument for xBusyHandler */
  ಪೂರ್ಣಾಂಕ sync_flags,                 /* Flags to sync db file with (or 0) */
  ಪೂರ್ಣಾಂಕ nBuf,                       /* Size of buffer nBuf */
  u8 *zBuf,                       /* Temporary buffer to use */
  ಪೂರ್ಣಾಂಕ *pnLog,                     /* OUT: Number of frames in WAL */
  ಪೂರ್ಣಾಂಕ *pnCkpt                     /* OUT: Number of backfilled frames in WAL */
);

/* Return the value to pass to a sqlite3_wal_hook callback, the
** number of frames in the WAL at the point of the last commit since
** sqlite3WalCallback() was called.  If no commits have occurred since
** the last call, then return 0.
*/
ಪೂರ್ಣಾಂಕ sqlite3WalCallback(Wal *pWal);

/* Tell the wal layer that an EXCLUSIVE lock has been obtained (or released)
** by the pager layer on the database file.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Wal_Exclusive_ಬಹುಲಕ(Wal *pWal, ಪೂರ್ಣಾಂಕ op);

/* Return true if the argument is non-NULL and the WAL module is using
** heap-memory for the wal-index. Otherwise, if the argument is NULL or the
** WAL module is using shared-memory, return false. 
*/
ಪೂರ್ಣಾಂಕ sqlite3_Wal_ರಾಶಿ_ಸ್ಮರಣೆ(Wal *pWal);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SNAPSHOT
ಪೂರ್ಣಾಂಕ sqlite3WalSnapshotGet(Wal *pWal, sqlite3_snapshot **ppSnapshot);
ಶೂನ್ಯ sqlite3WalSnapshotOpen(Wal *pWal, sqlite3_snapshot *pSnapshot);
ಪೂರ್ಣಾಂಕ sqlite3WalSnapshotRecover(Wal *pWal);
ಪೂರ್ಣಾಂಕ sqlite3WalSnapshotCheck(Wal *pWal, sqlite3_snapshot *pSnapshot);
ಶೂನ್ಯ sqlite3WalSnapshotUnlock(Wal *pWal);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ZIPVFS
/* If the WAL file is not empty, return the number of bytes of content
** stored in each frame (i.e. the db page-size when the WAL was created).
*/
ಪೂರ್ಣಾಂಕ sqlite3WalFramesize(Wal *pWal);
#ಆದರೆ_ಅಂತ್ಯ

/* Return the sqlite3_file object for the WAL file */
sqlite3_file *sqlite3WalFile(Wal *pWal);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
ಪೂರ್ಣಾಂಕ sqlite3_Wal_Write_ಬೀಗ(Wal *pWal, ಪೂರ್ಣಾಂಕ b_ಬೀಗ);
ಶೂನ್ಯ sqlite3WalDb(Wal *pWal, sqlite3 *db);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_USE_SEH
ಪೂರ್ಣಾಂಕ sqlite3WalSystemErrno(Wal*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  ಶೂನ್ಯ sqlite3WalStat(Wal*,sqlite3_str*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_WAL */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_WAL_H */
