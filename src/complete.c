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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPLETE

/*
** This is defined in tokenize.c.  We just have to import the definition.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_AMALGAMATION
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
#ವ್ಯಾಖ್ಯಾನಿಸು IdChar(C)  ((sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)C]&0x46)!=0)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ sqlite3IsEbcdicIdChar[];
#ವ್ಯಾಖ್ಯಾನಿಸು IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_AMALGAMATION */


/*
** Token types used by the sqlite3_complete() routine.  See the header
** comments on that procedure for additional information.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು tkSEMI    0
#ವ್ಯಾಖ್ಯಾನಿಸು tkWS      1
#ವ್ಯಾಖ್ಯಾನಿಸು tkOTHER   2
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
#ವ್ಯಾಖ್ಯಾನಿಸು tkEXPLAIN 3
#ವ್ಯಾಖ್ಯಾನಿಸು tkCREATE  4
#ವ್ಯಾಖ್ಯಾನಿಸು tk_ತಾತ್ಕಾಲಿಕ    5
#ವ್ಯಾಖ್ಯಾನಿಸು tkTRIGGER 6
#ವ್ಯಾಖ್ಯಾನಿಸು tk_ಕೊನೆ     7
#ಆದರೆ_ಅಂತ್ಯ

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
sqlite3_int64 sqlite3_incomplete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql){
  u8 state = 0;   /* Current state, using numbers defined in header comment */
  u8 token;       /* Value of the next token */
  u8 pending = 0; /* unmatched structure character */
  ಪೂರ್ಣಾಂಕ nParen = 0; /* Nested parentheses */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  /* A complex statement machine used to detect the end of a CREATE TRIGGER
  ** statement.  This is the normal case.
  */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 trans[8][8] = {
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
#ಇಲ್ಲದಿದ್ದರೆ
  /* If triggers are not supported by this compile then the statement machine
  ** used to detect the end of a statement is much simpler
  */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 trans[3][3] = {
                     /* Token:           */
     /* State:       **  SEMI  WS  OTHER */
     /* 0 NOTHING: */ {    1,  0,     2, },
     /* 1   START: */ {    1,  1,     2, },
     /* 2  NORMAL: */ {    1,  2,     2, },
  };
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRIGGER */
  /* Mapping state number to yy value for the return */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 statemap[8] = {
     /* 0 NOTHING */ 0,
     /* 1 START   */ 0,
     /* 2 NORMAL  */ 1,
     /* 3 EXPLAIN */ 1,
     /* 4 CREATE  */ 1,
     /* 5 TRIGGER */ 3,
     /* 6 SEMI    */ 2,
     /* 7 END     */ 1,
  };

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( zSql==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆಗ( *zSql ){
    ಆಯ್ಕೆ( *zSql ){
      ಸ್ಥಿತಿ ';': {  /* A semicolon */
        token = tkSEMI;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ ' ':
      ಸ್ಥಿತಿ '\r':
      ಸ್ಥಿತಿ '\t':
      ಸ್ಥಿತಿ '\n':
      ಸ್ಥಿತಿ '\f': {  /* White space is ignored */
        token = tkWS;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '/': {   /* C-style comments */
        ಆದರೆ( zSql[1]!='*' ){
          token = tkOTHER;
          ನಿಲ್ಲಿಸು;
        }
        zSql += 2;
        ಆಗ( zSql[0] && (zSql[0]!='*' || zSql[1]!='/') ){ zSql++; }
        ಆದರೆ( zSql[0]==0 ){
          pending = '/';
          ಹೋಗು incomplete_ಮುಕ್ತಾಯ;
        }
        zSql++;
        token = tkWS;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '-': {   /* SQL-style comments from "--" to end of line */
        ಆದರೆ( zSql[1]!='-' ){
          token = tkOTHER;
          ನಿಲ್ಲಿಸು;
        }
        ಆಗ( *zSql && *zSql!='\n' ){ zSql++; }
        ಆದರೆ( *zSql==0 ){
          pending = '-';
          ಹೋಗು incomplete_ಮುಕ್ತಾಯ;
        }
        token = tkWS;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '[': {   /* Microsoft-style identifiers in [...] */
        zSql++;
        ಆಗ( *zSql && *zSql!=']' ){ zSql++; }
        ಆದರೆ( *zSql==0 ){
          pending = ']';
          ಹೋಗು incomplete_ಮುಕ್ತಾಯ;
        }
        token = tkOTHER;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '`':     /* Grave-accent quoted symbols used by MySQL */
      ಸ್ಥಿತಿ '"':     /* single- and double-quoted strings */
      ಸ್ಥಿತಿ '\'': {
        ಪೂರ್ಣಾಂಕ c = *zSql;
        zSql++;
        ಆಗ( *zSql && *zSql!=c ){ zSql++; }
        ಆದರೆ( *zSql==0 ){
          pending = c;
          ಹೋಗು incomplete_ಮುಕ್ತಾಯ;
        }
        token = tkOTHER;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '(': {
        nParen++;
        token = tkOTHER;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ ')': {
        nParen--;
        token = tkOTHER;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c;
#ಆದರೆ_ಅಂತ್ಯ
        ಆದರೆ( IdChar((u8)*zSql) ){
          /* Keywords and unquoted identifiers */
          ಪೂರ್ಣಾಂಕ nId;
          ಗಾಗಿ(nId=1; IdChar(zSql[nId]); nId++){}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_TRIGGER
          token = tkOTHER;
#ಇಲ್ಲದಿದ್ದರೆ
          ಆಯ್ಕೆ( *zSql ){
            ಸ್ಥಿತಿ 'c': ಸ್ಥಿತಿ 'C': {
              ಆದರೆ( nId==6 && sqlite3StrNICmp(zSql, "create", 6)==0 ){
                token = tkCREATE;
              }ಇಲ್ಲದಿದ್ದರೆ{
                token = tkOTHER;
              }
              ನಿಲ್ಲಿಸು;
            }
            ಸ್ಥಿತಿ 't': ಸ್ಥಿತಿ 'T': {
              ಆದರೆ( nId==7 && sqlite3StrNICmp(zSql, "trigger", 7)==0 ){
                token = tkTRIGGER;
              }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nId==4 && sqlite3StrNICmp(zSql, "temp", 4)==0 ){
                token = tk_ತಾತ್ಕಾಲಿಕ;
              }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nId==9 && sqlite3StrNICmp(zSql, "temporary", 9)==0 ){
                token = tk_ತಾತ್ಕಾಲಿಕ;
              }ಇಲ್ಲದಿದ್ದರೆ{
                token = tkOTHER;
              }
              ನಿಲ್ಲಿಸು;
            }
            ಸ್ಥಿತಿ 'e':  ಸ್ಥಿತಿ 'E': {
              ಆದರೆ( nId==3 && sqlite3StrNICmp(zSql, "end", 3)==0 ){
                token = tk_ಕೊನೆ;
              }ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
              ಆದರೆ( nId==7 && sqlite3StrNICmp(zSql, "explain", 7)==0 ){
                token = tkEXPLAIN;
              }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
              {
                token = tkOTHER;
              }
              ನಿಲ್ಲಿಸು;
            }
            ಪೂರ್ವನಿಯೋಜಿತ: {
              token = tkOTHER;
              ನಿಲ್ಲಿಸು;
            }
          }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRIGGER */
          zSql += nId-1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* Operators and special symbols */
          token = tkOTHER;
        }
        ನಿಲ್ಲಿಸು;
      }
    }
    state = trans[state][token];
    zSql++;
  }
incomplete_ಮುಕ್ತಾಯ:
  ಆದರೆ( state==0 && pending==0 ) ಹಿಂತಿರುಗಿ SQLITE_ಖಾಲಿಗಳು;
  ಆದರೆ( state==1 ) nParen = 0;
  ಹಿಂತಿರುಗಿ (i64)((((u64)nParen)<<32) |
               ((u64)pending<<16) |
               ((u64)statemap[state]<<8) |
               (state>1 || pending!=0));
}
ಪೂರ್ಣಾಂಕ sqlite3_complete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql){
  ಹಿಂತಿರುಗಿ sqlite3_incomplete(zSql)==0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
/*
** This routine is the same as the sqlite3_complete() routine described
** above, except that the parameter is required to be UTF-16 encoded, not
** UTF-8.
*/
ಪೂರ್ಣಾಂಕ sqlite3_complete16(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zSql){
  sqlite3_ಮೌಲ್ಯ *pVal;
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zSql8;
  ಪೂರ್ಣಾಂಕ rc;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
  pVal = sqlite3_ಮೌಲ್ಯ_New(0);
  sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(pVal, -1, zSql, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zSql8 = sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(pVal, SQLITE_UTF8);
  ಆದರೆ( zSql8 ){
    rc = sqlite3_incomplete(zSql8)==0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_ಮೌಲ್ಯ_Free(pVal);
  ಹಿಂತಿರುಗಿ rc & 0xff;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UTF16 */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPLETE */
