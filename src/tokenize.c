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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdlib.h>

/* Character classes for tokenizing
**
** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
** using a lookup table, whereas a switch() directly on c uses a binary search.
** The lookup table is much faster.  To maximize speed, and to ensure that
** a lookup table is used, all of the classes need to be small integers and
** all of them need to be used within the switch.
*/
#నిర్వచించు CC_X          0    /* The letter 'x', or start of BLOB literal */
#నిర్వచించు CC_KYWD0      1    /* First letter of a keyword */
#నిర్వచించు CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#నిర్వచించు CC_DIGIT      3    /* Digits */
#నిర్వచించు CC_DOLLAR     4    /* '$' */
#నిర్వచించు CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#నిర్వచించు CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#నిర్వచించు CC_SPACE      7    /* Space characters */
#నిర్వచించు CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#నిర్వచించు CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#నిర్వచించు CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#నిర్వచించు CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#నిర్వచించు CC_LT        12    /* '<'.  Part of < or <= or <> */
#నిర్వచించు CC_GT        13    /* '>'.  Part of > or >= */
#నిర్వచించు CC_EQ        14    /* '='.  Part of = or == */
#నిర్వచించు CC_BANG      15    /* '!'.  Part of != */
#నిర్వచించు CC_SLASH     16    /* '/'.  / or c-style comment */
#నిర్వచించు CC_LP        17    /* '(' */
#నిర్వచించు CC_RP        18    /* ')' */
#నిర్వచించు CC_SEMI      19    /* ';' */
#నిర్వచించు CC_PLUS      20    /* '+' */
#నిర్వచించు CC_నక్షత్రం      21    /* '*' */
#నిర్వచించు CC_PERCENT   22    /* '%' */
#నిర్వచించు CC_COMMA     23    /* ',' */
#నిర్వచించు CC_AND       24    /* '&' */
#నిర్వచించు CC_TILDA     25    /* '~' */
#నిర్వచించు CC_DOT       26    /* '.' */
#నిర్వచించు CC_ID        27    /* unicode characters usable in IDs */
#నిర్వచించు CC_ILLEGAL   28    /* Illegal character */
#నిర్వచించు CC_NUL       29    /* 0x00 */
#నిర్వచించు CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

