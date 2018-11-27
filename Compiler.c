/* -------------------------------------------------

            CFG for tinyL LANGUAGE

     PROGRAM ::= STMTLIST .
     STMTLIST ::= STMT MORESTMTS
     MORESTMTS ::= ; STMTLIST | epsilon
     STMT ::= ASSIGN | READ | PRINT
     ASSIGN ::= VARIABLE = EXPR
     READ ::= ! VARIABLE
     PRINT ::= # VARIABLE
     EXPR ::= + EXPR EXPR |
              - EXPR EXPR |
              * EXPR EXPR |
              VARIABLE | 
              DIGIT
     VARIABLE ::= a | b | c | d | e 
     DIGIT ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

     NOTE: tokens are exactly a single character long

     Example expressions:

           +12.
           +1b.
           +*34-78.
           -*+1+2a58.

     Example programs;

         !a;!b;c=+3*ab;d=+c1;#d.
         b=-*+1+2a58;#b.

	FIRST SETS:

		FIRST(program) ::= {a, b, c, d, e, !, #}
		FIRST(stmtlist) ::= {a, b, c, d, e, !, #}
		FIRST(morestmt) ::= {;б epsilon}
		FIRST(stmt) ::= {a, b, c, d, e, !, #}
		FIRST(assign) ::= {a, b, c, d, e}
		FIRST(read) ::= {!}
		FIRST(print) ::= {#}
		FIRST(expr) ::= {+, -, #, a, b, c, d, e, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
		FIRST(variable) ::= {a, b, c, d, e}
		FIRST(digit) ::= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
		FIRST(a) = {a}
		FIRST(b) = {b}
		FIRST(c) = {c}
		FIRST(d) = {d}
		FIRST(e) = {e}
		FIRST(0) = {0}
		FIRST(1) = {1}
		FIRST(2) = {2}
		FIRST(3) = {3}
		FIRST(4) = {4}
		FIRST(5) = {5}
		FIRST(6) = {6}
		FIRST(7) = {7}
		FIRST(8) = {8}
		FIRST(9) = {9}
	
	FOLLOW SETS:

		FOLLOW(Program) ::= {$}
		FOLLOW(stmtlist) ::= {.}
		FOLLOW(morestmt) ::= {.}
		FOLLOW(stmt) ::= {;}
		FOLLOW(assign) ::= {;}
		FOLLOW(read) ::= {;}
		FOLLOW(print) ::= {;}
		FOLLOW(expr) ::= {+, -, *, a, b, c, d, e, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ;}
		FOLLOW(variable) ::= {=, ;, +, -, *, a, b, c, d, e, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
		FOLLOW(digit) ::= {+, -, *, a, b, c, d, e, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ;}


 ---------------------------------------------------
 */

/*

One parse method per non termial symbol

Non terminal symbol on the right hand side of a rewrite rule leads to a call to
the parse method of 

terminal leads to "consuming" the that token

Having '|' leads to an if-else in the parser

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "Instr.h"
#include "InstrUtils.h"
#include "Utils.h"

#define MAX_BUFFER_SIZE 500
#define EMPTY_FIELD 0xFFFFF
#define token *buffer

/* GLOBALS */
static char *buffer = NULL;	/* read buffer */
static int regnum = 1;		/* for next free virtual register number */
static FILE *outfile = NULL;	/* output of code generation */

/* Utilities */
static void CodeGen(OpCode opcode, int field1, int field2, int field3);
static inline void next_token();
static inline int next_register();
static inline int is_digit(char c);
static inline int to_digit(char c);
static inline int is_identifier(char c);
static char *read_input(FILE * f);

/* Routines for recursive descending parser LL(1) */
static void program();
static void stmtlist();
static void morestmts();
static void stmt();
static void assign();
static void read();
static void print();
static int expr();
static int variable();
static int digit();

/*************************************************************************/
/* Definitions for recursive descending parser LL(1)                     */
/*************************************************************************/
static int digit()
{
	int reg;

	if (!is_digit(token)) {
		ERROR("Expected digit\n");
		exit(EXIT_FAILURE);
	}
	reg = next_register();
	CodeGen(LOADI, reg, to_digit(token), EMPTY_FIELD);
	next_token();
	return reg;
}

static int variable()
{
	if(!is_identifier(token)){
		ERROR("Expected letter");
		exit(EXIT_FAILURE);
	}
	int reg = next_register();
	CodeGen(LOAD, reg, token, EMPTY_FIELD);
	next_token();
	return reg;
}

