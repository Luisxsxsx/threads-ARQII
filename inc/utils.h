#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Gerenciamento de Memória
 */
char **allocate_array_strings(int num_strings, int max_length);
void free_array_strings(char **array, int num_strings);
char *cp_string(const char *origem);

/**
 * Manipulação de Strings
 */
void to_lowercase(char *str);
void rm_spaces(char *str);
int compare_ignore_case(const char *a, const char *b);
char *trim_string(char *str);

/**
 * Validação e Logging
 */
bool verify_mem(void *ptr, const char *mensagem);
void log_message(const char *nivel, const char *mensagem, ...);
void log_time(const char *operacao, double tempo);

/**
 * Utilitários de Array
 */
int find_max(int *array, int tamanho);
int find_min(int *array, int tamanho);
double calc_med(int *array, int tamanho);
void print_array_strings(char **array, int tamanho, int limite);

#endif /* UTILS_H */