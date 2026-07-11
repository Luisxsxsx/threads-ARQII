#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include <pthread.h>
#include "utils.h"

/**
 * Estruturas de Dados
 */

// Estrutura para dados passados para cada thread
typedef struct
{
    char ***words;                              // Ponteiro para array de strings
    int initial;                                // Índice inicial (inclusive)
    int end;                                    // Índice final (inclusive)
    int depth;                                  // Profundidade atual na recursão
    int max_depth;                              // Profundidade máxima permitida
    int (*compare)(const char *, const char *); // Função de comparação
    long long *comparisions;                    // Contador de comparações (para métricas)
    long long *swaps;                           // Contador de swaps (para métricas)
} QuickSortArgs;

// Resultados da ordenação
typedef struct
{
    double total_time;      // Tempo total de ordenação
    long long comparisions; // Número de comparações realizadas
    long long swaps;        // Número de swaps realizadas
    int max_depth;
    int created_threads; // Número total de threads criadas
    double created_threads_time;
    double sync_time;
} SortResult;

/**
 * Funções de Ordenação
 */

// Versão sequencial (fallback)
void sequential_quick_sort(char **words,
                           int left,
                           int right,
                           int (*compare)(const char *, const char *),
                           long long *comparisions,
                           long long *swaps);

// Versão paralela (wrapper principal)
SortResult quick_sort_parallel(char **words,
                               int num_words,
                               int num_threads,
                               int (*compare)(const char *, const char *));

// Função executada pelas threads (para pthread_create)
void *quick_sort_worker(void *arg);

/**
 * Funções de Particionamento
 */

// Particionamento Lomuto para strings
int partition_strings(char **words,
                      int left,
                      int right,
                      int (*compare)(const char *, const char *),
                      long long *comparisions,
                      long long *swaps);

// Particionamento Hoare (alternativo)
int partition_hoare(char **words,
                    int left,
                    int right,
                    int (*compare)(const char *, const char *),
                    long long *comparisions,
                    long long *swaps);

/**
 * Funções de Comparação
 */

// Comparação padrão para strings
int compare_strings(const char *a, const char *b);

// Comparação ignorando maiúsculas/minúsculas
int compare_strings_ignore_case(const char *a, const char *b);

// Comparação por tamanho (para ordenação por comprimento)
int compare_by_size(const char *a, const char *b);

/**
 * Configuração e Controle
 */

// Define profundidade máxima de threads
void set_max_depth(int depth);

// Define o limite para fallback sequencial
void set_fallback_limit(int min_size);

// Obtém configurações atuais
int get_max_depth(void);
int get_fallback_limt(void);

/**
 * Validação
 */

// Verifica se array está ordenado
bool verify_sort(char **words,
                 int num_words,
                 int (*compare)(const char *, const char *));

// Imprime estatísticas da ordenação
void print_sort_statitics(SortResult *resultado);

#endif /* QUICK_SORT_H */