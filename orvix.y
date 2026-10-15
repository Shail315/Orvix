
%code requires {
    #include "src/ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include "src/ast.h"
#include "src/runtime.h"

int yylex(void);
void yyerror(const char *s);

extern FILE *yyin;
extern int yylineno;
%}

%define parse.error detailed

%union {
    int num;
    char *str;
    Node *node;
}

%token FORGE SPEAK ACQUIRE VERIFY DENY RECUR HALT
%token EQ NE LE GE AND OR INVALID
%token <num> NUMBER
%token <str> ID STRING

%type <node> program statements statement
%type <node> declaration assignment input_statement
%type <node> output_statement conditional loop
%type <node> block optional_else expression

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%precedence UMINUS

%start program

%%

program:
    statements
    {
        execute_program($1);
        printf("\n===== ORVIX PROGRAM FINISHED =====\n");
    }
;

statements:
    %empty
    {
        $$ = NULL;
    }
  | statements statement
    {
        $$ = append_node($1, $2);
    }
;

statement:
    declaration       { $$ = $1; }
  | assignment        { $$ = $1; }
  | input_statement   { $$ = $1; }
  | output_statement  { $$ = $1; }
  | conditional       { $$ = $1; }
  | loop              { $$ = $1; }
  | HALT ';'          { $$ = make_halt(); }
;

declaration:
    FORGE ID ';'
    {
        $$ = make_declaration($2);
    }
;

assignment:
    ID '=' expression ';'
    {
        $$ = make_assignment($1, $3);
    }
;

input_statement:
    ACQUIRE ID ';'
    {
        $$ = make_input($2);
    }
;

output_statement:
    SPEAK expression ';'
    {
        $$ = make_print_expr($2);
    }
;

conditional:
    VERIFY '(' expression ')' block optional_else
    {
        $$ = make_if($3, $5, $6);
    }
;

optional_else:
    DENY block
    {
        $$ = $2;
    }
  | %empty
    {
        $$ = NULL;
        }
;

loop:
    RECUR '(' expression ')' block
    {
        $$ = make_while($3, $5);
    }
;

block:
    '{' statements '}'
    {
        $$ = $2;
    }
;

expression:
    NUMBER
    {
        $$ = make_number($1);
    }
    | STRING
        {
                $$ = make_string($1);
        }
  | ID
    {
        $$ = make_variable($1);
    }
  | '(' expression ')'
    {
        $$ = $2;
    }
  | '-' expression %prec UMINUS
    {
        $$ = make_binary("-", make_number(0), $2);
    }
  | expression '+' expression
    {
        $$ = make_binary("+", $1, $3);
    }
  | expression '-' expression
    {
        $$ = make_binary("-", $1, $3);
    }
  | expression '*' expression
    {
        $$ = make_binary("*", $1, $3);
    }
  | expression '/' expression
    {
        $$ = make_binary("/", $1, $3);
    }
  | expression '%' expression
    {
        $$ = make_binary("%", $1, $3);
    }
  | expression EQ expression
    {
        $$ = make_binary("==", $1, $3);
    }
  | expression NE expression
    {
        $$ = make_binary("!=", $1, $3);
    }
  | expression '<' expression
    {
        $$ = make_binary("<", $1, $3);
    }
  | expression '>' expression
    {
        $$ = make_binary(">", $1, $3);
    }
  | expression LE expression
    {
        $$ = make_binary("<=", $1, $3);
    }
  | expression GE expression
    {
        $$ = make_binary(">=", $1, $3);
    }
  | expression AND expression
    {
        $$ = make_binary("&&", $1, $3);
    }
  | expression OR expression
    {
        $$ = make_binary("||", $1, $3);
    }
;

%%

void yyerror(const char *s)
{
    fprintf(stderr,
            "Syntax Error at line %d: %s\n",
            yylineno, s);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: ./orvix <source-file.orx>\n");
        return EXIT_FAILURE;
    }

    yyin = fopen(argv[1], "r");

    if (!yyin) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    printf("====================================\n");
    printf("         ORVIX LANGUAGE             \n");
    printf("====================================\n\n");

    int result = yyparse();

    fclose(yyin);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
