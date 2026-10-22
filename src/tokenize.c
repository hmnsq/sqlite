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

स्थिर स्थिरांक अचिन्हित अक्षर aiClass[] = {
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
# define charMap(X) sqlite3UpperToLower[(अचिन्हित अक्षर)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(अचिन्हित अक्षर)X]
स्थिरांक अचिन्हित अक्षर ebcdicToAscii[] = {
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
#define IdChar(C)  ((sqlite3CtypeMap[(अचिन्हित अक्षर)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
स्थिरांक अक्षर sqlite3IsEbcdicIdChar[] = {
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
पूर्णांक sqlite3IsIdChar(u8 c){ परत IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
स्थिर पूर्णांक getToken(स्थिरांक अचिन्हित अक्षर **pz){
  स्थिरांक अचिन्हित अक्षर *z = *pz;
  पूर्णांक t;                          /* Token type to return */
  करा {
    z += sqlite3GetToken(z, &t);
  }पर्यंत( t==TK_SPACE || t==TK_COMMENT );
  जर( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  परत t;
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
स्थिर पूर्णांक analyzeWindowKeyword(स्थिरांक अचिन्हित अक्षर *z){
  पूर्णांक t;
  t = getToken(&z);
  जर( t!=TK_ID ) परत TK_ID;
  t = getToken(&z);
  जर( t!=TK_AS ) परत TK_ID;
  परत TK_WINDOW;
}
स्थिर पूर्णांक analyzeOverKeyword(स्थिरांक अचिन्हित अक्षर *z, पूर्णांक lastToken){
  जर( lastToken==TK_RP ){
    पूर्णांक t = getToken(&z);
    जर( t==TK_LP || t==TK_ID ) परत TK_OVER;
  }
  परत TK_ID;
}
स्थिर पूर्णांक analyzeFilterKeyword(स्थिरांक अचिन्हित अक्षर *z, पूर्णांक lastToken){
  जर( lastToken==TK_RP && getToken(&z)==TK_LP ){
    परत TK_FILTER;
  }
  परत TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(स्थिरांक अचिन्हित अक्षर *z, पूर्णांक *tokenType){
  i64 i;
  पूर्णांक c;
  निवड( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    स्थिती CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      साठी(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      परत i;
    }
    स्थिती CC_MINUS: {
      जर( z[1]=='-' ){
        साठी(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        परत i;
      }अन्यथा जर( z[1]=='>' ){
        *tokenType = TK_PTR;
        परत 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      परत 1;
    }
    स्थिती CC_LP: {
      *tokenType = TK_LP;
      परत 1;
    }
    स्थिती CC_RP: {
      *tokenType = TK_RP;
      परत 1;
    }
    स्थिती CC_SEMI: {
      *tokenType = TK_SEMI;
      परत 1;
    }
    स्थिती CC_PLUS: {
      *tokenType = TK_PLUS;
      परत 1;
    }
    स्थिती CC_STAR: {
      *tokenType = TK_STAR;
      परत 1;
    }
    स्थिती CC_SLASH: {
      जर( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        परत 1;
      }
      साठी(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      जर( c ) i++;
      *tokenType = TK_COMMENT;
      परत i;
    }
    स्थिती CC_PERCENT: {
      *tokenType = TK_REM;
      परत 1;
    }
    स्थिती CC_EQ: {
      *tokenType = TK_EQ;
      परत 1 + (z[1]=='=');
    }
    स्थिती CC_LT: {
      जर( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        परत 2;
      }अन्यथा जर( c=='>' ){
        *tokenType = TK_NE;
        परत 2;
      }अन्यथा जर( c=='<' ){
        *tokenType = TK_LSHIFT;
        परत 2;
      }अन्यथा{
        *tokenType = TK_LT;
        परत 1;
      }
    }
    स्थिती CC_GT: {
      जर( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        परत 2;
      }अन्यथा जर( c=='>' ){
        *tokenType = TK_RSHIFT;
        परत 2;
      }अन्यथा{
        *tokenType = TK_GT;
        परत 1;
      }
    }
    स्थिती CC_BANG: {
      जर( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        परत 1;
      }अन्यथा{
        *tokenType = TK_NE;
        परत 2;
      }
    }
    स्थिती CC_PIPE: {
      जर( z[1]!='|' ){
        *tokenType = TK_BITOR;
        परत 1;
      }अन्यथा{
        *tokenType = TK_CONCAT;
        परत 2;
      }
    }
    स्थिती CC_COMMA: {
      *tokenType = TK_COMMA;
      परत 1;
    }
    स्थिती CC_AND: {
      *tokenType = TK_BITAND;
      परत 1;
    }
    स्थिती CC_TILDA: {
      *tokenType = TK_BITNOT;
      परत 1;
    }
    स्थिती CC_QUOTE: {
      पूर्णांक delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      साठी(i=1; (c=z[i])!=0; i++){
        जर( c==delim ){
          जर( z[i+1]==delim ){
            i++;
          }अन्यथा{
            थांबा;
          }
        }
      }
      जर( c=='\'' ){
        *tokenType = TK_STRING;
        परत i+1;
      }अन्यथा जर( c!=0 ){
        *tokenType = TK_ID;
        परत i+1;
      }अन्यथा{
        *tokenType = TK_ILLEGAL;
        परत i;
      }
    }
    स्थिती CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      जर( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        परत 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    स्थिती CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      जर( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        साठी(i=3; 1; i++){
          जर( sqlite3Isxdigit(z[i])==0 ){
            जर( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              थांबा;
            }
          }
        }
      }अन्यथा
#endif
        {
        साठी(i=0; 1; i++){
          जर( sqlite3Isdigit(z[i])==0 ){
            जर( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              थांबा;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        जर( z[i]=='.' ){
          जर( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          साठी(i++; 1; i++){
            जर( sqlite3Isdigit(z[i])==0 ){
              जर( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                थांबा;
              }
            }
          }
        }
        जर( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          जर( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          साठी(i+=2; 1; i++){
            जर( sqlite3Isdigit(z[i])==0 ){
              जर( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                थांबा;
              }
            }
          }
        }
#endif
      }
      पर्यंत( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      परत i;
    }
    स्थिती CC_QUOTE2: {
      साठी(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      परत i;
    }
    स्थिती CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      साठी(i=1; sqlite3Isdigit(z[i]); i++){}
      परत i;
    }
    स्थिती CC_DOLLAR:
    स्थिती CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      साठी(i=1; (c=z[i])!=0; i++){
        जर( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }अन्यथा जर( c=='(' && n>0 ){
          करा{
            i++;
          }पर्यंत( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          जर( c==')' ){
            i++;
          }अन्यथा{
            *tokenType = TK_ILLEGAL;
          }
          थांबा;
        }अन्यथा जर( c==':' && z[i+1]==':' ){
          i++;
#endif
        }अन्यथा{
          थांबा;
        }
      }
      जर( n==0 ) *tokenType = TK_ILLEGAL;
      परत i;
    }
    स्थिती CC_KYWD0: {
      जर( aiClass[z[1]]>CC_KYWD ){ i = 1;  थांबा; }
      साठी(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      जर( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        थांबा;
      }
      *tokenType = TK_ID;
      परत keywordCode((अक्षर*)z, i, tokenType);
    }
    स्थिती CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      जर( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        साठी(i=2; sqlite3Isxdigit(z[i]); i++){}
        जर( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          पर्यंत( z[i] && z[i]!='\'' ){ i++; }
        }
        जर( z[i] ) i++;
        परत i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    स्थिती CC_KYWD:
    स्थिती CC_ID: {
      i = 1;
      थांबा;
    }
    स्थिती CC_BOM: {
      जर( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        परत 3;
      }
      i = 1;
      थांबा;
    }
    स्थिती CC_NUL: {
      *tokenType = TK_ILLEGAL;
      परत 0;
    }
    पूर्वनिर्धारित: {
      *tokenType = TK_ILLEGAL;
      परत 1;
    }
  }
  पर्यंत( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  परत i;
}

/*
** Run the parser on the given SQL string.
*/
पूर्णांक sqlite3RunParser(Parse *pParse, स्थिरांक अक्षर *zSql){
  पूर्णांक nErr = 0;                   /* Number of errors encountered */
  शून्य *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  पूर्णांक tokenType;                  /* type of the next token */
  पूर्णांक lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  प्रतिज्ञा( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  जर( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  जर( db->flags & SQLITE_ParserTrace ){
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
  जर( pEngine==0 ){
    sqlite3OomFault(db);
    परत SQLITE_NOMEM_BKPT;
  }
#endif
  प्रतिज्ञा( pParse->pNewTable==0 );
  प्रतिज्ञा( pParse->pNewTrigger==0 );
  प्रतिज्ञा( pParse->nVar==0 );
  प्रतिज्ञा( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  पर्यंत( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    जर( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      थांबा;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    जर( tokenType>=TK_WINDOW ){
      प्रतिज्ञा( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#अन्यथा
    जर( tokenType>=TK_SPACE ){
      प्रतिज्ञा( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      जर( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        थांबा;
      }
      जर( tokenType==TK_SPACE ){
        zSql += n;
        सुरूठेवा;
      }
      जर( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        जर( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }अन्यथा जर( lastTokenParsed==0 ){
          थांबा;
        }अन्यथा{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }अन्यथा जर( tokenType==TK_WINDOW ){
        प्रतिज्ञा( n==6 );
        tokenType = analyzeWindowKeyword((स्थिरांक u8*)&zSql[6]);
      }अन्यथा जर( tokenType==TK_OVER ){
        प्रतिज्ञा( n==4 );
        tokenType = analyzeOverKeyword((स्थिरांक u8*)&zSql[4], lastTokenParsed);
      }अन्यथा जर( tokenType==TK_FILTER ){
        प्रतिज्ञा( n==6 );
        tokenType = analyzeFilterKeyword((स्थिरांक u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }अन्यथा जर( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        सुरूठेवा;
      }अन्यथा जर( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_संदेश(pParse, "unrecognized token: \"%T\"", &x);
        थांबा;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    प्रतिज्ञा( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    जर( pParse->rc!=SQLITE_OK ) थांबा;
  }
  प्रतिज्ञा( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_चळत,
      sqlite3_Parser_चळत_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#अन्यथा
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  जर( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  जर( pParse->z_Err_संदेश || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    जर( pParse->z_Err_संदेश==0 ){
      pParse->z_Err_संदेश = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    जर( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_संदेश, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  जर( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  जर( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  जर( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  प्रतिज्ञा( nErr==0 || pParse->rc!=SQLITE_OK );
  परत nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
स्थिर शून्य addSpaceSeparator(sqlite3_str *pStr){
  जर( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
अक्षर *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  स्थिरांक अक्षर *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  पूर्णांक i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  पूर्णांक tokenType;     /* type of current token */
  पूर्णांक prevType = 0;  /* Previous non-whitespace token */
  पूर्णांक nParen;        /* Number of nested levels of parentheses */
  पूर्णांक iStartIN;      /* Start of RHS of IN operator in z[] */
  पूर्णांक nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  प्रतिज्ञा( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  साठी(i=0; zSql[i] && pStr->accError==0; i+=n){
    जर( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((अचिन्हित अक्षर*)zSql+i, &tokenType);
    जर( NEVER(n<=0) ) थांबा;
    निवड( tokenType ){
      स्थिती TK_COMMENT:
      स्थिती TK_SPACE: {
        थांबा;
      }
      स्थिती TK_NULL: {
        जर( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          थांबा;
        }
        /* no break */ deliberate_fall_through
      }
      स्थिती TK_STRING:
      स्थिती TK_INTEGER:
      स्थिती TK_FLOAT:
      स्थिती TK_VARIABLE:
      स्थिती TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        थांबा;
      }
      स्थिती TK_LP: {
        nParen++;
        जर( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        थांबा;
      }
      स्थिती TK_RP: {
        जर( iStartIN>0 && nParen==nParenAtIN ){
          प्रतिज्ञा( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        थांबा;
      }
      स्थिती TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        जर( sqlite3Isquote(zSql[i]) ){
          अक्षर *zId = sqlite3DbStrNDup(db, zSql+i, n);
          पूर्णांक nId;
          पूर्णांक eType = 0;
          जर( zId==0 ) थांबा;
          sqlite3Dequote(zId);
          जर( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            थांबा;
          }
          nId = sqlite3Strlen30(zId);
          जर( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
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
        पर्यंत( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        थांबा;
      }
      स्थिती TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      पूर्वनिर्धारित: {
        जर( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        पर्यंत( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        थांबा;
      }
    }
  }
  जर( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  परत sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
