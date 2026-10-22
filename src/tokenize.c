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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <stdlib.h>

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

நிலையான மாறிலி unsigned எழுத்து aiClass[] = {
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
# define charMap(X) sqlite3UpperToLower[(unsigned எழுத்து)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned எழுத்து)X]
மாறிலி unsigned எழுத்து ebcdicToAscii[] = {
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
#அந்தர்பாவ "keywordhash.h"


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
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned எழுத்து)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
மாறிலி எழுத்து sqlite3IsEbcdicIdChar[] = {
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
முழு sqlite3IsIdChar(u8 c){ திரும்பு IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
நிலையான முழு getToken(மாறிலி unsigned எழுத்து **pz){
  மாறிலி unsigned எழுத்து *z = *pz;
  முழு t;                          /* Token type to return */
  செய் {
    z += sqlite3GetToken(z, &t);
  }போது( t==TK_SPACE || t==TK_COMMENT );
  எனில்( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  திரும்பு t;
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
நிலையான முழு analyzeWindowKeyword(மாறிலி unsigned எழுத்து *z){
  முழு t;
  t = getToken(&z);
  எனில்( t!=TK_ID ) திரும்பு TK_ID;
  t = getToken(&z);
  எனில்( t!=TK_AS ) திரும்பு TK_ID;
  திரும்பு TK_WINDOW;
}
நிலையான முழு analyzeOverKeyword(மாறிலி unsigned எழுத்து *z, முழு lastToken){
  எனில்( lastToken==TK_RP ){
    முழு t = getToken(&z);
    எனில்( t==TK_LP || t==TK_ID ) திரும்பு TK_OVER;
  }
  திரும்பு TK_ID;
}
நிலையான முழு analyzeFilterKeyword(மாறிலி unsigned எழுத்து *z, முழு lastToken){
  எனில்( lastToken==TK_RP && getToken(&z)==TK_LP ){
    திரும்பு TK_FILTER;
  }
  திரும்பு TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(மாறிலி unsigned எழுத்து *z, முழு *tokenType){
  i64 i;
  முழு c;
  தேர்வு( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    நிலை CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      க்கு(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      திரும்பு i;
    }
    நிலை CC_MINUS: {
      எனில்( z[1]=='-' ){
        க்கு(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        திரும்பு i;
      }இல்லையெனில் எனில்( z[1]=='>' ){
        *tokenType = TK_PTR;
        திரும்பு 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      திரும்பு 1;
    }
    நிலை CC_LP: {
      *tokenType = TK_LP;
      திரும்பு 1;
    }
    நிலை CC_RP: {
      *tokenType = TK_RP;
      திரும்பு 1;
    }
    நிலை CC_SEMI: {
      *tokenType = TK_SEMI;
      திரும்பு 1;
    }
    நிலை CC_PLUS: {
      *tokenType = TK_PLUS;
      திரும்பு 1;
    }
    நிலை CC_STAR: {
      *tokenType = TK_STAR;
      திரும்பு 1;
    }
    நிலை CC_SLASH: {
      எனில்( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        திரும்பு 1;
      }
      க்கு(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      எனில்( c ) i++;
      *tokenType = TK_COMMENT;
      திரும்பு i;
    }
    நிலை CC_PERCENT: {
      *tokenType = TK_REM;
      திரும்பு 1;
    }
    நிலை CC_EQ: {
      *tokenType = TK_EQ;
      திரும்பு 1 + (z[1]=='=');
    }
    நிலை CC_LT: {
      எனில்( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        திரும்பு 2;
      }இல்லையெனில் எனில்( c=='>' ){
        *tokenType = TK_NE;
        திரும்பு 2;
      }இல்லையெனில் எனில்( c=='<' ){
        *tokenType = TK_LSHIFT;
        திரும்பு 2;
      }இல்லையெனில்{
        *tokenType = TK_LT;
        திரும்பு 1;
      }
    }
    நிலை CC_GT: {
      எனில்( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        திரும்பு 2;
      }இல்லையெனில் எனில்( c=='>' ){
        *tokenType = TK_RSHIFT;
        திரும்பு 2;
      }இல்லையெனில்{
        *tokenType = TK_GT;
        திரும்பு 1;
      }
    }
    நிலை CC_BANG: {
      எனில்( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        திரும்பு 1;
      }இல்லையெனில்{
        *tokenType = TK_NE;
        திரும்பு 2;
      }
    }
    நிலை CC_PIPE: {
      எனில்( z[1]!='|' ){
        *tokenType = TK_BITOR;
        திரும்பு 1;
      }இல்லையெனில்{
        *tokenType = TK_CONCAT;
        திரும்பு 2;
      }
    }
    நிலை CC_COMMA: {
      *tokenType = TK_COMMA;
      திரும்பு 1;
    }
    நிலை CC_AND: {
      *tokenType = TK_BITAND;
      திரும்பு 1;
    }
    நிலை CC_TILDA: {
      *tokenType = TK_BITNOT;
      திரும்பு 1;
    }
    நிலை CC_QUOTE: {
      முழு delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      க்கு(i=1; (c=z[i])!=0; i++){
        எனில்( c==delim ){
          எனில்( z[i+1]==delim ){
            i++;
          }இல்லையெனில்{
            நிறுத்து;
          }
        }
      }
      எனில்( c=='\'' ){
        *tokenType = TK_STRING;
        திரும்பு i+1;
      }இல்லையெனில் எனில்( c!=0 ){
        *tokenType = TK_ID;
        திரும்பு i+1;
      }இல்லையெனில்{
        *tokenType = TK_ILLEGAL;
        திரும்பு i;
      }
    }
    நிலை CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      எனில்( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        திரும்பு 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    நிலை CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      எனில்( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        க்கு(i=3; 1; i++){
          எனில்( sqlite3Isxdigit(z[i])==0 ){
            எனில்( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }இல்லையெனில்{
              நிறுத்து;
            }
          }
        }
      }இல்லையெனில்
#endif
        {
        க்கு(i=0; 1; i++){
          எனில்( sqlite3Isdigit(z[i])==0 ){
            எனில்( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }இல்லையெனில்{
              நிறுத்து;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        எனில்( z[i]=='.' ){
          எனில்( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          க்கு(i++; 1; i++){
            எனில்( sqlite3Isdigit(z[i])==0 ){
              எனில்( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }இல்லையெனில்{
                நிறுத்து;
              }
            }
          }
        }
        எனில்( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          எனில்( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          க்கு(i+=2; 1; i++){
            எனில்( sqlite3Isdigit(z[i])==0 ){
              எனில்( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }இல்லையெனில்{
                நிறுத்து;
              }
            }
          }
        }
#endif
      }
      போது( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      திரும்பு i;
    }
    நிலை CC_QUOTE2: {
      க்கு(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      திரும்பு i;
    }
    நிலை CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      க்கு(i=1; sqlite3Isdigit(z[i]); i++){}
      திரும்பு i;
    }
    நிலை CC_DOLLAR:
    நிலை CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      க்கு(i=1; (c=z[i])!=0; i++){
        எனில்( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }இல்லையெனில் எனில்( c=='(' && n>0 ){
          செய்{
            i++;
          }போது( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          எனில்( c==')' ){
            i++;
          }இல்லையெனில்{
            *tokenType = TK_ILLEGAL;
          }
          நிறுத்து;
        }இல்லையெனில் எனில்( c==':' && z[i+1]==':' ){
          i++;
#endif
        }இல்லையெனில்{
          நிறுத்து;
        }
      }
      எனில்( n==0 ) *tokenType = TK_ILLEGAL;
      திரும்பு i;
    }
    நிலை CC_KYWD0: {
      எனில்( aiClass[z[1]]>CC_KYWD ){ i = 1;  நிறுத்து; }
      க்கு(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      எனில்( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        நிறுத்து;
      }
      *tokenType = TK_ID;
      திரும்பு keywordCode((எழுத்து*)z, i, tokenType);
    }
    நிலை CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      எனில்( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        க்கு(i=2; sqlite3Isxdigit(z[i]); i++){}
        எனில்( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          போது( z[i] && z[i]!='\'' ){ i++; }
        }
        எனில்( z[i] ) i++;
        திரும்பு i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    நிலை CC_KYWD:
    நிலை CC_ID: {
      i = 1;
      நிறுத்து;
    }
    நிலை CC_BOM: {
      எனில்( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        திரும்பு 3;
      }
      i = 1;
      நிறுத்து;
    }
    நிலை CC_NUL: {
      *tokenType = TK_ILLEGAL;
      திரும்பு 0;
    }
    இயல்பு: {
      *tokenType = TK_ILLEGAL;
      திரும்பு 1;
    }
  }
  போது( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  திரும்பு i;
}

/*
** Run the parser on the given SQL string.
*/
முழு sqlite3RunParser(Parse *pParse, மாறிலி எழுத்து *zSql){
  முழு nErr = 0;                   /* Number of errors encountered */
  வெறுமை *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  முழு tokenType;                  /* type of the next token */
  முழு lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  ப்ரதிஜ்ஞா( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  எனில்( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  எனில்( db->flags & SQLITE_ParserTrace ){
    printf("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }இல்லையெனில்{
    sqlite3ParserTrace(0, 0);
  }
#endif
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#இல்லையெனில்
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  எனில்( pEngine==0 ){
    sqlite3OomFault(db);
    திரும்பு SQLITE_NOMEM_BKPT;
  }
#endif
  ப்ரதிஜ்ஞா( pParse->pNewTable==0 );
  ப்ரதிஜ்ஞா( pParse->pNewTrigger==0 );
  ப்ரதிஜ்ஞா( pParse->nVar==0 );
  ப்ரதிஜ்ஞா( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  போது( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    எனில்( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    எனில்( tokenType>=TK_WINDOW ){
      ப்ரதிஜ்ஞா( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#இல்லையெனில்
    எனில்( tokenType>=TK_SPACE ){
      ப்ரதிஜ்ஞா( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      எனில்( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        நிறுத்து;
      }
      எனில்( tokenType==TK_SPACE ){
        zSql += n;
        தொடர்;
      }
      எனில்( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        எனில்( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }இல்லையெனில் எனில்( lastTokenParsed==0 ){
          நிறுத்து;
        }இல்லையெனில்{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }இல்லையெனில் எனில்( tokenType==TK_WINDOW ){
        ப்ரதிஜ்ஞா( n==6 );
        tokenType = analyzeWindowKeyword((மாறிலி u8*)&zSql[6]);
      }இல்லையெனில் எனில்( tokenType==TK_OVER ){
        ப்ரதிஜ்ஞா( n==4 );
        tokenType = analyzeOverKeyword((மாறிலி u8*)&zSql[4], lastTokenParsed);
      }இல்லையெனில் எனில்( tokenType==TK_FILTER ){
        ப்ரதிஜ்ஞா( n==6 );
        tokenType = analyzeFilterKeyword((மாறிலி u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }இல்லையெனில் எனில்( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        தொடர்;
      }இல்லையெனில் எனில்( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_செய்தி(pParse, "unrecognized token: \"%T\"", &x);
        நிறுத்து;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    ப்ரதிஜ்ஞா( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    எனில்( pParse->rc!=SQLITE_OK ) நிறுத்து;
  }
  ப்ரதிஜ்ஞா( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_அடுக்கு,
      sqlite3_Parser_அடுக்கு_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#இல்லையெனில்
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  எனில்( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  எனில்( pParse->z_Err_செய்தி || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    எனில்( pParse->z_Err_செய்தி==0 ){
      pParse->z_Err_செய்தி = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    எனில்( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_செய்தி, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  எனில்( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  எனில்( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  எனில்( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  ப்ரதிஜ்ஞா( nErr==0 || pParse->rc!=SQLITE_OK );
  திரும்பு nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
நிலையான வெறுமை addSpaceSeparator(sqlite3_str *pStr){
  எனில்( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
எழுத்து *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  மாறிலி எழுத்து *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  முழு i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  முழு tokenType;     /* type of current token */
  முழு prevType = 0;  /* Previous non-whitespace token */
  முழு nParen;        /* Number of nested levels of parentheses */
  முழு iStartIN;      /* Start of RHS of IN operator in z[] */
  முழு nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  ப்ரதிஜ்ஞா( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  க்கு(i=0; zSql[i] && pStr->accError==0; i+=n){
    எனில்( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((unsigned எழுத்து*)zSql+i, &tokenType);
    எனில்( NEVER(n<=0) ) நிறுத்து;
    தேர்வு( tokenType ){
      நிலை TK_COMMENT:
      நிலை TK_SPACE: {
        நிறுத்து;
      }
      நிலை TK_NULL: {
        எனில்( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          நிறுத்து;
        }
        /* no break */ deliberate_fall_through
      }
      நிலை TK_STRING:
      நிலை TK_INTEGER:
      நிலை TK_FLOAT:
      நிலை TK_VARIABLE:
      நிலை TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        நிறுத்து;
      }
      நிலை TK_LP: {
        nParen++;
        எனில்( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        நிறுத்து;
      }
      நிலை TK_RP: {
        எனில்( iStartIN>0 && nParen==nParenAtIN ){
          ப்ரதிஜ்ஞா( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        நிறுத்து;
      }
      நிலை TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        எனில்( sqlite3Isquote(zSql[i]) ){
          எழுத்து *zId = sqlite3DbStrNDup(db, zSql+i, n);
          முழு nId;
          முழு eType = 0;
          எனில்( zId==0 ) நிறுத்து;
          sqlite3Dequote(zId);
          எனில்( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            நிறுத்து;
          }
          nId = sqlite3Strlen30(zId);
          எனில்( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            addSpaceSeparator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }இல்லையெனில்{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }இல்லையெனில்{
          addSpaceSeparator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        போது( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        நிறுத்து;
      }
      நிலை TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      இயல்பு: {
        எனில்( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        போது( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        நிறுத்து;
      }
    }
  }
  எனில்( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  திரும்பு sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
