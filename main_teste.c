#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "inc/quick_sort.h"
#include "inc/utils.h"
#include "inc/text_processor.h"

/* ============================================================
 * CONSTANTES PARA TESTE
 * ============================================================ */

#define NUM_RUNS 5 // Execuções para calcular média

/* ============================================================
 * FUNÇÕES AUXILIARES
 * ============================================================ */

/**
 * Gera um array de strings aleatórias
 */
static char **generate_random_strings(int num_strings)
{
    const char *base_words[] = {
        "casa", "carro", "computador", "programa", "algoritmo",
        "dados", "estrutura", "processamento", "memoria", "cache",
        "processador", "thread", "sincronizacao", "mutex", "barreira",
        "concorrencia", "desempenho", "otimizacao", "benchmark", "teste",
        "quick", "sort", "busca", "texto", "arquivo",
        "paralelismo", "concorrente", "sistema", "operacional", "kernel"};
    int num_base_words = sizeof(base_words) / sizeof(base_words[0]);

    char **strings = malloc(num_strings * sizeof(char *));
    if (!strings)
        return NULL;

    for (int i = 0; i < num_strings; i++)
    {
        const char *base = base_words[rand() % num_base_words];
        char buffer[100];

        if (rand() % 3 == 0)
        {
            snprintf(buffer, sizeof(buffer), "%s%d", base, rand() % 100);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%s", base);
        }

        strings[i] = cp_string(buffer);
        if (!strings[i])
        {
            for (int j = 0; j < i; j++)
                free(strings[j]);
            free(strings);
            return NULL;
        }
    }

    return strings;
}

static char **copy_strings_array(char **original, int num_strings)
{
    char **copy = malloc(num_strings * sizeof(char *));
    if (!copy)
        return NULL;

    for (int i = 0; i < num_strings; i++)
    {
        copy[i] = cp_string(original[i]);
        if (!copy[i])
        {
            for (int j = 0; j < i; j++)
                free(copy[j]);
            free(copy);
            return NULL;
        }
    }

    return copy;
}

static void free_strings_array(char **strings, int num_strings)
{
    if (!strings)
        return;
    for (int i = 0; i < num_strings; i++)
    {
        if (strings[i])
            free(strings[i]);
    }
    free(strings);
}

static void print_sample(char **strings, int num_strings, int n)
{
    int count = (n < num_strings) ? n : num_strings;
    for (int i = 0; i < count; i++)
    {
        printf("  [%03d] \"%s\"\n", i, strings[i]);
    }
    if (num_strings > n)
        printf("  ... (%d elementos)\n", num_strings - n);
}

