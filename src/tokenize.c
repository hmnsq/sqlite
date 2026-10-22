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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>

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

ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ aiClass[] = {
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
# define charMap(X) sqlite3UpperToLower[(unsigned ವರ್ಣ)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned ವರ್ಣ)X]
ಧ್ರುವ unsigned ವರ್ಣ ebcdicToAscii[] = {
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
#ಅನ್ತರ್ಭಾವ "keywordhash.h"


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
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned ವರ್ಣ)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
ಧ್ರುವ ವರ್ಣ sqlite3IsEbcdicIdChar[] = {
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
ಪೂರ್ಣಾಙ್ಕ sqlite3IsIdChar(u8 c){ ಪ್ರತ್ಯಾಗಮ IdChar(c); }

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getToken(ಧ್ರುವ unsigned ವರ್ಣ **pz){
  ಧ್ರುವ unsigned ವರ್ಣ *z = *pz;
  ಪೂರ್ಣಾಙ್ಕ t;                          /* Token type to return */
  ಕುರು {
    z += sqlite3GetToken(z, &t);
  }ಯಾವತ್( t==TK_SPACE || t==TK_COMMENT );
  ಯದಿ( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  ಪ್ರತ್ಯಾಗಮ t;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ analyzeWindowKeyword(ಧ್ರುವ unsigned ವರ್ಣ *z){
  ಪೂರ್ಣಾಙ್ಕ t;
  t = getToken(&z);
  ಯದಿ( t!=TK_ID ) ಪ್ರತ್ಯಾಗಮ TK_ID;
  t = getToken(&z);
  ಯದಿ( t!=TK_AS ) ಪ್ರತ್ಯಾಗಮ TK_ID;
  ಪ್ರತ್ಯಾಗಮ TK_WINDOW;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ analyzeOverKeyword(ಧ್ರುವ unsigned ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ lastToken){
  ಯದಿ( lastToken==TK_RP ){
    ಪೂರ್ಣಾಙ್ಕ t = getToken(&z);
    ಯದಿ( t==TK_LP || t==TK_ID ) ಪ್ರತ್ಯಾಗಮ TK_OVER;
  }
  ಪ್ರತ್ಯಾಗಮ TK_ID;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ analyzeFilterKeyword(ಧ್ರುವ unsigned ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ lastToken){
  ಯದಿ( lastToken==TK_RP && getToken(&z)==TK_LP ){
    ಪ್ರತ್ಯಾಗಮ TK_FILTER;
  }
  ಪ್ರತ್ಯಾಗಮ TK_ID;
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ಧ್ರುವ unsigned ವರ್ಣ *z, ಪೂರ್ಣಾಙ್ಕ *tokenType){
  i64 i;
  ಪೂರ್ಣಾಙ್ಕ c;
  ವಿಕಲ್ಪ( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    ಪಕ್ಷ CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      ಪ್ರತಿ(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      ಪ್ರತ್ಯಾಗಮ i;
    }
    ಪಕ್ಷ CC_MINUS: {
      ಯದಿ( z[1]=='-' ){
        ಪ್ರತಿ(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        ಪ್ರತ್ಯಾಗಮ i;
      }ಅನ್ಯಥಾ ಯದಿ( z[1]=='>' ){
        *tokenType = TK_PTR;
        ಪ್ರತ್ಯಾಗಮ 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_LP: {
      *tokenType = TK_LP;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_RP: {
      *tokenType = TK_RP;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_SEMI: {
      *tokenType = TK_SEMI;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_PLUS: {
      *tokenType = TK_PLUS;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_STAR: {
      *tokenType = TK_STAR;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_SLASH: {
      ಯದಿ( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಪ್ರತಿ(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      ಯದಿ( c ) i++;
      *tokenType = TK_COMMENT;
      ಪ್ರತ್ಯಾಗಮ i;
    }
    ಪಕ್ಷ CC_PERCENT: {
      *tokenType = TK_REM;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_EQ: {
      *tokenType = TK_EQ;
      ಪ್ರತ್ಯಾಗಮ 1 + (z[1]=='=');
    }
    ಪಕ್ಷ CC_LT: {
      ಯದಿ( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        ಪ್ರತ್ಯಾಗಮ 2;
      }ಅನ್ಯಥಾ ಯದಿ( c=='>' ){
        *tokenType = TK_NE;
        ಪ್ರತ್ಯಾಗಮ 2;
      }ಅನ್ಯಥಾ ಯದಿ( c=='<' ){
        *tokenType = TK_LSHIFT;
        ಪ್ರತ್ಯಾಗಮ 2;
      }ಅನ್ಯಥಾ{
        *tokenType = TK_LT;
        ಪ್ರತ್ಯಾಗಮ 1;
      }
    }
    ಪಕ್ಷ CC_GT: {
      ಯದಿ( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        ಪ್ರತ್ಯಾಗಮ 2;
      }ಅನ್ಯಥಾ ಯದಿ( c=='>' ){
        *tokenType = TK_RSHIFT;
        ಪ್ರತ್ಯಾಗಮ 2;
      }ಅನ್ಯಥಾ{
        *tokenType = TK_GT;
        ಪ್ರತ್ಯಾಗಮ 1;
      }
    }
    ಪಕ್ಷ CC_BANG: {
      ಯದಿ( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        ಪ್ರತ್ಯಾಗಮ 1;
      }ಅನ್ಯಥಾ{
        *tokenType = TK_NE;
        ಪ್ರತ್ಯಾಗಮ 2;
      }
    }
    ಪಕ್ಷ CC_PIPE: {
      ಯದಿ( z[1]!='|' ){
        *tokenType = TK_BITOR;
        ಪ್ರತ್ಯಾಗಮ 1;
      }ಅನ್ಯಥಾ{
        *tokenType = TK_CONCAT;
        ಪ್ರತ್ಯಾಗಮ 2;
      }
    }
    ಪಕ್ಷ CC_COMMA: {
      *tokenType = TK_COMMA;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_AND: {
      *tokenType = TK_BITAND;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_TILDA: {
      *tokenType = TK_BITNOT;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ CC_QUOTE: {
      ಪೂರ್ಣಾಙ್ಕ delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      ಪ್ರತಿ(i=1; (c=z[i])!=0; i++){
        ಯದಿ( c==delim ){
          ಯದಿ( z[i+1]==delim ){
            i++;
          }ಅನ್ಯಥಾ{
            ಭಙ್ಗ;
          }
        }
      }
      ಯದಿ( c=='\'' ){
        *tokenType = TK_STRING;
        ಪ್ರತ್ಯಾಗಮ i+1;
      }ಅನ್ಯಥಾ ಯದಿ( c!=0 ){
        *tokenType = TK_ID;
        ಪ್ರತ್ಯಾಗಮ i+1;
      }ಅನ್ಯಥಾ{
        *tokenType = TK_ILLEGAL;
        ಪ್ರತ್ಯಾಗಮ i;
      }
    }
    ಪಕ್ಷ CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      ಯದಿ( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    ಪಕ್ಷ CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      ಯದಿ( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        ಪ್ರತಿ(i=3; 1; i++){
          ಯದಿ( sqlite3Isxdigit(z[i])==0 ){
            ಯದಿ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ಅನ್ಯಥಾ{
              ಭಙ್ಗ;
            }
          }
        }
      }ಅನ್ಯಥಾ
#endif
        {
        ಪ್ರತಿ(i=0; 1; i++){
          ಯದಿ( sqlite3Isdigit(z[i])==0 ){
            ಯದಿ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ಅನ್ಯಥಾ{
              ಭಙ್ಗ;
            }
          }
        }
#ifndef SQLITE_OMIT_FLOATING_POINT
        ಯದಿ( z[i]=='.' ){
          ಯದಿ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ಪ್ರತಿ(i++; 1; i++){
            ಯದಿ( sqlite3Isdigit(z[i])==0 ){
              ಯದಿ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ಅನ್ಯಥಾ{
                ಭಙ್ಗ;
              }
            }
          }
        }
        ಯದಿ( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          ಯದಿ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ಪ್ರತಿ(i+=2; 1; i++){
            ಯದಿ( sqlite3Isdigit(z[i])==0 ){
              ಯದಿ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ಅನ್ಯಥಾ{
                ಭಙ್ಗ;
              }
            }
          }
        }
#endif
      }
      ಯಾವತ್( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      ಪ್ರತ್ಯಾಗಮ i;
    }
    ಪಕ್ಷ CC_QUOTE2: {
      ಪ್ರತಿ(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      ಪ್ರತ್ಯಾಗಮ i;
    }
    ಪಕ್ಷ CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      ಪ್ರತಿ(i=1; sqlite3Isdigit(z[i]); i++){}
      ಪ್ರತ್ಯಾಗಮ i;
    }
    ಪಕ್ಷ CC_DOLLAR:
    ಪಕ್ಷ CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      ಪ್ರತಿ(i=1; (c=z[i])!=0; i++){
        ಯದಿ( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }ಅನ್ಯಥಾ ಯದಿ( c=='(' && n>0 ){
          ಕುರು{
            i++;
          }ಯಾವತ್( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          ಯದಿ( c==')' ){
            i++;
          }ಅನ್ಯಥಾ{
            *tokenType = TK_ILLEGAL;
          }
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ ಯದಿ( c==':' && z[i+1]==':' ){
          i++;
#endif
        }ಅನ್ಯಥಾ{
          ಭಙ್ಗ;
        }
      }
      ಯದಿ( n==0 ) *tokenType = TK_ILLEGAL;
      ಪ್ರತ್ಯಾಗಮ i;
    }
    ಪಕ್ಷ CC_KYWD0: {
      ಯದಿ( aiClass[z[1]]>CC_KYWD ){ i = 1;  ಭಙ್ಗ; }
      ಪ್ರತಿ(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      ಯದಿ( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        ಭಙ್ಗ;
      }
      *tokenType = TK_ID;
      ಪ್ರತ್ಯಾಗಮ keywordCode((ವರ್ಣ*)z, i, tokenType);
    }
    ಪಕ್ಷ CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      ಯದಿ( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        ಪ್ರತಿ(i=2; sqlite3Isxdigit(z[i]); i++){}
        ಯದಿ( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          ಯಾವತ್( z[i] && z[i]!='\'' ){ i++; }
        }
        ಯದಿ( z[i] ) i++;
        ಪ್ರತ್ಯಾಗಮ i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    ಪಕ್ಷ CC_KYWD:
    ಪಕ್ಷ CC_ID: {
      i = 1;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ CC_BOM: {
      ಯದಿ( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        ಪ್ರತ್ಯಾಗಮ 3;
      }
      i = 1;
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ CC_NUL: {
      *tokenType = TK_ILLEGAL;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಸ್ವಭಾವ: {
      *tokenType = TK_ILLEGAL;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಯಾವತ್( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  ಪ್ರತ್ಯಾಗಮ i;
}

/*
** Run the parser on the given SQL string.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3RunParser(Parse *pParse, ಧ್ರುವ ವರ್ಣ *zSql){
  ಪೂರ್ಣಾಙ್ಕ nErr = 0;                   /* Number of errors encountered */
  ಶೂನ್ಯ *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  ಪೂರ್ಣಾಙ್ಕ tokenType;                  /* type of the next token */
  ಪೂರ್ಣಾಙ್ಕ lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#endif
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  ಪ್ರತಿಜ್ಞಾ( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  ಯದಿ( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
#ifdef SQLITE_DEBUG
  ಯದಿ( db->flags & SQLITE_ParserTrace ){
    printf("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }ಅನ್ಯಥಾ{
    sqlite3ParserTrace(0, 0);
  }
#endif
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#ಅನ್ಯಥಾ
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  ಯದಿ( pEngine==0 ){
    sqlite3OomFault(db);
    ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
  }
#endif
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTable==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTrigger==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->nVar==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  ಯಾವತ್( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    ಯದಿ( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( tokenType>=TK_WINDOW ){
      ಪ್ರತಿಜ್ಞಾ( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ಅನ್ಯಥಾ
    ಯದಿ( tokenType>=TK_SPACE ){
      ಪ್ರತಿಜ್ಞಾ( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#endif /* SQLITE_OMIT_WINDOWFUNC */
      ಯದಿ( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        ಭಙ್ಗ;
      }
      ಯದಿ( tokenType==TK_SPACE ){
        zSql += n;
        ಅನುವರ್ತ;
      }
      ಯದಿ( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        ಯದಿ( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }ಅನ್ಯಥಾ ಯದಿ( lastTokenParsed==0 ){
          ಭಙ್ಗ;
        }ಅನ್ಯಥಾ{
          tokenType = TK_SEMI;
        }
        n = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
      }ಅನ್ಯಥಾ ಯದಿ( tokenType==TK_WINDOW ){
        ಪ್ರತಿಜ್ಞಾ( n==6 );
        tokenType = analyzeWindowKeyword((ಧ್ರುವ u8*)&zSql[6]);
      }ಅನ್ಯಥಾ ಯದಿ( tokenType==TK_OVER ){
        ಪ್ರತಿಜ್ಞಾ( n==4 );
        tokenType = analyzeOverKeyword((ಧ್ರುವ u8*)&zSql[4], lastTokenParsed);
      }ಅನ್ಯಥಾ ಯದಿ( tokenType==TK_FILTER ){
        ಪ್ರತಿಜ್ಞಾ( n==6 );
        tokenType = analyzeFilterKeyword((ಧ್ರುವ u8*)&zSql[6], lastTokenParsed);
#endif /* SQLITE_OMIT_WINDOWFUNC */
      }ಅನ್ಯಥಾ ಯದಿ( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        ಅನುವರ್ತ;
      }ಅನ್ಯಥಾ ಯದಿ( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_ಸಂದೇಶ(pParse, "unrecognized token: \"%T\"", &x);
        ಭಙ್ಗ;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    ಯದಿ( pParse->rc!=SQLITE_OK ) ಭಙ್ಗ;
  }
  ಪ್ರತಿಜ್ಞಾ( nErr==0 );
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_ರಾಶಿ,
      sqlite3_Parser_ರಾಶಿ_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
#ifdef sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#ಅನ್ಯಥಾ
  sqlite3ParserFree(pEngine, sqlite3_free);
#endif
  ಯದಿ( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  ಯದಿ( pParse->z_Err_ಸಂದೇಶ || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    ಯದಿ( pParse->z_Err_ಸಂದೇಶ==0 ){
      pParse->z_Err_ಸಂದೇಶ = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    ಯದಿ( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_ಸಂದೇಶ, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  ಯದಿ( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  ಯದಿ( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  ಯದಿ( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  ಪ್ರತಿಜ್ಞಾ( nErr==0 || pParse->rc!=SQLITE_OK );
  ಪ್ರತ್ಯಾಗಮ nErr;
}


#ifdef SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
ಸ್ಥಿರ ಶೂನ್ಯ addSpaceSeparator(sqlite3_str *pStr){
  ಯದಿ( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
ವರ್ಣ *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  ಧ್ರುವ ವರ್ಣ *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  ಪೂರ್ಣಾಙ್ಕ i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  ಪೂರ್ಣಾಙ್ಕ tokenType;     /* type of current token */
  ಪೂರ್ಣಾಙ್ಕ prevType = 0;  /* Previous non-whitespace token */
  ಪೂರ್ಣಾಙ್ಕ nParen;        /* Number of nested levels of parentheses */
  ಪೂರ್ಣಾಙ್ಕ iStartIN;      /* Start of RHS of IN operator in z[] */
  ಪೂರ್ಣಾಙ್ಕ nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  ಪ್ರತಿಜ್ಞಾ( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  ಪ್ರತಿ(i=0; zSql[i] && pStr->accError==0; i+=n){
    ಯದಿ( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((unsigned ವರ್ಣ*)zSql+i, &tokenType);
    ಯದಿ( NEVER(n<=0) ) ಭಙ್ಗ;
    ವಿಕಲ್ಪ( tokenType ){
      ಪಕ್ಷ TK_COMMENT:
      ಪಕ್ಷ TK_SPACE: {
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_NULL: {
        ಯದಿ( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          ಭಙ್ಗ;
        }
        /* no break */ deliberate_fall_through
      }
      ಪಕ್ಷ TK_STRING:
      ಪಕ್ಷ TK_INTEGER:
      ಪಕ್ಷ TK_FLOAT:
      ಪಕ್ಷ TK_VARIABLE:
      ಪಕ್ಷ TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_LP: {
        nParen++;
        ಯದಿ( prevType==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_RP: {
        ಯದಿ( iStartIN>0 && nParen==nParenAtIN ){
          ಪ್ರತಿಜ್ಞಾ( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        ಯದಿ( sqlite3Isquote(zSql[i]) ){
          ವರ್ಣ *zId = sqlite3DbStrNDup(db, zSql+i, n);
          ಪೂರ್ಣಾಙ್ಕ nId;
          ಪೂರ್ಣಾಙ್ಕ eType = 0;
          ಯದಿ( zId==0 ) ಭಙ್ಗ;
          sqlite3Dequote(zId);
          ಯದಿ( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            ಭಙ್ಗ;
          }
          nId = sqlite3Strlen30(zId);
          ಯದಿ( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            addSpaceSeparator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }ಅನ್ಯಥಾ{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }ಅನ್ಯಥಾ{
          addSpaceSeparator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        ಯಾವತ್( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      ಸ್ವಭಾವ: {
        ಯದಿ( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        ಯಾವತ್( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        ಭಙ್ಗ;
      }
    }
  }
  ಯದಿ( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  ಪ್ರತ್ಯಾಗಮ sqlite3_str_finish(pStr);
}
#endif /* SQLITE_ENABLE_NORMALIZE */
