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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ifndef SQLITE_OMIT_COMPLETE

/*
** This is defined in tokenize.c.  We just have to import the definition.
*/
#ifndef SQLITE_AMALGAMATION
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned ଅକ୍ଷର)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର sqlite3IsEbcdicIdChar[];
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
sqlite3_int64 sqlite3_incomplete(ଅଚଳ ଅକ୍ଷର *zSql){
  u8 state = 0;   /* Current state, using numbers defined in header comment */
  u8 token;       /* Value of the next token */
  u8 pending = 0; /* unmatched structure character */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nParen = 0; /* Nested parentheses */

#ifndef SQLITE_OMIT_TRIGGER
  /* A complex statement machine used to detect the end of a CREATE TRIGGER
  ** statement.  This is the normal case.
  */
  ସ୍ଥିର ଅଚଳ u8 trans[8][8] = {
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
#ଅନ୍ୟଥା
  /* If triggers are not supported by this compile then the statement machine
  ** used to detect the end of a statement is much simpler
  */
  ସ୍ଥିର ଅଚଳ u8 trans[3][3] = {
                     /* Token:           */
     /* State:       **  SEMI  WS  OTHER */
     /* 0 NOTHING: */ {    1,  0,     2, },
     /* 1   START: */ {    1,  1,     2, },
     /* 2  NORMAL: */ {    1,  2,     2, },
  };
#endif /* SQLITE_OMIT_TRIGGER */
  /* Mapping state number to yy value for the return */
  ସ୍ଥିର ଅଚଳ u8 statemap[8] = {
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
  ଯଦି( zSql==0 ){
    ଫେରନ୍ତୁ SQLITE_MISUSE_BKPT;
  }
#endif

  ଯେତେବେଳେ( *zSql ){
    ବାଛ( *zSql ){
      ସ୍ଥିତି ';': {  /* A semicolon */
        token = tkSEMI;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି ' ':
      ସ୍ଥିତି '\r':
      ସ୍ଥିତି '\t':
      ସ୍ଥିତି '\n':
      ସ୍ଥିତି '\f': {  /* White space is ignored */
        token = tkWS;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '/': {   /* C-style comments */
        ଯଦି( zSql[1]!='*' ){
          token = tkOTHER;
          ଭାଙ୍ଗ;
        }
        zSql += 2;
        ଯେତେବେଳେ( zSql[0] && (zSql[0]!='*' || zSql[1]!='/') ){ zSql++; }
        ଯଦି( zSql[0]==0 ){
          pending = '/';
          ଯାଅ incomplete_finish;
        }
        zSql++;
        token = tkWS;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '-': {   /* SQL-style comments from "--" to end of line */
        ଯଦି( zSql[1]!='-' ){
          token = tkOTHER;
          ଭାଙ୍ଗ;
        }
        ଯେତେବେଳେ( *zSql && *zSql!='\n' ){ zSql++; }
        ଯଦି( *zSql==0 ){
          pending = '-';
          ଯାଅ incomplete_finish;
        }
        token = tkWS;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '[': {   /* Microsoft-style identifiers in [...] */
        zSql++;
        ଯେତେବେଳେ( *zSql && *zSql!=']' ){ zSql++; }
        ଯଦି( *zSql==0 ){
          pending = ']';
          ଯାଅ incomplete_finish;
        }
        token = tkOTHER;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '`':     /* Grave-accent quoted symbols used by MySQL */
      ସ୍ଥିତି '"':     /* single- and double-quoted strings */
      ସ୍ଥିତି '\'': {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = *zSql;
        zSql++;
        ଯେତେବେଳେ( *zSql && *zSql!=c ){ zSql++; }
        ଯଦି( *zSql==0 ){
          pending = c;
          ଯାଅ incomplete_finish;
        }
        token = tkOTHER;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '(': {
        nParen++;
        token = tkOTHER;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି ')': {
        nParen--;
        token = tkOTHER;
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
#ifdef SQLITE_EBCDIC
        unsigned ଅକ୍ଷର c;
#endif
        ଯଦି( IdChar((u8)*zSql) ){
          /* Keywords and unquoted identifiers */
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nId;
          ପାଇଁ(nId=1; IdChar(zSql[nId]); nId++){}
#ifdef SQLITE_OMIT_TRIGGER
          token = tkOTHER;
#ଅନ୍ୟଥା
          ବାଛ( *zSql ){
            ସ୍ଥିତି 'c': ସ୍ଥିତି 'C': {
              ଯଦି( nId==6 && sqlite3StrNICmp(zSql, "create", 6)==0 ){
                token = tkCREATE;
              }ଅନ୍ୟଥା{
                token = tkOTHER;
              }
              ଭାଙ୍ଗ;
            }
            ସ୍ଥିତି 't': ସ୍ଥିତି 'T': {
              ଯଦି( nId==7 && sqlite3StrNICmp(zSql, "trigger", 7)==0 ){
                token = tkTRIGGER;
              }ଅନ୍ୟଥା ଯଦି( nId==4 && sqlite3StrNICmp(zSql, "temp", 4)==0 ){
                token = tkTEMP;
              }ଅନ୍ୟଥା ଯଦି( nId==9 && sqlite3StrNICmp(zSql, "temporary", 9)==0 ){
                token = tkTEMP;
              }ଅନ୍ୟଥା{
                token = tkOTHER;
              }
              ଭାଙ୍ଗ;
            }
            ସ୍ଥିତି 'e':  ସ୍ଥିତି 'E': {
              ଯଦି( nId==3 && sqlite3StrNICmp(zSql, "end", 3)==0 ){
                token = tkEND;
              }ଅନ୍ୟଥା
#ifndef SQLITE_OMIT_EXPLAIN
              ଯଦି( nId==7 && sqlite3StrNICmp(zSql, "explain", 7)==0 ){
                token = tkEXPLAIN;
              }ଅନ୍ୟଥା
#endif
              {
                token = tkOTHER;
              }
              ଭାଙ୍ଗ;
            }
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
              token = tkOTHER;
              ଭାଙ୍ଗ;
            }
          }
#endif /* SQLITE_OMIT_TRIGGER */
          zSql += nId-1;
        }ଅନ୍ୟଥା{
          /* Operators and special symbols */
          token = tkOTHER;
        }
        ଭାଙ୍ଗ;
      }
    }
    state = trans[state][token];
    zSql++;
  }
incomplete_finish:
  ଯଦି( state==0 && pending==0 ) ଫେରନ୍ତୁ SQLITE_EMPTY;
  ଯଦି( state==1 ) nParen = 0;
  ଫେରନ୍ତୁ (i64)((((u64)nParen)<<32) |
               ((u64)pending<<16) |
               ((u64)statemap[state]<<8) |
               (state>1 || pending!=0));
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_complete(ଅଚଳ ଅକ୍ଷର *zSql){
  ଫେରନ୍ତୁ sqlite3_incomplete(zSql)==0;
}

#ifndef SQLITE_OMIT_UTF16
/*
** This routine is the same as the sqlite3_complete() routine described
** above, except that the parameter is required to be UTF-16 encoded, not
** UTF-8.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_complete16(ଅଚଳ ଶୂନ୍ୟ *zSql){
  sqlite3_ମୂଲ୍ୟ *pVal;
  ଅକ୍ଷର ଅଚଳ *zSql8;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ rc;
#endif
  pVal = sqlite3_ମୂଲ୍ୟ_New(0);
  sqlite3_ମୂଲ୍ୟ_Set_Str(pVal, -1, zSql, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zSql8 = sqlite3_ମୂଲ୍ୟ_Text(pVal, SQLITE_UTF8);
  ଯଦି( zSql8 ){
    rc = sqlite3_incomplete(zSql8)==0;
  }ଅନ୍ୟଥା{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3_ମୂଲ୍ୟ_Free(pVal);
  ଫେରନ୍ତୁ rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_COMPLETE */
