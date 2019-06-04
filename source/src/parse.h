/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SRC_PARSE_H_INCLUDED
# define YY_YY_SRC_PARSE_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tBOOL = 258,
     tBREAK = 259,
     tCASE = 260,
     tCHAR = 261,
     tCONTINUE = 262,
     tDEFAULT = 263,
     tDO = 264,
     tELSE = 265,
     tFOR = 266,
     tGOTO = 267,
     tIF = 268,
     tINT = 269,
     tLONG = 270,
     tCONST = 271,
     tRETURN = 272,
     tSHORT = 273,
     tSIGNED = 274,
     tSIZEOF = 275,
     tSWITCH = 276,
     tTYPEDEF = 277,
     tUNSIGNED = 278,
     tVOID = 279,
     tWHILE = 280,
     QSTRING = 281,
     TYPEDEF_IDENT = 282,
     LEQ = 283,
     GEQ = 284,
     EQU = 285,
     NEQ = 286,
     ARROW = 287,
     LOGICAL_AND = 288,
     LOGICAL_OR = 289,
     IDENTIFIER = 290,
     CONSTANT = 291,
     PLUSPLUS = 292,
     MINUSMINUS = 293,
     PLUS_ASSIGN = 294,
     MINUS_ASSIGN = 295,
     MUL_ASSIGN = 296,
     DIV_ASSIGN = 297,
     MOD_ASSIGN = 298,
     OR_ASSIGN = 299,
     AND_ASSIGN = 300,
     XOR_ASSIGN = 301,
     LEFTSHIFT_ASSIGN = 302,
     RIGHTSHIFT_ASSIGN = 303,
     LEFTSHIFT = 304,
     RIGHTSHIFT = 305,
     ELLIPSIS = 306,
     LOWER_THAN_ELSE = 307
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_SRC_PARSE_H_INCLUDED  */
