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
#include "sqliteInt.h"
#include <stdlib.h>

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

static const unsigned char aiClass[] = {
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
# define charMap(X) sqlite3UpperToLower[(unsigned char)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned char)X]
const unsigned char ebcdicToAscii[] = {
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
#include "keywordhash.h"


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
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned char)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
const char sqlite3IsEbcdicIdChar[] = {
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
정수 sqlite3IsIdChar(u8 c){ 반환 IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
static 정수 getToken(const unsigned char **pz){
  const unsigned char *z = *pz;
  정수 t;                          /* Token type to return */
  do {
    z += sqlite3GetToken(z, &t);
  }동안( t==TK_SPACE || t==TK_COMMENT );
  만약( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  반환 t;
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
static 정수 analyzeWindowKeyword(const unsigned char *z){
  정수 t;
  t = getToken(&z);
  만약( t!=TK_ID ) 반환 TK_ID;
  t = getToken(&z);
  만약( t!=TK_AS ) 반환 TK_ID;
  반환 TK_WINDOW;
}
static 정수 analyzeOverKeyword(const unsigned char *z, 정수 lastToken){
  만약( lastToken==TK_RP ){
    정수 t = getToken(&z);
    만약( t==TK_LP || t==TK_ID ) 반환 TK_OVER;
  }
  반환 TK_ID;
}
static 정수 analyzeFilterKeyword(const unsigned char *z, 정수 lastToken){
  만약( lastToken==TK_RP && getToken(&z)==TK_LP ){
    반환 TK_FILTER;
  }
  반환 TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(const unsigned char *z, 정수 *tokenType){
  i64 i;
  정수 c;
  switch( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    case CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      위해(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      반환 i;
    }
    case CC_MINUS: {
      만약( z[1]=='-' ){
        위해(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        반환 i;
      }아니면 만약( z[1]=='>' ){
        *tokenType = TK_PTR;
        반환 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      반환 1;
    }
    case CC_LP: {
      *tokenType = TK_LP;
      반환 1;
    }
    case CC_RP: {
      *tokenType = TK_RP;
      반환 1;
    }
    case CC_SEMI: {
      *tokenType = TK_SEMI;
      반환 1;
    }
    case CC_PLUS: {
      *tokenType = TK_PLUS;
      반환 1;
    }
    case CC_STAR: {
      *tokenType = TK_STAR;
      반환 1;
    }
    case CC_SLASH: {
      만약( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        반환 1;
      }
      위해(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      만약( c ) i++;
      *tokenType = TK_COMMENT;
      반환 i;
    }
    case CC_PERCENT: {
      *tokenType = TK_REM;
      반환 1;
    }
    case CC_EQ: {
      *tokenType = TK_EQ;
      반환 1 + (z[1]=='=');
    }
    case CC_LT: {
      만약( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        반환 2;
      }아니면 만약( c=='>' ){
        *tokenType = TK_NE;
        반환 2;
      }아니면 만약( c=='<' ){
        *tokenType = TK_LSHIFT;
        반환 2;
      }아니면{
        *tokenType = TK_LT;
        반환 1;
      }
    }
    case CC_GT: {
      만약( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        반환 2;
      }아니면 만약( c=='>' ){
        *tokenType = TK_RSHIFT;
        반환 2;
      }아니면{
        *tokenType = TK_GT;
        반환 1;
      }
    }
    case CC_BANG: {
      만약( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        반환 1;
      }아니면{
        *tokenType = TK_NE;
        반환 2;
      }
    }
    case CC_PIPE: {
      만약( z[1]!='|' ){
        *tokenType = TK_BITOR;
        반환 1;
      }아니면{
        *tokenType = TK_CONCAT;
        반환 2;
      }
    }
    case CC_COMMA: {
      *tokenType = TK_COMMA;
      반환 1;
    }
    case CC_AND: {
      *tokenType = TK_BITAND;
      반환 1;
    }
    case CC_TILDA: {
      *tokenType = TK_BITNOT;
      반환 1;
    }
    case CC_QUOTE: {
      정수 delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      위해(i=1; (c=z[i])!=0; i++){
        만약( c==delim ){
          만약( z[i+1]==delim ){
            i++;
          }아니면{
            break;
          }
        }
      }
      만약( c=='\'' ){
        *tokenType = TK_STRING;
        반환 i+1;
      }아니면 만약( c!=0 ){
        *tokenType = TK_ID;
        반환 i+1;
      }아니면{
        *tokenType = TK_ILLEGAL;
        반환 i;
      }
    }
    case CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      만약( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        반환 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    case CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      만약( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        위해(i=3; 1; i++){
          만약( sqlite3Isxdigit(z[i])==0 ){
            만약( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }아니면{
              break;
            }
          }
        }
      }아니면
#endif
        {
        위해(i=0; 1; i++){
          만약( sqlite3Isdigit(z[i])==0 ){
            만약( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }아니면{
              break;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        만약( z[i]=='.' ){
          만약( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          위해(i++; 1; i++){
            만약( sqlite3Isdigit(z[i])==0 ){
              만약( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }아니면{
                break;
              }
            }
          }
        }
        만약( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          만약( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          위해(i+=2; 1; i++){
            만약( sqlite3Isdigit(z[i])==0 ){
              만약( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }아니면{
                break;
              }
            }
          }
        }
#endif
      }
      동안( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      반환 i;
    }
    case CC_QUOTE2: {
      위해(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      반환 i;
    }
    case CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      위해(i=1; sqlite3Isdigit(z[i]); i++){}
      반환 i;
    }
    case CC_DOLLAR:
    case CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      위해(i=1; (c=z[i])!=0; i++){
        만약( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }아니면 만약( c=='(' && n>0 ){
          do{
            i++;
          }동안( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          만약( c==')' ){
            i++;
          }아니면{
            *tokenType = TK_ILLEGAL;
          }
          break;
        }아니면 만약( c==':' && z[i+1]==':' ){
          i++;
#endif
        }아니면{
          break;
        }
      }
      만약( n==0 ) *tokenType = TK_ILLEGAL;
      반환 i;
    }
    case CC_KYWD0: {
      만약( aiClass[z[1]]>CC_KYWD ){ i = 1;  break; }
      위해(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      만약( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        break;
      }
      *tokenType = TK_ID;
      반환 keywordCode((char*)z, i, tokenType);
    }
    case CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      만약( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        위해(i=2; sqlite3Isxdigit(z[i]); i++){}
        만약( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          동안( z[i] && z[i]!='\'' ){ i++; }
        }
        만약( z[i] ) i++;
        반환 i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    case CC_KYWD:
    case CC_ID: {
      i = 1;
      break;
    }
    case CC_BOM: {
      만약( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        반환 3;
      }
      i = 1;
      break;
    }
    case CC_NUL: {
      *tokenType = TK_ILLEGAL;
      반환 0;
    }
    default: {
      *tokenType = TK_ILLEGAL;
      반환 1;
    }
  }
  동안( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  반환 i;
}

/*
** Run the parser on the given SQL string.
*/
정수 sqlite3RunParser(Parse *pParse, const char *zSql){
  정수 nErr = 0;                   /* Number of errors encountered */
  void *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  정수 tokenType;                  /* type of the next token */
  정수 lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  assert( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  만약( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  만약( db->flags & SQLITE_ParserTrace ){
    printf("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }아니면{
    sqlite3ParserTrace(0, 0);
  }
#endif
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#아니면
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  만약( pEngine==0 ){
    sqlite3OomFault(db);
    반환 SQLITE_NOMEM_BKPT;
  }
#endif
  assert( pParse->pNewTable==0 );
  assert( pParse->pNewTrigger==0 );
  assert( pParse->nVar==0 );
  assert( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  동안( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    만약( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      break;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    만약( tokenType>=TK_WINDOW ){
      assert( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#아니면
    만약( tokenType>=TK_SPACE ){
      assert( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      만약( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        break;
      }
      만약( tokenType==TK_SPACE ){
        zSql += n;
        continue;
      }
      만약( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        만약( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }아니면 만약( lastTokenParsed==0 ){
          break;
        }아니면{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }아니면 만약( tokenType==TK_WINDOW ){
        assert( n==6 );
        tokenType = analyzeWindowKeyword((const u8*)&zSql[6]);
      }아니면 만약( tokenType==TK_OVER ){
        assert( n==4 );
        tokenType = analyzeOverKeyword((const u8*)&zSql[4], lastTokenParsed);
      }아니면 만약( tokenType==TK_FILTER ){
        assert( n==6 );
        tokenType = analyzeFilterKeyword((const u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }아니면 만약( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        continue;
      }아니면 만약( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3ErrorMsg(pParse, "unrecognized token: \"%T\"", &x);
        break;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    assert( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    만약( pParse->rc!=SQLITE_OK ) break;
  }
  assert( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_STACK,
      sqlite3ParserStackPeak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#아니면
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  만약( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  만약( pParse->zErrMsg || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    만약( pParse->zErrMsg==0 ){
      pParse->zErrMsg = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    만약( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->zErrMsg, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  만약( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  만약( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  만약( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  assert( nErr==0 || pParse->rc!=SQLITE_OK );
  반환 nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
static void addSpaceSeparator(sqlite3_str *pStr){
  만약( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
char *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  const char *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  정수 i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  정수 tokenType;     /* type of current token */
  정수 prevType = 0;  /* Previous non-whitespace token */
  정수 nParen;        /* Number of nested levels of parentheses */
  정수 iStartIN;      /* Start of RHS of IN operator in z[] */
  정수 nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  assert( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  위해(i=0; zSql[i] && pStr->accError==0; i+=n){
    만약( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((unsigned char*)zSql+i, &tokenType);
    만약( NEVER(n<=0) ) break;
    switch( tokenType ){
      case TK_COMMENT:
      case TK_SPACE: {
        break;
      }
      case TK_NULL: {
        만약( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          break;
        }
        /* no break */ deliberate_fall_through
      }
      case TK_STRING:
      case TK_INTEGER:
      case TK_FLOAT:
      case TK_VARIABLE:
      case TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        break;
      }
      case TK_LP: {
        nParen++;
        만약( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        break;
      }
      case TK_RP: {
        만약( iStartIN>0 && nParen==nParenAtIN ){
          assert( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        break;
      }
      case TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        만약( sqlite3Isquote(zSql[i]) ){
          char *zId = sqlite3DbStrNDup(db, zSql+i, n);
          정수 nId;
          정수 eType = 0;
          만약( zId==0 ) break;
          sqlite3Dequote(zId);
          만약( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            break;
          }
          nId = sqlite3Strlen30(zId);
          만약( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            addSpaceSeparator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }아니면{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }아니면{
          addSpaceSeparator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        동안( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        break;
      }
      case TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      default: {
        만약( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        동안( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        break;
      }
    }
  }
  만약( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  반환 sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
