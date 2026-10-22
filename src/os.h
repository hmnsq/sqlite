/*
** 2001 September 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This header file (together with is companion C source-code file
** "os.c") attempt to abstract the underlying operating system so that
** the SQLite library will work on both POSIX and windows systems.
**
** This header file is #include-ed by sqliteInt.h and thus ends up
** being included by every source file.
*/
#यदि_अपरिभाषितम् _SQLITE_OS_H_
#परिभाषय _SQLITE_OS_H_

/*
** Attempt to automatically detect the operating system and setup the
** necessary pre-processor macros for it.
*/
#अन्तर्भाव "os_setup.h"

/* If the SET_FULLSYNC macro is not defined above, then make it
** a no-op
*/
#यदि_अपरिभाषितम् समुच्चयः_FULLSYNC
# define समुच्चयः_FULLSYNC(x,y)
#यदि_अन्तः

/* Maximum pathname length.  Note: FILENAME_MAX defined by stdio.h
*/
#यदि_अपरिभाषितम् SQLITE_MAX_PATHLEN
# define SQLITE_MAX_PATHLEN FILENAME_MAX
#यदि_अन्तः

/* Maximum number of symlinks that will be resolved while trying to
** expand a filename in xFullPathname() in the VFS.
*/
#यदि_अपरिभाषितम् SQLITE_MAX_SYMLINK
# define SQLITE_MAX_SYMLINK 200
#यदि_अन्तः

/*
** The default size of a disk sector
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_SECTOR_आकार
# define SQLITE_DEFAULT_SECTOR_आकार 4096
#यदि_अन्तः

/*
** Temporary files are named starting with this prefix followed by 16 random
** alphanumeric characters, and no file extension. They are stored in the
** OS's standard temporary file directory, and are deleted prior to exit.
** If sqlite is being embedded in another program, you may wish to change the
** prefix to reflect your program's name, so that if your program exits
** prematurely, old temporary files can be easily identified. This can be done
** using -DSQLITE_TEMP_FILE_PREFIX=myprefix_ on the compiler command line.
**
** 2006-10-31:  The default prefix used to be "sqlite_".  But then
** Mcafee started using SQLite in their anti-virus product and it
** started putting files with the "sqlite" name in the c:/temp folder.
** This annoyed many windows users.  Those users would then do a 
** Google search for "sqlite", find the telephone numbers of the
** developers and call to wake them up at night and complain.
** For this reason, the default name prefix is changed to be "sqlite" 
** spelled backwards.  So the temp files are still identified, but
** anybody smart enough to figure out the code is also likely smart
** enough to know that calling the developer will not help get rid
** of the file.
*/
#यदि_अपरिभाषितम् SQLITE_अस्थायी_FILE_उपसर्गः
# define SQLITE_अस्थायी_FILE_उपसर्गः "etilqs_"
#यदि_अन्तः

/*
** The following values may be passed as the second argument to
** sqlite3OsLock(). The various locks exhibit the following semantics:
**
** SHARED:    Any number of processes may hold a SHARED lock simultaneously.
** RESERVED:  A single process may hold a RESERVED lock on a file at
**            any time. Other processes may hold and obtain new SHARED locks.
** PENDING:   A single process may hold a PENDING lock on a file at
**            any one time. Existing SHARED locks may persist, but no new
**            SHARED locks may be obtained by other processes.
** EXCLUSIVE: An EXCLUSIVE lock precludes all other locks.
**
** PENDING_LOCK may not be passed directly to sqlite3OsLock(). Instead, a
** process that requests an EXCLUSIVE lock may actually obtain a PENDING
** lock. This can be upgraded to an EXCLUSIVE lock by a subsequent call to
** sqlite3OsLock().
*/
#परिभाषय NO_तालकम्         0
#परिभाषय SHARED_तालकम्     1
#परिभाषय RESERVED_तालकम्   2
#परिभाषय PENDING_तालकम्    3
#परिभाषय EXCLUSIVE_तालकम्  4

/*
** File Locking Notes:  (Mostly about windows but also some info for Unix)
**
** We cannot use LockFileEx() or UnlockFileEx() on Win95/98/ME because
** those functions are not available.  So we use only LockFile() and
** UnlockFile().
**
** LockFile() prevents not just writing but also reading by other processes.
** A SHARED_LOCK is obtained by locking a single randomly-chosen 
** byte out of a specific range of bytes. The lock byte is obtained at 
** random so two separate readers can probably access the file at the 
** same time, unless they are unlucky and choose the same lock byte.
** An EXCLUSIVE_LOCK is obtained by locking all bytes in the range.
** There can only be one writer.  A RESERVED_LOCK is obtained by locking
** a single byte of the file that is designated as the reserved lock byte.
** A PENDING_LOCK is obtained by locking a designated byte different from
** the RESERVED_LOCK byte.
**
** On WinNT/2K/XP systems, LockFileEx() and UnlockFileEx() are available,
** which means we can use reader/writer locks.  When reader/writer locks
** are used, the lock is placed on the same range of bytes that is used
** for probabilistic locking in Win95/98/ME.  Hence, the locking scheme
** will support two or more Win95 readers or two or more WinNT readers.
** But a single Win95 reader will lock out all WinNT readers and a single
** WinNT reader will lock out all other Win95 readers.
**
** The following #defines specify the range of bytes used for locking.
** SHARED_SIZE is the number of bytes available in the pool from which
** a random byte is selected for a shared lock.  The pool of bytes for
** shared locks begins at SHARED_FIRST. 
**
** The same locking strategy and
** byte ranges are used for Unix.  This leaves open the possibility of having
** clients on win95, winNT, and unix all talking to the same shared file
** and all locking correctly.  To do so would require that samba (or whatever
** tool is being used for file sharing) implements locks correctly between
** windows and unix.  I'm guessing that isn't likely to happen, but by
** using the same locking range we are at least open to the possibility.
**
** Locking in windows is manditory.  For this reason, we cannot store
** actual data in the bytes used for locking.  The pager never allocates
** the pages involved in locking therefore.  SHARED_SIZE is selected so
** that all locks will fit on a single page even at the minimum page size.
** PENDING_BYTE defines the beginning of the locks.  By default PENDING_BYTE
** is set high so that we don't have to allocate an unused page except
** for very large databases.  But one should test the page skipping logic 
** by setting PENDING_BYTE low and running the entire regression suite.
**
** Changing the value of PENDING_BYTE results in a subtly incompatible
** file format.  Depending on how it is changed, you might not notice
** the incompatibility right away, even running a full regression test.
** The default location of PENDING_BYTE is the first byte past the
** 1GB boundary.
**
*/
#यदि_परिभाषितम् SQLITE_OMIT_WSD
# define PENDING_BYTE     (0x40000000)
#अन्यथा
# define PENDING_BYTE      sqlite3PendingByte
#यदि_अन्तः
#परिभाषय RESERVED_BYTE     (PENDING_BYTE+1)
#परिभाषय SHARED_FIRST      (PENDING_BYTE+2)
#परिभाषय SHARED_आकार       510

