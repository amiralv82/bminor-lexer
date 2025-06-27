#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_IDENT_LEN 256
#define MAX_STR_LEN 1024

// Token types
const char *TOKEN_ID = "ID";
const char *TOKEN_KEYWORD = "keywords";
const char *TOKEN_INTEGER = "INTEGER";
const char *TOKEN_STRING = "STRING";
const char *TOKEN_OPERATOR = "operators";
const char *TOKEN_DELIMITER = "delimiters";

// Keywords
const char *keywords[] = {
    "array", "boolean", "char", "else", "false", "for", "function", "if",
    "integer", "print", "return", "string", "true", "void", "while"
};
const int num_keywords = 15;

// Symbol table
typedef struct Symbol {
    char *name;
    int code;
    struct Symbol *next;
} Symbol;

Symbol *symbol_table = NULL;
int next_code = 100;

// Helper function prototypes
bool is_delimiter(char c);
bool is_operator_start(char c);
bool is_multi_char_operator(char c1, char c2);
int get_id_code(const char *name);
void skip_line_comment(FILE *src, int *line, int *col);
void skip_block_comment(FILE *src, int *line, int *col, int start_line, int start_col);
void process_string(FILE *src, int *line, int *col);
void emit_token(const char *type, const char *value, const char *lexeme);

// Enhanced string processing with proper escape handling
void process_string(FILE *src, int *line, int *col) {
    char buffer[MAX_STR_LEN] = {0};
    int len = 0;
    int start_line = *line;
    int start_col = *col;
    int c;

    buffer[len++] = '"';
    (*col)++;
    
    while ((c = fgetc(src)) != EOF) {
        if (c == '"') {
            buffer[len++] = '"';
            buffer[len] = '\0';
            emit_token(TOKEN_STRING, buffer, buffer);
            (*col)++;
            return;
        }
        
        if (c == '\\') {
            int esc = fgetc(src);
            (*col)++;
            switch (esc) {
                case 'n': buffer[len++] = '\n'; break;
                case 't': buffer[len++] = '\t'; break;
                case '"': buffer[len++] = '"'; break;
                case '\\': buffer[len++] = '\\'; break;
                default:
                    buffer[len++] = '\\';
                    buffer[len++] = esc;
                    break;
            }
        } else if (c == '\n') {
            buffer[len++] = c;
            (*line)++;
            *col = 1;
        } else {
            buffer[len++] = c;
            (*col)++;
        }
        
        if (len >= MAX_STR_LEN - 2) {
            fprintf(stderr, "Error: String too long at line %d col %d\n", start_line, start_col);
            buffer[len] = '\0';
            emit_token(TOKEN_STRING, buffer, buffer);
            return;
        }
    }
    
    fprintf(stderr, "Error: Unterminated string starting at line %d col %d\n", 
            start_line, start_col);
    buffer[len] = '\0';
    emit_token(TOKEN_STRING, buffer, buffer);
}

// Improved comment handling with error tracking
void skip_block_comment(FILE *src, int *line, int *col, int start_line, int start_col) {
    int c;
    while ((c = fgetc(src)) != EOF) {
        (*col)++;
        if (c == '\n') {
            (*line)++;
            *col = 1;
        }
        else if (c == '*') {
            int next = fgetc(src);
            if (next == '/') return;
            ungetc(next, src);
        }
    }
    fprintf(stderr, "Error: Unterminated block comment starting at line %d col %d\n", 
            start_line, start_col);
}

void skip_line_comment(FILE *src, int *line, int *col) {
    int c;
    while ((c = fgetc(src)) != EOF && c != '\n') {
        (*col)++;
    }
    if (c == '\n') {
        (*line)++;
        *col = 1;
    }
}

