#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include <pthread.h>
#include "utils.h"

// Estrutura para dados passados para cada thread
typedef struct
{
    char ***words;                              
    int initial;                                
    int end;                                    
    int depth;                                  
    int max_depth;                              
    int (*compare)(const char *, const char *); 
    long long *comparisions;                    
    long long *swaps;                           
} QuickSortArgs;

// Resultados da ordenação
typedef struct
{
    double total_time;      
    long long comparisions; 
    long long swaps;        
    int max_depth;
    int created_threads; 
    double created_threads_time;
    double sync_time;
} SortResult;


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
void print_sort_stats(SortResult *resultado);

#endif /* QUICK_SORT_H */