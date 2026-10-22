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
#شامل "sqliteInt.h"
#شامل <stdlib.h>

/* Character classes for tokenizing
**
** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
** using a lookup table, whereas a switch() directly on c uses a binary search.
** The lookup table is much faster.  To maximize speed, and to ensure that
** a lookup table is used, all of the classes need to be small integers and
** all of them need to be used within the switch.
*/
#تعریف_کرو CC_X          0    /* The letter 'x', or start of BLOB literal */
#تعریف_کرو CC_KYWD0      1    /* First letter of a keyword */
#تعریف_کرو CC_KYWD       2    /* Alphabetics or '_'.  Usable in a keyword */
#تعریف_کرو CC_DIGIT      3    /* Digits */
#تعریف_کرو CC_DOLLAR     4    /* '$' */
#تعریف_کرو CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#تعریف_کرو CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#تعریف_کرو CC_SPACE      7    /* Space characters */
#تعریف_کرو CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#تعریف_کرو CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#تعریف_کرو CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#تعریف_کرو CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#تعریف_کرو CC_LT        12    /* '<'.  Part of < or <= or <> */
#تعریف_کرو CC_GT        13    /* '>'.  Part of > or >= */
#تعریف_کرو CC_EQ        14    /* '='.  Part of = or == */
#تعریف_کرو CC_BANG      15    /* '!'.  Part of != */
#تعریف_کرو CC_SLASH     16    /* '/'.  / or c-style comment */
#تعریف_کرو CC_LP        17    /* '(' */
#تعریف_کرو CC_RP        18    /* ')' */
#تعریف_کرو CC_SEMI      19    /* ';' */
#تعریف_کرو CC_PLUS      20    /* '+' */
#تعریف_کرو CC_ستارہ      21    /* '*' */
#تعریف_کرو CC_PERCENT   22    /* '%' */
#تعریف_کرو CC_COMMA     23    /* ',' */
#تعریف_کرو CC_AND       24    /* '&' */
#تعریف_کرو CC_TILDA     25    /* '~' */
#تعریف_کرو CC_DOT       26    /* '.' */
#تعریف_کرو CC_ID        27    /* unicode characters usable in IDs */
#تعریف_کرو CC_ILLEGAL   28    /* Illegal character */
#تعریف_کرو CC_NUL       29    /* 0x00 */
#تعریف_کرو CC_BOM       30    /* First byte of UTF8 BOM:  0xEF 0xBB 0xBF */

