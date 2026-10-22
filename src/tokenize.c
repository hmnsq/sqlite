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
#परिभाषय CC_X          0    /* The letter 'x', or start of BLOB literal */
#परिभाषय CC_KYWD0      1    /* First letter of a keyword */
#परिभाषय CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#परिभाषय CC_DIGIT      3    /* Digits */
#परिभाषय CC_DOLLAR     4    /* '$' */
#परिभाषय CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#परिभाषय CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#परिभाषय CC_SPACE      7    /* Space characters */
#परिभाषय CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#परिभाषय CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#परिभाषय CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#परिभाषय CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#परिभाषय CC_LT        12    /* '<'.  Part of < or <= or <> */
#परिभाषय CC_GT        13    /* '>'.  Part of > or >= */
#परिभाषय CC_EQ        14    /* '='.  Part of = or == */
#परिभाषय CC_BANG      15    /* '!'.  Part of != */
#परिभाषय CC_SLASH     16    /* '/'.  / or c-style comment */
#परिभाषय CC_LP        17    /* '(' */
#परिभाषय CC_RP        18    /* ')' */
#परिभाषय CC_SEMI      19    /* ';' */
#परिभाषय CC_PLUS      20    /* '+' */
#परिभाषय CC_तारकम्      21    /* '*' */
#परिभाषय CC_PERCENT   22    /* '%' */
#परिभाषय CC_COMMA     23    /* ',' */
#परिभाषय CC_AND       24    /* '&' */
#परिभाषय CC_TILDA     25    /* '~' */
#परिभाषय CC_DOT       26    /* '.' */
#परिभाषय CC_ID        27    /* unicode characters usable in IDs */
#परिभाषय CC_ILLEGAL   28    /* Illegal character */
#परिभाषय CC_NUL       29    /* 0x00 */
#परिभाषय CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

