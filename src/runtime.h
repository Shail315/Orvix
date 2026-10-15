
#ifndef ORVIX_RUNTIME_H
#define ORVIX_RUNTIME_H

#include "ast.h"

typedef enum {
	VALUE_NUMBER,
	VALUE_STRING
} ValueType;

typedef struct {
	ValueType type;
	int number;
	char *string;
} Value;

void runtime_init(void);
void execute_program(Node *program);
Value evaluate_expression(Node *expr);
void execute_statements(Node *statements);

#endif
