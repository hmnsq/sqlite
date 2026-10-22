/*
** 2005 May 23 
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
** This file contains functions used to access the internal hash tables
** of user defined functions and collation sequences.
*/

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Invoke the 'collation needed' callback to request a collation sequence
** in the encoding enc of name zName, length nName.
*/
ಸ್ಥಿರ ಶೂನ್ಯ callCollNeeded(sqlite3 *db, ಪೂರ್ಣಾಂಕ enc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಪ್ರತಿಜ್ಞಾ( !db->xCollNeeded || !db->xCollNeeded16 );
  ಆದರೆ( db->xCollNeeded ){
    ಅಕ್ಷರ *zExternal = sqlite3DbStrDup(db, zName);
    ಆದರೆ( !zExternal ) ಹಿಂತಿರುಗಿ;
    db->xCollNeeded(db->pCollNeededArg, db, enc, zExternal);
    sqlite3DbFree(db, zExternal);
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
  ಆದರೆ( db->xCollNeeded16 ){
    ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zExternal;
    sqlite3_ಮೌಲ್ಯ *pTmp = sqlite3_ಮೌಲ್ಯ_New(db);
    sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(pTmp, -1, zName, SQLITE_UTF8, SQLITE_STATIC);
    zExternal = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pTmp, SQLITE_UTF16NATIVE);
    ಆದರೆ( zExternal ){
      db->xCollNeeded16(db->pCollNeededArg, db, (ಪೂರ್ಣಾಂಕ)ENC(db), zExternal);
    }
    sqlite3_ಮೌಲ್ಯ_Free(pTmp);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** This routine is called if the collation factory fails to deliver a
** collation function in the best encoding but there may be other versions
** of this collation function (for other text encodings) available. Use one
** of these instead if they exist. Avoid a UTF-8 <-> UTF-16 conversion if
** possible.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ synthCollSeq(sqlite3 *db, CollSeq *pColl){
  CollSeq *pColl2;
  ಅಕ್ಷರ *z = pColl->zName;
  ಪೂರ್ಣಾಂಕ i;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 aEnc[] = { SQLITE_UTF16BE, SQLITE_UTF16LE, SQLITE_UTF8 };
  ಗಾಗಿ(i=0; i<3; i++){
    pColl2 = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, aEnc[i], z, 0);
    ಆದರೆ( pColl2->xCmp!=0 ){
      ಸ್ಮರಣೆ_ನಕಲು(pColl, pColl2, ಗಾತ್ರ(CollSeq));
      pColl->xDel = 0;         /* Do not copy the destructor */
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_ERROR;
}

/*
** This routine is called on a collation sequence before it is used to
** check that it is defined. An undefined collation sequence exists when
** a database is loaded that contains references to collation sequences
** that have not been defined by sqlite3_create_collation() etc.
**
** If required, this routine calls the 'collation needed' callback to
** request a definition of the collating sequence. If this doesn't work, 
** an equivalent collating sequence that uses a text encoding different
** from the main database is substituted, if one is available.
*/
ಪೂರ್ಣಾಂಕ sqlite3CheckCollSeq(Parse *pParse, CollSeq *pColl){
  ಆದರೆ( pColl && pColl->xCmp==0 ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = pColl->zName;
    sqlite3 *db = pParse->db;
    CollSeq *p = sqlite3GetCollSeq(pParse, ENC(db), pColl, zName);
    ಆದರೆ( !p ){
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
    ಪ್ರತಿಜ್ಞಾ( p==pColl );
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}



/*
** Locate and return an entry from the db.aCollSeq hash table. If the entry
** specified by zName and nName is not found and parameter 'create' is
** true, then create a new entry. Otherwise return NULL.
**
** Each pointer stored in the sqlite3.aCollSeq hash table contains an
** array of three CollSeq structures. The first is the collation sequence
** preferred for UTF-8, the second UTF-16le, and the third UTF-16be.
**
** Stored immediately after the three collation sequences is a copy of
** the collation sequence name. A pointer to this string is stored in
** each collation sequence structure.
*/
ಸ್ಥಿರ CollSeq *ಕಂಡುಹಿಡಿ_Coll_Seq_ನಮೂದು(
  sqlite3 *db,          /* Database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,    /* Name of the collating sequence */
  ಪೂರ್ಣಾಂಕ create            /* Create a new entry if true */
){
  CollSeq *pColl;
  pColl = sqlite3_Hash_ಕಂಡುಹಿಡಿ(&db->aCollSeq, zName);

  ಆದರೆ( 0==pColl && create ){
    ಪೂರ್ಣಾಂಕ nName = sqlite3Strlen30(zName) + 1;
    pColl = sqlite3DbMallocZero(db, 3*ಗಾತ್ರ(*pColl) + nName);
    ಆದರೆ( pColl ){
      CollSeq *pDel = 0;
      pColl[0].zName = (ಅಕ್ಷರ*)&pColl[3];
      pColl[0].enc = SQLITE_UTF8;
      pColl[1].zName = (ಅಕ್ಷರ*)&pColl[3];
      pColl[1].enc = SQLITE_UTF16LE;
      pColl[2].zName = (ಅಕ್ಷರ*)&pColl[3];
      pColl[2].enc = SQLITE_UTF16BE;
      ಸ್ಮರಣೆ_ನಕಲು(pColl[0].zName, zName, nName);
      pDel = sqlite3_Hash_ಸೇರಿಸು(&db->aCollSeq, pColl[0].zName, pColl);

      /* If a malloc() failure occurred in sqlite3HashInsert(), it will 
      ** return the pColl pointer to be deleted (because it wasn't added
      ** to the hash table).
      */
      ಪ್ರತಿಜ್ಞಾ( pDel==0 || pDel==pColl );
      ಆದರೆ( pDel!=0 ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, pDel);
        pColl = 0;
      }
    }
  }
  ಹಿಂತಿರುಗಿ pColl;
}

/*
** Parameter zName points to a UTF-8 encoded string nName bytes long.
** Return the CollSeq* pointer for the collation sequence named zName
** for the encoding 'enc' from the database 'db'.
**
** If the entry specified is not found and 'create' is true, then create a
** new entry.  Otherwise return NULL.
**
** A separate function sqlite3LocateCollSeq() is a wrapper around
** this routine.  sqlite3LocateCollSeq() invokes the collation factory
** if necessary and generates an error message if the collating sequence
** cannot be found.
**
** See also: sqlite3LocateCollSeq(), sqlite3GetCollSeq()
*/
CollSeq *sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(
  sqlite3 *db,          /* Database connection to search */
  u8 enc,               /* Desired text encoding */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,    /* Name of the collating sequence.  Might be NULL */
  ಪೂರ್ಣಾಂಕ create            /* True to create CollSeq if doesn't already exist */
){
  CollSeq *pColl;
  ಪ್ರತಿಜ್ಞಾ( SQLITE_UTF8==1 && SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
  ಪ್ರತಿಜ್ಞಾ( enc>=SQLITE_UTF8 && enc<=SQLITE_UTF16BE );
  ಆದರೆ( zName ){
    pColl = ಕಂಡುಹಿಡಿ_Coll_Seq_ನಮೂದು(db, zName, create);
    ಆದರೆ( pColl ) pColl += enc-1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pColl = db->pDfltColl;
  }
  ಹಿಂತಿರುಗಿ pColl;
}

/*
** Change the text encoding for a database connection. This means that
** the pDfltColl must change as well.
*/
ಶೂನ್ಯ sqlite3_ಗಣ_ಪಠ್ಯ_Encoding(sqlite3 *db, u8 enc){
  ಪ್ರತಿಜ್ಞಾ( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  db->enc = enc;
  /* EVIDENCE-OF: R-08308-17224 The default collating function for all
  ** strings is BINARY. 
  */
  db->pDfltColl = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, enc, sqlite3_Str_ದ್ವಿಮಾನ, 0);
  sqlite3ExpirePreparedStatements(db, 1);
}

/*
** This function is responsible for invoking the collation factory callback
** or substituting a collation sequence of a different encoding when the
** requested collation sequence is not available in the desired encoding.
** 
** If it is not NULL, then pColl must point to the database native encoding 
** collation sequence with name zName, length nName.
**
** The return value is either the collation sequence to be used in database
** db for collation type name zName, length nName, or NULL, if no collation
** sequence can be found.  If no collation is found, leave an error message.
**
** See also: sqlite3LocateCollSeq(), sqlite3FindCollSeq()
*/
CollSeq *sqlite3GetCollSeq(
  Parse *pParse,        /* Parsing context */
  u8 enc,               /* The desired encoding for the collating sequence */
  CollSeq *pColl,       /* Collating sequence with native encoding, or NULL */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName     /* Collating sequence name */
){
  CollSeq *p;
  sqlite3 *db = pParse->db;

  p = pColl;
  ಆದರೆ( !p ){
    p = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, enc, zName, 0);
  }
  ಆದರೆ( !p || !p->xCmp ){
    /* No collation sequence of this type for this encoding is registered.
    ** Call the collation factory to see if it can supply us with one.
    */
    callCollNeeded(db, enc, zName);
    p = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, enc, zName, 0);
  }
  ಆದರೆ( p && !p->xCmp && synthCollSeq(db, p) ){
    p = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( !p || p->xCmp );
  ಆದರೆ( p==0 ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such collation sequence: %s", zName);
    pParse->rc = SQLITE_ERROR_MISSING_COLLSEQ;
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** This function returns the collation sequence for database native text
** encoding identified by the string zName.
**
** If the requested collation sequence is not available, or not available
** in the database native encoding, the collation factory is invoked to
** request it. If the collation factory does not supply such a sequence,
** and the sequence is available in another text encoding, then that is
** returned instead.
**
** If no versions of the requested collations sequence are available, or
** another error occurs, NULL is returned and an error message written into
** pParse.
**
** This routine is a wrapper around sqlite3FindCollSeq().  This routine
** invokes the collation factory if the named collation cannot be found
** and generates an error message.
**
** See also: sqlite3FindCollSeq(), sqlite3GetCollSeq()
*/
CollSeq *sqlite3LocateCollSeq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  sqlite3 *db = pParse->db;
  u8 enc = ENC(db);
  u8 initbusy = db->init.busy;
  CollSeq *pColl;

  pColl = sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(db, enc, zName, initbusy);
  ಆದರೆ( !initbusy && (!pColl || !pColl->xCmp) ){
    pColl = sqlite3GetCollSeq(pParse, enc, pColl, zName);
  }

  ಹಿಂತಿರುಗಿ pColl;
}

/* During the search for the best function definition, this procedure
** is called to test how well the function passed as the first argument
** matches the request for a function with nArg arguments in a system
** that uses encoding enc. The value returned indicates how well the
** request is matched. A higher value indicates a better match.
**
** If nArg is -1 that means to only return a match (non-zero) if p->nArg
** is also -1.  In other words, we are searching for a function that
** takes a variable number of arguments.
**
** If nArg is -2 that means that we are searching for any function 
** regardless of the number of arguments it uses, so return a positive
** match score for any
**
** The returned value is always between 0 and 6, as follows:
**
** 0: Not a match.
** 1: UTF8/16 conversion required and function takes any number of arguments.
** 2: UTF16 byte order change required and function takes any number of args.
** 3: encoding matches and function takes any number of arguments
** 4: UTF8/16 conversion required - argument count matches exactly
** 5: UTF16 byte order conversion required - argument count matches exactly
** 6: Perfect match:  encoding and argument count match exactly.
**
** If nArg==(-2) then any function with a non-null xSFunc is
** a perfect match and any function with xSFunc NULL is
** a non-match.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು FUNC_PERFECT_ಹೊಂದಿಕೆ 6  /* The score for a perfect match */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ_Quality(
  FuncDef *p,     /* The function we are evaluating for match quality */
  ಪೂರ್ಣಾಂಕ nArg,       /* Desired number of arguments.  (-1)==any */
  u8 enc          /* Desired text encoding */
){
  ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ;
  ಪ್ರತಿಜ್ಞಾ( p->nArg>=(-4) && p->nArg!=(-2) );
  ಪ್ರತಿಜ್ಞಾ( nArg>=(-2) );

  /* Wrong number of arguments means "no match" */
  ಆದರೆ( p->nArg!=nArg ){
    ಆದರೆ( nArg==(-2) ) ಹಿಂತಿರುಗಿ p->xSFunc==0 ? 0 : FUNC_PERFECT_ಹೊಂದಿಕೆ;
    ಆದರೆ( p->nArg>=0 ) ಹಿಂತಿರುಗಿ 0;
    /* Special p->nArg values available to built-in functions only:
    **    -3     1 or more arguments required
    **    -4     2 or more arguments required
    */
    ಆದರೆ( p->nArg<(-2) && nArg<(-2-p->nArg) ) ಹಿಂತಿರುಗಿ 0;
  }

  /* Give a better score to a function with a specific number of arguments
  ** than to function that accepts any number of arguments. */
  ಆದರೆ( p->nArg==nArg ){
    ಹೊಂದಿಕೆ = 4;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹೊಂದಿಕೆ = 1;
  }

  /* Bonus points if the text encoding matches */
  ಆದರೆ( enc==(p->funcFlags & SQLITE_FUNC_ENCMASK) ){
    ಹೊಂದಿಕೆ += 2;  /* Exact encoding match */
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (enc & p->funcFlags & 2)!=0 ){
    ಹೊಂದಿಕೆ += 1;  /* Both are UTF16, but with different byte orders */
  }

  ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ;
}

/*
** Search a FuncDefHash for a function with the given name.  Return
** a pointer to the matching FuncDef if found, or 0 if there is no match.
*/
FuncDef *sqlite3_Function_ಹುಡುಕು(
  ಪೂರ್ಣಾಂಕ h,               /* Hash of the name */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc    /* Name of function */
){
  FuncDef *p;
  ಗಾಗಿ(p=sqlite3BuiltinFunctions.a[h]; p; p=p->u.pHash){
    ಪ್ರತಿಜ್ಞಾ( p->funcFlags & SQLITE_FUNC_BUILTIN );
    ಆದರೆ( sqlite3StrICmp(p->zName, zFunc)==0 ){
      ಹಿಂತಿರುಗಿ p;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Insert a new FuncDef into a FuncDefHash hash table.
*/
ಶೂನ್ಯ sqlite3_ಸೇರಿಸು_Builtin_Funcs(
  FuncDef *aDef,      /* List of global functions to be inserted */
  ಪೂರ್ಣಾಂಕ nDef            /* Length of the apDef[] list */
){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<nDef; i++){
    FuncDef *pOther;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = aDef[i].zName;
    ಪೂರ್ಣಾಂಕ nName = sqlite3Strlen30(zName);
    ಪೂರ್ಣಾಂಕ h = SQLITE_FUNC_HASH(zName[0], nName);
    ಪ್ರತಿಜ್ಞಾ( aDef[i].funcFlags & SQLITE_FUNC_BUILTIN );
    pOther = sqlite3_Function_ಹುಡುಕು(h, zName);
    ಆದರೆ( pOther ){
      ಪ್ರತಿಜ್ಞಾ( pOther!=&aDef[i] && pOther->p_ಮುಂದಿನ!=&aDef[i] );
      aDef[i].p_ಮುಂದಿನ = pOther->p_ಮುಂದಿನ;
      pOther->p_ಮುಂದಿನ = &aDef[i];
    }ಇಲ್ಲದಿದ್ದರೆ{
      aDef[i].p_ಮುಂದಿನ = 0;
      aDef[i].u.pHash = sqlite3BuiltinFunctions.a[h];
      sqlite3BuiltinFunctions.a[h] = &aDef[i];
    }
  }
}
  
  

/*
** Locate a user function given a name, a number of arguments and a flag
** indicating whether the function prefers UTF-16 over UTF-8.  Return a
** pointer to the FuncDef structure that defines that function, or return
** NULL if the function does not exist.
**
** If the createFlag argument is true, then a new (blank) FuncDef
** structure is created and liked into the "db" structure if a
** no matching function previously existed.
**
** If nArg is -2, then the first valid function found is returned.  A
** function is valid if xSFunc is non-zero.  The nArg==(-2)
** case is used to see if zName is a valid function name for some number
** of arguments.  If nArg is -2, then createFlag must be 0.
**
** If createFlag is false, then a function with the required name and
** number of arguments may be returned even if the eTextRep flag does not
** match that requested.
*/
FuncDef *sqlite3_ಕಂಡುಹಿಡಿ_Function(
  sqlite3 *db,       /* An open database */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, /* Name of the function.  zero-terminated */
  ಪೂರ್ಣಾಂಕ nArg,          /* Number of arguments.  -1 means any number */
  u8 enc,            /* Preferred text encoding */
  u8 createFlag      /* Create new entry if true and does not otherwise exist */
){
  FuncDef *p;         /* Iterator variable */
  FuncDef *p_ಉತ್ತಮ = 0; /* Best match found so far */
  ಪೂರ್ಣಾಂಕ ಉತ್ತಮ_Score = 0;  /* Score of best match */
  ಪೂರ್ಣಾಂಕ h;              /* Hash value */
  ಪೂರ್ಣಾಂಕ nName;          /* Length of the name */

  ಪ್ರತಿಜ್ಞಾ( nArg>=(-2) );
  ಪ್ರತಿಜ್ಞಾ( nArg>=(-1) || createFlag==0 );
  nName = sqlite3Strlen30(zName);

  /* First search for a match amongst the application-defined functions.
  */
  p = (FuncDef*)sqlite3_Hash_ಕಂಡುಹಿಡಿ(&db->aFunc, zName);
  ಆಗ( p ){
    ಪೂರ್ಣಾಂಕ score = ಹೊಂದಿಕೆ_Quality(p, nArg, enc);
    ಆದರೆ( score>ಉತ್ತಮ_Score ){
      p_ಉತ್ತಮ = p;
      ಉತ್ತಮ_Score = score;
    }
    p = p->p_ಮುಂದಿನ;
  }

  /* If no match is found, search the built-in functions.
  **
  ** If the DBFLAG_PreferBuiltin flag is set, then search the built-in
  ** functions even if a prior app-defined function was found.  And give
  ** priority to built-in functions.
  **
  ** Except, if createFlag is true, that means that we are trying to
  ** install a new function.  Whatever FuncDef structure is returned it will
  ** have fields overwritten with new information appropriate for the
  ** new function.  But the FuncDefs for built-in functions are read-only.
  ** So we must not search for built-ins when creating a new function.
  */ 
  ಆದರೆ( !createFlag && (p_ಉತ್ತಮ==0 || (db->mDbFlags & DBFLAG_PreferBuiltin)!=0) ){
    ಉತ್ತಮ_Score = 0;
    h = SQLITE_FUNC_HASH(sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[(u8)zName[0]], nName);
    p = sqlite3_Function_ಹುಡುಕು(h, zName);
    ಆಗ( p ){
      ಪೂರ್ಣಾಂಕ score = ಹೊಂದಿಕೆ_Quality(p, nArg, enc);
      ಆದರೆ( score>ಉತ್ತಮ_Score ){
        p_ಉತ್ತಮ = p;
        ಉತ್ತಮ_Score = score;
      }
      p = p->p_ಮುಂದಿನ;
    }
  }

  /* If the createFlag parameter is true and the search did not reveal an
  ** exact match for the name, number of arguments and encoding, then add a
  ** new entry to the hash table and return it.
  */
  ಆದರೆ( createFlag && ಉತ್ತಮ_Score<FUNC_PERFECT_ಹೊಂದಿಕೆ && 
      (p_ಉತ್ತಮ = sqlite3DbMallocZero(db, ಗಾತ್ರ(*p_ಉತ್ತಮ)+nName+1))!=0 ){
    FuncDef *pOther;
    u8 *z;
    p_ಉತ್ತಮ->zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)&p_ಉತ್ತಮ[1];
    p_ಉತ್ತಮ->nArg = (u16)nArg;
    p_ಉತ್ತಮ->funcFlags = enc;
    ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ*)&p_ಉತ್ತಮ[1], zName, nName+1);
    ಗಾಗಿ(z=(u8*)p_ಉತ್ತಮ->zName; *z; z++) *z = sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[*z];
    pOther = (FuncDef*)sqlite3_Hash_ಸೇರಿಸು(&db->aFunc, p_ಉತ್ತಮ->zName, p_ಉತ್ತಮ);
    ಆದರೆ( pOther==p_ಉತ್ತಮ ){
      sqlite3DbFree(db, p_ಉತ್ತಮ);
      sqlite3OomFault(db);
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಉತ್ತಮ->p_ಮುಂದಿನ = pOther;
    }
  }

  ಆದರೆ( p_ಉತ್ತಮ && (p_ಉತ್ತಮ->xSFunc || createFlag) ){
    ಹಿಂತಿರುಗಿ p_ಉತ್ತಮ;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Free all resources held by the schema structure. The void* argument points
** at a Schema struct. This function does not call sqlite3DbFree(db, ) on the 
** pointer itself, it just cleans up subsidiary resources (i.e. the contents
** of the schema hash tables).
**
** The Schema.cache_size variable is not cleared.
*/
ಶೂನ್ಯ sqlite3SchemaClear(ಶೂನ್ಯ *p){
  Hash temp1;
  Hash temp2;
  HashElem *pElem;
  Schema *pSchema = (Schema *)p;
  sqlite3 xdb;

  memset(&xdb, 0, ಗಾತ್ರ(xdb));
  temp1 = pSchema->tblHash;
  temp2 = pSchema->trigHash;
  sqlite3HashInit(&pSchema->trigHash);
  sqlite3HashClear(&pSchema->idxHash);
  ಗಾಗಿ(pElem=sqlite_Hash_ಮೊದಲ(&temp2); pElem; pElem=sqlite_Hash_ಮುಂದಿನ(pElem)){
    sqlite3DeleteTrigger(&xdb, (Trigger*)sqlite_Hash_ದತ್ತಾಂಶ(pElem));
  }

  sqlite3HashClear(&temp2);
  sqlite3HashInit(&pSchema->tblHash);
  ಗಾಗಿ(pElem=sqlite_Hash_ಮೊದಲ(&temp1); pElem; pElem=sqlite_Hash_ಮುಂದಿನ(pElem)){
    ಕೋಷ್ಟಕ *pTab = sqlite_Hash_ದತ್ತಾಂಶ(pElem);
    sqlite3_Delete_ಕೋಷ್ಟಕ(&xdb, pTab);
  }
  sqlite3HashClear(&temp1);
  sqlite3HashClear(&pSchema->fkeyHash);
  pSchema->pSeqTab = 0;
  ಆದರೆ( pSchema->schemaFlags & DB_SchemaLoaded ){
    pSchema->iGeneration++;
  }
  pSchema->schemaFlags &= ~(DB_SchemaLoaded|DB_ResetWanted);
}

/*
** Find and return the schema associated with a BTree.  Create
** a new one if necessary.
*/
Schema *sqlite3SchemaGet(sqlite3 *db, Btree *pBt){
  Schema * p;
  ಆದರೆ( pBt ){
    p = (Schema *)sqlite3BtreeSchema(pBt, ಗಾತ್ರ(Schema), sqlite3SchemaClear);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = (Schema *)sqlite3DbMallocZero(0, ಗಾತ್ರ(Schema));
  }
  ಆದರೆ( !p ){
    sqlite3OomFault(db);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ( 0==p->file_format ){
    sqlite3HashInit(&p->tblHash);
    sqlite3HashInit(&p->idxHash);
    sqlite3HashInit(&p->trigHash);
    sqlite3HashInit(&p->fkeyHash);
    p->enc = SQLITE_UTF8;
  }
  ಹಿಂತಿರುಗಿ p;
}
