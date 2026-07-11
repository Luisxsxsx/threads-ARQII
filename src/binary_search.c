#include "../inc/binary_search.h"
#include "../inc/utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

/**
 * Distância de Levenshtein - cálculo recursivo com programação dinâmica
 * 
 * Como funciona:
 * 1. Cria uma matriz (m+1) x (n+1) onde m e n são os tamanhos das strings
 * 2. Preenche a primeira linha e coluna com valores incrementais
 * 3. Para cada célula, calcula o custo mínimo entre:
 *    - Inserção (célula acima + 1)
 *    - Deleção (célula à esquerda + 1)
 *    - Substituição (célula diagonal + custo)
 * 4. O valor final está na célula (m, n)
 * 
 * Exemplo: distancia("casa", "caso") = 1 (substituir 'a' por 'o')
 */
static int levenshtein_distance_internal(const char* a, const char* b) {
    int len_a = strlen(a);
    int len_b = strlen(b);
    
    // Caso base: strings vazias
    if (len_a == 0) return len_b;
    if (len_b == 0) return len_a;
    
    // Aloca matriz de distâncias
    int** matrix = malloc((len_a + 1) * sizeof(int*));
    if (!matrix) return -1;
    
    for (int i = 0; i <= len_a; i++) {
        matrix[i] = malloc((len_b + 1) * sizeof(int));
        if (!matrix[i]) {
            // Limpeza em caso de erro
            for (int j = 0; j < i; j++) free(matrix[j]);
            free(matrix);
            return -1;
        }
    }
    
    // Inicializa primeira linha e coluna
    for (int i = 0; i <= len_a; i++) {
        matrix[i][0] = i;
    }
    for (int j = 0; j <= len_b; j++) {
        matrix[0][j] = j;
    }
    
    // Preenche a matriz
    for (int i = 1; i <= len_a; i++) {
        for (int j = 1; j <= len_b; j++) {
            // Custo da substituição (0 se iguais, 1 se diferentes)
            int cost = (a[i-1] == b[j-1]) ? 0 : 1;
            
            // Calcula o mínimo entre:
            // - Deleção (célula acima + 1)
            // - Inserção (célula à esquerda + 1)
            // - Substituição (célula diagonal + cost)
            int del = matrix[i-1][j] + 1;
            int ins = matrix[i][j-1] + 1;
            int sub = matrix[i-1][j-1] + cost;
            
            matrix[i][j] = (del < ins) ? (del < sub ? del : sub) : (ins < sub ? ins : sub);
        }
    }
    
    int result = matrix[len_a][len_b];
    
    // Libera matriz
    for (int i = 0; i <= len_a; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return result;
}

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

/* ============================================================
 * BUSCA DE MÚLTIPLAS OCORRÊNCIAS
 * ============================================================ */

/**
 * Encontra todas as ocorrências de uma palavra no array
 * 
 * Como funciona:
 * 1. Encontra a primeira ocorrência com busca binária
 * 2. Expande para trás (while words[i] == target)
 * 3. Expande para frente (while words[i] == target)
 * 4. Retorna array com todas as posições
 */
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

/**
 * Busca com contexto (palavras vizinhas)
 * 
 * Como funciona:
 * 1. Encontra todas as ocorrências da palavra
 * 2. Para cada ocorrência, pega N palavras antes e N depois
 * 3. Retorna estrutura com contexto
 */
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

/* ============================================================
 * BUSCAS AVANÇADAS
 * ============================================================ */

/**
 * Busca por prefixo (auto-complete)
 * 
 * Como funciona:
 * 1. Percorre o array (que está ordenado)
 * 2. Verifica se cada palavra começa com o prefixo
 * 3. Retorna as primeiras N palavras que correspondem
 * 
 * Otimização: Pode usar busca binária para encontrar o primeiro match
 */
char** search_by_prefix(const char** words, int num_words,
                        const char* prefix, int max_results,
                        int* num_found) {
    if (!words || num_words <= 0 || !prefix || !num_found || max_results <= 0) {
        *num_found = 0;
        return NULL;
    }
    
    // Converte prefixo para minúsculas se necessário
    char* prefix_lower = cp_string(prefix);
    if (!prefix_lower) {
        *num_found = 0;
        return NULL;
    }
    to_lowercase(prefix_lower);
    
    // Aloca array de resultados
    char** results = malloc(max_results * sizeof(char*));
    if (!results) {
        free(prefix_lower);
        *num_found = 0;
        return NULL;
    }
    
    int found = 0;
    int prefix_len = strlen(prefix_lower);
    
    // Percorre o array
    for (int i = 0; i < num_words && found < max_results; i++) {
        // Cria cópia minúscula para comparação
        char* word_lower = cp_string(words[i]);
        if (!word_lower) continue;
        to_lowercase(word_lower);
        
        // Verifica se começa com o prefixo
        if (strncmp(word_lower, prefix_lower, prefix_len) == 0) {
            results[found] = cp_string(words[i]);
            if (results[found]) {
                found++;
            }
        }
        
        free(word_lower);
    }
    
    free(prefix_lower);
    *num_found = found;
    
    // Se não encontrou nada, libera e retorna NULL
    if (found == 0) {
        free(results);
        return NULL;
    }
    
    return results;
}

/**
 * Busca por padrão com curinga (*)
 * 
 * Como funciona:
 * 1. O curinga (*) representa qualquer sequência de caracteres
 * 2. Exemplo: "c*s" encontra "casa", "cachorros", "carros"
 * 3. Percorre o array e verifica se cada palavra corresponde ao padrão
 */
char** search_by_pattern(const char** words, int num_words,
                         const char* pattern, int max_results,
                         int* num_found) {
    if (!words || num_words <= 0 || !pattern || !num_found || max_results <= 0) {
        *num_found = 0;
        return NULL;
    }
    
    // Aloca array de resultados
    char** results = malloc(max_results * sizeof(char*));
    if (!results) {
        *num_found = 0;
        return NULL;
    }
    
    int found = 0;
    
    // Percorre o array
    for (int i = 0; i < num_words && found < max_results; i++) {
        bool matches = true;
        int pattern_idx = 0;
        int word_idx = 0;
        int star_pos = -1;
        int match_pos = 0;
        
        // Implementação simples de matching com *
        while (pattern[pattern_idx] != '\0' && matches) {
            if (pattern[pattern_idx] == '*') {
                // Guarda posição do * para backtracking
                star_pos = pattern_idx;
                match_pos = word_idx;
                pattern_idx++;
            } else if (word_idx < (int)strlen(words[i]) &&
                       (pattern[pattern_idx] == '?' || 
                        pattern[pattern_idx] == words[i][word_idx])) {
                // Caractere corresponde
                pattern_idx++;
                word_idx++;
            } else if (star_pos != -1) {
                // Backtracking: tenta corresponder mais caracteres
                pattern_idx = star_pos + 1;
                match_pos++;
                word_idx = match_pos;
            } else {
                matches = false;
            }
        }
        
        // Verifica se chegou ao fim da palavra
        while (pattern[pattern_idx] == '*') {
            pattern_idx++;
        }
        
        if (matches && pattern[pattern_idx] == '\0' && word_idx == (int)strlen(words[i])) {
            results[found] = cp_string(words[i]);
            if (results[found]) {
                found++;
            }
        }
    }
    
    *num_found = found;
    
    if (found == 0) {
        free(results);
        return NULL;
    }
    
    return results;
}

/* ============================================================
 * VALIDAÇÃO E UTILIDADES
 * ============================================================ */

/**
 * Verifica se o array está ordenado (pré-requisito para busca binária)
 */
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

/**
 * Sugere correções para palavras usando distância de Levenshtein
 * 
 * Como funciona:
 * 1. Calcula a distância de Levenshtein entre a palavra alvo e cada palavra do array
 * 2. Retorna as palavras com distância <= max_distance
 * 3. Útil para correção ortográfica
 */
char** suggest_corrections(const char** words, int num_words,
                           const char* target, int max_distance,
                           int max_suggestions, int* num_suggestions) {
    if (!words || num_words <= 0 || !target || !num_suggestions || max_suggestions <= 0) {
        *num_suggestions = 0;
        return NULL;
    }
    
    // Aloca array de sugestões
    char** suggestions = malloc(max_suggestions * sizeof(char*));
    if (!suggestions) {
        *num_suggestions = 0;
        return NULL;
    }
    
    int found = 0;
    
    // Para cada palavra, calcula distância
    for (int i = 0; i < num_words && found < max_suggestions; i++) {
        int dist = levenshtein_distance(target, words[i]);
        
        if (dist >= 0 && dist <= max_distance) {
            suggestions[found] = cp_string(words[i]);
            if (suggestions[found]) {
                found++;
            }
        }
    }
    
    *num_suggestions = found;
    
    if (found == 0) {
        free(suggestions);
        return NULL;
    }
    
    return suggestions;
}

/**
 * Distância de Levenshtein (wrapper público)
 */
int levenshtein_distance(const char* a, const char* b) {
    if (!a || !b) return -1;
    return levenshtein_distance_internal(a, b);
}

/* ============================================================
 * LIBERAÇÃO DE MEMÓRIA
 * ============================================================ */

/**
 * Libera o resultado de uma busca
 */
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

/**
 * Libera resultados de busca com contexto
 */
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