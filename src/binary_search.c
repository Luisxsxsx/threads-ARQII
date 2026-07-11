#include "../inc/binary_search.h"
#include "../inc/utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

/**
 * Busca o índice de uma palavra usando busca binária
 * Retorna a posição ou -1 se não encontrada
 */
static int binary_search_index(const char** words, int num_words, 
                               const char* target, int* comparisons) {
    int left = 0;
    int right = num_words - 1;
    int cmp_count = 0;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        cmp_count++;
        
        int cmp = strcmp(words[mid], target);
        
        if (cmp == 0) {
            if (comparisons) *comparisons = cmp_count;
            return mid;  // Encontrou!
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (comparisons) *comparisons = cmp_count;
    return -1;  // Não encontrado
}

/**
 * Busca o índice ignorando maiúsculas/minúsculas
 */
static int binary_search_index_ignore_case(const char** words, int num_words,
                                           const char* target, int* comparisons) {
    int left = 0;
    int right = num_words - 1;
    int cmp_count = 0;
    
    // Cria cópia minúscula do alvo para comparar
    char* target_lower = cp_string(target);
    if (!target_lower) return -1;
    to_lowercase(target_lower);
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        cmp_count++;
        
        // Cria cópia minúscula da palavra atual
        char* word_lower = cp_string(words[mid]);
        if (!word_lower) {
            free(target_lower);
            return -1;
        }
        to_lowercase(word_lower);
        
        int cmp = strcmp(word_lower, target_lower);
        free(word_lower);
        
        if (cmp == 0) {
            free(target_lower);
            if (comparisons) *comparisons = cmp_count;
            return mid;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    free(target_lower);
    if (comparisons) *comparisons = cmp_count;
    return -1;
}

/* ============================================================
 * BUSCA BINÁRIA BÁSICA
 * ============================================================ */

/**
 * Busca uma palavra exata no array ordenado
 * 
 * Como funciona:
 * 1. Define left = 0, right = num_words - 1
 * 2. Enquanto left <= right:
 *    - Calcula mid = (left + right) / 2
 *    - Compara words[mid] com target
 *    - Se igual, retorna posição
 *    - Se target < words[mid], right = mid - 1
 *    - Se target > words[mid], left = mid + 1
 * 3. Retorna -1 se não encontrado
 * 
 * Complexidade: O(log n)
 */
SearchResult binary_search(const char** words, int num_words, const char* target) {
    SearchResult result = {0};
    struct timeval start, end;
    
    // Validação de parâmetros
    if (!words || num_words <= 0 || !target) {
        result.found = false;
        result.position = -1;
        result.occurrences = 0;
        result.positions = NULL;
        result.search_time = 0.0;
        result.comparisons_made = 0;
        return result;
    }
    
    // Verifica se o array está ordenado (pré-requisito)
    if (!check_array_sorted(words, num_words)) {
        log_message("WARN", "Array não está ordenado - busca binária pode falhar");
    }
    
    // Inicia cronômetro
    gettimeofday(&start, NULL);
    
    // Realiza a busca
    int pos = binary_search_index(words, num_words, target, &result.comparisons_made);
    
    // Para cronômetro
    gettimeofday(&end, NULL);
    result.search_time = (end.tv_sec - start.tv_sec) + 
                        (end.tv_usec - start.tv_usec) / 1000000.0;
    
    if (pos >= 0) {
        result.found = true;
        result.position = pos;
        result.occurrences = 1;
        
        // Aloca array para posições
        result.positions = malloc(sizeof(int));
        if (result.positions) {
            result.positions[0] = pos;
        }
    } else {
        result.found = false;
        result.position = -1;
        result.occurrences = 0;
        result.positions = NULL;
    }
    
    return result;
}

/**
 * Busca com opções (case-sensitive ou não)
 */
SearchResult binary_search_with_options(const char** words, int num_words,
                                        const char* target, bool case_sensitive) {
    SearchResult result = {0};
    struct timeval start, end;
    
    // Validação
    if (!words || num_words <= 0 || !target) {
        result.found = false;
        result.position = -1;
        result.occurrences = 0;
        result.positions = NULL;
        result.search_time = 0.0;
        result.comparisons_made = 0;
        return result;
    }
    
    // Inicia cronômetro
    gettimeofday(&start, NULL);
    
    int pos;
    if (case_sensitive) {
        pos = binary_search_index(words, num_words, target, &result.comparisons_made);
    } else {
        pos = binary_search_index_ignore_case(words, num_words, target, 
                                             &result.comparisons_made);
    }
    
    // Para cronômetro
    gettimeofday(&end, NULL);
    result.search_time = (end.tv_sec - start.tv_sec) + 
                        (end.tv_usec - start.tv_usec) / 1000000.0;
    
    if (pos >= 0) {
        result.found = true;
        result.position = pos;
        result.occurrences = 1;
        result.positions = malloc(sizeof(int));
        if (result.positions) {
            result.positions[0] = pos;
        }
    } else {
        result.found = false;
        result.position = -1;
        result.occurrences = 0;
        result.positions = NULL;
    }
    
    return result;
}

SearchResult find_all_occurrences(const char** words, int num_words, const char* target) {
    SearchResult result = {0};
    struct timeval start, end;
    
    if (!words || num_words <= 0 || !target) {
        result.found = false;
        result.position = -1;
        result.occurrences = 0;
        result.positions = NULL;
        result.search_time = 0.0;
        result.comparisons_made = 0;
        return result;
    }
    
    gettimeofday(&start, NULL);
    
    // Encontra a primeira ocorrência
    int first_pos = binary_search_index(words, num_words, target, 
                                       &result.comparisons_made);
    
    if (first_pos == -1) {
        gettimeofday(&end, NULL);
        result.search_time = (end.tv_sec - start.tv_sec) + 
                            (end.tv_usec - start.tv_usec) / 1000000.0;
        result.found = false;
        result.position = -1;
        result.occurrences = 0;
        result.positions = NULL;
        return result;
    }
    
    // Conta ocorrências (expande para trás)
    int count = 1;
    int left = first_pos - 1;
    while (left >= 0 && strcmp(words[left], target) == 0) {
        count++;
        left--;
    }
    
    // Expande para frente (começa da primeira posição + 1)
    int right = first_pos + 1;
    while (right < num_words && strcmp(words[right], target) == 0) {
        count++;
        right++;
    }
    
    // Aloca array para todas as posições
    int* all_positions = malloc(count * sizeof(int));
    if (!all_positions) {
        gettimeofday(&end, NULL);
        result.search_time = (end.tv_sec - start.tv_sec) + 
                            (end.tv_usec - start.tv_usec) / 1000000.0;
        result.found = true;
        result.position = first_pos;
        result.occurrences = count;
        result.positions = NULL;
        return result;
    }
    
    // Preenche array de posições (em ordem)
    int idx = 0;
    for (int i = left + 1; i < right; i++) {
        all_positions[idx++] = i;
    }
    
    gettimeofday(&end, NULL);
    result.search_time = (end.tv_sec - start.tv_sec) + 
                        (end.tv_usec - start.tv_usec) / 1000000.0;
    result.found = true;
    result.position = first_pos;
    result.occurrences = count;
    result.positions = all_positions;
    
    return result;
}

ContextResult* search_with_context(const char** words, int num_words,
                                   const char* target, int context_size,
                                   int* num_results) {
    if (!words || num_words <= 0 || !target || !num_results) {
        *num_results = 0;
        return NULL;
    }
    
    // Primeiro encontra todas as ocorrências
    SearchResult search = find_all_occurrences(words, num_words, target);
    if (!search.found || search.occurrences == 0) {
        *num_results = 0;
        free_search_result(&search);
        return NULL;
    }
    
    // Aloca array de resultados
    ContextResult* results = malloc(search.occurrences * sizeof(ContextResult));
    if (!results) {
        *num_results = 0;
        free_search_result(&search);
        return NULL;
    }
    
    // Para cada ocorrência, captura contexto
    for (int i = 0; i < search.occurrences; i++) {
        int pos = search.positions[i];
        
        // Copia a palavra
        results[i].word = cp_string(words[pos]);
        results[i].position = pos;
        
        // Contexto anterior (N palavras antes)
        int start = pos - context_size;
        if (start < 0) start = 0;
        
        // Calcula tamanho do contexto anterior
        int before_len = 0;
        for (int j = start; j < pos; j++) {
            before_len += strlen(words[j]) + 1;  // +1 para espaço
        }
        
        results[i].context_before = malloc((before_len + 1) * sizeof(char));
        if (results[i].context_before) {
            results[i].context_before[0] = '\0';
            for (int j = start; j < pos; j++) {
                strcat(results[i].context_before, words[j]);
                if (j < pos - 1) {
                    strcat(results[i].context_before, " ");
                }
            }
        }
        
        // Contexto posterior (N palavras depois)
        int end = pos + context_size;
        if (end >= num_words) end = num_words - 1;
        
        int after_len = 0;
        for (int j = pos + 1; j <= end; j++) {
            after_len += strlen(words[j]) + 1;
        }
        
        results[i].context_after = malloc((after_len + 1) * sizeof(char));
        if (results[i].context_after) {
            results[i].context_after[0] = '\0';
            for (int j = pos + 1; j <= end; j++) {
                strcat(results[i].context_after, words[j]);
                if (j < end) {
                    strcat(results[i].context_after, " ");
                }
            }
        }
    }
    
    *num_results = search.occurrences;
    free_search_result(&search);
    return results;
}

bool check_array_sorted(const char** words, int num_words) {
    if (!words || num_words <= 1) {
        return true;
    }
    
    for (int i = 0; i < num_words - 1; i++) {
        if (strcmp(words[i], words[i + 1]) > 0) {
            return false;
        }
    }
    
    return true;
}

void free_search_result(SearchResult* result) {
    if (!result) return;
    
    if (result->positions) {
        free(result->positions);
        result->positions = NULL;
    }
    
    result->found = false;
    result->position = -1;
    result->occurrences = 0;
    result->search_time = 0.0;
    result->comparisons_made = 0;
}

void free_context_results(ContextResult* results, int num) {
    if (!results) return;
    
    for (int i = 0; i < num; i++) {
        if (results[i].word) {
            free(results[i].word);
        }
        if (results[i].context_before) {
            free(results[i].context_before);
        }
        if (results[i].context_after) {
            free(results[i].context_after);
        }
    }
    
    free(results);
}