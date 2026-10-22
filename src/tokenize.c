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
** This file contains C code that splits an SQL input string up into
** individual tokens and sends those tokens one-by-one over to the
** parser for analysis.
*/
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

/* Character classes for tokenizing
**
** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
** using a lookup table, whereas a switch() directly on c uses a binary search.
** The lookup table is much faster.  To maximize speed, and to ensure that
** a lookup table is used, all of the classes need to be small integers and
** all of them need to be used within the switch.
*/
#define CC_X          0    /* The letter 'x', or start of BLOB literal */
#define CC_KYWD0      1    /* First letter of a keyword */
#define CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#define CC_DIGIT      3    /* Digits */
#define CC_DOLLAR     4    /* '$' */
#define CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#define CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#define CC_SPACE      7    /* Space characters */
#define CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#define CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#define CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#define CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#define CC_LT        12    /* '<'.  Part of < or <= or <> */
#define CC_GT        13    /* '>'.  Part of > or >= */
#define CC_EQ        14    /* '='.  Part of = or == */
#define CC_BANG      15    /* '!'.  Part of != */
#define CC_SLASH     16    /* '/'.  / or c-style comment */
#define CC_LP        17    /* '(' */
#define CC_RP        18    /* ')' */
#define CC_SEMI      19    /* ';' */
#define CC_PLUS      20    /* '+' */
#define CC_STAR      21    /* '*' */
#define CC_PERCENT   22    /* '%' */
#define CC_COMMA     23    /* ',' */
#define CC_AND       24    /* '&' */
#define CC_TILDA     25    /* '~' */
#define CC_DOT       26    /* '.' */
#define CC_ID        27    /* unicode characters usable in IDs */
#define CC_ILLEGAL   28    /* Illegal character */
#define CC_NUL       29    /* 0x00 */
#define CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର aiClass[] = {
#ifdef SQLITE_ASCII
/*         x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf */
/* 0x */   29, 28, 28, 28, 28, 28, 28, 28, 28,  7,  7, 28,  7,  7, 28, 28,
/* 1x */   28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
/* 2x */    7, 15,  8,  5,  4, 22, 24,  8, 17, 18, 21, 20, 23, 11, 26, 16,
/* 3x */    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  5, 19, 12, 14, 13,  6,
/* 4x */    5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 5x */    1,  1,  1,  1,  1,  1,  1,  1,  0,  2,  2,  9, 28, 28, 28,  2,
/* 6x */    8,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 7x */    1,  1,  1,  1,  1,  1,  1,  1,  0,  2,  2, 28, 10, 28, 25, 28,
/* 8x */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* 9x */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* Ax */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* Bx */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* Cx */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* Dx */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* Ex */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 30,
/* Fx */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27
#endif
#ifdef SQLITE_EBCDIC
/*         x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf */
/* 0x */   29, 28, 28, 28, 28,  7, 28, 28, 28, 28, 28, 28,  7,  7, 28, 28,
/* 1x */   28, 28, 28, 28, 28,  7, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
/* 2x */   28, 28, 28, 28, 28,  7, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
/* 3x */   28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
/* 4x */    7, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 26, 12, 17, 20, 10,
/* 5x */   24, 28, 28, 28, 28, 28, 28, 28, 28, 28, 15,  4, 21, 18, 19, 28,
/* 6x */   11, 16, 28, 28, 28, 28, 28, 28, 28, 28, 28, 23, 22,  2, 13,  6,
/* 7x */   28, 28, 28, 28, 28, 28, 28, 28, 28,  8,  5,  5,  5,  8, 14,  8,
/* 8x */   28,  1,  1,  1,  1,  1,  1,  1,  1,  1, 28, 28, 28, 28, 28, 28,
/* 9x */   28,  1,  1,  1,  1,  1,  1,  1,  1,  1, 28, 28, 28, 28, 28, 28,
/* Ax */   28, 25,  1,  1,  1,  1,  1,  0,  2,  2, 28, 28, 28, 28, 28, 28,
/* Bx */   28, 28, 28, 28, 28, 28, 28, 28, 28, 28,  9, 28, 28, 28, 28, 28,
/* Cx */   28,  1,  1,  1,  1,  1,  1,  1,  1,  1, 28, 28, 28, 28, 28, 28,
/* Dx */   28,  1,  1,  1,  1,  1,  1,  1,  1,  1, 28, 28, 28, 28, 28, 28,
/* Ex */   28, 28,  1,  1,  1,  1,  1,  0,  2,  2, 28, 28, 28, 28, 28, 28,
/* Fx */    3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 28, 28, 28, 28, 28, 28,
#endif
};