static void run_main_test()
{
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║          QUICK SORT PARALELO - TESTE PRINCIPAL            ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");

    /* ================================================================
     * CONFIGURAÇÕES DO TESTE
     * ================================================================ */

    // Diferentes tamanhos de array para testar
    int sizes[] = {1000, 5000, 10000, 50000, 100000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    // Diferentes números de threads
    int threads[] = {1, 2, 4, 8};
    int num_threads = sizeof(threads) / sizeof(threads[0]);

    // Configurações do Quick Sort
    set_max_depth(8);         // Profundidade máxima para threads
    set_fallback_limit(1000); // Fallback para arrays <= 1000 elementos

    printf("\nCONFIGURAÇÕES:\n");
    printf("  Profundidade máxima: %d\n", get_max_depth());
    printf("  Limite de fallback: %d\n", get_fallback_limt());
    printf("  Execuções por teste: %d\n", NUM_RUNS);

    /* ================================================================
     * TESTE 1: ORDENAÇÃO COM DIFERENTES THREADS
     * ================================================================ */

    printf("\n");
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│ TESTE 1: ORDENAÇÃO COM DIFERENTES NÚMEROS DE THREADS      │\n");
    printf("└───────────────────────────────────────────────────────────┘\n");

    // Usa um tamanho médio para este teste
    int test_size = 10000;
    printf("\nTamanho do array: %d palavras\n", test_size);

    // Gera array base
    srand(time(NULL));
    char **base_array = generate_random_strings(test_size);
    if (!base_array)
    {
        printf("ERRO: Falha ao gerar array base\n");
        return;
    }

    // Cabeçalho da tabela
    printf("\n");
    printf("  Threads  Tempo (s)   Comparações  Trocas   Speedup  Ordenado?\n");
    printf("  -------  ---------   -----------  ------   -------  ---------\n");

    double sequential_time = 0.0;

    // Testa cada número de threads
    for (int t = 0; t < num_threads; t++)
    {
        int num_th = threads[t];
        double total_time = 0.0;
        long long total_comparisions = 0;
        long long total_swaps = 0;
        bool all_sorted = true;

        // Executa NUM_RUNS vezes para calcular média
        for (int run = 0; run < NUM_RUNS; run++)
        {
            // Cria cópia do array
            char **copy = copy_strings_array(base_array, test_size);
            if (!copy)
            {
                printf("ERRO: Falha ao copiar array\n");
                break;
            }

            // Mede tempo de execução
            struct timeval start, end;
            gettimeofday(&start, NULL);

            // Executa Quick Sort
            SortResult result = quick_sort_parallel(copy, test_size, num_th,
                                                    compare_strings);

            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
                             (end.tv_usec - start.tv_usec) / 1000000.0;

            total_time += elapsed;
            total_comparisions += result.comparisions;
            total_swaps += result.swaps;

            // Verifica se está ordenado
            if (!verify_sort(copy, test_size, compare_strings))
            {
                all_sorted = false;
            }

            // Libera cópia
            for (int i = 0; i < test_size; i++)
                free(copy[i]);
            free(copy);
        }

        // Calcula médias
        double avg_time = total_time / NUM_RUNS;
        long long avg_comparisions = total_comparisions / NUM_RUNS;
        long long avg_swaps = total_swaps / NUM_RUNS;

        // Guarda tempo sequencial para calcular speedup
        if (t == 0)
        {
            sequential_time = avg_time;
        }

        // Calcula speedup
        double speedup = (sequential_time > 0) ? sequential_time / avg_time : 1.0;

        // Exibe resultados
        printf("  %3d      %.6f    %11lld  %7lld   %6.2fx  %s\n",
               num_th, avg_time, avg_comparisions, avg_swaps,
               speedup, all_sorted ? "SIM " : "NÃO ");
    }

    // Libera array base
    free_strings_array(base_array, test_size);

    /* ================================================================
     * TESTE 2: DIFERENTES TAMANHOS DE ARRAY
     * ================================================================ */

    printf("\n");
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│ TESTE 2: DIFERENTES TAMANHOS DE ARRAY (4 THREADS)         │\n");
    printf("└───────────────────────────────────────────────────────────┘\n");

    printf("\n");
    printf("  Tamanho   Tempo (s)   Comparações  Trocas   Speedup  Ordenado?\n");
    printf("  -------   ---------   -----------  ------   -------  ---------\n");

    int fixed_threads = 4;
    double base_time = 0.0;

    for (int s = 0; s < num_sizes; s++)
    {
        int size = sizes[s];

        // Gera array aleatório
        char **array = generate_random_strings(size);
        if (!array)
        {
            printf("ERRO: Falha ao gerar array de tamanho %d\n", size);
            continue;
        }

        double total_time = 0.0;
        long long total_comparisions = 0;
        long long total_swaps = 0;
        bool all_sorted = true;

        // Executa NUM_RUNS vezes
        for (int run = 0; run < NUM_RUNS; run++)
        {
            char **copy = copy_strings_array(array, size);
            if (!copy)
            {
                printf("ERRO: Falha ao copiar array\n");
                break;
            }

            struct timeval start, end;
            gettimeofday(&start, NULL);

            SortResult result = quick_sort_parallel(copy, size, fixed_threads,
                                                    compare_strings);

            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
                             (end.tv_usec - start.tv_usec) / 1000000.0;

            total_time += elapsed;
            total_comparisions += result.comparisions;
            total_swaps += result.swaps;

            if (!verify_sort(copy, size, compare_strings))
            {
                all_sorted = false;
            }

            for (int i = 0; i < size; i++)
                free(copy[i]);
            free(copy);
        }

        double avg_time = total_time / NUM_RUNS;
        long long avg_comparisions = total_comparisions / NUM_RUNS;
        long long avg_swaps = total_swaps / NUM_RUNS;

        // Speedup relativo ao primeiro tamanho
        if (s == 0)
            base_time = avg_time;
        double speedup = (base_time > 0) ? base_time / avg_time : 1.0;

        printf("  %7d   %.6f    %11lld  %7lld   %6.2fx  %s\n",
               size, avg_time, avg_comparisions, avg_swaps,
               speedup, all_sorted ? "SIM" : "NÃO");

        free_strings_array(array, size);
    }

    printf("\n");
    printf("┌───────────────────────────────────────────────────────────┐\n");
    printf("│ TESTE 3: INTEGRAÇÃO COM TOKENIZAÇÃO DE TEXTO              │\n");
    printf("└───────────────────────────────────────────────────────────┘\n");

    // Texto de exemplo para teste
    const char *text =
        "O rato roeu a roupa do rei de Roma. "
        "O rato é pequeno, mas corajoso. "
        "A roupa do rei era muito bonita. "
        "Roma é uma cidade antiga e fascinante. "
        "O rei gostava de andar pelas ruas de Roma. "
        "O rato e o rei se tornaram amigos improváveis.";

    printf("\nTexto de exemplo:\n");
    printf("  \"%s\"\n\n", text);

    // Tokeniza o texto
    int num_words;
    WordPosition *words = tokenize_txt_simple(text, &num_words);

    if (words && num_words > 0)
    {
        printf("Palavras únicas encontradas: %d\n", num_words);

        // Converte WordPosition para array de strings
        char **string_array = malloc(num_words * sizeof(char *));
        if (string_array)
        {
            for (int i = 0; i < num_words; i++)
            {
                string_array[i] = cp_string(words[i].word);
            }

            // Mostra palavras antes da ordenação
            printf("\nPalavras extraídas (amostra):\n");
            print_sample(string_array, num_words, 10);

            // Ordena com Quick Sort (4 threads)
            printf("\nOrdenando com 4 threads...\n");
            struct timeval start, end;
            gettimeofday(&start, NULL);

            SortResult result = quick_sort_parallel(string_array, num_words, 4,
                                                    compare_strings);

            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
                             (end.tv_usec - start.tv_usec) / 1000000.0;

            // Mostra palavras depois da ordenação
            printf("\nPalavras ordenadas (amostra):\n");
            print_sample(string_array, num_words, 10);

            // Verifica ordenação
            bool sorted = verify_sort(string_array, num_words, compare_strings);

            // Exibe estatísticas
            printf("\nEstatísticas:\n");
            printf("  Tempo de ordenação: %.6f segundos\n", elapsed);
            printf("  Comparações: %lld\n", result.comparisions);
            printf("  Trocas: %lld\n", result.swaps);
            printf("  Ordenado: %s\n", sorted ? "SIM " : "NÃO ");

            // Encontra palavras mais frequentes
            int freq_count;
            WordPosition *frequentes = find_frequent(words, num_words, 5, &freq_count);
            if (frequentes && freq_count > 0)
            {
                printf("\nPalavras mais frequentes:\n");
                for (int i = 0; i < freq_count; i++)
                {
                    printf("  '%s' aparece %d vez(es)\n",
                           frequentes[i].word, frequentes[i].num_position);
                }
                for (int i = 0; i < freq_count; i++)
                {
                    free(frequentes[i].word);
                    free(frequentes[i].position);
                }
                free(frequentes);
            }

            // Libera memória
            for (int i = 0; i < num_words; i++)
            {
                free(string_array[i]);
            }
            free(string_array);
        }

        destroy_words(words, num_words);
    }
}

/* ============================================================
 * FUNÇÃO PRINCIPAL
 * ============================================================ */

int main(int argc, char **argv)
{
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║     QUICK SORT PARALELO - TRABALHO ARQ II                 ║\n");
    printf("║     Ordenação de texto com quick-sort                     ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");

    // Inicializa seed para números aleatórios
    srand(time(NULL));

    // Executa teste principal
    run_main_test();

    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║              FIM DO PROGRAMA DE TESTE                     ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return 0;
}