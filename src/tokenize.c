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
#ವ್ಯಾಖ್ಯಾನಿಸು CC_X          0    /* The letter 'x', or start of BLOB literal */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_KYWD0      1    /* First letter of a keyword */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_DIGIT      3    /* Digits */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_DOLLAR     4    /* '$' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_SPACE      7    /* Space characters */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_LT        12    /* '<'.  Part of < or <= or <> */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_GT        13    /* '>'.  Part of > or >= */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_EQ        14    /* '='.  Part of = or == */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_BANG      15    /* '!'.  Part of != */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_SLASH     16    /* '/'.  / or c-style comment */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_LP        17    /* '(' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_RP        18    /* ')' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_SEMI      19    /* ';' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_PLUS      20    /* '+' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_ನಕ್ಷತ್ರ      21    /* '*' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_PERCENT   22    /* '%' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_COMMA     23    /* ',' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_AND       24    /* '&' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_TILDA     25    /* '~' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_DOT       26    /* '.' */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_ID        27    /* unicode characters usable in IDs */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_ILLEGAL   28    /* Illegal character */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_NUL       29    /* 0x00 */
#ವ್ಯಾಖ್ಯಾನಿಸು CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ aiClass[] = {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
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
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
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
#ಆದರೆ_ಅಂತ್ಯ
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
# define charMap(X) sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)X]
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
# define charMap(X) ebcdic_ಗಮ್ಯ_Ascii[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)X]
ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ebcdic_ಗಮ್ಯ_Ascii[] = {
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
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
#ವ್ಯಾಖ್ಯಾನಿಸು IdChar(C)  ((sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)C]&0x46)!=0)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EBCDIC
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ sqlite3IsEbcdicIdChar[] = {
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
#ವ್ಯಾಖ್ಯಾನಿಸು IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#ಆದರೆ_ಅಂತ್ಯ

/* Make the IdChar function accessible from ctime.c and alter.c */
ಪೂರ್ಣಾಂಕ sqlite3IsIdChar(u8 c){ ಹಿಂತಿರುಗಿ IdChar(c); }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getToken(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **pz){
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z = *pz;
  ಪೂರ್ಣಾಂಕ t;                          /* Token type to return */
  ಮಾಡು {
    z += sqlite3GetToken(z, &t);
  }ಆಗ( t==TK_SPACE || t==TK_COMMENT );
  ಆದರೆ( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_ಕಿಟಕಿ 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  ಹಿಂತಿರುಗಿ t;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ analyze_ಕಿಟಕಿ_Keyword(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z){
  ಪೂರ್ಣಾಂಕ t;
  t = getToken(&z);
  ಆದರೆ( t!=TK_ID ) ಹಿಂತಿರುಗಿ TK_ID;
  t = getToken(&z);
  ಆದರೆ( t!=TK_AS ) ಹಿಂತಿರುಗಿ TK_ID;
  ಹಿಂತಿರುಗಿ TK_ಕಿಟಕಿ;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ analyzeOverKeyword(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_Token){
  ಆದರೆ( ಕೊನೆಯ_Token==TK_RP ){
    ಪೂರ್ಣಾಂಕ t = getToken(&z);
    ಆದರೆ( t==TK_LP || t==TK_ID ) ಹಿಂತಿರುಗಿ TK_OVER;
  }
  ಹಿಂತಿರುಗಿ TK_ID;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ analyzeFilterKeyword(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_Token){
  ಆದರೆ( ಕೊನೆಯ_Token==TK_RP && getToken(&z)==TK_LP ){
    ಹಿಂತಿರುಗಿ TK_FILTER;
  }
  ಹಿಂತಿರುಗಿ TK_ID;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ *tokenType){
  i64 i;
  ಪೂರ್ಣಾಂಕ c;
  ಆಯ್ಕೆ( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    ಸ್ಥಿತಿ CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      ಗಾಗಿ(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      ಹಿಂತಿರುಗಿ i;
    }
    ಸ್ಥಿತಿ CC_MINUS: {
      ಆದರೆ( z[1]=='-' ){
        ಗಾಗಿ(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        ಹಿಂತಿರುಗಿ i;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( z[1]=='>' ){
        *tokenType = TK_PTR;
        ಹಿಂತಿರುಗಿ 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_LP: {
      *tokenType = TK_LP;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_RP: {
      *tokenType = TK_RP;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_SEMI: {
      *tokenType = TK_SEMI;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_PLUS: {
      *tokenType = TK_PLUS;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_ನಕ್ಷತ್ರ: {
      *tokenType = TK_ನಕ್ಷತ್ರ;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_SLASH: {
      ಆದರೆ( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        ಹಿಂತಿರುಗಿ 1;
      }
      ಗಾಗಿ(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      ಆದರೆ( c ) i++;
      *tokenType = TK_COMMENT;
      ಹಿಂತಿರುಗಿ i;
    }
    ಸ್ಥಿತಿ CC_PERCENT: {
      *tokenType = TK_REM;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_EQ: {
      *tokenType = TK_EQ;
      ಹಿಂತಿರುಗಿ 1 + (z[1]=='=');
    }
    ಸ್ಥಿತಿ CC_LT: {
      ಆದರೆ( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        ಹಿಂತಿರುಗಿ 2;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='>' ){
        *tokenType = TK_NE;
        ಹಿಂತಿರುಗಿ 2;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='<' ){
        *tokenType = TK_LSHIFT;
        ಹಿಂತಿರುಗಿ 2;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *tokenType = TK_LT;
        ಹಿಂತಿರುಗಿ 1;
      }
    }
    ಸ್ಥಿತಿ CC_GT: {
      ಆದರೆ( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        ಹಿಂತಿರುಗಿ 2;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='>' ){
        *tokenType = TK_RSHIFT;
        ಹಿಂತಿರುಗಿ 2;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *tokenType = TK_GT;
        ಹಿಂತಿರುಗಿ 1;
      }
    }
    ಸ್ಥಿತಿ CC_BANG: {
      ಆದರೆ( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        ಹಿಂತಿರುಗಿ 1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *tokenType = TK_NE;
        ಹಿಂತಿರುಗಿ 2;
      }
    }
    ಸ್ಥಿತಿ CC_PIPE: {
      ಆದರೆ( z[1]!='|' ){
        *tokenType = TK_BITOR;
        ಹಿಂತಿರುಗಿ 1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *tokenType = TK_CONCAT;
        ಹಿಂತಿರುಗಿ 2;
      }
    }
    ಸ್ಥಿತಿ CC_COMMA: {
      *tokenType = TK_COMMA;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_AND: {
      *tokenType = TK_BITAND;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_TILDA: {
      *tokenType = TK_BITNOT;
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ CC_QUOTE: {
      ಪೂರ್ಣಾಂಕ delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      ಗಾಗಿ(i=1; (c=z[i])!=0; i++){
        ಆದರೆ( c==delim ){
          ಆದರೆ( z[i+1]==delim ){
            i++;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ನಿಲ್ಲಿಸು;
          }
        }
      }
      ಆದರೆ( c=='\'' ){
        *tokenType = TK_STRING;
        ಹಿಂತಿರುಗಿ i+1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c!=0 ){
        *tokenType = TK_ID;
        ಹಿಂತಿರುಗಿ i+1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        *tokenType = TK_ILLEGAL;
        ಹಿಂತಿರುಗಿ i;
      }
    }
    ಸ್ಥಿತಿ CC_DOT: {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
      ಆದರೆ( !sqlite3Isdigit(z[1]) )
#ಆದರೆ_ಅಂತ್ಯ
      {
        *tokenType = TK_DOT;
        ಹಿಂತಿರುಗಿ 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    ಸ್ಥಿತಿ CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_HEX_INTEGER
      ಆದರೆ( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        ಗಾಗಿ(i=3; 1; i++){
          ಆದರೆ( sqlite3Isxdigit(z[i])==0 ){
            ಆದರೆ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ಇಲ್ಲದಿದ್ದರೆ{
              ನಿಲ್ಲಿಸು;
            }
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
        {
        ಗಾಗಿ(i=0; 1; i++){
          ಆದರೆ( sqlite3Isdigit(z[i])==0 ){
            ಆದರೆ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ಇಲ್ಲದಿದ್ದರೆ{
              ನಿಲ್ಲಿಸು;
            }
          }
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
        ಆದರೆ( z[i]=='.' ){
          ಆದರೆ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ಗಾಗಿ(i++; 1; i++){
            ಆದರೆ( sqlite3Isdigit(z[i])==0 ){
              ಆದರೆ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ಇಲ್ಲದಿದ್ದರೆ{
                ನಿಲ್ಲಿಸು;
              }
            }
          }
        }
        ಆದರೆ( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          ಆದರೆ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ಗಾಗಿ(i+=2; 1; i++){
            ಆದರೆ( sqlite3Isdigit(z[i])==0 ){
              ಆದರೆ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ಇಲ್ಲದಿದ್ದರೆ{
                ನಿಲ್ಲಿಸು;
              }
            }
          }
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
      ಆಗ( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      ಹಿಂತಿರುಗಿ i;
    }
    ಸ್ಥಿತಿ CC_QUOTE2: {
      ಗಾಗಿ(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      ಹಿಂತಿರುಗಿ i;
    }
    ಸ್ಥಿತಿ CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      ಗಾಗಿ(i=1; sqlite3Isdigit(z[i]); i++){}
      ಹಿಂತಿರುಗಿ i;
    }
    ಸ್ಥಿತಿ CC_DOLLAR:
    ಸ್ಥಿತಿ CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      ಗಾಗಿ(i=1; (c=z[i])!=0; i++){
        ಆದರೆ( IdChar(c) ){
          n++;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TCL_VARIABLE
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c=='(' && n>0 ){
          ಮಾಡು{
            i++;
          }ಆಗ( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          ಆದರೆ( c==')' ){
            i++;
          }ಇಲ್ಲದಿದ್ದರೆ{
            *tokenType = TK_ILLEGAL;
          }
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c==':' && z[i+1]==':' ){
          i++;
#ಆದರೆ_ಅಂತ್ಯ
        }ಇಲ್ಲದಿದ್ದರೆ{
          ನಿಲ್ಲಿಸು;
        }
      }
      ಆದರೆ( n==0 ) *tokenType = TK_ILLEGAL;
      ಹಿಂತಿರುಗಿ i;
    }
    ಸ್ಥಿತಿ CC_KYWD0: {
      ಆದರೆ( aiClass[z[1]]>CC_KYWD ){ i = 1;  ನಿಲ್ಲಿಸು; }
      ಗಾಗಿ(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      ಆದರೆ( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        ನಿಲ್ಲಿಸು;
      }
      *tokenType = TK_ID;
      ಹಿಂತಿರುಗಿ keyword_ಸಂಕೇತ((ಅಕ್ಷರ*)z, i, tokenType);
    }
    ಸ್ಥಿತಿ CC_X: {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      ಆದರೆ( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        ಗಾಗಿ(i=2; sqlite3Isxdigit(z[i]); i++){}
        ಆದರೆ( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          ಆಗ( z[i] && z[i]!='\'' ){ i++; }
        }
        ಆದರೆ( z[i] ) i++;
        ಹಿಂತಿರುಗಿ i;
      }
#ಆದರೆ_ಅಂತ್ಯ
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    ಸ್ಥಿತಿ CC_KYWD:
    ಸ್ಥಿತಿ CC_ID: {
      i = 1;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ CC_BOM: {
      ಆದರೆ( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        ಹಿಂತಿರುಗಿ 3;
      }
      i = 1;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ CC_NUL: {
      *tokenType = TK_ILLEGAL;
      ಹಿಂತಿರುಗಿ 0;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      *tokenType = TK_ILLEGAL;
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಆಗ( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  ಹಿಂತಿರುಗಿ i;
}

/*
** Run the parser on the given SQL string.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸತತ_Parser(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql){
  ಪೂರ್ಣಾಂಕ nErr = 0;                   /* Number of errors encountered */
  ಶೂನ್ಯ *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  ಪೂರ್ಣಾಂಕ tokenType;                  /* type of the next token */
  ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_Token_Parsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *p_ಪೋಷಕ_Parse = 0;        /* Outer parse context, if any */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#ಆದರೆ_ಅಂತ್ಯ
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  ಪ್ರತಿಜ್ಞಾ( zSql!=0 );
  mxSqlLen = db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_SQL_ಉದ್ದಳತೆ];
  ಆದರೆ( db->n_Vdbe_ಸಕ್ರಿಯ==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->z_ಬಾಲ = zSql;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಆದರೆ( db->flags & SQLITE_ParserTrace ){
    ಬರೆ("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3ParserTrace(0, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#ಇಲ್ಲದಿದ್ದರೆ
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  ಆದರೆ( pEngine==0 ){
    sqlite3OomFault(db);
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( pParse->p_New_ಕೋಷ್ಟಕ==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->pNewTrigger==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->nVar==0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->pVList==0 );
  p_ಪೋಷಕ_Parse = db->pParse;
  db->pParse = pParse;
  ಆಗ( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    ಆದರೆ( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( tokenType>=TK_ಕಿಟಕಿ ){
      ಪ್ರತಿಜ್ಞಾ( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_ಕಿಟಕಿ 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( tokenType>=TK_SPACE ){
      ಪ್ರತಿಜ್ಞಾ( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
      ಆದರೆ( Atomic_ತುಂಬು(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_ಅಡಚಣೆ;
        pParse->nErr++;
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( tokenType==TK_SPACE ){
        zSql += n;
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        ಆದರೆ( ಕೊನೆಯ_Token_Parsed==TK_SEMI ){
          tokenType = 0;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಕೊನೆಯ_Token_Parsed==0 ){
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ{
          tokenType = TK_SEMI;
        }
        n = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( tokenType==TK_ಕಿಟಕಿ ){
        ಪ್ರತಿಜ್ಞಾ( n==6 );
        tokenType = analyze_ಕಿಟಕಿ_Keyword((ಸ್ಥಿರಾಂಕ u8*)&zSql[6]);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( tokenType==TK_OVER ){
        ಪ್ರತಿಜ್ಞಾ( n==4 );
        tokenType = analyzeOverKeyword((ಸ್ಥಿರಾಂಕ u8*)&zSql[4], ಕೊನೆಯ_Token_Parsed);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( tokenType==TK_FILTER ){
        ಪ್ರತಿಜ್ಞಾ( n==6 );
        tokenType = analyzeFilterKeyword((ಸ್ಥಿರಾಂಕ u8*)&zSql[6], ಕೊನೆಯ_Token_Parsed);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        ಮುಂದುವರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_ಸಂದೇಶ(pParse, "unrecognized token: \"%T\"", &x);
        ನಿಲ್ಲಿಸು;
      }
    }
    pParse->s_ಕೊನೆಯ_Token.z = zSql;
    pParse->s_ಕೊನೆಯ_Token.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->s_ಕೊನೆಯ_Token);
    ಕೊನೆಯ_Token_Parsed = tokenType;
    zSql += n;
    ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    ಆದರೆ( pParse->rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
  }
  ಪ್ರತಿಜ್ಞಾ( nErr==0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3_ಸ್ಥಿತಿ_Highwater(SQLITE_ಸ್ಥಿತಿ_PARSER_ರಾಶಿ,
      sqlite3_Parser_ರಾಶಿ_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#ಆದರೆ_ಅಂತ್ಯ /* YYDEBUG */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3ParserFree(pEngine, sqlite3_free);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  ಆದರೆ( pParse->z_Err_ಸಂದೇಶ || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_ಮುಗಿದ) ){
    ಆದರೆ( pParse->z_Err_ಸಂದೇಶ==0 ){
      pParse->z_Err_ಸಂದೇಶ = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    ಆದರೆ( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_ಸಂದೇಶ, pParse->z_ಬಾಲ);
    }
    nErr++;
  }
  pParse->z_ಬಾಲ = zSql;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->ap_Vtab_ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( pParse->p_New_ಕೋಷ್ಟಕ && !ಒಳ_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3_Delete_ಕೋಷ್ಟಕ(db, pParse->p_New_ಕೋಷ್ಟಕ);
  }
  ಆದರೆ( pParse->pNewTrigger && !ಒಳ_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  ಆದರೆ( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = p_ಪೋಷಕ_Parse;
  ಪ್ರತಿಜ್ಞಾ( nErr==0 || pParse->rc!=SQLITE_OK );
  ಹಿಂತಿರುಗಿ nErr;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_Space_Separator(sqlite3_str *pStr){
  ಆದರೆ( pStr->nChar && sqlite3IsIdChar(pStr->z_ಪಠ್ಯ[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
ಅಕ್ಷರ *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  ಪೂರ್ಣಾಂಕ i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  ಪೂರ್ಣಾಂಕ tokenType;     /* type of current token */
  ಪೂರ್ಣಾಂಕ prevType = 0;  /* Previous non-whitespace token */
  ಪೂರ್ಣಾಂಕ nParen;        /* Number of nested levels of parentheses */
  ಪೂರ್ಣಾಂಕ i_ಆರಂಭ_ಒಳ;      /* Start of RHS of IN operator in z[] */
  ಪೂರ್ಣಾಂಕ n_Paren_At_ಒಳ;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = i_ಆರಂಭ_ಒಳ = n_Paren_At_ಒಳ = 0;
  pStr = sqlite3_str_new(db);
  ಪ್ರತಿಜ್ಞಾ( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  ಗಾಗಿ(i=0; zSql[i] && pStr->accError==0; i+=n){
    ಆದರೆ( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)zSql+i, &tokenType);
    ಆದರೆ( NEVER(n<=0) ) ನಿಲ್ಲಿಸು;
    ಆಯ್ಕೆ( tokenType ){
      ಸ್ಥಿತಿ TK_COMMENT:
      ಸ್ಥಿತಿ TK_SPACE: {
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_NULL: {
        ಆದರೆ( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          ನಿಲ್ಲಿಸು;
        }
        /* no break */ deliberate_fall_through
      }
      ಸ್ಥಿತಿ TK_STRING:
      ಸ್ಥಿತಿ TK_INTEGER:
      ಸ್ಥಿತಿ TK_FLOAT:
      ಸ್ಥಿತಿ TK_VARIABLE:
      ಸ್ಥಿತಿ TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_LP: {
        nParen++;
        ಆದರೆ( prevType==TK_ಒಳ ){
          i_ಆರಂಭ_ಒಳ = pStr->nChar;
          n_Paren_At_ಒಳ = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_RP: {
        ಆದರೆ( i_ಆರಂಭ_ಒಳ>0 && nParen==n_Paren_At_ಒಳ ){
          ಪ್ರತಿಜ್ಞಾ( pStr->nChar>=(u32)i_ಆರಂಭ_ಒಳ );
          pStr->nChar = i_ಆರಂಭ_ಒಳ+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          i_ಆರಂಭ_ಒಳ = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_ID: {
        i_ಆರಂಭ_ಒಳ = 0;
        j = pStr->nChar;
        ಆದರೆ( sqlite3Isquote(zSql[i]) ){
          ಅಕ್ಷರ *zId = sqlite3DbStrNDup(db, zSql+i, n);
          ಪೂರ್ಣಾಂಕ nId;
          ಪೂರ್ಣಾಂಕ eType = 0;
          ಆದರೆ( zId==0 ) ನಿಲ್ಲಿಸು;
          sqlite3Dequote(zId);
          ಆದರೆ( zSql[i]=='"' && sqlite3_Vdbe_ಬಳಸುತ್ತದೆ_Double_Quoted_String(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            ನಿಲ್ಲಿಸು;
          }
          nId = sqlite3Strlen30(zId);
          ಆದರೆ( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            ಕೂಡಿಸು_Space_Separator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಕೂಡಿಸು_Space_Separator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        ಆಗ( j<pStr->nChar ){
          pStr->z_ಪಠ್ಯ[j] = sqlite3Tolower(pStr->z_ಪಠ್ಯ[j]);
          j++;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ TK_SELECT: {
        i_ಆರಂಭ_ಒಳ = 0;
        /* no break */ deliberate_fall_through
      }
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಆದರೆ( sqlite3IsIdChar(zSql[i]) ) ಕೂಡಿಸು_Space_Separator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        ಆಗ( j<pStr->nChar ){
          pStr->z_ಪಠ್ಯ[j] = sqlite3Toupper(pStr->z_ಪಠ್ಯ[j]);
          j++;
        }
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಆದರೆ( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  ಹಿಂತಿರುಗಿ sqlite3_str_ಮುಕ್ತಾಯ(pStr);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_NORMALIZE */
