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

#अन्तर्भाव "sqliteInt.h"

/*
** Invoke the 'collation needed' callback to request a collation sequence
** in the encoding enc of name zName, length nName.
*/
स्थिर शून्य callCollNeeded(sqlite3 *db, पूर्णाङ्क enc, ध्रुव वर्ण *zName){
  प्रतिज्ञा( !db->xCollNeeded || !db->xCollNeeded16 );
  जइ( db->xCollNeeded ){
    वर्ण *zExternal = sqlite3DbStrDup(db, zName);
    जइ( !zExternal ) प्रत्यागम;
    db->xCollNeeded(db->pCollNeededArg, db, enc, zExternal);
    sqlite3DbFree(db, zExternal);
  }
#ifndef SQLITE_OMIT_UTF16
  जइ( db->xCollNeeded16 ){
    वर्ण ध्रुव *zExternal;
    sqlite3_मानम् *pTmp = sqlite3_मानम्_New(db);
    sqlite3_मानम्_Set_Str(pTmp, -1, zName, SQLITE_UTF8, SQLITE_STATIC);
    zExternal = sqlite3_मानम्_Text(pTmp, SQLITE_UTF16NATIVE);
    जइ( zExternal ){
      db->xCollNeeded16(db->pCollNeededArg, db, (पूर्णाङ्क)ENC(db), zExternal);
    }
    sqlite3_मानम्_Free(pTmp);
  }
#endif
}

