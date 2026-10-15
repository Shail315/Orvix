
%{
#include "ORVIX.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%option noyywrap
%option noinput nounput
%option yylineno

%%

"FORGE"       { return FORGE; }
"SPEAK"       { return SPEAK; }
"ACQUIRE"     { return ACQUIRE; }
"VERIFY"      { return VERIFY; }
"DENY"        { return DENY; }
"RECUR"       { return RECUR; }
"HALT"        { return HALT; }

"=="          { return EQ; }
"!="          { return NE; }
"<="          { return LE; }
">="          { return GE; }
"&&"          { return AND; }
"||"          { return OR; }

[0-9]+ {
    yylval.num = atoi(yytext);
    return NUMBER;
}

\"[^\"\n]*\" {
    size_t len = strlen(yytext);
    yylval.str = malloc(len - 1);

    if (!yylval.str) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    memcpy(yylval.str, yytext + 1, len - 2);
    yylval.str[len - 2] = '\0';

    return STRING;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);

    if (!yylval.str) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    return ID;
}

"//".*       { /* Ignore single-line comments */ }
[ \t\r\n]+   { /* Ignore whitespace */ }

[;{}()+*/%=<>!\-] { return yytext[0]; }

. {
    fprintf(stderr,
            "Lexical Error at line %d: Unknown character '%s'\n",
            yylineno, yytext);
    return INVALID;
}

%%
