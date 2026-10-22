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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdlib.h>

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

स्थिर अचर अचिन्हित वर्ण aiClass[] = {
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
# define charMap(X) sqlite3UpperToLower[(अचिन्हित वर्ण)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(अचिन्हित वर्ण)X]
अचर अचिन्हित वर्ण ebcdicToAscii[] = {
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
#अन्तर्भाव "keywordhash.h"


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
#define IdChar(C)  ((sqlite3CtypeMap[(अचिन्हित वर्ण)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
अचर वर्ण sqlite3IsEbcdicIdChar[] = {
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
पूर्णाङ्क sqlite3IsIdChar(u8 c){ फर्काऊ IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
स्थिर पूर्णाङ्क getToken(अचर अचिन्हित वर्ण **pz){
  अचर अचिन्हित वर्ण *z = *pz;
  पूर्णाङ्क t;                          /* Token type to return */
  गर {
    z += sqlite3GetToken(z, &t);
  }जबसम्म( t==TK_SPACE || t==TK_COMMENT );
  यदि( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  फर्काऊ t;
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
स्थिर पूर्णाङ्क analyzeWindowKeyword(अचर अचिन्हित वर्ण *z){
  पूर्णाङ्क t;
  t = getToken(&z);
  यदि( t!=TK_ID ) फर्काऊ TK_ID;
  t = getToken(&z);
  यदि( t!=TK_AS ) फर्काऊ TK_ID;
  फर्काऊ TK_WINDOW;
}
स्थिर पूर्णाङ्क analyzeOverKeyword(अचर अचिन्हित वर्ण *z, पूर्णाङ्क lastToken){
  यदि( lastToken==TK_RP ){
    पूर्णाङ्क t = getToken(&z);
    यदि( t==TK_LP || t==TK_ID ) फर्काऊ TK_OVER;
  }
  फर्काऊ TK_ID;
}
स्थिर पूर्णाङ्क analyzeFilterKeyword(अचर अचिन्हित वर्ण *z, पूर्णाङ्क lastToken){
  यदि( lastToken==TK_RP && getToken(&z)==TK_LP ){
    फर्काऊ TK_FILTER;
  }
  फर्काऊ TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(अचर अचिन्हित वर्ण *z, पूर्णाङ्क *tokenType){
  i64 i;
  पूर्णाङ्क c;
  विकल्प( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    पक्ष CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      लागि(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      फर्काऊ i;
    }
    पक्ष CC_MINUS: {
      यदि( z[1]=='-' ){
        लागि(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        फर्काऊ i;
      }अन्यथा यदि( z[1]=='>' ){
        *tokenType = TK_PTR;
        फर्काऊ 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      फर्काऊ 1;
    }
    पक्ष CC_LP: {
      *tokenType = TK_LP;
      फर्काऊ 1;
    }
    पक्ष CC_RP: {
      *tokenType = TK_RP;
      फर्काऊ 1;
    }
    पक्ष CC_SEMI: {
      *tokenType = TK_SEMI;
      फर्काऊ 1;
    }
    पक्ष CC_PLUS: {
      *tokenType = TK_PLUS;
      फर्काऊ 1;
    }
    पक्ष CC_STAR: {
      *tokenType = TK_STAR;
      फर्काऊ 1;
    }
    पक्ष CC_SLASH: {
      यदि( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        फर्काऊ 1;
      }
      लागि(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      यदि( c ) i++;
      *tokenType = TK_COMMENT;
      फर्काऊ i;
    }
    पक्ष CC_PERCENT: {
      *tokenType = TK_REM;
      फर्काऊ 1;
    }
    पक्ष CC_EQ: {
      *tokenType = TK_EQ;
      फर्काऊ 1 + (z[1]=='=');
    }
    पक्ष CC_LT: {
      यदि( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        फर्काऊ 2;
      }अन्यथा यदि( c=='>' ){
        *tokenType = TK_NE;
        फर्काऊ 2;
      }अन्यथा यदि( c=='<' ){
        *tokenType = TK_LSHIFT;
        फर्काऊ 2;
      }अन्यथा{
        *tokenType = TK_LT;
        फर्काऊ 1;
      }
    }
    पक्ष CC_GT: {
      यदि( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        फर्काऊ 2;
      }अन्यथा यदि( c=='>' ){
        *tokenType = TK_RSHIFT;
        फर्काऊ 2;
      }अन्यथा{
        *tokenType = TK_GT;
        फर्काऊ 1;
      }
    }
    पक्ष CC_BANG: {
      यदि( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        फर्काऊ 1;
      }अन्यथा{
        *tokenType = TK_NE;
        फर्काऊ 2;
      }
    }
    पक्ष CC_PIPE: {
      यदि( z[1]!='|' ){
        *tokenType = TK_BITOR;
        फर्काऊ 1;
      }अन्यथा{
        *tokenType = TK_CONCAT;
        फर्काऊ 2;
      }
    }
    पक्ष CC_COMMA: {
      *tokenType = TK_COMMA;
      फर्काऊ 1;
    }
    पक्ष CC_AND: {
      *tokenType = TK_BITAND;
      फर्काऊ 1;
    }
    पक्ष CC_TILDA: {
      *tokenType = TK_BITNOT;
      फर्काऊ 1;
    }
    पक्ष CC_QUOTE: {
      पूर्णाङ्क delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      लागि(i=1; (c=z[i])!=0; i++){
        यदि( c==delim ){
          यदि( z[i+1]==delim ){
            i++;
          }अन्यथा{
            रोक;
          }
        }
      }
      यदि( c=='\'' ){
        *tokenType = TK_STRING;
        फर्काऊ i+1;
      }अन्यथा यदि( c!=0 ){
        *tokenType = TK_ID;
        फर्काऊ i+1;
      }अन्यथा{
        *tokenType = TK_ILLEGAL;
        फर्काऊ i;
      }
    }
    पक्ष CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      यदि( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        फर्काऊ 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    पक्ष CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      यदि( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        लागि(i=3; 1; i++){
          यदि( sqlite3Isxdigit(z[i])==0 ){
            यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              रोक;
            }
          }
        }
      }अन्यथा
#endif
        {
        लागि(i=0; 1; i++){
          यदि( sqlite3Isdigit(z[i])==0 ){
            यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              रोक;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        यदि( z[i]=='.' ){
          यदि( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          लागि(i++; 1; i++){
            यदि( sqlite3Isdigit(z[i])==0 ){
              यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                रोक;
              }
            }
          }
        }
        यदि( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          यदि( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          लागि(i+=2; 1; i++){
            यदि( sqlite3Isdigit(z[i])==0 ){
              यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                रोक;
              }
            }
          }
        }
#endif
      }
      जबसम्म( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      फर्काऊ i;
    }
    पक्ष CC_QUOTE2: {
      लागि(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      फर्काऊ i;
    }
    पक्ष CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      लागि(i=1; sqlite3Isdigit(z[i]); i++){}
      फर्काऊ i;
    }
    पक्ष CC_DOLLAR:
    पक्ष CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      लागि(i=1; (c=z[i])!=0; i++){
        यदि( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }अन्यथा यदि( c=='(' && n>0 ){
          गर{
            i++;
          }जबसम्म( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          यदि( c==')' ){
            i++;
          }अन्यथा{
            *tokenType = TK_ILLEGAL;
          }
          रोक;
        }अन्यथा यदि( c==':' && z[i+1]==':' ){
          i++;
#endif
        }अन्यथा{
          रोक;
        }
      }
      यदि( n==0 ) *tokenType = TK_ILLEGAL;
      फर्काऊ i;
    }
    पक्ष CC_KYWD0: {
      यदि( aiClass[z[1]]>CC_KYWD ){ i = 1;  रोक; }
      लागि(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      यदि( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        रोक;
      }
      *tokenType = TK_ID;
      फर्काऊ keywordCode((वर्ण*)z, i, tokenType);
    }
    पक्ष CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      यदि( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        लागि(i=2; sqlite3Isxdigit(z[i]); i++){}
        यदि( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          जबसम्म( z[i] && z[i]!='\'' ){ i++; }
        }
        यदि( z[i] ) i++;
        फर्काऊ i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    पक्ष CC_KYWD:
    पक्ष CC_ID: {
      i = 1;
      रोक;
    }
    पक्ष CC_BOM: {
      यदि( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        फर्काऊ 3;
      }
      i = 1;
      रोक;
    }
    पक्ष CC_NUL: {
      *tokenType = TK_ILLEGAL;
      फर्काऊ 0;
    }
    स्वभाव: {
      *tokenType = TK_ILLEGAL;
      फर्काऊ 1;
    }
  }
  जबसम्म( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  फर्काऊ i;
}

/*
** Run the parser on the given SQL string.
*/
पूर्णाङ्क sqlite3RunParser(Parse *pParse, अचर वर्ण *zSql){
  पूर्णाङ्क nErr = 0;                   /* Number of errors encountered */
  शून्य *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  पूर्णाङ्क tokenType;                  /* type of the next token */
  पूर्णाङ्क lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  प्रतिज्ञा( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  यदि( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  यदि( db->flags & SQLITE_ParserTrace ){
    म_लिखो("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(मानक_निकास, "parser: ");
  }अन्यथा{
    sqlite3ParserTrace(0, 0);
  }
#endif
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#अन्यथा
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  यदि( pEngine==0 ){
    sqlite3OomFault(db);
    फर्काऊ SQLITE_NOMEM_BKPT;
  }
#endif
  प्रतिज्ञा( pParse->pNewTable==0 );
  प्रतिज्ञा( pParse->pNewTrigger==0 );
  प्रतिज्ञा( pParse->nVar==0 );
  प्रतिज्ञा( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  जबसम्म( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    यदि( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      रोक;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    यदि( tokenType>=TK_WINDOW ){
      प्रतिज्ञा( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#अन्यथा
    यदि( tokenType>=TK_SPACE ){
      प्रतिज्ञा( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      यदि( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        रोक;
      }
      यदि( tokenType==TK_SPACE ){
        zSql += n;
        जारी;
      }
      यदि( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        यदि( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }अन्यथा यदि( lastTokenParsed==0 ){
          रोक;
        }अन्यथा{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }अन्यथा यदि( tokenType==TK_WINDOW ){
        प्रतिज्ञा( n==6 );
        tokenType = analyzeWindowKeyword((अचर u8*)&zSql[6]);
      }अन्यथा यदि( tokenType==TK_OVER ){
        प्रतिज्ञा( n==4 );
        tokenType = analyzeOverKeyword((अचर u8*)&zSql[4], lastTokenParsed);
      }अन्यथा यदि( tokenType==TK_FILTER ){
        प्रतिज्ञा( n==6 );
        tokenType = analyzeFilterKeyword((अचर u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }अन्यथा यदि( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        जारी;
      }अन्यथा यदि( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_सन्देश(pParse, "unrecognized token: \"%T\"", &x);
        रोक;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    प्रतिज्ञा( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    यदि( pParse->rc!=SQLITE_OK ) रोक;
  }
  प्रतिज्ञा( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_थाक,
      sqlite3_Parser_थाक_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#अन्यथा
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  यदि( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  यदि( pParse->z_Err_सन्देश || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    यदि( pParse->z_Err_सन्देश==0 ){
      pParse->z_Err_सन्देश = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    यदि( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_सन्देश, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  यदि( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  यदि( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  यदि( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  प्रतिज्ञा( nErr==0 || pParse->rc!=SQLITE_OK );
  फर्काऊ nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
स्थिर शून्य addSpaceSeparator(sqlite3_str *pStr){
  यदि( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
वर्ण *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  अचर वर्ण *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  पूर्णाङ्क i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  पूर्णाङ्क tokenType;     /* type of current token */
  पूर्णाङ्क prevType = 0;  /* Previous non-whitespace token */
  पूर्णाङ्क nParen;        /* Number of nested levels of parentheses */
  पूर्णाङ्क iStartIN;      /* Start of RHS of IN operator in z[] */
  पूर्णाङ्क nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  प्रतिज्ञा( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  लागि(i=0; zSql[i] && pStr->accError==0; i+=n){
    यदि( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((अचिन्हित वर्ण*)zSql+i, &tokenType);
    यदि( NEVER(n<=0) ) रोक;
    विकल्प( tokenType ){
      पक्ष TK_COMMENT:
      पक्ष TK_SPACE: {
        रोक;
      }
      पक्ष TK_NULL: {
        यदि( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          रोक;
        }
        /* no break */ deliberate_fall_through
      }
      पक्ष TK_STRING:
      पक्ष TK_INTEGER:
      पक्ष TK_FLOAT:
      पक्ष TK_VARIABLE:
      पक्ष TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        रोक;
      }
      पक्ष TK_LP: {
        nParen++;
        यदि( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        रोक;
      }
      पक्ष TK_RP: {
        यदि( iStartIN>0 && nParen==nParenAtIN ){
          प्रतिज्ञा( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        रोक;
      }
      पक्ष TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        यदि( sqlite3Isquote(zSql[i]) ){
          वर्ण *zId = sqlite3DbStrNDup(db, zSql+i, n);
          पूर्णाङ्क nId;
          पूर्णाङ्क eType = 0;
          यदि( zId==0 ) रोक;
          sqlite3Dequote(zId);
          यदि( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            रोक;
          }
          nId = sqlite3Strlen30(zId);
          यदि( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            addSpaceSeparator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }अन्यथा{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }अन्यथा{
          addSpaceSeparator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        जबसम्म( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        रोक;
      }
      पक्ष TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      स्वभाव: {
        यदि( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        जबसम्म( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        रोक;
      }
    }
  }
  यदि( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  फर्काऊ sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