/*
** This routine is called if the collation factory fails to deliver a
** collation function in the best encoding but there may be other versions
** of this collation function (for other text encodings) available. Use one
** of these instead if they exist. Avoid a UTF-8 <-> UTF-16 conversion if
** possible.
*/
स्थिर पूर्णाङ्क synthCollSeq(sqlite3 *db, CollSeq *pColl){
  CollSeq *pColl2;
  वर्ण *z = pColl->zName;
  पूर्णाङ्क i;
  स्थिर ध्रुव u8 aEnc[] = { SQLITE_UTF16BE, SQLITE_UTF16LE, SQLITE_UTF8 };
  प्रति(i=0; i<3; i++){
    pColl2 = sqlite3FindCollSeq(db, aEnc[i], z, 0);
    जइ( pColl2->xCmp!=0 ){
      memcpy(pColl, pColl2, परिमाण(CollSeq));
      pColl->xDel = 0;         /* Do not copy the destructor */
      प्रत्यागम SQLITE_OK;
    }
  }
  प्रत्यागम SQLITE_ERROR;
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
पूर्णाङ्क sqlite3CheckCollSeq(Parse *pParse, CollSeq *pColl){
  जइ( pColl && pColl->xCmp==0 ){
    ध्रुव वर्ण *zName = pColl->zName;
    sqlite3 *db = pParse->db;
    CollSeq *p = sqlite3GetCollSeq(pParse, ENC(db), pColl, zName);
    जइ( !p ){
      प्रत्यागम SQLITE_ERROR;
    }
    प्रतिज्ञा( p==pColl );
  }
  प्रत्यागम SQLITE_OK;
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
स्थिर CollSeq *findCollSeqEntry(
  sqlite3 *db,          /* Database connection */
  ध्रुव वर्ण *zName,    /* Name of the collating sequence */
  पूर्णाङ्क create            /* Create a new entry if true */
){
  CollSeq *pColl;
  pColl = sqlite3HashFind(&db->aCollSeq, zName);

  जइ( 0==pColl && create ){
    पूर्णाङ्क nName = sqlite3Strlen30(zName) + 1;
    pColl = sqlite3DbMallocZero(db, 3*परिमाण(*pColl) + nName);
    जइ( pColl ){
      CollSeq *pDel = 0;
      pColl[0].zName = (वर्ण*)&pColl[3];
      pColl[0].enc = SQLITE_UTF8;
      pColl[1].zName = (वर्ण*)&pColl[3];
      pColl[1].enc = SQLITE_UTF16LE;
      pColl[2].zName = (वर्ण*)&pColl[3];
      pColl[2].enc = SQLITE_UTF16BE;
      memcpy(pColl[0].zName, zName, nName);
      pDel = sqlite3_Hash_निवेशय(&db->aCollSeq, pColl[0].zName, pColl);

      /* If a malloc() failure occurred in sqlite3HashInsert(), it will 
      ** return the pColl pointer to be deleted (because it wasn't added
      ** to the hash table).
      */
      प्रतिज्ञा( pDel==0 || pDel==pColl );
      जइ( pDel!=0 ){
        sqlite3OomFault(db);
        sqlite3DbFree(db, pDel);
        pColl = 0;
      }
    }
  }
  प्रत्यागम pColl;
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
CollSeq *sqlite3FindCollSeq(
  sqlite3 *db,          /* Database connection to search */
  u8 enc,               /* Desired text encoding */
  ध्रुव वर्ण *zName,    /* Name of the collating sequence.  Might be NULL */
  पूर्णाङ्क create            /* True to create CollSeq if doesn't already exist */
){
  CollSeq *pColl;
  प्रतिज्ञा( SQLITE_UTF8==1 && SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
  प्रतिज्ञा( enc>=SQLITE_UTF8 && enc<=SQLITE_UTF16BE );
  जइ( zName ){
    pColl = findCollSeqEntry(db, zName, create);
    जइ( pColl ) pColl += enc-1;
  }अण्णहा{
    pColl = db->pDfltColl;
  }
  प्रत्यागम pColl;
}

/*
** Change the text encoding for a database connection. This means that
** the pDfltColl must change as well.
*/
शून्य sqlite3SetTextEncoding(sqlite3 *db, u8 enc){
  प्रतिज्ञा( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  db->enc = enc;
  /* EVIDENCE-OF: R-08308-17224 The default collating function for all
  ** strings is BINARY. 
  */
  db->pDfltColl = sqlite3FindCollSeq(db, enc, sqlite3_Str_द्विमान, 0);
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
  ध्रुव वर्ण *zName     /* Collating sequence name */
){
  CollSeq *p;
  sqlite3 *db = pParse->db;

  p = pColl;
  जइ( !p ){
    p = sqlite3FindCollSeq(db, enc, zName, 0);
  }
  जइ( !p || !p->xCmp ){
    /* No collation sequence of this type for this encoding is registered.
    ** Call the collation factory to see if it can supply us with one.
    */
    callCollNeeded(db, enc, zName);
    p = sqlite3FindCollSeq(db, enc, zName, 0);
  }
  जइ( p && !p->xCmp && synthCollSeq(db, p) ){
    p = 0;
  }
  प्रतिज्ञा( !p || p->xCmp );
  जइ( p==0 ){
    sqlite3_Error_सन्देशः(pParse, "no such collation sequence: %s", zName);
    pParse->rc = SQLITE_ERROR_MISSING_COLLSEQ;
  }
  प्रत्यागम p;
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
CollSeq *sqlite3LocateCollSeq(Parse *pParse, ध्रुव वर्ण *zName){
  sqlite3 *db = pParse->db;
  u8 enc = ENC(db);
  u8 initbusy = db->init.busy;
  CollSeq *pColl;

  pColl = sqlite3FindCollSeq(db, enc, zName, initbusy);
  जइ( !initbusy && (!pColl || !pColl->xCmp) ){
    pColl = sqlite3GetCollSeq(pParse, enc, pColl, zName);
  }

  प्रत्यागम pColl;
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
#define FUNC_PERFECT_MATCH 6  /* The score for a perfect match */
स्थिर पूर्णाङ्क matchQuality(
  FuncDef *p,     /* The function we are evaluating for match quality */
  पूर्णाङ्क nArg,       /* Desired number of arguments.  (-1)==any */
  u8 enc          /* Desired text encoding */
){
  पूर्णाङ्क match;
  प्रतिज्ञा( p->nArg>=(-4) && p->nArg!=(-2) );
  प्रतिज्ञा( nArg>=(-2) );

  /* Wrong number of arguments means "no match" */
  जइ( p->nArg!=nArg ){
    जइ( nArg==(-2) ) प्रत्यागम p->xSFunc==0 ? 0 : FUNC_PERFECT_MATCH;
    जइ( p->nArg>=0 ) प्रत्यागम 0;
    /* Special p->nArg values available to built-in functions only:
    **    -3     1 or more arguments required
    **    -4     2 or more arguments required
    */
    जइ( p->nArg<(-2) && nArg<(-2-p->nArg) ) प्रत्यागम 0;
  }

  /* Give a better score to a function with a specific number of arguments
  ** than to function that accepts any number of arguments. */
  जइ( p->nArg==nArg ){
    match = 4;
  }अण्णहा{
    match = 1;
  }

  /* Bonus points if the text encoding matches */
  जइ( enc==(p->funcFlags & SQLITE_FUNC_ENCMASK) ){
    match += 2;  /* Exact encoding match */
  }अण्णहा जइ( (enc & p->funcFlags & 2)!=0 ){
    match += 1;  /* Both are UTF16, but with different byte orders */
  }

  प्रत्यागम match;
}

/*
** Search a FuncDefHash for a function with the given name.  Return
** a pointer to the matching FuncDef if found, or 0 if there is no match.
*/
FuncDef *sqlite3_Function_अन्वेषणम्(
  पूर्णाङ्क h,               /* Hash of the name */
  ध्रुव वर्ण *zFunc    /* Name of function */
){
  FuncDef *p;
  प्रति(p=sqlite3BuiltinFunctions.a[h]; p; p=p->u.pHash){
    प्रतिज्ञा( p->funcFlags & SQLITE_FUNC_BUILTIN );
    जइ( sqlite3StrICmp(p->zName, zFunc)==0 ){
      प्रत्यागम p;
    }
  }
  प्रत्यागम 0;
}

/*
** Insert a new FuncDef into a FuncDefHash hash table.
*/
शून्य sqlite3_निवेशय_Builtin_Funcs(
  FuncDef *aDef,      /* List of global functions to be inserted */
  पूर्णाङ्क nDef            /* Length of the apDef[] list */
){
  पूर्णाङ्क i;
  प्रति(i=0; i<nDef; i++){
    FuncDef *pOther;
    ध्रुव वर्ण *zName = aDef[i].zName;
    पूर्णाङ्क nName = sqlite3Strlen30(zName);
    पूर्णाङ्क h = SQLITE_FUNC_HASH(zName[0], nName);
    प्रतिज्ञा( aDef[i].funcFlags & SQLITE_FUNC_BUILTIN );
    pOther = sqlite3_Function_अन्वेषणम्(h, zName);
    जइ( pOther ){
      प्रतिज्ञा( pOther!=&aDef[i] && pOther->pNext!=&aDef[i] );
      aDef[i].pNext = pOther->pNext;
      pOther->pNext = &aDef[i];
    }अण्णहा{
      aDef[i].pNext = 0;
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
FuncDef *sqlite3FindFunction(
  sqlite3 *db,       /* An open database */
  ध्रुव वर्ण *zName, /* Name of the function.  zero-terminated */
  पूर्णाङ्क nArg,          /* Number of arguments.  -1 means any number */
  u8 enc,            /* Preferred text encoding */
  u8 createFlag      /* Create new entry if true and does not otherwise exist */
){
  FuncDef *p;         /* Iterator variable */
  FuncDef *pBest = 0; /* Best match found so far */
  पूर्णाङ्क bestScore = 0;  /* Score of best match */
  पूर्णाङ्क h;              /* Hash value */
  पूर्णाङ्क nName;          /* Length of the name */

  प्रतिज्ञा( nArg>=(-2) );
  प्रतिज्ञा( nArg>=(-1) || createFlag==0 );
  nName = sqlite3Strlen30(zName);

  /* First search for a match amongst the application-defined functions.
  */
  p = (FuncDef*)sqlite3HashFind(&db->aFunc, zName);
  जाव( p ){
    पूर्णाङ्क score = matchQuality(p, nArg, enc);
    जइ( score>bestScore ){
      pBest = p;
      bestScore = score;
    }
    p = p->pNext;
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
  जइ( !createFlag && (pBest==0 || (db->mDbFlags & DBFLAG_PreferBuiltin)!=0) ){
    bestScore = 0;
    h = SQLITE_FUNC_HASH(sqlite3UpperToLower[(u8)zName[0]], nName);
    p = sqlite3_Function_अन्वेषणम्(h, zName);
    जाव( p ){
      पूर्णाङ्क score = matchQuality(p, nArg, enc);
      जइ( score>bestScore ){
        pBest = p;
        bestScore = score;
      }
      p = p->pNext;
    }
  }

  /* If the createFlag parameter is true and the search did not reveal an
  ** exact match for the name, number of arguments and encoding, then add a
  ** new entry to the hash table and return it.
  */
  जइ( createFlag && bestScore<FUNC_PERFECT_MATCH && 
      (pBest = sqlite3DbMallocZero(db, परिमाण(*pBest)+nName+1))!=0 ){
    FuncDef *pOther;
    u8 *z;
    pBest->zName = (ध्रुव वर्ण*)&pBest[1];
    pBest->nArg = (u16)nArg;
    pBest->funcFlags = enc;
    memcpy((वर्ण*)&pBest[1], zName, nName+1);
    प्रति(z=(u8*)pBest->zName; *z; z++) *z = sqlite3UpperToLower[*z];
    pOther = (FuncDef*)sqlite3_Hash_निवेशय(&db->aFunc, pBest->zName, pBest);
    जइ( pOther==pBest ){
      sqlite3DbFree(db, pBest);
      sqlite3OomFault(db);
      प्रत्यागम 0;
    }अण्णहा{
      pBest->pNext = pOther;
    }
  }

  जइ( pBest && (pBest->xSFunc || createFlag) ){
    प्रत्यागम pBest;
  }
  प्रत्यागम 0;
}

/*
** Free all resources held by the schema structure. The void* argument points
** at a Schema struct. This function does not call sqlite3DbFree(db, ) on the 
** pointer itself, it just cleans up subsidiary resources (i.e. the contents
** of the schema hash tables).
**
** The Schema.cache_size variable is not cleared.
*/
शून्य sqlite3SchemaClear(शून्य *p){
  Hash temp1;
  Hash temp2;
  HashElem *pElem;
  Schema *pSchema = (Schema *)p;
  sqlite3 xdb;

  memset(&xdb, 0, परिमाण(xdb));
  temp1 = pSchema->tblHash;
  temp2 = pSchema->trigHash;
  sqlite3HashInit(&pSchema->trigHash);
  sqlite3HashClear(&pSchema->idxHash);
  प्रति(pElem=sqliteHashFirst(&temp2); pElem; pElem=sqliteHashNext(pElem)){
    sqlite3DeleteTrigger(&xdb, (Trigger*)sqliteHashData(pElem));
  }

  sqlite3HashClear(&temp2);
  sqlite3HashInit(&pSchema->tblHash);
  प्रति(pElem=sqliteHashFirst(&temp1); pElem; pElem=sqliteHashNext(pElem)){
    Table *pTab = sqliteHashData(pElem);
    sqlite3DeleteTable(&xdb, pTab);
  }
  sqlite3HashClear(&temp1);
  sqlite3HashClear(&pSchema->fkeyHash);
  pSchema->pSeqTab = 0;
  जइ( pSchema->schemaFlags & DB_SchemaLoaded ){
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
  जइ( pBt ){
    p = (Schema *)sqlite3BtreeSchema(pBt, परिमाण(Schema), sqlite3SchemaClear);
  }अण्णहा{
    p = (Schema *)sqlite3DbMallocZero(0, परिमाण(Schema));
  }
  जइ( !p ){
    sqlite3OomFault(db);
  }अण्णहा जइ ( 0==p->file_format ){
    sqlite3HashInit(&p->tblHash);
    sqlite3HashInit(&p->idxHash);
    sqlite3HashInit(&p->trigHash);
    sqlite3HashInit(&p->fkeyHash);
    p->enc = SQLITE_UTF8;
  }
  प्रत्यागम p;
}
