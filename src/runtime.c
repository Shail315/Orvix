
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "runtime.h"

#define MAX_SYMBOLS 500
#define MAX_LOOP_ITERATIONS 100000

typedef struct {
    char *name;
    Value value;
} Symbol;

static Symbol symbols[MAX_SYMBOLS];
static int symbol_count = 0;
static int halted = 0;

static int find_symbol(const char *name)
{
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0)
            return i;
    }

    return -1;
}

void runtime_init(void)
{
    symbol_count = 0;
    halted = 0;
}

static Value number_value(int number)
{
    Value value = { VALUE_NUMBER, number, NULL };
    return value;
}

static Value string_value(char *string)
{
    Value value = { VALUE_STRING, 0, string };
    return value;
}

static Value get_value(const char *name)
{
    int index = find_symbol(name);

    if (index < 0) {
        fprintf(stderr,
                "Semantic Error: Variable '%s' is not declared.\n",
                name);
        return number_value(0);
    }

    return symbols[index].value;
}

static void declare_variable(const char *name)
{
    if (find_symbol(name) >= 0) {
        fprintf(stderr,
                "Semantic Error: Variable '%s' already declared.\n",
                name);
        return;
    }

    if (symbol_count >= MAX_SYMBOLS) {
        fprintf(stderr, "Semantic Error: Symbol table is full.\n");
        return;
    }

    symbols[symbol_count].name = strdup(name);
    symbols[symbol_count].value = number_value(0);
    symbol_count++;

    printf("[ORVIX] Variable '%s' created.\n", name);
}

static void assign_variable(const char *name, Value value)
{
    int index = find_symbol(name);

    if (index < 0) {
        fprintf(stderr,
                "Semantic Error: Variable '%s' is not declared.\n",
                name);
        return;
    }

    if (value.type == VALUE_STRING) {
        char *copy = strdup(value.string);

        if (!copy) {
            fprintf(stderr, "ORVIX: Out of memory\n");
            exit(EXIT_FAILURE);
        }

        free(symbols[index].value.string);
        symbols[index].value = string_value(copy);
    } else {
        free(symbols[index].value.string);
        symbols[index].value = value;
    }
}

static int is_truthy(Value value)
{
    if (value.type == VALUE_STRING)
        return value.string[0] != '\0';

    return value.number != 0;
}

static int as_number(Value value)
{
    if (value.type == VALUE_NUMBER)
        return value.number;

    char *end;
    long number = strtol(value.string, &end, 10);

    if (*value.string != '\0' && *end == '\0' && number >= INT_MIN && number <= INT_MAX)
        return (int) number;

    fprintf(stderr, "Runtime Error: Expected a number but got \"%s\".\n",
            value.string);
    return 0;
}

static const char *as_string(Value value, char *buffer, size_t size)
{
    if (value.type == VALUE_STRING)
        return value.string;

    snprintf(buffer, size, "%d", value.number);
    return buffer;
}