స్థిర స్థిరరాశి గుర్తులేని అక్షరం aiClass[] = {
#నిర్వచితమైతే SQLITE_ASCII
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
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
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
#అయితే_ముగింపు
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
#నిర్వచితమైతే SQLITE_ASCII
# define charMap(X) sqlite3_పై_గమ్యం_క్రింది[(గుర్తులేని అక్షరం)X]
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
# define charMap(X) ebcdic_గమ్యం_Ascii[(గుర్తులేని అక్షరం)X]
స్థిరరాశి గుర్తులేని అక్షరం ebcdic_గమ్యం_Ascii[] = {
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
#అయితే_ముగింపు

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
#అన్తర్భావ "keywordhash.h"


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
#నిర్వచితమైతే SQLITE_ASCII
#నిర్వచించు IdChar(C)  ((sqlite3CtypeMap[(గుర్తులేని అక్షరం)C]&0x46)!=0)
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_EBCDIC
స్థిరరాశి అక్షరం sqlite3IsEbcdicIdChar[] = {
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
#నిర్వచించు IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#అయితే_ముగింపు

/* Make the IdChar function accessible from ctime.c and alter.c */
పూర్ణాంకం sqlite3IsIdChar(u8 c){ తిరిగి IdChar(c); }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
స్థిర పూర్ణాంకం getToken(స్థిరరాశి గుర్తులేని అక్షరం **pz){
  స్థిరరాశి గుర్తులేని అక్షరం *z = *pz;
  పూర్ణాంకం t;                          /* Token type to return */
  చేయి {
    z += sqlite3GetToken(z, &t);
  }అయితే( t==TK_SPACE || t==TK_COMMENT );
  ఒకవేళ( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_కిటికీ 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  తిరిగి t;
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
స్థిర పూర్ణాంకం analyze_కిటికీ_Keyword(స్థిరరాశి గుర్తులేని అక్షరం *z){
  పూర్ణాంకం t;
  t = getToken(&z);
  ఒకవేళ( t!=TK_ID ) తిరిగి TK_ID;
  t = getToken(&z);
  ఒకవేళ( t!=TK_AS ) తిరిగి TK_ID;
  తిరిగి TK_కిటికీ;
}
స్థిర పూర్ణాంకం analyzeOverKeyword(స్థిరరాశి గుర్తులేని అక్షరం *z, పూర్ణాంకం చివరి_Token){
  ఒకవేళ( చివరి_Token==TK_RP ){
    పూర్ణాంకం t = getToken(&z);
    ఒకవేళ( t==TK_LP || t==TK_ID ) తిరిగి TK_OVER;
  }
  తిరిగి TK_ID;
}
స్థిర పూర్ణాంకం analyzeFilterKeyword(స్థిరరాశి గుర్తులేని అక్షరం *z, పూర్ణాంకం చివరి_Token){
  ఒకవేళ( చివరి_Token==TK_RP && getToken(&z)==TK_LP ){
    తిరిగి TK_FILTER;
  }
  తిరిగి TK_ID;
}
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(స్థిరరాశి గుర్తులేని అక్షరం *z, పూర్ణాంకం *tokenType){
  i64 i;
  పూర్ణాంకం c;
  ఎంపిక( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    స్థితి CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      కొరకు(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      తిరిగి i;
    }
    స్థితి CC_MINUS: {
      ఒకవేళ( z[1]=='-' ){
        కొరకు(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        తిరిగి i;
      }లేకపోతే ఒకవేళ( z[1]=='>' ){
        *tokenType = TK_PTR;
        తిరిగి 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      తిరిగి 1;
    }
    స్థితి CC_LP: {
      *tokenType = TK_LP;
      తిరిగి 1;
    }
    స్థితి CC_RP: {
      *tokenType = TK_RP;
      తిరిగి 1;
    }
    స్థితి CC_SEMI: {
      *tokenType = TK_SEMI;
      తిరిగి 1;
    }
    స్థితి CC_PLUS: {
      *tokenType = TK_PLUS;
      తిరిగి 1;
    }
    స్థితి CC_నక్షత్రం: {
      *tokenType = TK_నక్షత్రం;
      తిరిగి 1;
    }
    స్థితి CC_SLASH: {
      ఒకవేళ( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        తిరిగి 1;
      }
      కొరకు(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      ఒకవేళ( c ) i++;
      *tokenType = TK_COMMENT;
      తిరిగి i;
    }
    స్థితి CC_PERCENT: {
      *tokenType = TK_REM;
      తిరిగి 1;
    }
    స్థితి CC_EQ: {
      *tokenType = TK_EQ;
      తిరిగి 1 + (z[1]=='=');
    }
    స్థితి CC_LT: {
      ఒకవేళ( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        తిరిగి 2;
      }లేకపోతే ఒకవేళ( c=='>' ){
        *tokenType = TK_NE;
        తిరిగి 2;
      }లేకపోతే ఒకవేళ( c=='<' ){
        *tokenType = TK_LSHIFT;
        తిరిగి 2;
      }లేకపోతే{
        *tokenType = TK_LT;
        తిరిగి 1;
      }
    }
    స్థితి CC_GT: {
      ఒకవేళ( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        తిరిగి 2;
      }లేకపోతే ఒకవేళ( c=='>' ){
        *tokenType = TK_RSHIFT;
        తిరిగి 2;
      }లేకపోతే{
        *tokenType = TK_GT;
        తిరిగి 1;
      }
    }
    స్థితి CC_BANG: {
      ఒకవేళ( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        తిరిగి 1;
      }లేకపోతే{
        *tokenType = TK_NE;
        తిరిగి 2;
      }
    }
    స్థితి CC_PIPE: {
      ఒకవేళ( z[1]!='|' ){
        *tokenType = TK_BITOR;
        తిరిగి 1;
      }లేకపోతే{
        *tokenType = TK_CONCAT;
        తిరిగి 2;
      }
    }
    స్థితి CC_COMMA: {
      *tokenType = TK_COMMA;
      తిరిగి 1;
    }
    స్థితి CC_AND: {
      *tokenType = TK_BITAND;
      తిరిగి 1;
    }
    స్థితి CC_TILDA: {
      *tokenType = TK_BITNOT;
      తిరిగి 1;
    }
    స్థితి CC_QUOTE: {
      పూర్ణాంకం delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      కొరకు(i=1; (c=z[i])!=0; i++){
        ఒకవేళ( c==delim ){
          ఒకవేళ( z[i+1]==delim ){
            i++;
          }లేకపోతే{
            ఆపు;
          }
        }
      }
      ఒకవేళ( c=='\'' ){
        *tokenType = TK_STRING;
        తిరిగి i+1;
      }లేకపోతే ఒకవేళ( c!=0 ){
        *tokenType = TK_ID;
        తిరిగి i+1;
      }లేకపోతే{
        *tokenType = TK_ILLEGAL;
        తిరిగి i;
      }
    }
    స్థితి CC_DOT: {
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
      ఒకవేళ( !sqlite3Isdigit(z[1]) )
#అయితే_ముగింపు
      {
        *tokenType = TK_DOT;
        తిరిగి 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    స్థితి CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_HEX_INTEGER
      ఒకవేళ( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        కొరకు(i=3; 1; i++){
          ఒకవేళ( sqlite3Isxdigit(z[i])==0 ){
            ఒకవేళ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }లేకపోతే{
              ఆపు;
            }
          }
        }
      }లేకపోతే
#అయితే_ముగింపు
        {
        కొరకు(i=0; 1; i++){
          ఒకవేళ( sqlite3Isdigit(z[i])==0 ){
            ఒకవేళ( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }లేకపోతే{
              ఆపు;
            }
          }
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
        ఒకవేళ( z[i]=='.' ){
          ఒకవేళ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          కొరకు(i++; 1; i++){
            ఒకవేళ( sqlite3Isdigit(z[i])==0 ){
              ఒకవేళ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }లేకపోతే{
                ఆపు;
              }
            }
          }
        }
        ఒకవేళ( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          ఒకవేళ( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          కొరకు(i+=2; 1; i++){
            ఒకవేళ( sqlite3Isdigit(z[i])==0 ){
              ఒకవేళ( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }లేకపోతే{
                ఆపు;
              }
            }
          }
        }
#అయితే_ముగింపు
      }
      అయితే( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      తిరిగి i;
    }
    స్థితి CC_QUOTE2: {
      కొరకు(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      తిరిగి i;
    }
    స్థితి CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      కొరకు(i=1; sqlite3Isdigit(z[i]); i++){}
      తిరిగి i;
    }
    స్థితి CC_DOLLAR:
    స్థితి CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      కొరకు(i=1; (c=z[i])!=0; i++){
        ఒకవేళ( IdChar(c) ){
          n++;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TCL_VARIABLE
        }లేకపోతే ఒకవేళ( c=='(' && n>0 ){
          చేయి{
            i++;
          }అయితే( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          ఒకవేళ( c==')' ){
            i++;
          }లేకపోతే{
            *tokenType = TK_ILLEGAL;
          }
          ఆపు;
        }లేకపోతే ఒకవేళ( c==':' && z[i+1]==':' ){
          i++;
#అయితే_ముగింపు
        }లేకపోతే{
          ఆపు;
        }
      }
      ఒకవేళ( n==0 ) *tokenType = TK_ILLEGAL;
      తిరిగి i;
    }
    స్థితి CC_KYWD0: {
      ఒకవేళ( aiClass[z[1]]>CC_KYWD ){ i = 1;  ఆపు; }
      కొరకు(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      ఒకవేళ( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        ఆపు;
      }
      *tokenType = TK_ID;
      తిరిగి keyword_సంకేతం((అక్షరం*)z, i, tokenType);
    }
    స్థితి CC_X: {
#నిర్వచితం_కాకపోతే SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      ఒకవేళ( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        కొరకు(i=2; sqlite3Isxdigit(z[i]); i++){}
        ఒకవేళ( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          అయితే( z[i] && z[i]!='\'' ){ i++; }
        }
        ఒకవేళ( z[i] ) i++;
        తిరిగి i;
      }
#అయితే_ముగింపు
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    స్థితి CC_KYWD:
    స్థితి CC_ID: {
      i = 1;
      ఆపు;
    }
    స్థితి CC_BOM: {
      ఒకవేళ( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        తిరిగి 3;
      }
      i = 1;
      ఆపు;
    }
    స్థితి CC_NUL: {
      *tokenType = TK_ILLEGAL;
      తిరిగి 0;
    }
    అప్రమేయ: {
      *tokenType = TK_ILLEGAL;
      తిరిగి 1;
    }
  }
  అయితే( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  తిరిగి i;
}

/*
** Run the parser on the given SQL string.
*/
పూర్ణాంకం sqlite3_నిరంతరం_Parser(Parse *pParse, స్థిరరాశి అక్షరం *zSql){
  పూర్ణాంకం nErr = 0;                   /* Number of errors encountered */
  శూన్య *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  పూర్ణాంకం tokenType;                  /* type of the next token */
  పూర్ణాంకం చివరి_Token_Parsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *p_మాతృ_Parse = 0;        /* Outer parse context, if any */
#నిర్వచితమైతే sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#అయితే_ముగింపు
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  ప్రతిజ్ఞా( zSql!=0 );
  mxSqlLen = db->a_హద్దు[SQLITE_హద్దు_SQL_నిడివి];
  ఒకవేళ( db->n_Vdbe_చురుకైన==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->z_తోక = zSql;
#నిర్వచితమైతే SQLITE_DEBUG
  ఒకవేళ( db->flags & SQLITE_ParserTrace ){
    రాయి("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }లేకపోతే{
    sqlite3ParserTrace(0, 0);
  }
#అయితే_ముగింపు
#నిర్వచితమైతే sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#లేకపోతే
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  ఒకవేళ( pEngine==0 ){
    sqlite3OomFault(db);
    తిరిగి SQLITE_NOMEM_BKPT;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pParse->p_New_పట్టిక==0 );
  ప్రతిజ్ఞా( pParse->pNewTrigger==0 );
  ప్రతిజ్ఞా( pParse->nVar==0 );
  ప్రతిజ్ఞా( pParse->pVList==0 );
  p_మాతృ_Parse = db->pParse;
  db->pParse = pParse;
  అయితే( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    ఒకవేళ( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( tokenType>=TK_కిటికీ ){
      ప్రతిజ్ఞా( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_కిటికీ 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#లేకపోతే
    ఒకవేళ( tokenType>=TK_SPACE ){
      ప్రతిజ్ఞా( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
      ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_అంతరాయం;
        pParse->nErr++;
        ఆపు;
      }
      ఒకవేళ( tokenType==TK_SPACE ){
        zSql += n;
        కొనసాగు;
      }
      ఒకవేళ( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        ఒకవేళ( చివరి_Token_Parsed==TK_SEMI ){
          tokenType = 0;
        }లేకపోతే ఒకవేళ( చివరి_Token_Parsed==0 ){
          ఆపు;
        }లేకపోతే{
          tokenType = TK_SEMI;
        }
        n = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      }లేకపోతే ఒకవేళ( tokenType==TK_కిటికీ ){
        ప్రతిజ్ఞా( n==6 );
        tokenType = analyze_కిటికీ_Keyword((స్థిరరాశి u8*)&zSql[6]);
      }లేకపోతే ఒకవేళ( tokenType==TK_OVER ){
        ప్రతిజ్ఞా( n==4 );
        tokenType = analyzeOverKeyword((స్థిరరాశి u8*)&zSql[4], చివరి_Token_Parsed);
      }లేకపోతే ఒకవేళ( tokenType==TK_FILTER ){
        ప్రతిజ్ఞా( n==6 );
        tokenType = analyzeFilterKeyword((స్థిరరాశి u8*)&zSql[6], చివరి_Token_Parsed);
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
      }లేకపోతే ఒకవేళ( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        కొనసాగు;
      }లేకపోతే ఒకవేళ( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_సందేశం(pParse, "unrecognized token: \"%T\"", &x);
        ఆపు;
      }
    }
    pParse->s_చివరి_Token.z = zSql;
    pParse->s_చివరి_Token.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->s_చివరి_Token);
    చివరి_Token_Parsed = tokenType;
    zSql += n;
    ప్రతిజ్ఞా( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    ఒకవేళ( pParse->rc!=SQLITE_OK ) ఆపు;
  }
  ప్రతిజ్ఞా( nErr==0 );
#నిర్వచితమైతే YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3_స్థితి_Highwater(SQLITE_స్థితి_PARSER_దొంతర,
      sqlite3_Parser_దొంతర_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#అయితే_ముగింపు /* YYDEBUG */
#నిర్వచితమైతే sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#లేకపోతే
  sqlite3ParserFree(pEngine, sqlite3_free);
#అయితే_ముగింపు
  ఒకవేళ( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  ఒకవేళ( pParse->z_Err_సందేశం || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_పూర్తి) ){
    ఒకవేళ( pParse->z_Err_సందేశం==0 ){
      pParse->z_Err_సందేశం = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    ఒకవేళ( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_సందేశం, pParse->z_తోక);
    }
    nErr++;
  }
  pParse->z_తోక = zSql;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->ap_Vtab_తాళం);
#అయితే_ముగింపు

  ఒకవేళ( pParse->p_New_పట్టిక && !లోపల_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3_Delete_పట్టిక(db, pParse->p_New_పట్టిక);
  }
  ఒకవేళ( pParse->pNewTrigger && !లోపల_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  ఒకవేళ( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = p_మాతృ_Parse;
  ప్రతిజ్ఞా( nErr==0 || pParse->rc!=SQLITE_OK );
  తిరిగి nErr;
}


#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
స్థిర శూన్య కూడు_Space_Separator(sqlite3_str *pStr){
  ఒకవేళ( pStr->nChar && sqlite3IsIdChar(pStr->z_పాఠ్యం[pStr->nChar-1]) ){
    sqlite3_str_append(pStr, " ", 1);
  }
}

/*
** Compute a normalization of the SQL given by zSql[0..nSql-1].  Return
** the normalization in space obtained from sqlite3DbMalloc().  Or return
** NULL if anything goes wrong or if zSql is NULL.
*/
అక్షరం *sqlite3Normalize(
  Vdbe *pVdbe,       /* VM being reprepared */
  స్థిరరాశి అక్షరం *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  పూర్ణాంకం i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  పూర్ణాంకం tokenType;     /* type of current token */
  పూర్ణాంకం prevType = 0;  /* Previous non-whitespace token */
  పూర్ణాంకం nParen;        /* Number of nested levels of parentheses */
  పూర్ణాంకం i_ప్రారంభం_లోపల;      /* Start of RHS of IN operator in z[] */
  పూర్ణాంకం n_Paren_At_లోపల;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = i_ప్రారంభం_లోపల = n_Paren_At_లోపల = 0;
  pStr = sqlite3_str_new(db);
  ప్రతిజ్ఞా( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  కొరకు(i=0; zSql[i] && pStr->accError==0; i+=n){
    ఒకవేళ( tokenType!=TK_SPACE ){
      prevType = tokenType;
    }
    n = sqlite3GetToken((గుర్తులేని అక్షరం*)zSql+i, &tokenType);
    ఒకవేళ( NEVER(n<=0) ) ఆపు;
    ఎంపిక( tokenType ){
      స్థితి TK_COMMENT:
      స్థితి TK_SPACE: {
        ఆపు;
      }
      స్థితి TK_NULL: {
        ఒకవేళ( prevType==TK_IS || prevType==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          ఆపు;
        }
        /* no break */ deliberate_fall_through
      }
      స్థితి TK_STRING:
      స్థితి TK_INTEGER:
      స్థితి TK_FLOAT:
      స్థితి TK_VARIABLE:
      స్థితి TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        ఆపు;
      }
      స్థితి TK_LP: {
        nParen++;
        ఒకవేళ( prevType==TK_లోపల ){
          i_ప్రారంభం_లోపల = pStr->nChar;
          n_Paren_At_లోపల = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        ఆపు;
      }
      స్థితి TK_RP: {
        ఒకవేళ( i_ప్రారంభం_లోపల>0 && nParen==n_Paren_At_లోపల ){
          ప్రతిజ్ఞా( pStr->nChar>=(u32)i_ప్రారంభం_లోపల );
          pStr->nChar = i_ప్రారంభం_లోపల+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          i_ప్రారంభం_లోపల = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        ఆపు;
      }
      స్థితి TK_ID: {
        i_ప్రారంభం_లోపల = 0;
        j = pStr->nChar;
        ఒకవేళ( sqlite3Isquote(zSql[i]) ){
          అక్షరం *zId = sqlite3DbStrNDup(db, zSql+i, n);
          పూర్ణాంకం nId;
          పూర్ణాంకం eType = 0;
          ఒకవేళ( zId==0 ) ఆపు;
          sqlite3Dequote(zId);
          ఒకవేళ( zSql[i]=='"' && sqlite3_Vdbe_ఉపయోగిస్తుంది_Double_Quoted_String(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            ఆపు;
          }
          nId = sqlite3Strlen30(zId);
          ఒకవేళ( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            కూడు_Space_Separator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }లేకపోతే{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }లేకపోతే{
          కూడు_Space_Separator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        అయితే( j<pStr->nChar ){
          pStr->z_పాఠ్యం[j] = sqlite3Tolower(pStr->z_పాఠ్యం[j]);
          j++;
        }
        ఆపు;
      }
      స్థితి TK_SELECT: {
        i_ప్రారంభం_లోపల = 0;
        /* no break */ deliberate_fall_through
      }
      అప్రమేయ: {
        ఒకవేళ( sqlite3IsIdChar(zSql[i]) ) కూడు_Space_Separator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        అయితే( j<pStr->nChar ){
          pStr->z_పాఠ్యం[j] = sqlite3Toupper(pStr->z_పాఠ్యం[j]);
          j++;
        }
        ఆపు;
      }
    }
  }
  ఒకవేళ( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  తిరిగి sqlite3_str_ముగింపు(pStr);
}
#అయితే_ముగింపు /* SQLITE_ENABLE_NORMALIZE */
