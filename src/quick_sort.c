#include "../inc/quick_sort.h"
#include "../inc/utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

/* ============================================================
 * CONSTANTES INTERNAS
 * ============================================================ */

#define DEFAULT_MAX_DEPTH 10        // Profundidade máxima padrão para threads
#define DEFAULT_FALLBACK_LIMIT 1000 // Abaixo disso, usa sequencial
#define MIN_PARALLEL_SIZE 10000     // Tamanho mínimo para usar paralelo

/* ============================================================
 * VARIÁVEIS GLOBAIS (Configuração)
 * ============================================================ */

static int global_max_depth = DEFAULT_MAX_DEPTH;
static int global_fallback_limit = DEFAULT_FALLBACK_LIMIT;

/* ============================================================
 * FUNÇÕES AUXILIARES INTERNAS (STATIC)
 * ============================================================ */

/**
 * Troca duas strings em um array
 * Usada durante o particionamento
 */
static void swap_strings(char **a, char **b)
{
    char *temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Calcula a profundidade máxima baseada no número de threads
 * Isso garante aproximadamente num_threads threads
 */
static int calculate_max_depth(int num_threads)
{
    if (num_threads <= 1)
        return 0;
    return (int)(log2(num_threads)) + 1;
}

/**
 * Verifica se deve usar paralelo ou sequencial
 * Baseado no tamanho do array e configurações
 */
static bool should_use_parallel(int size, int num_threads)
{
    return (size > MIN_PARALLEL_SIZE && num_threads > 1);
}

/* ============================================================
 * FUNÇÕES DE COMPARAÇÃO (Públicas)
 * ============================================================ */

/**
 * Comparação padrão para strings (strcmp)
 * Retorna: 0 se iguais, negativo se a < b, positivo se a > b
 */
int compare_strings(const char *a, const char *b)
{
    if (!a && !b)
        return 0;
    if (!a)
        return -1;
    if (!b)
        return 1;
    return strcmp(a, b);
}

/**
 * Comparação ignorando maiúsculas/minúsculas
 * Útil para busca case-insensitive
 */
int compare_strings_ignore_case(const char *a, const char *b)
{
    if (!a && !b)
        return 0;
    if (!a)
        return -1;
    if (!b)
        return 1;
    return compare_ignore_case(a, b); // Função de utils.h
}

/**
 * Comparação por tamanho da string
 * Útil para ordenar palavras por comprimento
 */
int compare_by_size(const char *a, const char *b)
{
    if (!a && !b)
        return 0;
    if (!a)
        return -1;
    if (!b)
        return 1;

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    if (len_a < len_b)
        return -1;
    if (len_a > len_b)
        return 1;
    return strcmp(a, b); // Se mesmo tamanho, ordem alfabética
}

/* ============================================================
 * FUNÇÕES DE PARTICIONAMENTO
 * ============================================================ */

/**
 * Particionamento Lomuto para strings
 *
 * Como funciona:
 * 1. Escolhe o último elemento como pivô
 * 2. Percorre o array da esquerda para a direita
 * 3. Mantém uma região de elementos menores que o pivô
 * 4. No final, coloca o pivô na posição correta
 *
 * Retorna: posição final do pivô
 */
int partition_strings(char **words, int left, int right,
                      int (*compare)(const char *, const char *),
                      long long *comparisions, long long *swaps)
{
    // Escolhe o último elemento como pivô
    char *pivot = words[right];
    int i = left - 1; // Índice do último elemento menor que o pivô

    // Percorre todos os elementos exceto o pivô
    for (int j = left; j < right; j++)
    {
        // Incrementa contador de comparações
        if (comparisions)
            (*comparisions)++;

        // Se elemento atual é menor ou igual ao pivô
        if (compare(words[j], pivot) <= 0)
        {
            i++; // Expande a região de elementos menores
            if (i != j)
            { // Evita troca desnecessária
                swap_strings(&words[i], &words[j]);
                if (swaps)
                    (*swaps)++;
            }
        }
    }

    // Coloca o pivô na posição correta (após todos os menores)
    if (i + 1 != right)
    {
        swap_strings(&words[i + 1], &words[right]);
        if (swaps)
            (*swaps)++;
    }

    return i + 1; // Retorna posição do pivô
}

/**
 * Particionamento Hoare (alternativo ao Lomuto)
 *
 * Como funciona:
 * 1. Escolhe o elemento do meio como pivô
 * 2. Dois ponteiros: um da esquerda, um da direita
 * 3. Move o ponteiro esquerdo até encontrar elemento >= pivô
 * 4. Move o ponteiro direito até encontrar elemento <= pivô
 * 5. Troca os elementos e continua
 *
 * Vantagem: Geralmente faz menos trocas que Lomuto
 */
int partition_hoare(char **words, int left, int right,
                    int (*compare)(const char *, const char *),
                    long long *comparisions, long long *swaps)
{
    // Escolhe o elemento do meio como pivô
    char *pivot = words[left + (right - left) / 2];
    int i = left - 1;
    int j = right + 1;

    while (1)
    {
        // Move i para a direita enquanto elementos < pivô
        do
        {
            i++;
            if (comparisions)
                (*comparisions)++;
        } while (compare(words[i], pivot) < 0);

        // Move j para a esquerda enquanto elementos > pivô
        do
        {
            j--;
            if (comparisions)
                (*comparisions)++;
        } while (compare(words[j], pivot) > 0);

        // Se os ponteiros se cruzaram, terminamos
        if (i >= j)
        {
            return j; // Retorna posição de partição
        }

        // Troca elementos que estão no lado errado
        if (i != j)
        {
            swap_strings(&words[i], &words[j]);
            if (swaps)
                (*swaps)++;
        }
    }
}

/* ============================================================
 * QUICK SORT SEQUENCIAL (FALLBACK)
 * ============================================================ */

/**
 * Versão sequencial do Quick Sort
 * Usada como fallback quando:
 * - Array é pequeno (abaixo do limite)
 * - Profundidade máxima foi atingida
 * - Número de threads = 1
 */
void sequential_quick_sort(char **words, int left, int right,
                           int (*compare)(const char *, const char *),
                           long long *comparisions, long long *swaps)
{
    // Caso base: array vazio ou com 1 elemento
    if (left >= right)
        return;

    // Verifica se o array é muito pequeno para continuar recursão
    if (right - left < 10)
    {
        // Para arrays pequenos, insertion sort seria melhor
        // Mas mantemos Quick Sort para simplicidade
    }

    // Particiona o array (usando Lomuto como padrão)
    int pivot = partition_strings(words, left, right,
                                  compare, comparisions, swaps);

    // Ordena recursivamente as duas metades
    sequential_quick_sort(words, left, pivot - 1,
                          compare, comparisions, swaps);
    sequential_quick_sort(words, pivot + 1, right,
                          compare, comparisions, swaps);
}

/* ============================================================
 * QUICK SORT PARALELO (WORKER)
 * ============================================================ */

/**
 * Função executada por cada thread
 * Esta é a função que será passada para pthread_create
 *
 * Como funciona o paralelismo:
 * 1. Cada thread recebe uma parte do array para ordenar
 * 2. Se a profundidade atual < profundidade máxima:
 *    - Cria duas novas threads para as metades (esquerda/direita)
 *    - Espera ambas terminarem (pthread_join)
 * 3. Se profundidade máxima atingida:
 *    - Usa versão sequencial (fallback)
 */
void *quick_sort_worker(void *arg)
{
    // Converte o argumento para o tipo correto
    QuickSortArgs *args = (QuickSortArgs *)arg;

    // Extrai os dados da estrutura
    char ***words_ptr = args->words;
    char **words = *words_ptr; // Dereferencia o ponteiro duplo
    int left = args->initial;
    int right = args->end;
    int depth = args->depth;
    int max_depth = args->max_depth;
    int (*compare)(const char *, const char *) = args->compare;
    long long *comparisions = args->comparisions;
    long long *swaps = args->swaps;

    // Caso base: array vazio ou com 1 elemento
    if (left >= right)
    {
        return NULL;
    }

    // Verifica se o tamanho é muito pequeno para paralelizar
    if ((right - left) < global_fallback_limit)
    {
        // Usa sequencial para arrays pequenos
        sequential_quick_sort(words, left, right,
                              compare, comparisions, swaps);
        return NULL;
    }

    // Particiona o array
    int pivot = partition_strings(words, left, right,
                                  compare, comparisions, swaps);

    // Verifica se deve criar threads ou usar sequencial
    if (depth < max_depth)
    {
        // Prepara dados para as threads filhas
        QuickSortArgs left_args = {
            .words = words_ptr,
            .initial = left,
            .end = pivot - 1,
            .depth = depth + 1,
            .max_depth = max_depth,
            .compare = compare,
            .comparisions = comparisions,
            .swaps = swaps};

        QuickSortArgs right_args = {
            .words = words_ptr,
            .initial = pivot + 1,
            .end = right,
            .depth = depth + 1,
            .max_depth = max_depth,
            .compare = compare,
            .comparisions = comparisions,
            .swaps = swaps};

        pthread_t left_thread, right_thread;

        // Cria threads para ambas as metades
        // Nota: Usamos o mesmo ponteiro words_ptr em ambas
        // Isso é seguro porque as threads trabalham em partes diferentes do array
        pthread_create(&left_thread, NULL, quick_sort_worker, &left_args);
        pthread_create(&right_thread, NULL, quick_sort_worker, &right_args);

        // Aguarda ambas as threads terminarem
        pthread_join(left_thread, NULL);
        pthread_join(right_thread, NULL);
    }
    else
    {
        // Profundidade máxima atingida: usa sequencial
        sequential_quick_sort(words, left, pivot - 1,
                              compare, comparisions, swaps);
        sequential_quick_sort(words, pivot + 1, right,
                              compare, comparisions, swaps);
    }

    return NULL;
}

/* ============================================================
 * QUICK SORT PARALELO (WRAPPER)
 * ============================================================ */

/**
 * Wrapper principal para Quick Sort paralelo
 *
 * Fluxo de execução:
 * 1. Verifica se o array deve ser ordenado em paralelo
 * 2. Se sim, cria a thread inicial e espera
 * 3. Coleta métricas (tempo, comparações, etc.)
 * 4. Retorna SortResult com todas as estatísticas
 */
SortResult quick_sort_parallel(char **words, int num_words,
                               int num_threads,
                               int (*compare)(const char *, const char *))
{
    SortResult result = {0};
    struct timeval start, end;

    // Validação de parâmetros
    if (!words || num_words <= 1)
    {
        result.total_time = 0.0;
        result.comparisions = 0;
        result.swaps = 0;
        return result;
    }

    // Verifica se deve usar paralelo
    if (!should_use_parallel(num_words, num_threads))
    {
        // Usa sequencial
        gettimeofday(&start, NULL);
        sequential_quick_sort(words, 0, num_words - 1,
                              compare, &result.comparisions, &result.swaps);
        gettimeofday(&end, NULL);

        result.total_time = (end.tv_sec - start.tv_sec) +
                            (end.tv_usec - start.tv_usec) / 1000000.0;
        result.max_depth = 0;
        result.created_threads = 0;
        return result;
    }

    // Calcula profundidade máxima baseada no número de threads
    int max_depth = calculate_max_depth(num_threads);
    if (max_depth > global_max_depth)
    {
        max_depth = global_max_depth;
    }

    // Inicializa contadores
    long long comparisions = 0;
    long long swaps = 0;

    // Prepara dados para a thread principal
    QuickSortArgs args = {
        .words = &words,
        .initial = 0,
        .end = num_words - 1,
        .depth = 0,
        .max_depth = max_depth,
        .compare = compare,
        .comparisions = &comparisions,
        .swaps = &swaps};

    // Inicia cronômetro
    gettimeofday(&start, NULL);

    // Executa Quick Sort paralelo (thread principal é a primeira)
    quick_sort_worker(&args);

    // Para cronômetro
    gettimeofday(&end, NULL);

    // Preenche resultado
    result.total_time = (end.tv_sec - start.tv_sec) +
                        (end.tv_usec - start.tv_usec) / 1000000.0;
    result.comparisions = comparisions;
    result.swaps = swaps;
    result.max_depth = max_depth;

    // Estima número de threads criadas (2^max_depth - 1)
    result.created_threads = (1 << max_depth) - 1;
    if (result.created_threads < 1)
        result.created_threads = 1;

    return result;
}

/* ============================================================
 * FUNÇÕES DE CONFIGURAÇÃO
 * ============================================================ */

/**
 * Define a profundidade máxima de threads
 * Valores mais altos criam mais threads, mas aumentam overhead
 */
void set_max_depth(int depth)
{
    if (depth > 0 && depth < 20)
    { // Limite máximo razoável
        global_max_depth = depth;
    }
}

/**
 * Define o limite para fallback sequencial
 * Arrays menores que este valor usam versão sequencial
 */
void set_fallback_limit(int min_size)
{
    if (min_size > 0)
    {
        global_fallback_limit = min_size;
    }
}

/**
 * Retorna a profundidade máxima atual
 */
int get_max_depth(void)
{
    return global_max_depth;
}

/**
 * Retorna o limite de fallback atual
 */
int get_fallback_limt(void)
{
    return global_fallback_limit;
}

/* ============================================================
 * FUNÇÕES DE VALIDAÇÃO
 * ============================================================ */

/**
 * Verifica se o array está ordenado corretamente
 * Útil para validação dos resultados
 */
bool verify_sort(char **words, int num_words,
                 int (*compare)(const char *, const char *))
{
    if (!words || num_words <= 1)
    {
        return true;
    }

    // Verifica se cada elemento é menor ou igual ao próximo
    for (int i = 0; i < num_words - 1; i++)
    {
        if (compare(words[i], words[i + 1]) > 0)
        {
            // Elemento fora de ordem
            log_message("ERROR", "Array não está ordenado na posição %d: '%s' > '%s'",
                        i, words[i], words[i + 1]);
            return false;
        }
    }

    return true;
}

/**
 * Imprime estatísticas da ordenação de forma formatada
 * Útil para análise de desempenho
 */
void print_sort_statitics(SortResult *result)
{
    if (!result)
        return;

    printf("\n=== ESTATÍSTICAS DA ORDENAÇÃO ===\n");
    printf("Tempo total: %.6f segundos\n", result->total_time);
    printf("Comparações: %lld\n", result->comparisions);
    printf("Trocas: %lld\n", result->swaps);
    printf("Profundidade máxima: %d\n", result->max_depth);
    printf("Threads criadas: %d\n", result->created_threads);
    printf("Tempo criação de threads: %.6f segundos\n", result->created_threads_time);
    printf("Tempo sincronização: %.6f segundos\n", result->sync_time);
    printf("==================================\n\n");
}