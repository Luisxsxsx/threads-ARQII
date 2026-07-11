#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "utils.h"

/**
 * Estruturas de Dados
 */

// Resultado de uma busca individual
typedef struct {
    bool found;                
    int position;              
    int occurrences;           
    int* positions;            
    double search_time;        
    int comparisons_made;      
} SearchResult;

// Resultado de busca com contexto (palavras vizinhas)
typedef struct {
    char* word;
    int position;
    char* context_before;      
    char* context_after;       
} ContextResult;


// Busca uma palavra exata no array ordenado
SearchResult binary_search(const char** words,
                           int num_words,
                           const char* target);

// Busca com opções (case-sensitive ou não)
SearchResult binary_search_with_options(const char** words,
                                        int num_words,
                                        const char* target,
                                        bool case_sensitive);

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



// Verifica se array está ordenado (pré-requisito para busca binária)
bool check_array_sorted(const char** words, int num_words);

// Libera resultado de busca
void free_search_result(SearchResult* result);
void free_context_results(ContextResult* results, int num);

#endif /* BINARY_SEARCH_H */