/*
** The charMap() macro maps alphabetic characters (only) into their
** lower-case ASCII equivalent.  On ASCII machines, this is just
** an upper-to-lower case map.  On EBCDIC machines we also need
** to adjust the encoding.  The mapping is only valid for alphabetics
** which are the only characters for which this feature is used. 
**
** Used by keywordhash.h
*/
#ifdef SQLITE_ASCII
# define charMap(X) sqlite3UpperToLower[(unsigned ଅକ୍ଷର)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned ଅକ୍ଷର)X]
ଅଚଳ unsigned ଅକ୍ଷର ebcdicToAscii[] = {
/* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 1x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 2x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 3x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 4x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 5x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 95,  0,  0,  /* 6x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 7x */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* 8x */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* 9x */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ax */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Bx */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* Cx */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* Dx */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ex */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Fx */
};
#endif

/*
** The sqlite3KeywordCode function looks up an identifier to determine if
** it is a keyword.  If it is a keyword, the token code of that keyword is 
** returned.  If the input is not a keyword, TK_ID is returned.
**
** The implementation of this routine was generated by a program,
** mkkeywordhash.c, located in the tool subdirectory of the distribution.
** The output of the mkkeywordhash.c program is written into a file
** named keywordhash.h and then included into this source file by
** the #include below.
*/
#ଅନ୍ତର୍ଭାଵ "keywordhash.h"


/*
** If X is a character that can be used in an identifier then
** IdChar(X) will be true.  Otherwise it is false.
**
** For ASCII, any character with the high-order bit set is
** allowed in an identifier.  For 7-bit characters, 
** sqlite3IsIdChar[X] must be 1.
**
** For EBCDIC, the rules are more complex but have the same
** end result.
**
** Ticket #1066.  the SQL standard does not allow '$' in the
** middle of identifiers.  But many SQL implementations do. 
** SQLite will allow '$' in identifiers for compatibility.
** But the feature is undocumented.
*/
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned ଅକ୍ଷର)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
ଅଚଳ ଅକ୍ଷର sqlite3IsEbcdicIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 4x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,  /* 5x */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,  /* 6x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  /* 7x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0,  /* 8x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0,  /* 9x */
    1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,  /* Ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* Bx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Cx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Dx */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Ex */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,  /* Fx */
};
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#endif