/*
** Wrapper around OS specific sqlite3_os_init() function.
*/
पूर्णाङ्क sqlite3OsInit(शून्य);

/* 
** Functions for accessing sqlite3_file methods 
*/
शून्य sqlite3OsClose(sqlite3_file*);
पूर्णाङ्क sqlite3OsRead(sqlite3_file*, शून्य*, पूर्णाङ्क amt, i64 विस्थापनम्);
पूर्णाङ्क sqlite3OsWrite(sqlite3_file*, ध्रुव शून्य*, पूर्णाङ्क amt, i64 विस्थापनम्);
पूर्णाङ्क sqlite3OsTruncate(sqlite3_file*, i64 आकार);
पूर्णाङ्क sqlite3OsSync(sqlite3_file*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Os_File_आकार(sqlite3_file*, i64 *p_आकार);
पूर्णाङ्क sqlite3_Os_तालकम्(sqlite3_file*, पूर्णाङ्क);
पूर्णाङ्क sqlite3OsUnlock(sqlite3_file*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Os_Check_Reserved_तालकम्(sqlite3_file *id, पूर्णाङ्क *p_Res_बहिर्गमनम्);
पूर्णाङ्क sqlite3OsFileControl(sqlite3_file*,पूर्णाङ्क,शून्य*);
शून्य sqlite3OsFileControlHint(sqlite3_file*,पूर्णाङ्क,शून्य*);
#परिभाषय SQLITE_FCNTL_DB_UNCHANGED 0xca093fa0
पूर्णाङ्क sqlite3_Os_Sector_आकार(sqlite3_file *id);
पूर्णाङ्क sqlite3OsDeviceCharacteristics(sqlite3_file *id);
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
पूर्णाङ्क sqlite3OsShmMap(sqlite3_file *,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,शून्य चञ्चल **);
पूर्णाङ्क sqlite3_Os_Shm_तालकम्(sqlite3_file *id, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3OsShmBarrier(sqlite3_file *id);
पूर्णाङ्क sqlite3OsShmUnmap(sqlite3_file *id, पूर्णाङ्क);
#यदि_अन्तः /* SQLITE_OMIT_WAL */
पूर्णाङ्क sqlite3OsFetch(sqlite3_file *id, i64, पूर्णाङ्क, शून्य **);
पूर्णाङ्क sqlite3OsUnfetch(sqlite3_file *, i64, शून्य *);


/* 
** Functions for accessing sqlite3_vfs methods 
*/
पूर्णाङ्क sqlite3OsOpen(sqlite3_vfs *, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क, पूर्णाङ्क *);
पूर्णाङ्क sqlite3OsDelete(sqlite3_vfs *, ध्रुव वर्ण *, पूर्णाङ्क);
पूर्णाङ्क sqlite3OsAccess(sqlite3_vfs *, ध्रुव वर्ण *, पूर्णाङ्क, पूर्णाङ्क *p_Res_बहिर्गमनम्);
पूर्णाङ्क sqlite3_Os_पूर्णम्_Pathname(sqlite3_vfs *, ध्रुव वर्ण *, पूर्णाङ्क, वर्ण *);
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
शून्य *sqlite3OsDlOpen(sqlite3_vfs *, ध्रुव वर्ण *);
शून्य sqlite3OsDlError(sqlite3_vfs *, पूर्णाङ्क, वर्ण *);
शून्य (*sqlite3OsDlSym(sqlite3_vfs *, शून्य *, ध्रुव वर्ण *))(शून्य);
शून्य sqlite3OsDlClose(sqlite3_vfs *, शून्य *);
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */
पूर्णाङ्क sqlite3OsRandomness(sqlite3_vfs *, पूर्णाङ्क, वर्ण *);
पूर्णाङ्क sqlite3OsSleep(sqlite3_vfs *, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Os_Get_चरमः_Error(sqlite3_vfs*);
पूर्णाङ्क sqlite3OsCurrentTimeInt64(sqlite3_vfs *, sqlite3_int64*);

/*
** Convenience functions for opening and closing files using 
** sqlite3_malloc() to obtain space for the file-handle structure.
*/
पूर्णाङ्क sqlite3OsOpenMalloc(sqlite3_vfs *, ध्रुव वर्ण *, sqlite3_file **, पूर्णाङ्क,पूर्णाङ्क*);
शून्य sqlite3OsCloseFree(sqlite3_file *);

#यदि_अन्तः /* _SQLITE_OS_H_ */