static Value concatenate(Value left, Value right)
{
    char left_buffer[32];
    char right_buffer[32];
    const char *left_text = as_string(left, left_buffer, sizeof(left_buffer));
    const char *right_text = as_string(right, right_buffer, sizeof(right_buffer));
    size_t length = strlen(left_text) + strlen(right_text) + 1;
    char *result = malloc(length);

    if (!result) {
        fprintf(stderr, "ORVIX: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    snprintf(result, length, "%s%s", left_text, right_text);
    return string_value(result);
}

Value evaluate_expression(Node *expr)
{
    if (!expr) return number_value(0);

    if (expr->type == N_NUMBER)
        return number_value(expr->value);

    if (expr->type == N_STRING)
        return string_value(expr->text);

    if (expr->type == N_VARIABLE)
        return get_value(expr->name);

    if (expr->type != N_BINARY)
        return number_value(0);

    Value left = evaluate_expression(expr->left);
    Value right = evaluate_expression(expr->right);

    if (strcmp(expr->op, "+") == 0 &&
        (left.type == VALUE_STRING || right.type == VALUE_STRING))
        return concatenate(left, right);

    if (strcmp(expr->op, "==") == 0 || strcmp(expr->op, "!=") == 0) {
        int equal;

        if (left.type == VALUE_STRING || right.type == VALUE_STRING) {
            char left_buffer[32];
            char right_buffer[32];
            equal = strcmp(as_string(left, left_buffer, sizeof(left_buffer)),
                           as_string(right, right_buffer, sizeof(right_buffer))) == 0;
        } else {
            equal = left.number == right.number;
        }

        return number_value(strcmp(expr->op, "==") == 0 ? equal : !equal);
    }

    if (strcmp(expr->op, "&&") == 0)
        return number_value(is_truthy(left) && is_truthy(right));

    if (strcmp(expr->op, "||") == 0)
        return number_value(is_truthy(left) || is_truthy(right));

    if (left.type == VALUE_STRING || right.type == VALUE_STRING) {
        char left_buffer[32];
        char right_buffer[32];
        int comparison = strcmp(as_string(left, left_buffer, sizeof(left_buffer)),
                                as_string(right, right_buffer, sizeof(right_buffer)));

        if (strcmp(expr->op, "<") == 0) return number_value(comparison < 0);
        if (strcmp(expr->op, ">") == 0) return number_value(comparison > 0);
        if (strcmp(expr->op, "<=") == 0) return number_value(comparison <= 0);
        if (strcmp(expr->op, ">=") == 0) return number_value(comparison >= 0);
    }

    int a = as_number(left);
    int b = as_number(right);

    if (strcmp(expr->op, "+") == 0) return number_value(a + b);
    if (strcmp(expr->op, "-") == 0) return number_value(a - b);
    if (strcmp(expr->op, "*") == 0) return number_value(a * b);

    if (strcmp(expr->op, "/") == 0) {
        if (b == 0) {
            fprintf(stderr,
                    "Semantic Error: Division by zero.\n");
            return number_value(0);
        }
        return number_value(a / b);
    }

    if (strcmp(expr->op, "%") == 0) {
        if (b == 0) {
            fprintf(stderr,
                    "Semantic Error: Modulo by zero.\n");
            return number_value(0);
        }
        return number_value(a % b);
    }

    if (strcmp(expr->op, "<") == 0) return number_value(a < b);
    if (strcmp(expr->op, ">") == 0) return number_value(a > b);
    if (strcmp(expr->op, "<=") == 0) return number_value(a <= b);
    if (strcmp(expr->op, ">=") == 0) return number_value(a >= b);
    fprintf(stderr, "Runtime Error: Unknown operator '%s'.\n",
            expr->op);
    return number_value(0);
}

void execute_statements(Node *statements)
{
    for (Node *n = statements; n && !halted; n = n->next) {
        switch (n->type) {
        case N_DECLARATION:
            declare_variable(n->name);
            break;

        case N_ASSIGNMENT:
            assign_variable(n->name,
                            evaluate_expression(n->left));
            break;

        case N_INPUT: {
            char input[1024];
            printf("Enter value for %s: ", n->name);
            fflush(stdout);

            if (!fgets(input, sizeof(input), stdin)) {
                fprintf(stderr, "Input Error: Expected a value.\n");
                halted = 1;
                break;
            }

            input[strcspn(input, "\n")] = '\0';
            char *end;
            errno = 0;
            long number = strtol(input, &end, 10);

            if (*input != '\0' && *end == '\0' && errno == 0 &&
                number >= INT_MIN && number <= INT_MAX) {
                assign_variable(n->name, number_value((int) number));
            } else {
                assign_variable(n->name, string_value(input));
            }
            break;
        }

        case N_PRINT_EXPR:
        {
            Value value = evaluate_expression(n->left);
            if (value.type == VALUE_STRING)
                printf("%s\n", value.string);
            else
                printf("%d\n", value.number);
            break;
        }

        case N_PRINT_STRING:
            printf("%s\n", n->text);
            break;

        case N_IF:
            if (is_truthy(evaluate_expression(n->left)))
                execute_statements(n->body);
            else
                execute_statements(n->else_body);
            break;

        case N_WHILE: {
            int iterations = 0;

            while (!halted && is_truthy(evaluate_expression(n->left))) {
                if (iterations++ >= MAX_LOOP_ITERATIONS) {
                    fprintf(stderr,
                            "Runtime Error: Loop limit exceeded.\n");
                    halted = 1;
                    break;
                }

                execute_statements(n->body);
            }
            break;
        }

        case N_HALT:
            halted = 1;
            break;

        default:
            fprintf(stderr, "Runtime Error: Invalid statement.\n");
            halted = 1;
            break;
        }
    }
}

void execute_program(Node *program)
{
    runtime_init();
    execute_statements(program);
}
