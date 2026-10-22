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
#अन्तर्भाव "sqliteInt.h"
#ifndef SQLITE_OMIT_COMPLETE

/*
** This is defined in tokenize.c.  We just have to import the definition.
*/
#ifndef SQLITE_AMALGAMATION
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(अचिन्हित अक्षर)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
बाह्य स्थिरांक अक्षर sqlite3IsEbcdicIdChar[];
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#endif
#endif /* SQLITE_AMALGAMATION */


/*
** Token types used by the sqlite3_complete() routine.  See the header
** comments on that procedure for additional information.
*/
#define tkSEMI    0
#define tkWS      1
#define tkOTHER   2
#ifndef SQLITE_OMIT_TRIGGER
#define tkEXPLAIN 3
#define tkCREATE  4
#define tkTEMP    5
#define tkTRIGGER 6
#define tkEND     7
#endif

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
sqlite3_int64 sqlite3_incomplete(स्थिरांक अक्षर *zSql){
  u8 state = 0;   /* Current state, using numbers defined in header comment */
  u8 token;       /* Value of the next token */
  u8 pending = 0; /* unmatched structure character */
  पूर्णांक nParen = 0; /* Nested parentheses */

#ifndef SQLITE_OMIT_TRIGGER
  /* A complex statement machine used to detect the end of a CREATE TRIGGER
  ** statement.  This is the normal case.
  */
  स्थिर स्थिरांक u8 trans[8][8] = {
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
#अन्यथा
  /* If triggers are not supported by this compile then the statement machine
  ** used to detect the end of a statement is much simpler
  */
  स्थिर स्थिरांक u8 trans[3][3] = {
                     /* Token:           */
     /* State:       **  SEMI  WS  OTHER */
     /* 0 NOTHING: */ {    1,  0,     2, },
     /* 1   START: */ {    1,  1,     2, },
     /* 2  NORMAL: */ {    1,  2,     2, },
  };
#endif /* SQLITE_OMIT_TRIGGER */
  /* Mapping state number to yy value for the return */
  स्थिर स्थिरांक u8 statemap[8] = {
     /* 0 NOTHING */ 0,
     /* 1 START   */ 0,
     /* 2 NORMAL  */ 1,
     /* 3 EXPLAIN */ 1,
     /* 4 CREATE  */ 1,
     /* 5 TRIGGER */ 3,
     /* 6 SEMI    */ 2,
     /* 7 END     */ 1,
  };

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( zSql==0 ){
    परत SQLITE_MISUSE_BKPT;
  }
#endif

  पर्यंत( *zSql ){
    निवड( *zSql ){
      स्थिती ';': {  /* A semicolon */
        token = tkSEMI;
        थांबा;
      }
      स्थिती ' ':
      स्थिती '\r':
      स्थिती '\t':
      स्थिती '\n':
      स्थिती '\f': {  /* White space is ignored */
        token = tkWS;
        थांबा;
      }
      स्थिती '/': {   /* C-style comments */
        जर( zSql[1]!='*' ){
          token = tkOTHER;
          थांबा;
        }
        zSql += 2;
        पर्यंत( zSql[0] && (zSql[0]!='*' || zSql[1]!='/') ){ zSql++; }
        जर( zSql[0]==0 ){
          pending = '/';
          जा incomplete_finish;
        }
        zSql++;
        token = tkWS;
        थांबा;
      }
      स्थिती '-': {   /* SQL-style comments from "--" to end of line */
        जर( zSql[1]!='-' ){
          token = tkOTHER;
          थांबा;
        }
        पर्यंत( *zSql && *zSql!='\n' ){ zSql++; }
        जर( *zSql==0 ){
          pending = '-';
          जा incomplete_finish;
        }
        token = tkWS;
        थांबा;
      }
      स्थिती '[': {   /* Microsoft-style identifiers in [...] */
        zSql++;
        पर्यंत( *zSql && *zSql!=']' ){ zSql++; }
        जर( *zSql==0 ){
          pending = ']';
          जा incomplete_finish;
        }
        token = tkOTHER;
        थांबा;
      }
      स्थिती '`':     /* Grave-accent quoted symbols used by MySQL */
      स्थिती '"':     /* single- and double-quoted strings */
      स्थिती '\'': {
        पूर्णांक c = *zSql;
        zSql++;
        पर्यंत( *zSql && *zSql!=c ){ zSql++; }
        जर( *zSql==0 ){
          pending = c;
          जा incomplete_finish;
        }
        token = tkOTHER;
        थांबा;
      }
      स्थिती '(': {
        nParen++;
        token = tkOTHER;
        थांबा;
      }
      स्थिती ')': {
        nParen--;
        token = tkOTHER;
        थांबा;
      }
      पूर्वनिर्धारित: {
#ifdef SQLITE_EBCDIC
        अचिन्हित अक्षर c;
#endif
        जर( IdChar((u8)*zSql) ){
          /* Keywords and unquoted identifiers */
          पूर्णांक nId;
          साठी(nId=1; IdChar(zSql[nId]); nId++){}
#ifdef SQLITE_OMIT_TRIGGER
          token = tkOTHER;
#अन्यथा
          निवड( *zSql ){
            स्थिती 'c': स्थिती 'C': {
              जर( nId==6 && sqlite3StrNICmp(zSql, "create", 6)==0 ){
                token = tkCREATE;
              }अन्यथा{
                token = tkOTHER;
              }
              थांबा;
            }
            स्थिती 't': स्थिती 'T': {
              जर( nId==7 && sqlite3StrNICmp(zSql, "trigger", 7)==0 ){
                token = tkTRIGGER;
              }अन्यथा जर( nId==4 && sqlite3StrNICmp(zSql, "temp", 4)==0 ){
                token = tkTEMP;
              }अन्यथा जर( nId==9 && sqlite3StrNICmp(zSql, "temporary", 9)==0 ){
                token = tkTEMP;
              }अन्यथा{
                token = tkOTHER;
              }
              थांबा;
            }
            स्थिती 'e':  स्थिती 'E': {
              जर( nId==3 && sqlite3StrNICmp(zSql, "end", 3)==0 ){
                token = tkEND;
              }अन्यथा
#ifndef SQLITE_OMIT_EXPLAIN
              जर( nId==7 && sqlite3StrNICmp(zSql, "explain", 7)==0 ){
                token = tkEXPLAIN;
              }अन्यथा
#endif
              {
                token = tkOTHER;
              }
              थांबा;
            }
            पूर्वनिर्धारित: {
              token = tkOTHER;
              थांबा;
            }
          }
#endif /* SQLITE_OMIT_TRIGGER */
          zSql += nId-1;
        }अन्यथा{
          /* Operators and special symbols */
          token = tkOTHER;
        }
        थांबा;
      }
    }
    state = trans[state][token];
    zSql++;
  }
incomplete_finish:
  जर( state==0 && pending==0 ) परत SQLITE_EMPTY;
  जर( state==1 ) nParen = 0;
  परत (i64)((((u64)nParen)<<32) |
               ((u64)pending<<16) |
               ((u64)statemap[state]<<8) |
               (state>1 || pending!=0));
}
पूर्णांक sqlite3_complete(स्थिरांक अक्षर *zSql){
  परत sqlite3_incomplete(zSql)==0;
}

#ifndef SQLITE_OMIT_UTF16
/*
** This routine is the same as the sqlite3_complete() routine described
** above, except that the parameter is required to be UTF-16 encoded, not
** UTF-8.
*/
पूर्णांक sqlite3_complete16(स्थिरांक शून्य *zSql){
  sqlite3_मूल्य *pVal;
  अक्षर स्थिरांक *zSql8;
  पूर्णांक rc;

#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  जर( rc ) परत rc;
#endif
  pVal = sqlite3_मूल्य_New(0);
  sqlite3_मूल्य_Set_Str(pVal, -1, zSql, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zSql8 = sqlite3_मूल्य_Text(pVal, SQLITE_UTF8);
  जर( zSql8 ){
    rc = sqlite3_incomplete(zSql8)==0;
  }अन्यथा{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_मूल्य_Free(pVal);
  परत rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_COMPLETE */