static int expr()
{
	int reg, left_reg, right_reg;

	switch (token) {
		case '+':
			next_token();
			left_reg = expr();
			right_reg = expr();
			reg = next_register();
			//printf("\nADD REG#: %d\n", reg);
			CodeGen(ADD, reg, left_reg, right_reg);
			return reg;
		case '-':
			next_token();
			left_reg = expr();
			right_reg = expr();
			reg = next_register();
			//printf("\nSUB REG#: %d\n", reg);
			CodeGen(SUB, reg, left_reg, right_reg);
			return reg;
		case '*':
			next_token();
			left_reg = expr();
			right_reg = expr();
			reg = next_register();
			//printf("\nMUL REG#: %d\n", reg);
			CodeGen(MUL, reg, left_reg, right_reg);
			return reg;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return digit();
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
			return variable();
		default:
			ERROR("Symbol %c unknown\n", token);
			exit(EXIT_FAILURE);
	}
}

static void assign()
{
	int reg, var = token;
	next_token();
	if(token == '='){
		next_token();
		reg = expr();
		CodeGen(STORE, var, reg, EMPTY_FIELD);
	}
	
}

static void read()
{
	next_token();
	CodeGen(READ, token, EMPTY_FIELD, EMPTY_FIELD);
	next_token();
	return;
}

static void print()
{
	next_token();
	if(!is_identifier(token)){
		ERROR("Wrong input\n");
		exit(EXIT_FAILURE);
	}
	CodeGen(WRITE, token, EMPTY_FIELD, EMPTY_FIELD);
	next_token();
}


static void stmt()
{
	/* YOUR CODE GOES HERE */
	switch(token){
		case '!':
			read();
			break;
		case '#':
			print();
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
			assign();
			break;
		default:
			ERROR("Symbol %c unknown\n", token);
			exit(EXIT_FAILURE);
	}
}

static void morestmts()
{
	/* YOUR CODE GOES HERE */
	switch(token){
		case '.':
			return;
		case ';':
			next_token();
			stmtlist();
			return;
		default:
			ERROR("Symbol %c unknown\n", token);
			exit(EXIT_FAILURE);
	}
}

static void stmtlist()
{
	stmt();
	morestmts();
}

static void program()
{
	stmtlist();
	if (token != '.') {
		ERROR("Program error.  Current input symbol is %c\n", token);
		exit(EXIT_FAILURE);
	};
}

/*************************************************************************/
/* Utility definitions                                                   */
/*************************************************************************/
static void CodeGen(OpCode opcode, int field1, int field2, int field3)
{
	Instruction instr;
	//printf("\nOPCODE: %u\n", opcode);
	//printf("\nFIELD ONE: %d\n", field1);
	if (!outfile) {
		ERROR("File error\n");
		exit(EXIT_FAILURE);
	}
	instr.opcode = opcode;
	instr.field1 = field1;
	instr.field2 = field2;
	instr.field3 = field3;
	PrintInstruction(outfile, &instr);
}

static inline void next_token()
{
	if (*buffer == '\0') {
		ERROR("End of program input\n");
		exit(EXIT_FAILURE);
	}
	printf("%c ", *buffer);
	if (*buffer == ';')
		printf("\n");
	buffer++;
	if (*buffer == '\0') {
		ERROR("End of program input\n");
		exit(EXIT_FAILURE);
	}
	if (*buffer == '.')
		printf(".\n");
}

static inline int next_register()
{
	return regnum++;
}

static inline int is_digit(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

static inline int to_digit(char c)
{
	if (is_digit(c))
		return c - '0';
	WARNING("Non-digit passed to %s, returning zero\n", __func__);
	return 0;
}

static inline int is_identifier(char c)
{
	if (c >= 'a' && c <= 'e')
		return 1;
	return 0;
}

static char *read_input(FILE * f)
{
	size_t size, i;
	char *b;
	int c;

	for (b = NULL, size = 0, i = 0;;) {
		if (i >= size) {
			size = (size == 0) ? MAX_BUFFER_SIZE : size * 2;
			b = (char *)realloc(b, size * sizeof(char));
			if (!b) {
				ERROR("Realloc failed\n");
				exit(EXIT_FAILURE);
			}
		}
		c = fgetc(f);
		if (EOF == c) {
			b[i] = '\0';
			break;
		}
		if (isspace(c))
			continue;
		b[i] = c;
		i++;
	}
	return b;
}

/*************************************************************************/
/* Main function                                                         */
/*************************************************************************/

int main(int argc, char *argv[])
{
	const char *outfilename = "tinyL.out";
	char *input;
	FILE *infile;

	printf("------------------------------------------------\n");
	printf("CS314 compiler for tinyL\n");
	printf("------------------------------------------------\n");
	if (argc != 2) {
		ERROR("Use of command:\n  compile <tinyL file>\n");
		exit(EXIT_FAILURE);
	}
	infile = fopen(argv[1], "r");
	if (!infile) {
		ERROR("Cannot open input file \"%s\"\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	outfile = fopen(outfilename, "w");
	if (!outfile) {
		ERROR("Cannot open output file \"%s\"\n", outfilename);
		exit(EXIT_FAILURE);
	}
	input = read_input(infile);
	buffer = input;
	program();
	printf("\nCode written to file \"%s\".\n\n", outfilename);
	free(input);
	fclose(infile);
	fclose(outfile);
	return EXIT_SUCCESS;
}