/* Make the IdChar function accessible from ctime.c and alter.c */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3IsIdChar(u8 c){ ଫେରନ୍ତୁ IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getToken(ଅଚଳ unsigned ଅକ୍ଷର **pz){
  ଅଚଳ unsigned ଅକ୍ଷର *z = *pz;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t;                          /* Token type to return */
  କର {
    z += sqlite3GetToken(z, &t);
  }ଯେତେବେଳେ( t==TK_SPACE || t==TK_COMMENT );
  ଯଦି( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  ଫେରନ୍ତୁ t;
}

/*
** The following three functions are called immediately after the tokenizer
** reads the keywords WINDOW, OVER and FILTER, respectively, to determine
** whether the token should be treated as a keyword or an SQL identifier.
** This cannot be handled by the usual lemon %fallback method, due to
** the ambiguity in some constructions. e.g.
**
**   SELECT sum(x) OVER ...
**
** In the above, "OVER" might be a keyword, or it might be an alias for the 
** sum(x) expression. If a "%fallback ID OVER" directive were added to 
** grammar, then SQLite would always treat "OVER" as an alias, making it
** impossible to call a window-function without a FILTER clause.
**
** WINDOW is treated as a keyword if:
**
**   * the following token is an identifier, or a keyword that can fallback
**     to being an identifier, and
**   * the token after than one is TK_AS.
**
** OVER is a keyword if:
**
**   * the previous token was TK_RP, and
**   * the next token is either TK_LP or an identifier.
**
** FILTER is a keyword if:
**
**   * the previous token was TK_RP, and
**   * the next token is TK_LP.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା analyzeWindowKeyword(ଅଚଳ unsigned ଅକ୍ଷର *z){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t;
  t = getToken(&z);
  ଯଦି( t!=TK_ID ) ଫେରନ୍ତୁ TK_ID;
  t = getToken(&z);
  ଯଦି( t!=TK_AS ) ଫେରନ୍ତୁ TK_ID;
  ଫେରନ୍ତୁ TK_WINDOW;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା analyzeOverKeyword(ଅଚଳ unsigned ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastToken){
  ଯଦି( lastToken==TK_RP ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getToken(&z);
    ଯଦି( t==TK_LP || t==TK_ID ) ଫେରନ୍ତୁ TK_OVER;
  }
  ଫେରନ୍ତୁ TK_ID;
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା analyzeFilterKeyword(ଅଚଳ unsigned ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastToken){
  ଯଦି( lastToken==TK_RP && getToken(&z)==TK_LP ){
    ଫେରନ୍ତୁ TK_FILTER;
  }
  ଫେରନ୍ତୁ TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ଅଚଳ unsigned ଅକ୍ଷର *z, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tokenType){
  i64 i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
  ବାଛ( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    ସ୍ଥିତି CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      ପାଇଁ(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      ଫେରନ୍ତୁ i;
    }
    ସ୍ଥିତି CC_MINUS: {
      ଯଦି( z[1]=='-' ){
        ପାଇଁ(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        ଫେରନ୍ତୁ i;
      }ଅନ୍ୟଥା ଯଦି( z[1]=='>' ){
        *tokenType = TK_PTR;
        ଫେରନ୍ତୁ 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_LP: {
      *tokenType = TK_LP;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_RP: {
      *tokenType = TK_RP;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_SEMI: {
      *tokenType = TK_SEMI;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_PLUS: {
      *tokenType = TK_PLUS;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_STAR: {
      *tokenType = TK_STAR;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_SLASH: {
      ଯଦି( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        ଫେରନ୍ତୁ 1;
      }
      ପାଇଁ(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      ଯଦି( c ) i++;
      *tokenType = TK_COMMENT;
      ଫେରନ୍ତୁ i;
    }
    ସ୍ଥିତି CC_PERCENT: {
      *tokenType = TK_REM;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_EQ: {
      *tokenType = TK_EQ;
      ଫେରନ୍ତୁ 1 + (z[1]=='=');
    }
    ସ୍ଥିତି CC_LT: {
      ଯଦି( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        ଫେରନ୍ତୁ 2;
      }ଅନ୍ୟଥା ଯଦି( c=='>' ){
        *tokenType = TK_NE;
        ଫେରନ୍ତୁ 2;
      }ଅନ୍ୟଥା ଯଦି( c=='<' ){
        *tokenType = TK_LSHIFT;
        ଫେରନ୍ତୁ 2;
      }ଅନ୍ୟଥା{
        *tokenType = TK_LT;
        ଫେରନ୍ତୁ 1;
      }
    }
    ସ୍ଥିତି CC_GT: {
      ଯଦି( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        ଫେରନ୍ତୁ 2;
      }ଅନ୍ୟଥା ଯଦି( c=='>' ){
        *tokenType = TK_RSHIFT;
        ଫେରନ୍ତୁ 2;
      }ଅନ୍ୟଥା{
        *tokenType = TK_GT;
        ଫେରନ୍ତୁ 1;
      }
    }
    ସ୍ଥିତି CC_BANG: {
      ଯଦି( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        ଫେରନ୍ତୁ 1;
      }ଅନ୍ୟଥା{
        *tokenType = TK_NE;
        ଫେରନ୍ତୁ 2;
      }
    }
    ସ୍ଥିତି CC_PIPE: {
      ଯଦି( z[1]!='|' ){
        *tokenType = TK_BITOR;
        ଫେରନ୍ତୁ 1;
      }ଅନ୍ୟଥା{
        *tokenType = TK_CONCAT;
        ଫେରନ୍ତୁ 2;
      }
    }
    ସ୍ଥିତି CC_COMMA: {
      *tokenType = TK_COMMA;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_AND: {
      *tokenType = TK_BITAND;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_TILDA: {
      *tokenType = TK_BITNOT;
      ଫେରନ୍ତୁ 1;
    }
    ସ୍ଥିତି CC_QUOTE: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      ପାଇଁ(i=1; (c=z[i])!=0; i++){
        ଯଦି( c==delim ){
          ଯଦି( z[i+1]==delim ){
            i++;
          }ଅନ୍ୟଥା{
            ଭାଙ୍ଗ;
          }
        }
      }
      ଯଦି( c=='\'' ){
        *tokenType = TK_STRING;
        ଫେରନ୍ତୁ i+1;
      }ଅନ୍ୟଥା ଯଦି( c!=0 ){
        *tokenType = TK_ID;
        ଫେରନ୍ତୁ i+1;
      }ଅନ୍ୟଥା{
        *tokenType = TK_ILLEGAL;
        ଫେରନ୍ତୁ i;
      }
    }
    ସ୍ଥିତି CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      ଯଦି( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        ଫେରନ୍ତୁ 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    ସ୍ଥିତି CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      ଯଦି( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        ପାଇଁ(i=3; 1; i++){
          ଯଦି( sqlite3Isxdigit(z[i])==0 ){
            ଯଦି( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ଅନ୍ୟଥା{
              ଭାଙ୍ଗ;
            }
          }
        }
      }ଅନ୍ୟଥା
#endif
        {
        ପାଇଁ(i=0; 1; i++){
          ଯଦି( sqlite3Isdigit(z[i])==0 ){
            ଯଦି( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ଅନ୍ୟଥା{
              ଭାଙ୍ଗ;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        ଯଦି( z[i]=='.' ){
          ଯଦି( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ପାଇଁ(i++; 1; i++){
            ଯଦି( sqlite3Isdigit(z[i])==0 ){
              ଯଦି( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ଅନ୍ୟଥା{
                ଭାଙ୍ଗ;
              }
            }
          }
        }
        ଯଦି( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          ଯଦି( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ପାଇଁ(i+=2; 1; i++){
            ଯଦି( sqlite3Isdigit(z[i])==0 ){
              ଯଦି( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ଅନ୍ୟଥା{
                ଭାଙ୍ଗ;
              }
            }
          }
        }
#endif
      }
      ଯେତେବେଳେ( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      ଫେରନ୍ତୁ i;
    }
    ସ୍ଥିତି CC_QUOTE2: {
      ପାଇଁ(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      ଫେରନ୍ତୁ i;
    }
    ସ୍ଥିତି CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      ପାଇଁ(i=1; sqlite3Isdigit(z[i]); i++){}
      ଫେରନ୍ତୁ i;
    }
    ସ୍ଥିତି CC_DOLLAR:
    ସ୍ଥିତି CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      ପାଇଁ(i=1; (c=z[i])!=0; i++){
        ଯଦି( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }ଅନ୍ୟଥା ଯଦି( c=='(' && n>0 ){
          କର{
            i++;
          }ଯେତେବେଳେ( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          ଯଦି( c==')' ){
            i++;
          }ଅନ୍ୟଥା{
            *tokenType = TK_ILLEGAL;
          }
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା ଯଦି( c==':' && z[i+1]==':' ){
          i++;
#endif
        }ଅନ୍ୟଥା{
          ଭାଙ୍ଗ;
        }
      }
      ଯଦି( n==0 ) *tokenType = TK_ILLEGAL;
      ଫେରନ୍ତୁ i;
    }
    ସ୍ଥିତି CC_KYWD0: {
      ଯଦି( aiClass[z[1]]>CC_KYWD ){ i = 1;  ଭାଙ୍ଗ; }
      ପାଇଁ(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      ଯଦି( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        ଭାଙ୍ଗ;
      }
      *tokenType = TK_ID;
      ଫେରନ୍ତୁ keywordCode((ଅକ୍ଷର*)z, i, tokenType);
    }
    ସ୍ଥିତି CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      ଯଦି( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        ପାଇଁ(i=2; sqlite3Isxdigit(z[i]); i++){}
        ଯଦି( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          ଯେତେବେଳେ( z[i] && z[i]!='\'' ){ i++; }
        }
        ଯଦି( z[i] ) i++;
        ଫେରନ୍ତୁ i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    ସ୍ଥିତି CC_KYWD:
    ସ୍ଥିତି CC_ID: {
      i = 1;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି CC_BOM: {
      ଯଦି( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        ଫେରନ୍ତୁ 3;
      }
      i = 1;
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି CC_NUL: {
      *tokenType = TK_ILLEGAL;
      ଫେରନ୍ତୁ 0;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      *tokenType = TK_ILLEGAL;
      ଫେରନ୍ତୁ 1;
    }
  }
  ଯେତେବେଳେ( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  ଫେରନ୍ତୁ i;
}

/*
** Run the parser on the given SQL string.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3RunParser(Parse *pParse, ଅଚଳ ଅକ୍ଷର *zSql){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nErr = 0;                   /* Number of errors encountered */
  ଶୂନ୍ୟ *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tokenType;                  /* type of the next token */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  ପ୍ରତିଜ୍ଞା( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  ଯଦି( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  ଯଦି( db->flags & SQLITE_ParserTrace ){
    printf("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }ଅନ୍ୟଥା{
    sqlite3ParserTrace(0, 0);
  }
#endif
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#ଅନ୍ୟଥା
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  ଯଦି( pEngine==0 ){
    sqlite3OomFault(db);
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
#endif
  ପ୍ରତିଜ୍ଞା( pParse->pNewTable==0 );
  ପ୍ରତିଜ୍ଞା( pParse->pNewTrigger==0 );
  ପ୍ରତିଜ୍ଞା( pParse->nVar==0 );
  ପ୍ରତିଜ୍ଞା( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  ଯେତେବେଳେ( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    ଯଦି( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( tokenType>=TK_WINDOW ){
      ପ୍ରତିଜ୍ଞା( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ଅନ୍ୟଥା
    ଯଦି( tokenType>=TK_SPACE ){
      ପ୍ରତିଜ୍ଞା( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      ଯଦି( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        ଭାଙ୍ଗ;
      }
      ଯଦି( tokenType==TK_SPACE ){
        zSql += n;
        ଜାରି;
      }
      ଯଦି( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        ଯଦି( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }ଅନ୍ୟଥା ଯଦି( lastTokenParsed==0 ){
          ଭାଙ୍ଗ;
        }ଅନ୍ୟଥା{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }ଅନ୍ୟଥା ଯଦି( tokenType==TK_WINDOW ){
        ପ୍ରତିଜ୍ଞା( n==6 );
        tokenType = analyzeWindowKeyword((ଅଚଳ u8*)&zSql[6]);
      }ଅନ୍ୟଥା ଯଦି( tokenType==TK_OVER ){
        ପ୍ରତିଜ୍ଞା( n==4 );
        tokenType = analyzeOverKeyword((ଅଚଳ u8*)&zSql[4], lastTokenParsed);
      }ଅନ୍ୟଥା ଯଦି( tokenType==TK_FILTER ){
        ପ୍ରତିଜ୍ଞା( n==6 );
        tokenType = analyzeFilterKeyword((ଅଚଳ u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }ଅନ୍ୟଥା ଯଦି( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        ଜାରି;
      }ଅନ୍ୟଥା ଯଦି( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "unrecognized token: \"%T\"", &x);
        ଭାଙ୍ଗ;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    ଯଦି( pParse->rc!=SQLITE_OK ) ଭାଙ୍ଗ;
  }
  ପ୍ରତିଜ୍ଞା( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_ଗଦା,
      sqlite3_Parser_ଗଦା_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#ଅନ୍ୟଥା
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  ଯଦି( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  ଯଦି( pParse->z_Err_ସନ୍ଦେଶ || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    ଯଦି( pParse->z_Err_ସନ୍ଦେଶ==0 ){
      pParse->z_Err_ସନ୍ଦେଶ = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    ଯଦି( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_ସନ୍ଦେଶ, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  ଯଦି( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  ଯଦି( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  ଯଦି( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  ପ୍ରତିଜ୍ଞା( nErr==0 || pParse->rc!=SQLITE_OK );
  ଫେରନ୍ତୁ nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
ସ୍ଥିର ଶୂନ୍ୟ addSpaceSeparator(sqlite3_str *pStr){
  ଯଦି( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
ଅକ୍ଷର *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  ଅଚଳ ଅକ୍ଷର *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tokenType;     /* type of current token */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା prevType = 0;  /* Previous non-whitespace token */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nParen;        /* Number of nested levels of parentheses */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStartIN;      /* Start of RHS of IN operator in z[] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  ପ୍ରତିଜ୍ଞା( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  ପାଇଁ(i=0; zSql[i] && pStr->accError==0; i+=n){
    ଯଦି( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((unsigned ଅକ୍ଷର*)zSql+i, &tokenType);
    ଯଦି( NEVER(n<=0) ) ଭାଙ୍ଗ;
    ବାଛ( tokenType ){
      ସ୍ଥିତି TK_COMMENT:
      ସ୍ଥିତି TK_SPACE: {
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_NULL: {
        ଯଦି( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          ଭାଙ୍ଗ;
        }
        /* no break */ deliberate_fall_through
      }
      ସ୍ଥିତି TK_STRING:
      ସ୍ଥିତି TK_INTEGER:
      ସ୍ଥିତି TK_FLOAT:
      ସ୍ଥିତି TK_VARIABLE:
      ସ୍ଥିତି TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_LP: {
        nParen++;
        ଯଦି( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_RP: {
        ଯଦି( iStartIN>0 && nParen==nParenAtIN ){
          ପ୍ରତିଜ୍ଞା( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        ଯଦି( sqlite3Isquote(zSql[i]) ){
          ଅକ୍ଷର *zId = sqlite3DbStrNDup(db, zSql+i, n);
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nId;
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = 0;
          ଯଦି( zId==0 ) ଭାଙ୍ଗ;
          sqlite3Dequote(zId);
          ଯଦି( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            ଭାଙ୍ଗ;
          }
          nId = sqlite3Strlen30(zId);
          ଯଦି( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            addSpaceSeparator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }ଅନ୍ୟଥା{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }ଅନ୍ୟଥା{
          addSpaceSeparator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        ଯେତେବେଳେ( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ଯଦି( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        ଯେତେବେଳେ( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        ଭାଙ୍ଗ;
      }
    }
  }
  ଯଦି( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  ଫେରନ୍ତୁ sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
