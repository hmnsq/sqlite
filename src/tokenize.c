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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* Character classes for tokenizing
**
** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
** using a lookup table, whereas a switch() directly on c uses a binary search.
** The lookup table is much faster.  To maximize speed, and to ensure that
** a lookup table is used, all of the classes need to be small integers and
** all of them need to be used within the switch.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_X          0    /* The letter 'x', or start of BLOB literal */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_KYWD0      1    /* First letter of a keyword */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_DIGIT      3    /* Digits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_DOLLAR     4    /* '$' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_SPACE      7    /* Space characters */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_LT        12    /* '<'.  Part of < or <= or <> */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_GT        13    /* '>'.  Part of > or >= */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_EQ        14    /* '='.  Part of = or == */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_BANG      15    /* '!'.  Part of != */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_SLASH     16    /* '/'.  / or c-style comment */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_LP        17    /* '(' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_RP        18    /* ')' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_SEMI      19    /* ';' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_PLUS      20    /* '+' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_ਤਾਰਾ      21    /* '*' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_PERCENT   22    /* '%' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_COMMA     23    /* ',' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_AND       24    /* '&' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_TILDA     25    /* '~' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_DOT       26    /* '.' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_ID        27    /* unicode characters usable in IDs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_ILLEGAL   28    /* Illegal character */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_NUL       29    /* 0x00 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ aiClass[] = {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
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
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
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
#ਜੇ_ਅੰਤ
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
# define charMap(X) sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)X]
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
# define charMap(X) ebcdic_ਮੰਜ਼ਿਲ_Ascii[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)X]
ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ebcdic_ਮੰਜ਼ਿਲ_Ascii[] = {
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
#ਜੇ_ਅੰਤ

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
#ਅਨ੍ਤਰ੍ਭਾਵ "keywordhash.h"


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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IdChar(C)  ((sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)C]&0x46)!=0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EBCDIC
ਅਚਲ ਅੱਖਰ sqlite3IsEbcdicIdChar[] = {
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#ਜੇ_ਅੰਤ

/* Make the IdChar function accessible from ctime.c and alter.c */
ਪੂਰਨ sqlite3IsIdChar(u8 c){ ਵਾਪਸ IdChar(c); }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
ਸਥਿਰ ਪੂਰਨ getToken(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **pz){
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z = *pz;
  ਪੂਰਨ t;                          /* Token type to return */
  ਕਰੋ {
    z += sqlite3GetToken(z, &t);
  }ਜਦੋਂ( t==TK_SPACE || t==TK_COMMENT );
  ਜੇ( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_ਖਿੜਕੀ 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  ਵਾਪਸ t;
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
ਸਥਿਰ ਪੂਰਨ analyze_ਖਿੜਕੀ_Keyword(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z){
  ਪੂਰਨ t;
  t = getToken(&z);
  ਜੇ( t!=TK_ID ) ਵਾਪਸ TK_ID;
  t = getToken(&z);
  ਜੇ( t!=TK_AS ) ਵਾਪਸ TK_ID;
  ਵਾਪਸ TK_ਖਿੜਕੀ;
}
ਸਥਿਰ ਪੂਰਨ analyzeOverKeyword(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z, ਪੂਰਨ ਆਖਰੀ_Token){
  ਜੇ( ਆਖਰੀ_Token==TK_RP ){
    ਪੂਰਨ t = getToken(&z);
    ਜੇ( t==TK_LP || t==TK_ID ) ਵਾਪਸ TK_OVER;
  }
  ਵਾਪਸ TK_ID;
}
ਸਥਿਰ ਪੂਰਨ analyzeFilterKeyword(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z, ਪੂਰਨ ਆਖਰੀ_Token){
  ਜੇ( ਆਖਰੀ_Token==TK_RP && getToken(&z)==TK_LP ){
    ਵਾਪਸ TK_FILTER;
  }
  ਵਾਪਸ TK_ID;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *z, ਪੂਰਨ *tokenType){
  i64 i;
  ਪੂਰਨ c;
  ਚੋਣ( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    ਹਾਲਤ CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      ਲਈ(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      ਵਾਪਸ i;
    }
    ਹਾਲਤ CC_MINUS: {
      ਜੇ( z[1]=='-' ){
        ਲਈ(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        ਵਾਪਸ i;
      }ਨਹੀਂਤਾਂ ਜੇ( z[1]=='>' ){
        *tokenType = TK_PTR;
        ਵਾਪਸ 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_LP: {
      *tokenType = TK_LP;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_RP: {
      *tokenType = TK_RP;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_SEMI: {
      *tokenType = TK_SEMI;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_PLUS: {
      *tokenType = TK_PLUS;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_ਤਾਰਾ: {
      *tokenType = TK_ਤਾਰਾ;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_SLASH: {
      ਜੇ( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        ਵਾਪਸ 1;
      }
      ਲਈ(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      ਜੇ( c ) i++;
      *tokenType = TK_COMMENT;
      ਵਾਪਸ i;
    }
    ਹਾਲਤ CC_PERCENT: {
      *tokenType = TK_REM;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_EQ: {
      *tokenType = TK_EQ;
      ਵਾਪਸ 1 + (z[1]=='=');
    }
    ਹਾਲਤ CC_LT: {
      ਜੇ( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        ਵਾਪਸ 2;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='>' ){
        *tokenType = TK_NE;
        ਵਾਪਸ 2;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='<' ){
        *tokenType = TK_LSHIFT;
        ਵਾਪਸ 2;
      }ਨਹੀਂਤਾਂ{
        *tokenType = TK_LT;
        ਵਾਪਸ 1;
      }
    }
    ਹਾਲਤ CC_GT: {
      ਜੇ( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        ਵਾਪਸ 2;
      }ਨਹੀਂਤਾਂ ਜੇ( c=='>' ){
        *tokenType = TK_RSHIFT;
        ਵਾਪਸ 2;
      }ਨਹੀਂਤਾਂ{
        *tokenType = TK_GT;
        ਵਾਪਸ 1;
      }
    }
    ਹਾਲਤ CC_BANG: {
      ਜੇ( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        ਵਾਪਸ 1;
      }ਨਹੀਂਤਾਂ{
        *tokenType = TK_NE;
        ਵਾਪਸ 2;
      }
    }
    ਹਾਲਤ CC_PIPE: {
      ਜੇ( z[1]!='|' ){
        *tokenType = TK_BITOR;
        ਵਾਪਸ 1;
      }ਨਹੀਂਤਾਂ{
        *tokenType = TK_CONCAT;
        ਵਾਪਸ 2;
      }
    }
    ਹਾਲਤ CC_COMMA: {
      *tokenType = TK_COMMA;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_AND: {
      *tokenType = TK_BITAND;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_TILDA: {
      *tokenType = TK_BITNOT;
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ CC_QUOTE: {
      ਪੂਰਨ delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      ਲਈ(i=1; (c=z[i])!=0; i++){
        ਜੇ( c==delim ){
          ਜੇ( z[i+1]==delim ){
            i++;
          }ਨਹੀਂਤਾਂ{
            ਰੋਕੋ;
          }
        }
      }
      ਜੇ( c=='\'' ){
        *tokenType = TK_STRING;
        ਵਾਪਸ i+1;
      }ਨਹੀਂਤਾਂ ਜੇ( c!=0 ){
        *tokenType = TK_ID;
        ਵਾਪਸ i+1;
      }ਨਹੀਂਤਾਂ{
        *tokenType = TK_ILLEGAL;
        ਵਾਪਸ i;
      }
    }
    ਹਾਲਤ CC_DOT: {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
      ਜੇ( !sqlite3Isdigit(z[1]) )
#ਜੇ_ਅੰਤ
      {
        *tokenType = TK_DOT;
        ਵਾਪਸ 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    ਹਾਲਤ CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_HEX_INTEGER
      ਜੇ( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        ਲਈ(i=3; 1; i++){
          ਜੇ( sqlite3Isxdigit(z[i])==0 ){
            ਜੇ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ਨਹੀਂਤਾਂ{
              ਰੋਕੋ;
            }
          }
        }
      }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
        {
        ਲਈ(i=0; 1; i++){
          ਜੇ( sqlite3Isdigit(z[i])==0 ){
            ਜੇ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ਨਹੀਂਤਾਂ{
              ਰੋਕੋ;
            }
          }
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
        ਜੇ( z[i]=='.' ){
          ਜੇ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ਲਈ(i++; 1; i++){
            ਜੇ( sqlite3Isdigit(z[i])==0 ){
              ਜੇ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ਨਹੀਂਤਾਂ{
                ਰੋਕੋ;
              }
            }
          }
        }
        ਜੇ( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          ਜੇ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          ਲਈ(i+=2; 1; i++){
            ਜੇ( sqlite3Isdigit(z[i])==0 ){
              ਜੇ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ਨਹੀਂਤਾਂ{
                ਰੋਕੋ;
              }
            }
          }
        }
#ਜੇ_ਅੰਤ
      }
      ਜਦੋਂ( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      ਵਾਪਸ i;
    }
    ਹਾਲਤ CC_QUOTE2: {
      ਲਈ(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      ਵਾਪਸ i;
    }
    ਹਾਲਤ CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      ਲਈ(i=1; sqlite3Isdigit(z[i]); i++){}
      ਵਾਪਸ i;
    }
    ਹਾਲਤ CC_DOLLAR:
    ਹਾਲਤ CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      ਲਈ(i=1; (c=z[i])!=0; i++){
        ਜੇ( IdChar(c) ){
          n++;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TCL_VARIABLE
        }ਨਹੀਂਤਾਂ ਜੇ( c=='(' && n>0 ){
          ਕਰੋ{
            i++;
          }ਜਦੋਂ( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          ਜੇ( c==')' ){
            i++;
          }ਨਹੀਂਤਾਂ{
            *tokenType = TK_ILLEGAL;
          }
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( c==':' && z[i+1]==':' ){
          i++;
#ਜੇ_ਅੰਤ
        }ਨਹੀਂਤਾਂ{
          ਰੋਕੋ;
        }
      }
      ਜੇ( n==0 ) *tokenType = TK_ILLEGAL;
      ਵਾਪਸ i;
    }
    ਹਾਲਤ CC_KYWD0: {
      ਜੇ( aiClass[z[1]]>CC_KYWD ){ i = 1;  ਰੋਕੋ; }
      ਲਈ(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      ਜੇ( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        ਰੋਕੋ;
      }
      *tokenType = TK_ID;
      ਵਾਪਸ keyword_ਕੋਡ((ਅੱਖਰ*)z, i, tokenType);
    }
    ਹਾਲਤ CC_X: {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      ਜੇ( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        ਲਈ(i=2; sqlite3Isxdigit(z[i]); i++){}
        ਜੇ( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          ਜਦੋਂ( z[i] && z[i]!='\'' ){ i++; }
        }
        ਜੇ( z[i] ) i++;
        ਵਾਪਸ i;
      }
#ਜੇ_ਅੰਤ
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    ਹਾਲਤ CC_KYWD:
    ਹਾਲਤ CC_ID: {
      i = 1;
      ਰੋਕੋ;
    }
    ਹਾਲਤ CC_BOM: {
      ਜੇ( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        ਵਾਪਸ 3;
      }
      i = 1;
      ਰੋਕੋ;
    }
    ਹਾਲਤ CC_NUL: {
      *tokenType = TK_ILLEGAL;
      ਵਾਪਸ 0;
    }
    ਮੂਲ: {
      *tokenType = TK_ILLEGAL;
      ਵਾਪਸ 1;
    }
  }
  ਜਦੋਂ( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  ਵਾਪਸ i;
}

/*
** Run the parser on the given SQL string.
*/
ਪੂਰਨ sqlite3_ਲੜੀਵਾਰ_Parser(Parse *pParse, ਅਚਲ ਅੱਖਰ *zSql){
  ਪੂਰਨ nErr = 0;                   /* Number of errors encountered */
  ਖਾਲੀ *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  ਪੂਰਨ tokenType;                  /* type of the next token */
  ਪੂਰਨ ਆਖਰੀ_Token_Parsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *p_ਮਾਪਾ_Parse = 0;        /* Outer parse context, if any */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#ਜੇ_ਅੰਤ
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  ਪ੍ਰਤਿਜ੍ਞਾ( zSql!=0 );
  mxSqlLen = db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_SQL_ਲੰਮਾਈ];
  ਜੇ( db->n_Vdbe_ਸਰਗਰਮ==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->z_ਪੂਛ = zSql;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਜੇ( db->flags & SQLITE_ParserTrace ){
    ਲਿਖੋ("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }ਨਹੀਂਤਾਂ{
    sqlite3ParserTrace(0, 0);
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#ਨਹੀਂਤਾਂ
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  ਜੇ( pEngine==0 ){
    sqlite3OomFault(db);
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->p_New_ਸਾਰਣੀ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pNewTrigger==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nVar==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pVList==0 );
  p_ਮਾਪਾ_Parse = db->pParse;
  db->pParse = pParse;
  ਜਦੋਂ( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    ਜੇ( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( tokenType>=TK_ਖਿੜਕੀ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_ਖਿੜਕੀ 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ਨਹੀਂਤਾਂ
    ਜੇ( tokenType>=TK_SPACE ){
      ਪ੍ਰਤਿਜ੍ਞਾ( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
      ਜੇ( Atomic_ਲੱਦੋ(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_ਰੁਕਾਵਟ;
        pParse->nErr++;
        ਰੋਕੋ;
      }
      ਜੇ( tokenType==TK_SPACE ){
        zSql += n;
        ਜਾਰੀ;
      }
      ਜੇ( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        ਜੇ( ਆਖਰੀ_Token_Parsed==TK_SEMI ){
          tokenType = 0;
        }ਨਹੀਂਤਾਂ ਜੇ( ਆਖਰੀ_Token_Parsed==0 ){
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ{
          tokenType = TK_SEMI;
        }
        n = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      }ਨਹੀਂਤਾਂ ਜੇ( tokenType==TK_ਖਿੜਕੀ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( n==6 );
        tokenType = analyze_ਖਿੜਕੀ_Keyword((ਅਚਲ u8*)&zSql[6]);
      }ਨਹੀਂਤਾਂ ਜੇ( tokenType==TK_OVER ){
        ਪ੍ਰਤਿਜ੍ਞਾ( n==4 );
        tokenType = analyzeOverKeyword((ਅਚਲ u8*)&zSql[4], ਆਖਰੀ_Token_Parsed);
      }ਨਹੀਂਤਾਂ ਜੇ( tokenType==TK_FILTER ){
        ਪ੍ਰਤਿਜ੍ਞਾ( n==6 );
        tokenType = analyzeFilterKeyword((ਅਚਲ u8*)&zSql[6], ਆਖਰੀ_Token_Parsed);
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
      }ਨਹੀਂਤਾਂ ਜੇ( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        ਜਾਰੀ;
      }ਨਹੀਂਤਾਂ ਜੇ( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unrecognized token: \"%T\"", &x);
        ਰੋਕੋ;
      }
    }
    pParse->s_ਆਖਰੀ_Token.z = zSql;
    pParse->s_ਆਖਰੀ_Token.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->s_ਆਖਰੀ_Token);
    ਆਖਰੀ_Token_Parsed = tokenType;
    zSql += n;
    ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    ਜੇ( pParse->rc!=SQLITE_OK ) ਰੋਕੋ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( nErr==0 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3_ਸਥਿਤੀ_Highwater(SQLITE_ਸਥਿਤੀ_PARSER_ਢੇਰ,
      sqlite3_Parser_ਢੇਰ_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#ਜੇ_ਅੰਤ /* YYDEBUG */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#ਨਹੀਂਤਾਂ
  sqlite3ParserFree(pEngine, sqlite3_free);
#ਜੇ_ਅੰਤ
  ਜੇ( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  ਜੇ( pParse->z_Err_ਸੁਨੇਹਾ || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_ਪੂਰਾ) ){
    ਜੇ( pParse->z_Err_ਸੁਨੇਹਾ==0 ){
      pParse->z_Err_ਸੁਨੇਹਾ = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    ਜੇ( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_ਸੁਨੇਹਾ, pParse->z_ਪੂਛ);
    }
    nErr++;
  }
  pParse->z_ਪੂਛ = zSql;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->ap_Vtab_ਤਾਲਾ);
#ਜੇ_ਅੰਤ

  ਜੇ( pParse->p_New_ਸਾਰਣੀ && !ਵਿੱਚ_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3_Delete_ਸਾਰਣੀ(db, pParse->p_New_ਸਾਰਣੀ);
  }
  ਜੇ( pParse->pNewTrigger && !ਵਿੱਚ_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  ਜੇ( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = p_ਮਾਪਾ_Parse;
  ਪ੍ਰਤਿਜ੍ਞਾ( nErr==0 || pParse->rc!=SQLITE_OK );
  ਵਾਪਸ nErr;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_Space_Separator(sqlite3_str *pStr){
  ਜੇ( pStr->nChar && sqlite3IsIdChar(pStr->z_ਪਾਠ[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
ਅੱਖਰ *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  ਅਚਲ ਅੱਖਰ *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  ਪੂਰਨ i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  ਪੂਰਨ tokenType;     /* type of current token */
  ਪੂਰਨ prevType = 0;  /* Previous non-whitespace token */
  ਪੂਰਨ nParen;        /* Number of nested levels of parentheses */
  ਪੂਰਨ i_ਸ਼ੁਰੂ_ਵਿੱਚ;      /* Start of RHS of IN operator in z[] */
  ਪੂਰਨ n_Paren_At_ਵਿੱਚ;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = i_ਸ਼ੁਰੂ_ਵਿੱਚ = n_Paren_At_ਵਿੱਚ = 0;
  pStr = sqlite3_str_new(db);
  ਪ੍ਰਤਿਜ੍ਞਾ( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  ਲਈ(i=0; zSql[i] && pStr->accError==0; i+=n){
    ਜੇ( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)zSql+i, &tokenType);
    ਜੇ( NEVER(n<=0) ) ਰੋਕੋ;
    ਚੋਣ( tokenType ){
      ਹਾਲਤ TK_COMMENT:
      ਹਾਲਤ TK_SPACE: {
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_NULL: {
        ਜੇ( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          ਰੋਕੋ;
        }
        /* no break */ deliberate_fall_through
      }
      ਹਾਲਤ TK_STRING:
      ਹਾਲਤ TK_INTEGER:
      ਹਾਲਤ TK_FLOAT:
      ਹਾਲਤ TK_VARIABLE:
      ਹਾਲਤ TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_LP: {
        nParen++;
        ਜੇ( prevType==TK_ਵਿੱਚ ){
          i_ਸ਼ੁਰੂ_ਵਿੱਚ = pStr->nChar;
          n_Paren_At_ਵਿੱਚ = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_RP: {
        ਜੇ( i_ਸ਼ੁਰੂ_ਵਿੱਚ>0 && nParen==n_Paren_At_ਵਿੱਚ ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pStr->nChar>=(u32)i_ਸ਼ੁਰੂ_ਵਿੱਚ );
          pStr->nChar = i_ਸ਼ੁਰੂ_ਵਿੱਚ+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          i_ਸ਼ੁਰੂ_ਵਿੱਚ = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_ID: {
        i_ਸ਼ੁਰੂ_ਵਿੱਚ = 0;
        j = pStr->nChar;
        ਜੇ( sqlite3Isquote(zSql[i]) ){
          ਅੱਖਰ *zId = sqlite3DbStrNDup(db, zSql+i, n);
          ਪੂਰਨ nId;
          ਪੂਰਨ eType = 0;
          ਜੇ( zId==0 ) ਰੋਕੋ;
          sqlite3Dequote(zId);
          ਜੇ( zSql[i]=='"' && sqlite3_Vdbe_ਵਰਤਦਾ_Double_Quoted_String(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            ਰੋਕੋ;
          }
          nId = sqlite3Strlen30(zId);
          ਜੇ( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            ਜੋੜੋ_Space_Separator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }ਨਹੀਂਤਾਂ{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }ਨਹੀਂਤਾਂ{
          ਜੋੜੋ_Space_Separator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        ਜਦੋਂ( j<pStr->nChar ){
          pStr->z_ਪਾਠ[j] = sqlite3Tolower(pStr->z_ਪਾਠ[j]);
          j++;
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ TK_SELECT: {
        i_ਸ਼ੁਰੂ_ਵਿੱਚ = 0;
        /* no break */ deliberate_fall_through
      }
      ਮੂਲ: {
        ਜੇ( sqlite3IsIdChar(zSql[i]) ) ਜੋੜੋ_Space_Separator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        ਜਦੋਂ( j<pStr->nChar ){
          pStr->z_ਪਾਠ[j] = sqlite3Toupper(pStr->z_ਪਾਠ[j]);
          j++;
        }
        ਰੋਕੋ;
      }
    }
  }
  ਜੇ( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  ਵਾਪਸ sqlite3_str_ਅੰਤ(pStr);
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_NORMALIZE */