ساکن ثابت غیرعلامتی حرف aiClass[] = {
#اگر_تعریف_شدہ SQLITE_ASCII
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
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
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
#اگر_ختم
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
#اگر_تعریف_شدہ SQLITE_ASCII
# define charMap(X) sqlite3_بالائی_مقصد_زیریں[(غیرعلامتی حرف)X]
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
# define charMap(X) ebcdic_مقصد_Ascii[(غیرعلامتی حرف)X]
ثابت غیرعلامتی حرف ebcdic_مقصد_Ascii[] = {
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
#اگر_ختم

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
#شامل "keywordhash.h"


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
#اگر_تعریف_شدہ SQLITE_ASCII
#تعریف_کرو IdChar(C)  ((sqlite3CtypeMap[(غیرعلامتی حرف)C]&0x46)!=0)
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_EBCDIC
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
#تعریف_کرو IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#اگر_ختم

/* Make the IdChar function accessible from ctime.c and alter.c */
صحیح sqlite3IsIdChar(u8 c){ واپس IdChar(c); }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** Return the id of the next token in string (*pz). Before returning, set
** (*pz) to point to the byte following the parsed token.
*/
ساکن صحیح getToken(ثابت غیرعلامتی حرف **pz){
  ثابت غیرعلامتی حرف *z = *pz;
  صحیح t;                          /* Token type to return */
  کرو {
    z += sqlite3GetToken(z, &t);
  }جب( t==TK_SPACE || t==TK_COMMENT );
  اگر( t==TK_ID 
   || t==TK_STRING 
   || t==TK_JOIN_KW 
   || t==TK_کھڑکی 
   || t==TK_OVER 
   || sqlite3ParserFallback(t)==TK_ID 
  ){
    t = TK_ID;
  }
  *pz = z;
  واپس t;
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
ساکن صحیح analyze_کھڑکی_Keyword(ثابت غیرعلامتی حرف *z){
  صحیح t;
  t = getToken(&z);
  اگر( t!=TK_ID ) واپس TK_ID;
  t = getToken(&z);
  اگر( t!=TK_AS ) واپس TK_ID;
  واپس TK_کھڑکی;
}
ساکن صحیح analyzeOverKeyword(ثابت غیرعلامتی حرف *z, صحیح آخری_Token){
  اگر( آخری_Token==TK_RP ){
    صحیح t = getToken(&z);
    اگر( t==TK_LP || t==TK_ID ) واپس TK_OVER;
  }
  واپس TK_ID;
}
ساکن صحیح analyzeFilterKeyword(ثابت غیرعلامتی حرف *z, صحیح آخری_Token){
  اگر( آخری_Token==TK_RP && getToken(&z)==TK_LP ){
    واپس TK_FILTER;
  }
  واپس TK_ID;
}
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
i64 sqlite3GetToken(ثابت غیرعلامتی حرف *z, صحیح *tokenType){
  i64 i;
  صحیح c;
  انتخاب( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    حالت CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      برائے(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      واپس i;
    }
    حالت CC_MINUS: {
      اگر( z[1]=='-' ){
        برائے(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_COMMENT;
        واپس i;
      }ورنہ اگر( z[1]=='>' ){
        *tokenType = TK_PTR;
        واپس 2 + (z[2]=='>');
      }
      *tokenType = TK_MINUS;
      واپس 1;
    }
    حالت CC_LP: {
      *tokenType = TK_LP;
      واپس 1;
    }
    حالت CC_RP: {
      *tokenType = TK_RP;
      واپس 1;
    }
    حالت CC_SEMI: {
      *tokenType = TK_SEMI;
      واپس 1;
    }
    حالت CC_PLUS: {
      *tokenType = TK_PLUS;
      واپس 1;
    }
    حالت CC_ستارہ: {
      *tokenType = TK_ستارہ;
      واپس 1;
    }
    حالت CC_SLASH: {
      اگر( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        واپس 1;
      }
      برائے(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      اگر( c ) i++;
      *tokenType = TK_COMMENT;
      واپس i;
    }
    حالت CC_PERCENT: {
      *tokenType = TK_REM;
      واپس 1;
    }
    حالت CC_EQ: {
      *tokenType = TK_EQ;
      واپس 1 + (z[1]=='=');
    }
    حالت CC_LT: {
      اگر( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        واپس 2;
      }ورنہ اگر( c=='>' ){
        *tokenType = TK_NE;
        واپس 2;
      }ورنہ اگر( c=='<' ){
        *tokenType = TK_LSHIFT;
        واپس 2;
      }ورنہ{
        *tokenType = TK_LT;
        واپس 1;
      }
    }
    حالت CC_GT: {
      اگر( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        واپس 2;
      }ورنہ اگر( c=='>' ){
        *tokenType = TK_RSHIFT;
        واپس 2;
      }ورنہ{
        *tokenType = TK_GT;
        واپس 1;
      }
    }
    حالت CC_BANG: {
      اگر( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        واپس 1;
      }ورنہ{
        *tokenType = TK_NE;
        واپس 2;
      }
    }
    حالت CC_PIPE: {
      اگر( z[1]!='|' ){
        *tokenType = TK_BITOR;
        واپس 1;
      }ورنہ{
        *tokenType = TK_CONCAT;
        واپس 2;
      }
    }
    حالت CC_COMMA: {
      *tokenType = TK_COMMA;
      واپس 1;
    }
    حالت CC_AND: {
      *tokenType = TK_BITAND;
      واپس 1;
    }
    حالت CC_TILDA: {
      *tokenType = TK_BITNOT;
      واپس 1;
    }
    حالت CC_QUOTE: {
      صحیح delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      برائے(i=1; (c=z[i])!=0; i++){
        اگر( c==delim ){
          اگر( z[i+1]==delim ){
            i++;
          }ورنہ{
            توڑو;
          }
        }
      }
      اگر( c=='\'' ){
        *tokenType = TK_STRING;
        واپس i+1;
      }ورنہ اگر( c!=0 ){
        *tokenType = TK_ID;
        واپس i+1;
      }ورنہ{
        *tokenType = TK_ILLEGAL;
        واپس i;
      }
    }
    حالت CC_DOT: {
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
      اگر( !sqlite3Isdigit(z[1]) )
#اگر_ختم
      {
        *tokenType = TK_DOT;
        واپس 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
      /* no break */ deliberate_fall_through
    }
    حالت CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );  testcase( z[0]=='.' );
      *tokenType = TK_INTEGER;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_HEX_INTEGER
      اگر( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        برائے(i=3; 1; i++){
          اگر( sqlite3Isxdigit(z[i])==0 ){
            اگر( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ورنہ{
              توڑو;
            }
          }
        }
      }ورنہ
#اگر_ختم
        {
        برائے(i=0; 1; i++){
          اگر( sqlite3Isdigit(z[i])==0 ){
            اگر( z[i]==SQLITE_DIGIT_SEPARATOR ){
              *tokenType = TK_QNUMBER;
            }ورنہ{
              توڑو;
            }
          }
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
        اگر( z[i]=='.' ){
          اگر( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          برائے(i++; 1; i++){
            اگر( sqlite3Isdigit(z[i])==0 ){
              اگر( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ورنہ{
                توڑو;
              }
            }
          }
        }
        اگر( (z[i]=='e' || z[i]=='E') &&
             ( sqlite3Isdigit(z[i+1]) 
              || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
             )
        ){
          اگر( *tokenType==TK_INTEGER ) *tokenType = TK_FLOAT;
          برائے(i+=2; 1; i++){
            اگر( sqlite3Isdigit(z[i])==0 ){
              اگر( z[i]==SQLITE_DIGIT_SEPARATOR ){
                *tokenType = TK_QNUMBER;
              }ورنہ{
                توڑو;
              }
            }
          }
        }
#اگر_ختم
      }
      جب( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      واپس i;
    }
    حالت CC_QUOTE2: {
      برائے(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      واپس i;
    }
    حالت CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      برائے(i=1; sqlite3Isdigit(z[i]); i++){}
      واپس i;
    }
    حالت CC_DOLLAR:
    حالت CC_VARALPHA: {
      i64 n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      برائے(i=1; (c=z[i])!=0; i++){
        اگر( IdChar(c) ){
          n++;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TCL_VARIABLE
        }ورنہ اگر( c=='(' && n>0 ){
          کرو{
            i++;
          }جب( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          اگر( c==')' ){
            i++;
          }ورنہ{
            *tokenType = TK_ILLEGAL;
          }
          توڑو;
        }ورنہ اگر( c==':' && z[i+1]==':' ){
          i++;
#اگر_ختم
        }ورنہ{
          توڑو;
        }
      }
      اگر( n==0 ) *tokenType = TK_ILLEGAL;
      واپس i;
    }
    حالت CC_KYWD0: {
      اگر( aiClass[z[1]]>CC_KYWD ){ i = 1;  توڑو; }
      برائے(i=2; aiClass[z[i]]<=CC_KYWD; i++){}
      اگر( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        توڑو;
      }
      *tokenType = TK_ID;
      واپس keyword_رمز((حرف*)z, i, tokenType);
    }
    حالت CC_X: {
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      اگر( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        برائے(i=2; sqlite3Isxdigit(z[i]); i++){}
        اگر( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          جب( z[i] && z[i]!='\'' ){ i++; }
        }
        اگر( z[i] ) i++;
        واپس i;
      }
#اگر_ختم
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
      /* no break */ deliberate_fall_through
    }
    حالت CC_KYWD:
    حالت CC_ID: {
      i = 1;
      توڑو;
    }
    حالت CC_BOM: {
      اگر( z[1]==0xbb && z[2]==0xbf ){
        *tokenType = TK_SPACE;
        واپس 3;
      }
      i = 1;
      توڑو;
    }
    حالت CC_NUL: {
      *tokenType = TK_ILLEGAL;
      واپس 0;
    }
    طےشدہ: {
      *tokenType = TK_ILLEGAL;
      واپس 1;
    }
  }
  جب( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  واپس i;
}

/*
** Run the parser on the given SQL string.
*/
صحیح sqlite3_تسلسل_Parser(Parse *pParse, ثابت حرف *zSql){
  صحیح nErr = 0;                   /* Number of errors encountered */
  خالی *pEngine;                  /* The LEMON-generated LALR(1) parser */
  i64 n = 0;                      /* Length of the next token token */
  صحیح tokenType;                  /* type of the next token */
  صحیح آخری_Token_Parsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  i64 mxSqlLen;                   /* Max length of an SQL string */
  Parse *p_والد_Parse = 0;        /* Outer parse context, if any */
#اگر_تعریف_شدہ sqlite3Parser_ENGINEALWAYSONSTACK
  yyParser sEngine;    /* Space to hold the Lemon-generated Parser object */
#اگر_ختم
  VVA_ONLY( u8 startedWithOom = db->mallocFailed );

  أكد( zSql!=0 );
  mxSqlLen = db->a_حد_بندی[SQLITE_حد_بندی_SQL_لمبائی];
  اگر( db->n_Vdbe_فعال==0 ){
    AtomicStore(&db->u1.isInterrupted, 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->z_دم = zSql;
#اگر_تعریف_شدہ SQLITE_DEBUG
  اگر( db->flags & SQLITE_ParserTrace ){
    لکھو("parser: [[[%s]]]\n", zSql);
    sqlite3ParserTrace(stdout, "parser: ");
  }ورنہ{
    sqlite3ParserTrace(0, 0);
  }
#اگر_ختم
#اگر_تعریف_شدہ sqlite3Parser_ENGINEALWAYSONSTACK
  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);
#ورنہ
  pEngine = sqlite3ParserAlloc(sqlite3Malloc, pParse);
  اگر( pEngine==0 ){
    sqlite3OomFault(db);
    واپس SQLITE_NOMEM_BKPT;
  }
#اگر_ختم
  أكد( pParse->p_New_جدول==0 );
  أكد( pParse->pNewTrigger==0 );
  أكد( pParse->nVar==0 );
  أكد( pParse->pVList==0 );
  p_والد_Parse = db->pParse;
  db->pParse = pParse;
  جب( 1 ){
    n = sqlite3GetToken((u8*)zSql, &tokenType);
    mxSqlLen -= n;
    اگر( mxSqlLen<0 ){
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( tokenType>=TK_کھڑکی ){
      أكد( tokenType==TK_SPACE || tokenType==TK_OVER || tokenType==TK_FILTER
           || tokenType==TK_ILLEGAL || tokenType==TK_کھڑکی 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#ورنہ
    اگر( tokenType>=TK_SPACE ){
      أكد( tokenType==TK_SPACE || tokenType==TK_ILLEGAL 
           || tokenType==TK_QNUMBER || tokenType==TK_COMMENT
      );
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
      اگر( Atomic_بھرو(&db->u1.isInterrupted) ){
        pParse->rc = SQLITE_مداخلت;
        pParse->nErr++;
        توڑو;
      }
      اگر( tokenType==TK_SPACE ){
        zSql += n;
        جاری;
      }
      اگر( zSql[0]==0 ){
        /* Upon reaching the end of input, call the parser two more times
        ** with tokens TK_SEMI and 0, in that order. */
        اگر( آخری_Token_Parsed==TK_SEMI ){
          tokenType = 0;
        }ورنہ اگر( آخری_Token_Parsed==0 ){
          توڑو;
        }ورنہ{
          tokenType = TK_SEMI;
        }
        n = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      }ورنہ اگر( tokenType==TK_کھڑکی ){
        أكد( n==6 );
        tokenType = analyze_کھڑکی_Keyword((ثابت u8*)&zSql[6]);
      }ورنہ اگر( tokenType==TK_OVER ){
        أكد( n==4 );
        tokenType = analyzeOverKeyword((ثابت u8*)&zSql[4], آخری_Token_Parsed);
      }ورنہ اگر( tokenType==TK_FILTER ){
        أكد( n==6 );
        tokenType = analyzeFilterKeyword((ثابت u8*)&zSql[6], آخری_Token_Parsed);
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
      }ورنہ اگر( tokenType==TK_COMMENT
             && (db->init.busy || (db->flags & SQLITE_Comments)!=0)
      ){
        /* Ignore SQL comments if either (1) we are reparsing the schema or
        ** (2) SQLITE_DBCONFIG_ENABLE_COMMENTS is turned on (the default). */
        zSql += n;
        جاری;
      }ورنہ اگر( tokenType!=TK_QNUMBER ){
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3_Error_پیغام(pParse, "unrecognized token: \"%T\"", &x);
        توڑو;
      }
    }
    pParse->s_آخری_Token.z = zSql;
    pParse->s_آخری_Token.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->s_آخری_Token);
    آخری_Token_Parsed = tokenType;
    zSql += n;
    أكد( db->mallocFailed==0 || pParse->rc!=SQLITE_OK || startedWithOom );
    اگر( pParse->rc!=SQLITE_OK ) توڑو;
  }
  أكد( nErr==0 );
#اگر_تعریف_شدہ YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3_حالت_Highwater(SQLITE_حالت_PARSER_ڈھیر,
      sqlite3_Parser_ڈھیر_Peak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#اگر_ختم /* YYDEBUG */
#اگر_تعریف_شدہ sqlite3Parser_ENGINEALWAYSONSTACK
  sqlite3ParserFinalize(pEngine);
#ورنہ
  sqlite3ParserFree(pEngine, sqlite3_free);
#اگر_ختم
  اگر( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  اگر( pParse->z_Err_پیغام || (pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_مکمل) ){
    اگر( pParse->z_Err_پیغام==0 ){
      pParse->z_Err_پیغام = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    اگر( (pParse->prepFlags & SQLITE_PREPARE_DONT_LOG)==0 ){
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->z_Err_پیغام, pParse->z_دم);
    }
    nErr++;
  }
  pParse->z_دم = zSql;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->ap_Vtab_تالا);
#اگر_ختم

  اگر( pParse->p_New_جدول && !اندر_SPECIAL_PARSE ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3_Delete_جدول(db, pParse->p_New_جدول);
  }
  اگر( pParse->pNewTrigger && !اندر_RENAME_OBJECT ){
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  اگر( pParse->pVList ) sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = p_والد_Parse;
  أكد( nErr==0 || pParse->rc!=SQLITE_OK );
  واپس nErr;
}


#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
/*
** Insert a single space character into pStr if the current string
** ends with an identifier
*/
ساکن خالی جوڑو_Space_Separator(sqlite3_str *pStr){
  اگر( pStr->nChar && sqlite3IsIdChar(pStr->z_متن[pStr->nChar-1]) ){
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
  صحیح i;             /* Next unread byte of zSql[] */
  i64 n;             /* length of current token */
  صحیح tokenType;     /* type of current token */
  صحیح پچھلا_Type = 0;  /* Previous non-whitespace token */
  صحیح nParen;        /* Number of nested levels of parentheses */
  صحیح i_آغاز_اندر;      /* Start of RHS of IN operator in z[] */
  صحیح n_Paren_At_اندر;    /* Value of nParent at start of RHS of IN operator */
  u32 j;             /* Bytes of normalized SQL generated so far */
  sqlite3_str *pStr; /* The normalized SQL string under construction */

  db = sqlite3VdbeDb(pVdbe);
  tokenType = -1;
  nParen = i_آغاز_اندر = n_Paren_At_اندر = 0;
  pStr = sqlite3_str_new(db);
  أكد( pStr!=0 );  /* sqlite3_str_new() never returns NULL */
  برائے(i=0; zSql[i] && pStr->accError==0; i+=n){
    اگر( tokenType!=TK_SPACE ){
      پچھلا_Type = tokenType;
    }
    n = sqlite3GetToken((غیرعلامتی حرف*)zSql+i, &tokenType);
    اگر( NEVER(n<=0) ) توڑو;
    انتخاب( tokenType ){
      حالت TK_COMMENT:
      حالت TK_SPACE: {
        توڑو;
      }
      حالت TK_NULL: {
        اگر( پچھلا_Type==TK_IS || پچھلا_Type==TK_NOT ){
          sqlite3_str_append(pStr, " NULL", 5);
          توڑو;
        }
        /* no break */ deliberate_fall_through
      }
      حالت TK_STRING:
      حالت TK_INTEGER:
      حالت TK_FLOAT:
      حالت TK_VARIABLE:
      حالت TK_BLOB: {
        sqlite3_str_append(pStr, "?", 1);
        توڑو;
      }
      حالت TK_LP: {
        nParen++;
        اگر( پچھلا_Type==TK_اندر ){
          i_آغاز_اندر = pStr->nChar;
          n_Paren_At_اندر = nParen;
        }
        sqlite3_str_append(pStr, "(", 1);
        توڑو;
      }
      حالت TK_RP: {
        اگر( i_آغاز_اندر>0 && nParen==n_Paren_At_اندر ){
          أكد( pStr->nChar>=(u32)i_آغاز_اندر );
          pStr->nChar = i_آغاز_اندر+1;
          sqlite3_str_append(pStr, "?,?,?", 5);
          i_آغاز_اندر = 0;
        }
        nParen--;
        sqlite3_str_append(pStr, ")", 1);
        توڑو;
      }
      حالت TK_ID: {
        i_آغاز_اندر = 0;
        j = pStr->nChar;
        اگر( sqlite3Isquote(zSql[i]) ){
          حرف *zId = sqlite3DbStrNDup(db, zSql+i, n);
          صحیح nId;
          صحیح eType = 0;
          اگر( zId==0 ) توڑو;
          sqlite3Dequote(zId);
          اگر( zSql[i]=='"' && sqlite3_Vdbe_استعمال_کرتا_Double_Quoted_String(pVdbe, zId) ){
            sqlite3_str_append(pStr, "?", 1);
            sqlite3DbFree(db, zId);
            توڑو;
          }
          nId = sqlite3Strlen30(zId);
          اگر( sqlite3GetToken((u8*)zId, &eType)==nId && eType==TK_ID ){
            جوڑو_Space_Separator(pStr);
            sqlite3_str_append(pStr, zId, nId);
          }ورنہ{
            sqlite3_str_appendf(pStr, "\"%w\"", zId);
          }
          sqlite3DbFree(db, zId);
        }ورنہ{
          جوڑو_Space_Separator(pStr);
          sqlite3_str_append(pStr, zSql+i, n);
        }
        جب( j<pStr->nChar ){
          pStr->z_متن[j] = sqlite3Tolower(pStr->z_متن[j]);
          j++;
        }
        توڑو;
      }
      حالت TK_SELECT: {
        i_آغاز_اندر = 0;
        /* no break */ deliberate_fall_through
      }
      طےشدہ: {
        اگر( sqlite3IsIdChar(zSql[i]) ) جوڑو_Space_Separator(pStr);
        j = pStr->nChar;
        sqlite3_str_append(pStr, zSql+i, n);
        جب( j<pStr->nChar ){
          pStr->z_متن[j] = sqlite3Toupper(pStr->z_متن[j]);
          j++;
        }
        توڑو;
      }
    }
  }
  اگر( tokenType!=TK_SEMI ) sqlite3_str_append(pStr, ";", 1);
  واپس sqlite3_str_اختتام(pStr);
}
#اگر_ختم /* SQLITE_ENABLE_NORMALIZE */
