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
#समावेश "sqliteInt.h"
#समावेश <stdlib.h>

/* Character classes for tokenizing
**
** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
** using a lookup table, whereas a switch() directly on c uses a binary search.
** The lookup table is much faster.  To maximize speed, and to ensure that
** a lookup table is used, all of the classes need to be small integers and
** all of them need to be used within the switch.
*/
#घोषणा CC_X          0    /* The letter 'x', or start of BLOB literal */
#घोषणा CC_KYWD0      1    /* First letter of a keyword */
#घोषणा CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#घोषणा CC_DIGIT      3    /* Digits */
#घोषणा CC_DOLLAR     4    /* '$' */
#घोषणा CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#घोषणा CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#घोषणा CC_SPACE      7    /* Space characters */
#घोषणा CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#घोषणा CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#घोषणा CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#घोषणा CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#घोषणा CC_LT        12    /* '<'.  Part of < or <= or <> */
#घोषणा CC_GT        13    /* '>'.  Part of > or >= */
#घोषणा CC_EQ        14    /* '='.  Part of = or == */
#घोषणा CC_BANG      15    /* '!'.  Part of != */
#घोषणा CC_SLASH     16    /* '/'.  / or c-style comment */
#घोषणा CC_LP        17    /* '(' */
#घोषणा CC_RP        18    /* ')' */
#घोषणा CC_SEMI      19    /* ';' */
#घोषणा CC_PLUS      20    /* '+' */
#घोषणा CC_STAR      21    /* '*' */
#घोषणा CC_PERCENT   22    /* '%' */
#घोषणा CC_COMMA     23    /* ',' */
#घोषणा CC_AND       24    /* '&' */
#घोषणा CC_TILDA     25    /* '~' */
#घोषणा CC_DOT       26    /* '.' */
#घोषणा CC_ID        27    /* unicode characters usable in IDs */
#घोषणा CC_ILLEGAL   28    /* Illegal character */
#घोषणा CC_NUL       29    /* 0x00 */
#घोषणा CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

