#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "utils.h"

/**
 * Estruturas de Dados
 */

// Resultado de uma busca individual
typedef struct {
    bool found;                // Se a palavra foi encontrada
    int position;              // Posição no array (primeira ocorrência)
    int occurrences;           // Total de ocorrências
    int* positions;            // Array com todas as posições
    double search_time;        // Tempo gasto na busca
    int comparisons_made;      // Quantas comparações foram feitas
} SearchResult;

// Resultado de busca com contexto (palavras vizinhas)
typedef struct {
    char* word;
    int position;
    char* context_before;      // Palavras antes (até 5)
    char* context_after;       // Palavras depois (até 5)
} ContextResult;

/**
 * Busca Binária Básica
 */

// Busca uma palavra exata no array ordenado
SearchResult binary_search(const char** words,
                           int num_words,
                           const char* target);

// Busca com opções (case-sensitive ou não)
SearchResult binary_search_with_options(const char** words,
                                        int num_words,
                                        const char* target,
                                        bool case_sensitive);

/**
 * Busca de Múltiplas Ocorrências
 */

// Encontra todas as ocorrências de uma palavra
SearchResult find_all_occurrences(const char** words,
                                  int num_words,
                                  const char* target);

// Encontra ocorrências com contexto (palavras vizinhas)
ContextResult* search_with_context(const char** words,
                                   int num_words,
                                   const char* target,
                                   int context_size,
                                   int* num_results);

/**
 * Buscas Avançadas
 */

// Busca por prefixo (auto-complete)
char** search_by_prefix(const char** words,
                        int num_words,
                        const char* prefix,
                        int max_results,
                        int* num_found);

// Busca por padrão com curinga (*)
char** search_by_pattern(const char** words,
                         int num_words,
                         const char* pattern,
                         int max_results,
                         int* num_found);

/**
 * Validação e Utilidades
 */

// Verifica se array está ordenado (pré-requisito para busca binária)
bool check_array_sorted(const char** words, int num_words);

// Sugere correções para palavras (distância de Levenshtein)
char** suggest_corrections(const char** words,
                           int num_words,
                           const char* target,
                           int max_distance,
                           int max_suggestions,
                           int* num_suggestions);

// Calcula distância de Levenshtein entre duas palavras
int levenshtein_distance(const char* a, const char* b);

// Libera resultado de busca
void free_search_result(SearchResult* result);
void free_context_results(ContextResult* results, int num);

#endif /* BINARY_SEARCH_H */