// Complete scanning function with accurate column tracking
void scan(FILE *src) {
    int line = 1, col = 1;
    int c;
    
    while ((c = fgetc(src)) != EOF) {
        if (isspace(c)) {
            if (c == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            continue;
        }

        if (c == '"') {
            process_string(src, &line, &col);
            continue;
        }

        if (c == '/') {
            int next = fgetc(src);
            col++;
            if (next == '/') {
                skip_line_comment(src, &line, &col);
            } else if (next == '*') {
                skip_block_comment(src, &line, &col, line, col-1);
            } else {
                ungetc(next, src);
                char op[2] = {c, '\0'};
                emit_token(TOKEN_OPERATOR, op, op);
            }
            continue;
        }

        if (is_operator_start(c)) {
            int next = fgetc(src);
            char op[3] = {c, (char)next, '\0'};
            
            if (is_multi_char_operator(c, next)) {
                col++;
                emit_token(TOKEN_OPERATOR, op, op);
            } else {
                ungetc(next, src);
                op[1] = '\0';
                emit_token(TOKEN_OPERATOR, op, op);
            }
            col++;
            continue;
        }

        if (is_delimiter(c)) {
            char delim[2] = {c, '\0'};
            emit_token(TOKEN_DELIMITER, delim, delim);
            col++;
            continue;
        }

        if (isalpha(c) || c == '_') {
            char ident[MAX_IDENT_LEN];
            int len = 0;
            ident[len++] = c;
            
            while ((c = fgetc(src)) != EOF && (isalnum(c) || c == '_')) {
                if (len < MAX_IDENT_LEN - 1) ident[len++] = c;
                col++;
            }
            ungetc(c, src);
            ident[len] = '\0';
            
            bool is_kw = false;
            for (int i = 0; i < num_keywords; i++) {
                if (strcmp(ident, keywords[i]) == 0) {
                    emit_token(TOKEN_KEYWORD, ident, ident);
                    is_kw = true;
                    break;
                }
            }
            
            if (!is_kw) {
                char code[10];
                sprintf(code, "%d", get_id_code(ident));
                emit_token(TOKEN_ID, code, ident);
            }
            continue;
        }

        if (isdigit(c)) {
            char num[MAX_IDENT_LEN];
            int len = 0;
            num[len++] = c;
            
            while ((c = fgetc(src)) != EOF && isdigit(c)) {
                if (len < MAX_IDENT_LEN - 1) num[len++] = c;
                col++;
            }
            ungetc(c, src);
            num[len] = '\0';
            emit_token(TOKEN_INTEGER, num, num);
            continue;
        }

        fprintf(stderr, "Error: Invalid character '%c' at line %d col %d\n", c, line, col);
        col++;
    }
}

// Supporting functions
bool is_delimiter(char c) { return strchr("()[]{},;:\"'", c) != NULL; }
bool is_operator_start(char c) { return strchr("+-!*/%^<>=|&", c) != NULL; }
bool is_multi_char_operator(char c1, char c2) {
    return (c1 == ':' && c2 == '=') || (c1 == '=' && c2 == '=') || (c1 == '+' && c2 == '+');
}

int get_id_code(const char *name) {
    Symbol *current = symbol_table;
    while (current) {
        if (strcmp(current->name, name) == 0) return current->code;
        current = current->next;
    }
    
    Symbol *new_symbol = malloc(sizeof(Symbol));
    new_symbol->name = strdup(name);
    new_symbol->code = next_code++;
    new_symbol->next = symbol_table;
    symbol_table = new_symbol;
    return new_symbol->code;
}

void emit_token(const char *type, const char *value, const char *lexeme) {
    printf("%-20s\t%-15s\t%s\n", lexeme, type, value);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }
    
    FILE *src = fopen(argv[1], "r");
    if (!src) {
        perror("File open error");
        return 1;
    }
    
    printf("%-20s\t%-15s\t%s\n", "Token", "Token Type", "Token Value");
    printf("------------------------------------------------------------\n");
    scan(src);
    fclose(src);
    
    // Cleanup symbol table
    Symbol *current = symbol_table;
    while (current) {
        Symbol *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    
    return 0;
}