अटल स्थिर अचिन्हित अक्षर aiClass[] = {
#अगर_घोषित SQLITE_ASCII
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
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
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
#पूर्ण_अगर
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
#अगर_घोषित SQLITE_ASCII
# define charMap(X) sqlite3UpperToLower[(अचिन्हित अक्षर)X]
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(अचिन्हित अक्षर)X]
स्थिर अचिन्हित अक्षर ebcdicToAscii[] = {
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
#पूर्ण_अगर

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
#समावेश "keywordhash.h"


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
#अगर_घोषित SQLITE_ASCII
#घोषणा IdChar(C)  ((sqlite3CtypeMap[(अचिन्हित अक्षर)C]&0x46)!=0)
#पूर्ण_अगर
#अगर_घोषित SQLITE_EBCDIC
स्थिर अक्षर sqlite3IsEbcdicIdChar[] = {
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
#घोषणा IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#पूर्ण_अगर

/* Make the IdChar function accessible from ctime.c and alter.c */
पूर्णांक sqlite3IsIdChar(u8 c){ वापस IdChar(c); }

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
अटल पूर्णांक getToken(स्थिर अचिन्हित अक्षर **pz){
  स्थिर अचिन्हित अक्षर *z = *pz;
  पूर्णांक t;                          /* Token type to return */
  करो {
    z += sqlite3GetToken(z, &t);
  }जबतक( t==TK_SPACE || t==TK_COMMENT );
  अगर( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_WINDOW 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  वापस t;
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
अटल पूर्णांक analyzeWindowKeyword(स्थिर अचिन्हित अक्षर *z){
  पूर्णांक t;
  t = getToken(&z);
  अगर( t!=TK_ID ) वापस TK_ID;
  t = getToken(&z);
  अगर( t!=TK_AS ) वापस TK_ID;
  वापस TK_WINDOW;
}
अटल पूर्णांक analyzeOverKeyword(स्थिर अचिन्हित अक्षर *z, पूर्णांक lastToken){
  अगर( lastToken==TK_RP ){
    पूर्णांक t = getToken(&z);
    अगर( t==TK_LP || t==TK_ID ) वापस TK_OVER;
  }
  वापस TK_ID;
}
अटल पूर्णांक analyzeFilterKeyword(स्थिर अचिन्हित अक्षर *z, पूर्णांक lastToken){
  अगर( lastToken==TK_RP && getToken(&z)==TK_LP ){
    वापस TK_FILTER;
  }
  वापस TK_ID;
}
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(स्थिर अचिन्हित अक्षर *z, पूर्णांक *tokenType){
  i64 i;
  पूर्णांक c;
  चयन( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    हाल CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      क्रम(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      वापस i;
    }
    हाल CC_MINUS: {
      अगर( z[1]=='-' ){
        क्रम(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        वापस i;
      }अन्यथा अगर( z[1]=='>' ){
        *tokenType = TK_PTR;
        वापस 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      वापस 1;
    }
    हाल CC_LP: {
      *tokenType = TK_LP;
      वापस 1;
    }
    हाल CC_RP: {
      *tokenType = TK_RP;
      वापस 1;
    }
    हाल CC_SEMI: {
      *tokenType = TK_SEMI;
      वापस 1;
    }
    हाल CC_PLUS: {
      *tokenType = TK_PLUS;
      वापस 1;
    }
    हाल CC_STAR: {
      *tokenType = TK_STAR;
      वापस 1;
    }
    हाल CC_SLASH: {
      अगर( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        वापस 1;
      }
      क्रम(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      अगर( c ) i++;
      *tokenType = TK_COMMENT;
      वापस i;
    }
    हाल CC_PERCENT: {
      *tokenType = TK_REM;
      वापस 1;
    }
    हाल CC_EQ: {
      *tokenType = TK_EQ;
      वापस 1 + (z[1]=='=');
    }
    हाल CC_LT: {
      अगर( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        वापस 2;
      }अन्यथा अगर( c=='>' ){
        *tokenType = TK_NE;
        वापस 2;
      }अन्यथा अगर( c=='<' ){
        *tokenType = TK_LSHIFT;
        वापस 2;
      }अन्यथा{
        *tokenType = TK_LT;
        वापस 1;
      }
    }
    हाल CC_GT: {
      अगर( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        वापस 2;
      }अन्यथा अगर( c=='>' ){
        *tokenType = TK_RSHIFT;
        वापस 2;
      }अन्यथा{
        *tokenType = TK_GT;
        वापस 1;
      }
    }
    हाल CC_BANG: {
      अगर( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        वापस 1;
      }अन्यथा{
        *tokenType = TK_NE;
        वापस 2;
      }
    }
    हाल CC_PIPE: {
      अगर( z[1]!='|' ){
        *tokenType = TK_BITOR;
        वापस 1;
      }अन्यथा{
        *tokenType = TK_CONCAT;
        वापस 2;
      }
    }
    हाल CC_COMMA: {
      *tokenType = TK_COMMA;
      वापस 1;
    }
    हाल CC_AND: {
      *tokenType = TK_BITAND;
      वापस 1;
    }
    हाल CC_TILDA: {
      *tokenType = TK_BITNOT;
      वापस 1;
    }
    हाल CC_QUOTE: {
      पूर्णांक delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      क्रम(i=1; (c=z[i])!=0; i++){
        अगर( c==delim ){
          अगर( z[i+1]==delim ){
            i++;
          }अन्यथा{
            अवरोध;
          }
        }
      }
      अगर( c=='\'' ){
        *tokenType = TK_STRING;
        वापस i+1;
      }अन्यथा अगर( c!=0 ){
        *tokenType = TK_ID;
        वापस i+1;
      }अन्यथा{
        *tokenType = TK_ILLEGAL;
        वापस i;
      }
    }
    हाल CC_DOT: {
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
      अगर( !sqlite3Isdigit(z[1]) )
#पूर्ण_अगर
      {
        *tokenType = TK_DOT;
        वापस 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    हाल CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#अगर_अघोषित SQLITE_OMIT_HEX_INTEGER
      अगर( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        क्रम(i=3; 1; i++){
          अगर( sqlite3Isxdigit(z[i])==0 ){
            अगर( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              अवरोध;
            }
          }
        }
      }अन्यथा
#पूर्ण_अगर
        {
        क्रम(i=0; 1; i++){
          अगर( sqlite3Isdigit(z[i])==0 ){
            अगर( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }अन्यथा{
              अवरोध;
            }
          }
        }
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
        अगर( z[i]=='.' ){
          अगर( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          क्रम(i++; 1; i++){
            अगर( sqlite3Isdigit(z[i])==0 ){
              अगर( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                अवरोध;
              }
            }
          }
        }
        अगर( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          अगर( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          क्रम(i+=2; 1; i++){
            अगर( sqlite3Isdigit(z[i])==0 ){
              अगर( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }अन्यथा{
                अवरोध;
              }
            }
          }
        }
#पूर्ण_अगर
      }
      जबतक( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      वापस i;
    }
    हाल CC_QUOTE2: {
      क्रम(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      वापस i;
    }
    हाल CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      क्रम(i=1; sqlite3Isdigit(z[i]); i++){}
      वापस i;
    }
    हाल CC_DOLLAR:
    हाल CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      क्रम(i=1; (c=z[i])!=0; i++){
        अगर( IdChar(c) ){
          n++;
#अगर_अघोषित SQLITE_OMIT_TCL_VARIABLE
        }अन्यथा अगर( c=='(' && n>0 ){
          करो{
            i++;
          }जबतक( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          अगर( c==')' ){
            i++;
          }अन्यथा{
            *tokenType = TK_ILLEGAL;
          }
          अवरोध;
        }अन्यथा अगर( c==':' && z[i+1]==':' ){
          i++;
#पूर्ण_अगर
        }अन्यथा{
          अवरोध;
        }
      }
      अगर( n==0 ) *tokenType = TK_ILLEGAL;
      वापस i;
    }
    हाल CC_KYWD0: {
      अगर( aiClass[z[1]]>CC_KYWD ){ i = 1;  अवरोध; }
      क्रम(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      अगर( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        अवरोध;
      }
      *tokenType = TK_ID;
      वापस keywordCode((अक्षर*)z, i, tokenType);
    }
    हाल CC_X: {
#अगर_अघोषित SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      अगर( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        क्रम(i=2; sqlite3Isxdigit(z[i]); i++){}
        अगर( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          जबतक( z[i] && z[i]!='\'' ){ i++; }
        }
        अगर( z[i] ) i++;
        वापस i;
      }
#पूर्ण_अगर
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    हाल CC_KYWD:
    हाल CC_ID: {
      i = 1;
      अवरोध;
    }
    हाल CC_BOM: {
      अगर( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        वापस 3;
      }
      i = 1;
      अवरोध;
    }
    हाल CC_NUL: {
      *tokenType = TK_ILLEGAL;
      वापस 0;
    }
    शेष: {
      *tokenType = TK_ILLEGAL;
      वापस 1;
    }
  }
  जबतक( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  वापस i;
}

/*
** Run the parser on the given SQL string.
*/
पूर्णांक sqlite3RunParser(Parse *pParse, स्थिर अक्षर *zSql){
  पूर्णांक nErr = 0;                   /* Number of errors encountered */
  व्योम *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  पूर्णांक tokenType;                  /* type of the next token */
  पूर्णांक lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *pParentParse = 0;        /* Outer parse context, if any */
#अगर_घोषित sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#पूर्ण_अगर
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  निश्चित( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_लंबाई];
  अगर( db->nVdbeActive==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->z_पुच्छ = zSql;
#अगर_घोषित SQLITE_DEBUG
  अगर( db->flags & SQLITE_ParserTrace ){
    म_लिखो("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(मानक_निकास, "parser: ");
  }अन्यथा{
    sqlite3ParserTrace(0, 0);
  }
#पूर्ण_अगर
#अगर_घोषित sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#अन्यथा
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  अगर( pEngine==0 ){
    sqlite3OomFault(db);
    वापस SQLITE_NOMEM_BKPT;
  }
#पूर्ण_अगर
  निश्चित( pParse->pNewTable==0 );
  निश्चित( pParse->pNewTrigger==0 );
  निश्चित( pParse->nVar==0 );
  निश्चित( pParse->pVList==0 );
  pParentParse = db->pParse;
  db->pParse = pParse;
  जबतक( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    अगर( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
    अगर( tokenType>=TK_WINDOW ){
      निश्चित( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_WINDOW 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#अन्यथा
    अगर( tokenType>=TK_SPACE ){
      निश्चित( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */
      अगर( AtomicLoad(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        अवरोध;
      }
      अगर( tokenType==TK_SPACE ){
        zSql += n;
        जारी;
      }
      अगर( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        अगर( lastTokenParsed==TK_SEMI ){
          tokenType = 0;
        }अन्यथा अगर( lastTokenParsed==0 ){
          अवरोध;
        }अन्यथा{
          tokenType = TK_SEMI;
        }
        n = 0;
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      }अन्यथा अगर( tokenType==TK_WINDOW ){
        निश्चित( n==6 );
        tokenType = analyzeWindowKeyword((स्थिर u8*)&zSql[6]);
      }अन्यथा अगर( tokenType==TK_OVER ){
        निश्चित( n==4 );
        tokenType = analyzeOverKeyword((स्थिर u8*)&zSql[4], lastTokenParsed);
      }अन्यथा अगर( tokenType==TK_FILTER ){
        निश्चित( n==6 );
        tokenType = analyzeFilterKeyword((स्थिर u8*)&zSql[6], lastTokenParsed);
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */
      }अन्यथा अगर( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        जारी;
      }अन्यथा अगर( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_संदेश(pParse, "unrecognized token: \"%T\"", &x);
        अवरोध;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;
    निश्चित( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    अगर( pParse->rc!=SQLITE_OK ) अवरोध;
  }
  निश्चित( nErr==0 );
#अगर_घोषित YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_ढेर,
      sqlite3_Parser_ढेर_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#पूर्ण_अगर /* YYDEBUG */
#अगर_घोषित sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#अन्यथा
  sqlite3ParserFree(pEngine, sqlite3_free);
#पूर्ण_अगर
  अगर( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  अगर( pParse->z_Err_संदेश || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE) ){
    अगर( pParse->z_Err_संदेश==0 ){
      pParse->z_Err_संदेश = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    अगर( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_संदेश, pParse->z_पुच्छ);
    }
    nErr++;
  }
  pParse->z_पुच्छ = zSql;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#पूर्ण_अगर

  अगर( pParse->pNewTable && !IN_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  अगर( pParse->pNewTrigger && !IN_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  अगर( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;
  निश्चित( nErr==0 || pParse->rc!=SQLITE_OK );
  वापस nErr;
}


#अगर_घोषित SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
अटल व्योम addSpaceSeparator(sqlite3_str *pStr){
  अगर( pStr->nChar && sqlite3IsIdChar(pStr->zText[pStr->nChar-1]) ){
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
  स्थिर अक्षर *zSql   /* The original SQL string */
){
  sqlite3 *db;       /* The database connection */
  पूर्णांक i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  पूर्णांक tokenType;     /* type of current token */
  पूर्णांक पूर्व_Type = 0;  /* Previous non-whitespace token */
  पूर्णांक nParen;        /* Number of nested levels of parentheses */
  पूर्णांक iStartIN;      /* Start of RHS of IN operator in z[] */
  पूर्णांक nParenAtIN;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = iStartIN = nParenAtIN = 0;
  pStr = sqlite3_str_new(db);
  निश्चित( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  क्रम(i=0; zSql[i] && pStr->accError==0; i+=n){
    अगर( tokenType!=TK_SPACE ){
      पूर्व_Type = tokenType;
    }
    n = sqlite3GetToken((अचिन्हित अक्षर*)zSql+i, &tokenType);
    अगर( NEVER(n<=0) ) अवरोध;
    चयन( tokenType ){
      हाल TK_COMMENT:
      हाल TK_SPACE: {
        अवरोध;
      }
      हाल TK_NULL: {
        अगर( पूर्व_Type==TK_IS || पूर्व_Type==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          अवरोध;
        }
        /* no break */ deliberate_fall_through
      }
      हाल TK_STRING:
      हाल TK_INTEGER:
      हाल TK_FLOAT:
      हाल TK_VARIABLE:
      हाल TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        अवरोध;
      }
      हाल TK_LP: {
        nParen++;
        अगर( पूर्व_Type==TK_IN ){
          iStartIN = pStr->nChar;
          nParenAtIN = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        अवरोध;
      }
      हाल TK_RP: {
        अगर( iStartIN>0 && nParen==nParenAtIN ){
          निश्चित( pStr->nChar>=(u32)iStartIN );
          pStr->nChar = iStartIN+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          iStartIN = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        अवरोध;
      }
      हाल TK_ID: {
        iStartIN = 0;
        j = pStr->nChar;
        अगर( sqlite3Isquote(zSql[i]) ){
          अक्षर *zId = sqlite3DbStrNDup(db, zSql+i, n);
          पूर्णांक nId;
          पूर्णांक eType = 0;
          अगर( zId==0 ) अवरोध;
          sqlite3Dequote(zId);
          अगर( zSql[i]=='"' && sqlite3VdbeUsesDoubleQuotedString(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            अवरोध;
          }
          nId = sqlite3Strlen30(zId);
          अगर( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
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
        जबतक( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Tolower(pStr->zText[j]);
          j++;
        }
        अवरोध;
      }
      हाल TK_SELECT: {
        iStartIN = 0;
        /* no break */ deliberate_fall_through
      }
      शेष: {
        अगर( sqlite3IsIdChar(zSql[i]) ) addSpaceSeparator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        जबतक( j<pStr->nChar ){
          pStr->zText[j] = sqlite3Toupper(pStr->zText[j]);
          j++;
        }
        अवरोध;
      }
    }
  }
  अगर( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  वापस sqlite3_str_finish(pStr);
}
#पूर्ण_अगर /* SQLITE_ENABLE_NORMALIZE */