स्थिर ध्रुव अचिह्नित वर्ण aiClass[] = {
#यदि_परिभाषितम् SQLITE_ASCII
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
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_EBCDIC
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
#यदि_अन्तः
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
#यदि_परिभाषितम् SQLITE_ASCII
# define charMap(X) sqlite3_ऊर्ध्वस्थम्_गन्तव्यम्_अधस्तनम्[(अचिह्नित वर्ण)X]
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_EBCDIC
# define charMap(X) ebcdic_गन्तव्यम्_Ascii[(अचिह्नित वर्ण)X]
ध्रुव अचिह्नित वर्ण ebcdic_गन्तव्यम्_Ascii[] = {
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
#यदि_अन्तः

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
#यदि_परिभाषितम् SQLITE_ASCII
#परिभाषय IdChar(C)  ((sqlite3CtypeMap[(अचिह्नित वर्ण)C]&0x46)!=0)
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_EBCDIC
ध्रुव वर्ण sqlite3IsEbcdicIdChar[] = {
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
#परिभाषय IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#यदि_अन्तः

/* Make the IdChar function accessible from ctime.c and alter.c */
पूर्णाङ्क sqlite3IsIdChar(u8 c){ प्रत्यागम IdChar(c); }

#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
स्थिर पूर्णाङ्क getToken(ध्रुव अचिह्नित वर्ण **pz){
  ध्रुव अचिह्नित वर्ण *z = *pz;
  पूर्णाङ्क t;                          /* Token type to return */
  कुरु {
    z += sqlite3GetToken(z, &t);
  }यावत्( t==TK_SPACE || t==TK_COMMENT );
  यदि( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_गवाक्षः 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  प्रत्यागम t;
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
स्थिर पूर्णाङ्क analyze_गवाक्षः_Keyword(ध्रुव अचिह्नित वर्ण *z){
  पूर्णाङ्क t;
  t = getToken(&z);
  यदि( t!=TK_ID ) प्रत्यागम TK_ID;
  t = getToken(&z);
  यदि( t!=TK_AS ) प्रत्यागम TK_ID;
  प्रत्यागम TK_गवाक्षः;
}
स्थिर पूर्णाङ्क analyzeOverKeyword(ध्रुव अचिह्नित वर्ण *z, पूर्णाङ्क चरमः_Token){
  यदि( चरमः_Token==TK_RP ){
    पूर्णाङ्क t = getToken(&z);
    यदि( t==TK_LP || t==TK_ID ) प्रत्यागम TK_OVER;
  }
  प्रत्यागम TK_ID;
}
स्थिर पूर्णाङ्क analyzeFilterKeyword(ध्रुव अचिह्नित वर्ण *z, पूर्णाङ्क चरमः_Token){
  यदि( चरमः_Token==TK_RP && getToken(&z)==TK_LP ){
    प्रत्यागम TK_FILTER;
  }
  प्रत्यागम TK_ID;
}
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ध्रुव अचिह्नित वर्ण *z, पूर्णाङ्क *tokenType){
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
      प्रति(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      प्रत्यागम i;
    }
    पक्ष CC_MINUS: {
      यदि( z[1]=='-' ){
        प्रति(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        प्रत्यागम i;
      }अन्यथा यदि( z[1]=='>' ){
        *tokenType = TK_PTR;
        प्रत्यागम 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      प्रत्यागम 1;
    }
    पक्ष CC_LP: {
      *tokenType = TK_LP;
      प्रत्यागम 1;
    }
    पक्ष CC_RP: {
      *tokenType = TK_RP;
      प्रत्यागम 1;
    }
    पक्ष CC_SEMI: {
      *tokenType = TK_SEMI;
      प्रत्यागम 1;
    }
    पक्ष CC_PLUS: {
      *tokenType = TK_PLUS;
      प्रत्यागम 1;
    }
    पक्ष CC_तारकम्: {
      *tokenType = TK_तारकम्;
      प्रत्यागम 1;
    }
    पक्ष CC_SLASH: {
      यदि( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        प्रत्यागम 1;
      }
      प्रति(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      यदि( c ) i++;
      *tokenType = TK_COMMENT;
      प्रत्यागम i;
    }
    पक्ष CC_PERCENT: {
      *tokenType = TK_REM;
      प्रत्यागम 1;
    }
    पक्ष CC_EQ: {
      *tokenType = TK_EQ;
      प्रत्यागम 1 + (z[1]=='=');
    }
    पक्ष CC_LT: {
      यदि( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        प्रत्यागम 2;
      }अन्यथा यदि( c=='>' ){
        *tokenType = TK_NE;
        प्रत्यागम 2;
      }अन्यथा यदि( c=='<' ){
        *tokenType = TK_LSHIFT;
        प्रत्यागम 2;
      }अन्यथा{
        *tokenType = TK_LT;
        प्रत्यागम 1;
      }
    }
    पक्ष CC_GT: {
      यदि( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        प्रत्यागम 2;
      }अन्यथा यदि( c=='>' ){
        *tokenType = TK_RSHIFT;
        प्रत्यागम 2;
      }अन्यथा{
        *tokenType = TK_GT;
        प्रत्यागम 1;
      }
    }
    पक्ष CC_BANG: {
      यदि( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        प्रत्यागम 1;
      }अन्यथा{
        *tokenType = TK_NE;
        प्रत्यागम 2;
      }
    }
    पक्ष CC_PIPE: {
      यदि( z[1]!='|' ){
        *tokenType = TK_BITOR;
        प्रत्यागम 1;
      }अन्यथा{
        *tokenType = TK_CONCAT;
        प्रत्यागम 2;
      }
    }
    पक्ष CC_COMMA: {
      *tokenType = TK_COMMA;
      प्रत्यागम 1;
    }
    पक्ष CC_AND: {
      *tokenType = TK_BITAND;
      प्रत्यागम 1;
    }
    पक्ष CC_TILDA: {
      *tokenType = TK_BITNOT;
      प्रत्यागम 1;
    }
    पक्ष CC_QUOTE: {
      पूर्णाङ्क delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      प्रति(i=1; (c=z[i])!=0; i++){
        यदि( c==delim ){
          यदि( z[i+1]==delim ){
            i++;
          }अन्यथा{
            भङ्ग;
          }
        }
      }
      यदि( c=='\'' ){
        *tokenType = TK_STRING;
        प्रत्यागम i+1;
      }अन्यथा यदि( c!=0 ){
        *tokenType = TK_ID;
        प्रत्यागम i+1;
      }अन्यथा{
        *tokenType = TK_ILLEGAL;
        प्रत्यागम i;
      }
    }
    पक्ष CC_DOT: {
#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
      यदि( !sqlite3Isdigit(z[1]) )
#यदि_अन्तः
      {
        *tokenType = TK_DOT;
        प्रत्यागम 1;
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
#यदि_अपरिभाषितम् SQLITE_OMIT_HEX_INTEGER
      यदि( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        प्रति(i=3; 1; i++){
          यदि( sqlite3Isxdigit(z[i])==0 ){
            यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              भङ्ग;
            }
          }
        }
      }अन्यथा
#यदि_अन्तः
        {
        प्रति(i=0; 1; i++){
          यदि( sqlite3Isdigit(z[i])==0 ){
            यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              भङ्ग;
            }
          }
        }
#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
        यदि( z[i]=='.' ){
          यदि( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          प्रति(i++; 1; i++){
            यदि( sqlite3Isdigit(z[i])==0 ){
              यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                भङ्ग;
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
          प्रति(i+=2; 1; i++){
            यदि( sqlite3Isdigit(z[i])==0 ){
              यदि( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                भङ्ग;
              }
            }
          }
        }
#यदि_अन्तः
      }
      यावत्( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      प्रत्यागम i;
    }
    पक्ष CC_QUOTE2: {
      प्रति(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      प्रत्यागम i;
    }
    पक्ष CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      प्रति(i=1; sqlite3Isdigit(z[i]); i++){}
      प्रत्यागम i;
    }
    पक्ष CC_DOLLAR:
    पक्ष CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      प्रति(i=1; (c=z[i])!=0; i++){
        यदि( IdChar(c) ){
          n++;
#यदि_अपरिभाषितम् SQLITE_OMIT_TCL_VARIABLE
        }अन्यथा यदि( c=='(' && n>0 ){
          कुरु{
            i++;
          }यावत्( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          यदि( c==')' ){
            i++;
          }अन्यथा{
            *tokenType = TK_ILLEGAL;
          }
          भङ्ग;
        }अन्यथा यदि( c==':' && z[i+1]==':' ){
          i++;
#यदि_अन्तः
        }अन्यथा{
          भङ्ग;
        }
      }
      यदि( n==0 ) *tokenType = TK_ILLEGAL;
      प्रत्यागम i;
    }
    पक्ष CC_KYWD0: {
      यदि( aiClass[z[1]]>CC_KYWD ){ i = 1;  भङ्ग; }
      प्रति(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      यदि( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        भङ्ग;
      }
      *tokenType = TK_ID;
      प्रत्यागम keyword_संहिता((वर्ण*)z, i, tokenType);
    }
    पक्ष CC_X: {
#यदि_अपरिभाषितम् SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      यदि( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        प्रति(i=2; sqlite3Isxdigit(z[i]); i++){}
        यदि( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          यावत्( z[i] && z[i]!='\'' ){ i++; }
        }
        यदि( z[i] ) i++;
        प्रत्यागम i;
      }
#यदि_अन्तः
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    पक्ष CC_KYWD:
    पक्ष CC_ID: {
      i = 1;
      भङ्ग;
    }
    पक्ष CC_BOM: {
      यदि( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        प्रत्यागम 3;
      }
      i = 1;
      भङ्ग;
    }
    पक्ष CC_NUL: {
      *tokenType = TK_ILLEGAL;
      प्रत्यागम 0;
    }
    स्वभाव: {
      *tokenType = TK_ILLEGAL;
      प्रत्यागम 1;
    }
  }
  यावत्( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  प्रत्यागम i;
}

/*
** Run the parser on the given SQL string.
*/
पूर्णाङ्क sqlite3_सततम्_Parser(Parse *pParse, ध्रुव वर्ण *zSql){
  पूर्णाङ्क nErr = 0;                   /* Number of errors encountered */
  शून्य *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  पूर्णाङ्क tokenType;                  /* type of the next token */
  पूर्णाङ्क चरमः_Token_Parsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#यदि_परिभाषितम् sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#यदि_अन्तः
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  प्रतिज्ञा( zSql!=0 );
  mxSqlLen = db->a_सीमा[SQLITE_सीमा_SQL_दीर्घता];
  यदि( db->n_Vdbe_सक्रिय==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->z_पुच्छ = zSql;
#यदि_परिभाषितम् SQLITE_DEBUG
  यदि( db->flags & SQLITE_ParserTrace ){
    प्रारूप_लिख("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }अन्यथा{
    sqlite3ParserTrace(0, 0);
  }
#यदि_अन्तः
#यदि_परिभाषितम् sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#अन्यथा
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  यदि( pEngine==0 ){
    sqlite3OomFault(db);
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
#यदि_अन्तः
  प्रतिज्ञा( pParse->p_New_सारणी==0 );
  प्रतिज्ञा( pParse->pNewTrigger==0 );
  प्रतिज्ञा( pParse->nVar==0 );
  प्रतिज्ञा( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  यावत्( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    यदि( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      भङ्ग;
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( tokenType>=TK_गवाक्षः ){
      प्रतिज्ञा( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_गवाक्षः 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#अन्यथा
    यदि( tokenType>=TK_SPACE ){
      प्रतिज्ञा( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
      यदि( Atomic_आरोपय(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_व्यवधानम्;
        pParse->nErr++;
        भङ्ग;
      }
      यदि( tokenType==TK_SPACE ){
        zSql += n;
        अनुवर्त;
      }
      यदि( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        यदि( चरमः_Token_Parsed==TK_SEMI ){
          tokenType = 0;
        }अन्यथा यदि( चरमः_Token_Parsed==0 ){
          भङ्ग;
        }अन्यथा{
          tokenType = TK_SEMI;
        }
        n = 0;
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
      }अन्यथा यदि( tokenType==TK_गवाक्षः ){
        प्रतिज्ञा( n==6 );
        tokenType = analyze_गवाक्षः_Keyword((ध्रुव u8*)&zSql[6]);
      }अन्यथा यदि( tokenType==TK_OVER ){
        प्रतिज्ञा( n==4 );
        tokenType = analyzeOverKeyword((ध्रुव u8*)&zSql[4], चरमः_Token_Parsed);
      }अन्यथा यदि( tokenType==TK_FILTER ){
        प्रतिज्ञा( n==6 );
        tokenType = analyzeFilterKeyword((ध्रुव u8*)&zSql[6], चरमः_Token_Parsed);
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
      }अन्यथा यदि( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        अनुवर्त;
      }अन्यथा यदि( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_सन्देशः(pParse, "unrecognized token: \"%T\"", &x);
        भङ्ग;
      }
    }
    pParse->s_चरमः_Token.z = zSql;
    pParse->s_चरमः_Token.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->s_चरमः_Token);
    चरमः_Token_Parsed = tokenType;
    zSql += n;
    प्रतिज्ञा( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    यदि( pParse->rc!=SQLITE_OK ) भङ्ग;
  }
  प्रतिज्ञा( nErr==0 );
#यदि_परिभाषितम् YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3_स्थितिः_Highwater(SQLITE_स्थितिः_PARSER_राशिः,
      sqlite3_Parser_राशिः_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#यदि_अन्तः /* YYDEBUG */
#यदि_परिभाषितम् sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#अन्यथा
  sqlite3ParserFree(pEngine, sqlite3_free);
#यदि_अन्तः
  यदि( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  यदि( pParse->z_Err_सन्देशः || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_समाप्तम्) ){
    यदि( pParse->z_Err_सन्देशः==0 ){
      pParse->z_Err_सन्देशः = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    यदि( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_सन्देशः, pParse->z_पुच्छ);
    }
    nErr++;
  }
  pParse->z_पुच्छ = zSql;
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->ap_Vtab_तालकम्);
#यदि_अन्तः

  यदि( pParse->p_New_सारणी && !अन्तर्_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3_Delete_सारणी(db, pParse->p_New_सारणी);
  }
  यदि( pParse->pNewTrigger && !अन्तर्_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  यदि( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  प्रतिज्ञा( nErr==0 || pParse->rc!=SQLITE_OK );
  प्रत्यागम nErr;
}


#यदि_परिभाषितम् SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
स्थिर शून्य सङ्कलय_Space_Separator(sqlite3_str *pStr){
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
  ध्रुव वर्ण *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  पूर्णाङ्क i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  पूर्णाङ्क tokenType;     /* type of current token */
  पूर्णाङ्क पूर्व_Type = 0;  /* Previous non-whitespace token */
  पूर्णाङ्क nParen;        /* Number of nested levels of parentheses */
  पूर्णाङ्क i_Start_अन्तर्;      /* Start of RHS of IN operator in z[] */
  पूर्णाङ्क n_Paren_At_अन्तर्;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = i_Start_अन्तर् = n_Paren_At_अन्तर् = 0;
  pStr = sqlite3_str_new(db);
  प्रतिज्ञा( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  प्रति(i=0; zSql[i] && pStr->accError==0; i+=n){
    यदि( tokenType!=TK_SPACE ){
      पूर्व_Type = tokenType;
    }
    n = sqlite3GetToken((अचिह्नित वर्ण*)zSql+i, &tokenType);
    यदि( NEVER(n<=0) ) भङ्ग;
    विकल्प( tokenType ){
      पक्ष TK_COMMENT:
      पक्ष TK_SPACE: {
        भङ्ग;
      }
      पक्ष TK_NULL: {
        यदि( पूर्व_Type==TK_IS || पूर्व_Type==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          भङ्ग;
        }
        /* no break */ deliberate_fall_through
      }
      पक्ष TK_STRING:
      पक्ष TK_INTEGER:
      पक्ष TK_FLOAT:
      पक्ष TK_VARIABLE:
      पक्ष TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        भङ्ग;
      }
      पक्ष TK_LP: {
        nParen++;
        यदि( पूर्व_Type==TK_अन्तर् ){
          i_Start_अन्तर् = pStr->nChar;
          n_Paren_At_अन्तर् = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        भङ्ग;
      }
      पक्ष TK_RP: {
        यदि( i_Start_अन्तर्>0 && nParen==n_Paren_At_अन्तर् ){
          प्रतिज्ञा( pStr->nChar>=(u32)i_Start_अन्तर् );
          pStr->nChar = i_Start_अन्तर्+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          i_Start_अन्तर् = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        भङ्ग;
      }
      पक्ष TK_ID: {
        i_Start_अन्तर् = 0;
        j = pStr->nChar;
        यदि( sqlite3Isquote(zSql[i]) ){
          वर्ण *zId = sqlite3DbStrNDup(db, zSql+i, n);
          पूर्णाङ्क nId;
          पूर्णाङ्क eType = 0;
          यदि( zId==0 ) भङ्ग;
          sqlite3Dequote(zId);
          यदि( zSql[i]=='"' && sqlite3_Vdbe_उपयुङ्क्ते_Double_Quoted_String(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            भङ्ग;
          }
          nId = sqlite3Strlen30(zId);
          यदि( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            सङ्कलय_Space_Separator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }अन्यथा{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }अन्यथा{
          सङ्कलय_Space_Separator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        यावत्( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        भङ्ग;
      }
      पक्ष TK_SELECT: {
        i_Start_अन्तर् = 0;
        /* no break */ deliberate_fall_through
      }
      स्वभाव: {
        यदि( sqlite3IsIdChar(zSql[i]) ) सङ्कलय_Space_Separator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        यावत्( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        भङ्ग;
      }
    }
  }
  यदि( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  प्रत्यागम sqlite3_str_finish(pStr);
}
#यदि_अन्तः /* SQLITE_ENABLE_NORMALIZE */
