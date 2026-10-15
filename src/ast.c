
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static Node *new_node(NodeType type)
{
    Node *n = calloc(1, sizeof(Node));

    if (!n) {
        fprintf(stderr, "ORVIX: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    n->type = type;
    return n;
}

Node *make_number(int value)
{
    Node *n = new_node(N_NUMBER);
    n->value = value;
    return n;
}

Node *make_string(char *text)
{
    Node *n = new_node(N_STRING);
    n->text = text;
    return n;
}

Node *make_variable(char *name)
{
    Node *n = new_node(N_VARIABLE);
    n->name = name;
    return n;
}

Node *make_binary(char *op, Node *left, Node *right)
{
    Node *n = new_node(N_BINARY);
    snprintf(n->op, sizeof(n->op), "%s", op);
    n->left = left;
    n->right = right;
    return n;
}

Node *make_declaration(char *name)
{
    Node *n = new_node(N_DECLARATION);
    n->name = name;
    return n;
}

Node *make_assignment(char *name, Node *expr)
{
    Node *n = new_node(N_ASSIGNMENT);
    n->name = name;
    n->left = expr;
    return n;
}

Node *make_input(char *name)
{
    Node *n = new_node(N_INPUT);
    n->name = name;
    return n;
}

Node *make_print_expr(Node *expr)
{
    Node *n = new_node(N_PRINT_EXPR);
    n->left = expr;
    return n;
}

Node *make_print_string(char *text)
{
    Node *n = new_node(N_PRINT_STRING);
    n->text = text;
    return n;
}

Node *make_if(Node *condition, Node *body, Node *else_body)
{
    Node *n = new_node(N_IF);
    n->left = condition;
    n->body = body;
    n->else_body = else_body;
    return n;
}

Node *make_while(Node *condition, Node *body)
{
    Node *n = new_node(N_WHILE);
    n->left = condition;
    n->body = body;
    return n;
}

Node *make_halt(void)
{
    return new_node(N_HALT);
}

Node *append_node(Node *list, Node *node)
{
    if (!list) return node;

    Node *tail = list;
    while (tail->next)
        tail = tail->next;

    tail->next = node;
    return list;
}
