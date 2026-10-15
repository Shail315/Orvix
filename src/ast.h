
#ifndef ORVIX_AST_H
#define ORVIX_AST_H

typedef struct Node Node;

typedef enum {
    N_NUMBER,
    N_STRING,
    N_VARIABLE,
    N_BINARY,
    N_DECLARATION,
    N_ASSIGNMENT,
    N_INPUT,
    N_PRINT_EXPR,
    N_PRINT_STRING,
    N_IF,
    N_WHILE,
    N_HALT
} NodeType;

struct Node {
    NodeType type;
    int value;
    char *name;
    char *text;
    char op[3];

    Node *left;
    Node *right;
    Node *body;
    Node *else_body;
    Node *next;
};

Node *make_number(int value);
Node *make_string(char *text);
Node *make_variable(char *name);
Node *make_binary(char *op, Node *left, Node *right);

Node *make_declaration(char *name);
Node *make_assignment(char *name, Node *expr);
Node *make_input(char *name);
Node *make_print_expr(Node *expr);
Node *make_print_string(char *text);
Node *make_if(Node *condition, Node *body, Node *else_body);
Node *make_while(Node *condition, Node *body);
Node *make_halt(void);
Node *append_node(Node *list, Node *node);

#endif
