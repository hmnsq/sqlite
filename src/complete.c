/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** An tokenizer for SQL
**
** This file contains C code that implements the sqlite3_complete() API.
** This code used to be part of the tokenizer.c source file.  But by
** separating it out, the code will be automatically omitted from
** static links that do not use it.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPLETE

/*
** This is defined in tokenize.c.  We just have to import the definition.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_AMALGAMATION
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IdChar(C)  ((sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)C]&0x46)!=0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ sqlite3IsEbcdicIdChar[];
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* SQLITE_AMALGAMATION */


/*
** Token types used by the sqlite3_complete() routine.  See the header
** comments on that procedure for additional information.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tkSEMI    0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tkWS      1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tkOTHER   2
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tkEXPLAIN 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tkCREATE  4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tk_ਆਰਜ਼ੀ    5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tkTRIGGER 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tk_ਸਿਰਾ     7
#ਜੇ_ਅੰਤ

/*
** Return zero if the given SQL string is complete - if all comments,
** string and blob literals, and quoted identifiers have been closed and
** if the entire string ends with ";" and possibly with ";END;" if the
** string is a CREATE TRIGGER statement.  A non-zero return indicates
** that the string is empty or incomplete.  Bits of the return value
** indicate what is missing and is needed to close out the statement.
**
** Special handling is require for CREATE TRIGGER statements.
** Whenever the CREATE TRIGGER keywords are seen, the statement
** must end with ";END;".
**
** Let the return code be a value R.  R is split up into various
** subfields, at byte boundaries:
**
**    R = 0xwwwwwwww00xxyyzz
**
** In other words, zz is the least significant byte, yy is the next
** most significant byte, xx is the third byte, wwwwwwww is the upper
** four bytes of the return value.
**
**   zz == SQLITE_OK       Input is complete
**   zz == SQLITE_ERROR    Input is incomplete
**   zz == SQLITE_EMPTY    Input contains only whitespace
**   zz == SQLITE_MISUSE   Input is a NULL pointer
**   zz != 0               New values for zz may be added in the future
**
**   yy == 0x01            Need a semicolon at the end
**   yy == 0x02            Need "END" and a semicolon
**   yy == 0x03            Need semicolon, "END", and semicolon
**   yy != 0               New values for yy may be added in the future
**
**   xx == '\''            Incomplete string or blob literal
**   xx == '"'             Incomplete quoted identifier
**   xx == '`'             Incompelte MySQL-style quoted identifier
**   xx == ']'             Incomplete SQLServer-style quoted identifer
**   xx == '-'             Incomplete SQL-style comment
**   xx == '/'             Incomplete C-style comment
**   xx != 0               New values of xx may be added in the future
**
**   wwwwwwww              Interpret as a signed integer, the number
**                         of unmatched "(".  Negative means there are
**                         more ")" and "(".
**
**   ((R>>24)&0xff)!=0     New uses for the 4th byte may be added
**                         in the future
**
** This implementation uses a state machine with 8 states:
**
**   (0) NOTHING   We have not yet seen a non-whitespace character.
**
**   (1) START     At the beginning or end of an SQL statement.  This routine
**                 returns 1 if it ends in the START state and 0 if it ends
**                 in any other state.
**
**   (2) NORMAL    We are in the middle of statement which ends with a single
**                 semicolon.
**
**   (3) EXPLAIN   The keyword EXPLAIN has been seen at the beginning of 
**                 a statement.
**
**   (4) CREATE    The keyword CREATE has been seen at the beginning of a
**                 statement, possibly preceded by EXPLAIN and/or followed by
**                 TEMP or TEMPORARY
**
**   (5) TRIGGER   We are in the middle of a trigger definition that must be
**                 ended by a semicolon, the keyword END, and another semicolon.
**
**   (6) SEMI      We've seen the first semicolon in the ";END;" that occurs at
**                 the end of a trigger definition.
**
**   (7) END       We've seen the ";END" of the ";END;" that occurs at the end
**                 of a trigger definition.
**
** Transitions between states above are determined by tokens extracted
** from the input.  The following tokens are significant:
**
**   (0) tkSEMI      A semicolon.
**   (1) tkWS        Whitespace.
**   (2) tkOTHER     Any other SQL token.
**   (3) tkEXPLAIN   The "explain" keyword.
**   (4) tkCREATE    The "create" keyword.
**   (5) tkTEMP      The "temp" or "temporary" keyword.
**   (6) tkTRIGGER   The "trigger" keyword.
**   (7) tkEND       The "end" keyword.
**
** Whitespace never causes a state transition and is always ignored.
** This means that a SQL string of all whitespace is invalid.
**
** If we compile with SQLITE_OMIT_TRIGGER, all of the computation needed
** to recognize the end of a trigger can be omitted.  All we have to do
** is look for a semicolon that is not part of an string or comment.
*/
sqlite3_int64 sqlite3_incomplete(ਅਚਲ ਅੱਖਰ *zSql){
  u8 state = 0;   /* Current state, using numbers defined in header comment */
  u8 token;       /* Value of the next token */
  u8 pending = 0; /* unmatched structure character */
  ਪੂਰਨ nParen = 0; /* Nested parentheses */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  /* A complex statement machine used to detect the end of a CREATE TRIGGER
  ** statement.  This is the normal case.
  */
  ਸਥਿਰ ਅਚਲ u8 trans[8][8] = {
                     /* Token:                                                */
     /* State:       **  SEMI  WS  OTHER  EXPLAIN  CREATE  TEMP  TRIGGER  END */
     /* 0 NOTHING: */ {    1,  0,     2,       3,      4,    2,       2,   2, },
     /* 1   START: */ {    1,  1,     2,       3,      4,    2,       2,   2, },
     /* 2  NORMAL: */ {    1,  2,     2,       2,      2,    2,       2,   2, },
     /* 3 EXPLAIN: */ {    1,  3,     3,       2,      4,    2,       2,   2, },
     /* 4  CREATE: */ {    1,  4,     2,       2,      2,    4,       5,   2, },
     /* 5 TRIGGER: */ {    6,  5,     5,       5,      5,    5,       5,   5, },
     /* 6    SEMI: */ {    6,  6,     5,       5,      5,    5,       5,   7, },
     /* 7     END: */ {    1,  7,     5,       5,      5,    5,       5,   5, },
  };
#ਨਹੀਂਤਾਂ
  /* If triggers are not supported by this compile then the statement machine
  ** used to detect the end of a statement is much simpler
  */
  ਸਥਿਰ ਅਚਲ u8 trans[3][3] = {
                     /* Token:           */
     /* State:       **  SEMI  WS  OTHER */
     /* 0 NOTHING: */ {    1,  0,     2, },
     /* 1   START: */ {    1,  1,     2, },
     /* 2  NORMAL: */ {    1,  2,     2, },
  };
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRIGGER */
  /* Mapping state number to yy value for the return */
  ਸਥਿਰ ਅਚਲ u8 statemap[8] = {
     /* 0 NOTHING */ 0,
     /* 1 START   */ 0,
     /* 2 NORMAL  */ 1,
     /* 3 EXPLAIN */ 1,
     /* 4 CREATE  */ 1,
     /* 5 TRIGGER */ 3,
     /* 6 SEMI    */ 2,
     /* 7 END     */ 1,
  };

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( zSql==0 ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
#ਜੇ_ਅੰਤ

  ਜਦੋਂ( *zSql ){
    ਚੋਣ( *zSql ){
      ਹਾਲਤ ';': {  /* A semicolon */
        token = tkSEMI;
        ਰੋਕੋ;
      }
      ਹਾਲਤ ' ':
      ਹਾਲਤ '\r':
      ਹਾਲਤ '\t':
      ਹਾਲਤ '\n':
      ਹਾਲਤ '\f': {  /* White space is ignored */
        token = tkWS;
        ਰੋਕੋ;
      }
      ਹਾਲਤ '/': {   /* C-style comments */
        ਜੇ( zSql[1]!='*' ){
          token = tkOTHER;
          ਰੋਕੋ;
        }
        zSql += 2;
        ਜਦੋਂ( zSql[0] && (zSql[0]!='*' || zSql[1]!='/') ){ zSql++; }
        ਜੇ( zSql[0]==0 ){
          pending = '/';
          ਜਾਓ incomplete_ਅੰਤ;
        }
        zSql++;
        token = tkWS;
        ਰੋਕੋ;
      }
      ਹਾਲਤ '-': {   /* SQL-style comments from "--" to end of line */
        ਜੇ( zSql[1]!='-' ){
          token = tkOTHER;
          ਰੋਕੋ;
        }
        ਜਦੋਂ( *zSql && *zSql!='\n' ){ zSql++; }
        ਜੇ( *zSql==0 ){
          pending = '-';
          ਜਾਓ incomplete_ਅੰਤ;
        }
        token = tkWS;
        ਰੋਕੋ;
      }
      ਹਾਲਤ '[': {   /* Microsoft-style identifiers in [...] */
        zSql++;
        ਜਦੋਂ( *zSql && *zSql!=']' ){ zSql++; }
        ਜੇ( *zSql==0 ){
          pending = ']';
          ਜਾਓ incomplete_ਅੰਤ;
        }
        token = tkOTHER;
        ਰੋਕੋ;
      }
      ਹਾਲਤ '`':     /* Grave-accent quoted symbols used by MySQL */
      ਹਾਲਤ '"':     /* single- and double-quoted strings */
      ਹਾਲਤ '\'': {
        ਪੂਰਨ c = *zSql;
        zSql++;
        ਜਦੋਂ( *zSql && *zSql!=c ){ zSql++; }
        ਜੇ( *zSql==0 ){
          pending = c;
          ਜਾਓ incomplete_ਅੰਤ;
        }
        token = tkOTHER;
        ਰੋਕੋ;
      }
      ਹਾਲਤ '(': {
        nParen++;
        token = tkOTHER;
        ਰੋਕੋ;
      }
      ਹਾਲਤ ')': {
        nParen--;
        token = tkOTHER;
        ਰੋਕੋ;
      }
      ਮੂਲ: {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c;
#ਜੇ_ਅੰਤ
        ਜੇ( IdChar((u8)*zSql) ){
          /* Keywords and unquoted identifiers */
          ਪੂਰਨ nId;
          ਲਈ(nId=1; IdChar(zSql[nId]); nId++){}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
          token = tkOTHER;
#ਨਹੀਂਤਾਂ
          ਚੋਣ( *zSql ){
            ਹਾਲਤ 'c': ਹਾਲਤ 'C': {
              ਜੇ( nId==6 && sqlite3StrNICmp(zSql, "create", 6)==0 ){
                token = tkCREATE;
              }ਨਹੀਂਤਾਂ{
                token = tkOTHER;
              }
              ਰੋਕੋ;
            }
            ਹਾਲਤ 't': ਹਾਲਤ 'T': {
              ਜੇ( nId==7 && sqlite3StrNICmp(zSql, "trigger", 7)==0 ){
                token = tkTRIGGER;
              }ਨਹੀਂਤਾਂ ਜੇ( nId==4 && sqlite3StrNICmp(zSql, "temp", 4)==0 ){
                token = tk_ਆਰਜ਼ੀ;
              }ਨਹੀਂਤਾਂ ਜੇ( nId==9 && sqlite3StrNICmp(zSql, "temporary", 9)==0 ){
                token = tk_ਆਰਜ਼ੀ;
              }ਨਹੀਂਤਾਂ{
                token = tkOTHER;
              }
              ਰੋਕੋ;
            }
            ਹਾਲਤ 'e':  ਹਾਲਤ 'E': {
              ਜੇ( nId==3 && sqlite3StrNICmp(zSql, "end", 3)==0 ){
                token = tk_ਸਿਰਾ;
              }ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
              ਜੇ( nId==7 && sqlite3StrNICmp(zSql, "explain", 7)==0 ){
                token = tkEXPLAIN;
              }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
              {
                token = tkOTHER;
              }
              ਰੋਕੋ;
            }
            ਮੂਲ: {
              token = tkOTHER;
              ਰੋਕੋ;
            }
          }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRIGGER */
          zSql += nId-1;
        }ਨਹੀਂਤਾਂ{
          /* Operators and special symbols */
          token = tkOTHER;
        }
        ਰੋਕੋ;
      }
    }
    state = trans[state][token];
    zSql++;
  }
incomplete_ਅੰਤ:
  ਜੇ( state==0 && pending==0 ) ਵਾਪਸ SQLITE_ਖਾਲੀ_ਥਾਵਾਂ;
  ਜੇ( state==1 ) nParen = 0;
  ਵਾਪਸ (i64)((((u64)nParen)<<32) |
               ((u64)pending<<16) |
               ((u64)statemap[state]<<8) |
               (state>1 || pending!=0));
}
ਪੂਰਨ sqlite3_complete(ਅਚਲ ਅੱਖਰ *zSql){
  ਵਾਪਸ sqlite3_incomplete(zSql)==0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
/*
** This routine is the same as the sqlite3_complete() routine described
** above, except that the parameter is required to be UTF-16 encoded, not
** UTF-8.
*/
ਪੂਰਨ sqlite3_complete16(ਅਚਲ ਖਾਲੀ *zSql){
  sqlite3_ਮੁੱਲ *pVal;
  ਅੱਖਰ ਅਚਲ *zSql8;
  ਪੂਰਨ rc;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
  pVal = sqlite3_ਮੁੱਲ_New(0);
  sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(pVal, -1, zSql, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zSql8 = sqlite3_ਮੁੱਲ_ਪਾਠ_2(pVal, SQLITE_UTF8);
  ਜੇ( zSql8 ){
    rc = sqlite3_incomplete(zSql8)==0;
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_ਮੁੱਲ_Free(pVal);
  ਵਾਪਸ rc & 0xff;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UTF16 */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPLETE */
