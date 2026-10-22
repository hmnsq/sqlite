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
#ضمن "sqliteInt.h"
#ضمن <stdlib.h>

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

ساكن ثابت unsigned حرف aiClass[] = {
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
# define charMap(X) sqlite3UpperToLower[(unsigned حرف)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned حرف)X]
ثابت unsigned حرف ebcdicToAscii[] = {
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
#ضمن "keywordhash.h"


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
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned حرف)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
ثابت حرف sqlite3IsEbcdicIdChar[] = {
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
صحيح sqlite3IsIdChar(u8 c){ החזר IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
ساكن صحيح getToken(ثابت unsigned حرف **pz){
  ثابت unsigned حرف *z = *pz;
  صحيح t;                          /* Token type to return */
  עשה {
    z += sqlite3GetToken(z, &t);
  }כלעוד( t==TK_SPACE || t==TK_COMMENT );
  אם( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  החזר t;
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
ساكن صحيح analyzeWindowKeyword(ثابت unsigned حرف *z){
  صحيح t;
  t = getToken(&z);
  אם( t!=TK_ID ) החזר TK_ID;
  t = getToken(&z);
  אם( t!=TK_AS ) החזר TK_ID;
  החזר TK_WINDOW;
}
ساكن صحيح analyzeOverKeyword(ثابت unsigned حرف *z, صحيح lastToken){
  אם( lastToken==TK_RP ){
    صحيح t = getToken(&z);
    אם( t==TK_LP || t==TK_ID ) החזר TK_OVER;
  }
  החזר TK_ID;
}
ساكن صحيح analyzeFilterKeyword(ثابت unsigned حرف *z, صحيح lastToken){
  אם( lastToken==TK_RP && getToken(&z)==TK_LP ){
    החזר TK_FILTER;
  }
  החזר TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ثابت unsigned حرف *z, صحيح *tokenType){
  i64 i;
  صحيح c;
  بدل( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    حالة CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      עבור(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      החזר i;
    }
    حالة CC_MINUS: {
      אם( z[1]=='-' ){
        עבור(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        החזר i;
      }אחרת אם( z[1]=='>' ){
        *tokenType = TK_PTR;
        החזר 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      החזר 1;
    }
    حالة CC_LP: {
      *tokenType = TK_LP;
      החזר 1;
    }
    حالة CC_RP: {
      *tokenType = TK_RP;
      החזר 1;
    }
    حالة CC_SEMI: {
      *tokenType = TK_SEMI;
      החזר 1;
    }
    حالة CC_PLUS: {
      *tokenType = TK_PLUS;
      החזר 1;
    }
    حالة CC_STAR: {
      *tokenType = TK_STAR;
      החזר 1;
    }
    حالة CC_SLASH: {
      אם( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        החזר 1;
      }
      עבור(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      אם( c ) i++;
      *tokenType = TK_COMMENT;
      החזר i;
    }
    حالة CC_PERCENT: {
      *tokenType = TK_REM;
      החזר 1;
    }
    حالة CC_EQ: {
      *tokenType = TK_EQ;
      החזר 1 + (z[1]=='=');
    }
    حالة CC_LT: {
      אם( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        החזר 2;
      }אחרת אם( c=='>' ){
        *tokenType = TK_NE;
        החזר 2;
      }אחרת אם( c=='<' ){
        *tokenType = TK_LSHIFT;
        החזר 2;
      }אחרת{
        *tokenType = TK_LT;
        החזר 1;
      }
    }
    حالة CC_GT: {
      אם( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        החזר 2;
      }אחרת אם( c=='>' ){
        *tokenType = TK_RSHIFT;
        החזר 2;
      }אחרת{
        *tokenType = TK_GT;
        החזר 1;
      }
    }
    حالة CC_BANG: {
      אם( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        החזר 1;
      }אחרת{
        *tokenType = TK_NE;
        החזר 2;
      }
    }
    حالة CC_PIPE: {
      אם( z[1]!='|' ){
        *tokenType = TK_BITOR;
        החזר 1;
      }אחרת{
        *tokenType = TK_CONCAT;
        החזר 2;
      }
    }
    حالة CC_COMMA: {
      *tokenType = TK_COMMA;
      החזר 1;
    }
    حالة CC_AND: {
      *tokenType = TK_BITAND;
      החזר 1;
    }
    حالة CC_TILDA: {
      *tokenType = TK_BITNOT;
      החזר 1;
    }
    حالة CC_QUOTE: {
      صحيح delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      עבור(i=1; (c=z[i])!=0; i++){
        אם( c==delim ){
          אם( z[i+1]==delim ){
            i++;
          }אחרת{
            اكسر;
          }
        }
      }
      אם( c=='\'' ){
        *tokenType = TK_STRING;
        החזר i+1;
      }אחרת אם( c!=0 ){
        *tokenType = TK_ID;
        החזר i+1;
      }אחרת{
        *tokenType = TK_ILLEGAL;
        החזר i;
      }
    }
    حالة CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      אם( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        החזר 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    حالة CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      אם( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        עבור(i=3; 1; i++){
          אם( sqlite3Isxdigit(z[i])==0 ){
            אם( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }אחרת{
              اكسر;
            }
          }
        }
      }אחרת
#endif
        {
        עבור(i=0; 1; i++){
          אם( sqlite3Isdigit(z[i])==0 ){
            אם( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }אחרת{
              اكسر;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        אם( z[i]=='.' ){
          אם( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          עבור(i++; 1; i++){
            אם( sqlite3Isdigit(z[i])==0 ){
              אם( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }אחרת{
                اكسر;
              }
            }
          }
        }
        אם( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          אם( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          עבור(i+=2; 1; i++){
            אם( sqlite3Isdigit(z[i])==0 ){
              אם( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }אחרת{
                اكسر;
              }
            }
          }
        }
#endif
      }
      כלעוד( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      החזר i;
    }
    حالة CC_QUOTE2: {
      עבור(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      החזר i;
    }
    حالة CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      עבור(i=1; sqlite3Isdigit(z[i]); i++){}
      החזר i;
    }
    حالة CC_DOLLAR:
    حالة CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      עבור(i=1; (c=z[i])!=0; i++){
        אם( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }אחרת אם( c=='(' && n>0 ){
          עשה{
            i++;
          }כלעוד( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          אם( c==')' ){
            i++;
          }אחרת{
            *tokenType = TK_ILLEGAL;
          }
          اكسر;
        }אחרת אם( c==':' && z[i+1]==':' ){
          i++;
#endif
        }אחרת{
          اكسر;
        }
      }
      אם( n==0 ) *tokenType = TK_ILLEGAL;
      החזר i;
    }
    حالة CC_KYWD0: {
      אם( aiClass[z[1]]>CC_KYWD ){ i = 1;  اكسر; }
      עבור(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      אם( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        اكسر;
      }
      *tokenType = TK_ID;
      החזר keywordCode((حرف*)z, i, tokenType);
    }
    حالة CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      אם( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        עבור(i=2; sqlite3Isxdigit(z[i]); i++){}
        אם( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          כלעוד( z[i] && z[i]!='\'' ){ i++; }
        }
        אם( z[i] ) i++;
        החזר i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    حالة CC_KYWD:
    حالة CC_ID: {
      i = 1;
      اكسر;
    }
    حالة CC_BOM: {
      אם( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        החזר 3;
      }
      i = 1;
      اكسر;
    }
    حالة CC_NUL: {
      *tokenType = TK_ILLEGAL;
      החזר 0;
    }
    افتراضي: {
      *tokenType = TK_ILLEGAL;
      החזר 1;
    }
  }
  כלעוד( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  החזר i;
}

/*
** Run the parser on the given SQL string.
*/
صحيح sqlite3RunParser(Parse *pParse, ثابت حرف *zSql){
  صحيح nErr = 0;                   /* Number of errors encountered */
  فراغ *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  صحيح tokenType;                  /* type of the next token */
  صحيح lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  أكد( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  אם( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  אם( db->flags & SQLITE_ParserTrace ){
    printf("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }אחרת{
    sqlite3ParserTrace(0, 0);
  }
#endif
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#אחרת
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  אם( pEngine==0 ){
    sqlite3OomFault(db);
    החזר SQLITE_NOMEM_BKPT;
  }
#endif
  أكد( pParse->pNewTable==0 );
  أكد( pParse->pNewTrigger==0 );
  أكد( pParse->nVar==0 );
  أكد( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  כלעוד( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    אם( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      اكسر;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    אם( tokenType>=TK_WINDOW ){
      أكد( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#אחרת
    אם( tokenType>=TK_SPACE ){
      أكد( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      אם( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        اكسر;
      }
      אם( tokenType==TK_SPACE ){
        zSql += n;
        استمر;
      }
      אם( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        אם( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }אחרת אם( lastTokenParsed==0 ){
          اكسر;
        }אחרת{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }אחרת אם( tokenType==TK_WINDOW ){
        أكد( n==6 );
        tokenType = analyzeWindowKeyword((ثابت u8*)&zSql[6]);
      }אחרת אם( tokenType==TK_OVER ){
        أكد( n==4 );
        tokenType = analyzeOverKeyword((ثابت u8*)&zSql[4], lastTokenParsed);
      }אחרת אם( tokenType==TK_FILTER ){
        أكد( n==6 );
        tokenType = analyzeFilterKeyword((ثابت u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }אחרת אם( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        استمر;
      }אחרת אם( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3ErrorMsg(pParse, "unrecognized token: \"%T\"", &x);
        اكسر;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    أكد( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    אם( pParse->rc!=SQLITE_OK ) اكسر;
  }
  أكد( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_STACK,
      sqlite3ParserStackPeak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#אחרת
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  אם( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  אם( pParse->zErrMsg || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    אם( pParse->zErrMsg==0 ){
      pParse->zErrMsg = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    אם( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->zErrMsg, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  אם( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  אם( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  אם( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  أكد( nErr==0 || pParse->rc!=SQLITE_OK );
  החזר nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
ساكن فراغ addSpaceSeparator(sqlite3_str *pStr){
  אם( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
حرف *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  ثابت حرف *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  صحيح i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  صحيح tokenType;     /* type of current token */
  صحيح prevType = 0;  /* Previous non-whitespace token */
  صحيح nParen;        /* Number of nested levels of parentheses */
  صحيح iStartIN;      /* Start of RHS of IN operator in z[] */
  صحيح nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  أكد( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  עבור(i=0; zSql[i] && pStr->accError==0; i+=n){
    אם( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((unsigned حرف*)zSql+i, &tokenType);
    אם( NEVER(n<=0) ) اكسر;
    بدل( tokenType ){
      حالة TK_COMMENT:
      حالة TK_SPACE: {
        اكسر;
      }
      حالة TK_NULL: {
        אם( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          اكسر;
        }
        /* no break */ deliberate_fall_through
      }
      حالة TK_STRING:
      حالة TK_INTEGER:
      حالة TK_FLOAT:
      حالة TK_VARIABLE:
      حالة TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        اكسر;
      }
      حالة TK_LP: {
        nParen++;
        אם( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        اكسر;
      }
      حالة TK_RP: {
        אם( iStartIN>0 && nParen==nParenAtIN ){
          أكد( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        اكسر;
      }
      حالة TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        אם( sqlite3Isquote(zSql[i]) ){
          حرف *zId = sqlite3DbStrNDup(db, zSql+i, n);
          صحيح nId;
          صحيح eType = 0;
          אם( zId==0 ) اكسر;
          sqlite3Dequote(zId);
          אם( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            اكسر;
          }
          nId = sqlite3Strlen30(zId);
          אם( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            addSpaceSeparator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }אחרת{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }אחרת{
          addSpaceSeparator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        כלעוד( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        اكسر;
      }
      حالة TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      افتراضي: {
        אם( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        כלעוד( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        اكسر;
      }
    }
  }
  אם( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  החזר sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
