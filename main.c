#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "inc/text_processor.h"
#include "inc/quick_sort.h"
#include "inc/binary_search.h"
#include "inc/utils.h"


static void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

static bool ask_to_continue(void) {
    char resposta;
    printf("\nDeseja buscar outra palavra? (s/N): ");
    scanf(" %c", &resposta);
    clear_input_buffer();
    return (resposta == 's' || resposta == 'S');
}

/**
 * Imprime uma linha separadora
 */
static void print_separator(char c, int size) {
    for (int i = 0; i < size; i++) {
        printf("%c", c);
    }
    printf("\n");
}


static void print_word_sample(char** words, int num_words) {
    printf("\nPalavras ordenadas (amostra das primeiras 20):\n");
    print_separator('-', 60);
    
    int sample_size = (num_words < 20) ? num_words : 20;
    for (int i = 0; i < sample_size; i++) {
        printf("  [%3d] \"%s\"\n", i, words[i]);
    }
    
    if (num_words > 20) {
        printf("  ... (%d palavras não mostradas)\n", num_words - 20);
    }
    print_separator('-', 60);
}



static void print_search_result(SearchResult result, const char* target) {
    printf("\nResultado da busca:\n");
    print_separator('-', 60);
    
    if (result.found) {
        printf("   Palavra '%s' ENCONTRADA!\n", target);
        printf("  Posição: %d\n", result.position);
        printf("  Ocorrências: %d\n", result.occurrences);
        printf("  Tempo de busca: %.6f segundos\n", result.search_time);
        printf("  Comparações realizadas: %d\n", result.comparisons_made);
        
        if (result.occurrences > 1 && result.positions) {
            printf("  Posições: ");
            for (int i = 0; i < result.occurrences && i < 10; i++) {
                printf("%d ", result.positions[i]);
            }
            if (result.occurrences > 10) {
                printf("... (+%d)", result.occurrences - 10);
            }
            printf("\n");
        }
    } else {
        printf("   Palavra '%s' NÃO ENCONTRADA\n", target);
        printf("  Tempo de busca: %.6f segundos\n", result.search_time);
        printf("  Comparações realizadas: %d\n", result.comparisons_made);
    }
    print_separator('-', 60);
}

static void print_context(ContextResult* contexts, int num_results, const char* target) {
    if (!contexts || num_results == 0) {
        printf("\nNenhum contexto disponível.\n");
        return;
    }
    
    printf("\nContexto da palavra '%s':\n", target);
    print_separator('-', 60);
    
    for (int i = 0; i < num_results && i < 5; i++) {
        printf("\nOcorrência %d (posição %d):\n", i + 1, contexts[i].position);
        
        if (contexts[i].context_before && strlen(contexts[i].context_before) > 0) {
            printf("  Antes: %s\n", contexts[i].context_before);
        } else {
            printf("  Antes: (início do texto)\n");
        }
        
        printf("  Palavra: [%s]\n", contexts[i].word);
        
        if (contexts[i].context_after && strlen(contexts[i].context_after) > 0) {
            printf("  Depois: %s\n", contexts[i].context_after);
        } else {
            printf("  Depois: (fim do texto)\n");
        }
    }
    
    if (num_results > 5) {
        printf("\n  ... (%d ocorrências não mostradas)\n", num_results - 5);
    }
    print_separator('-', 60);
}

static void process_search(char** sorted_words, int num_words) {
    char target[100];
    
    do {
        printf("\n");
        print_separator('-', 60);
        printf("Digite a palavra para buscar: ");
        scanf("%99s", target);
        clear_input_buffer();
        
        // Converte para minúsculas para busca case-insensitive
        char target_lower[100];
        strcpy(target_lower, target);
        to_lowercase(target_lower);
        
        // Realiza busca
        SearchResult result = binary_search_with_options(
            (const char**)sorted_words, 
            num_words, 
            target_lower,
            false  // case-insensitive
        );
        
        print_search_result(result, target);
        
        // Se encontrou, mostra contexto
        if (result.found) {
            int num_contexts;
            ContextResult* contexts = search_with_context(
                (const char**)sorted_words,
                num_words,
                target_lower,
                3,  // contexto de 3 palavras
                &num_contexts
            );
            
            print_context(contexts, num_contexts, target);
            
            // Libera memória do contexto
            if (contexts) {
                free_context_results(contexts, num_contexts);
            }
        }
        
        // Libera resultado da busca
        free_search_result(&result);
        
    } while (ask_to_continue());
}

// ============================================================ 

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("Como usar:\n");
        printf("  %s \"texto para buscar palavras\"\n\n", argv[0]);
        printf("Exemplo:\n");
        printf("  %s \"O rato roeu a roupa do rei de roma. O rato era feio e o rei Bonito\"\n\n", argv[0]);
        printf("Se o texto tiver espaços, coloque entre aspas.\n");
        return 1;
    }
    
    const char* text = argv[1];
    printf("Texto recebido:\n");
    printf("  \"%s\"\n", text);
    printf("\n");
    

    printf("Passo 1: Tokenizando o texto...\n");
    print_separator('-', 60);
    
    int num_words;
    WordPosition* words = tokenize_txt_simple(text, &num_words);
    
    if (!words || num_words == 0) {
        printf("Nenhuma palavra encontrada no texto.\n");
        return 1;
    }
    
    printf("Palavras únicas encontradas: %d\n", num_words);
    
    // Mostra algumas palavras extraídas
    printf("\nPalavras extraídas (amostra):\n");
    int sample = (num_words < 10) ? num_words : 10;
    for (int i = 0; i < sample; i++) {
        printf("  [%2d] \"%s\"\n", i, words[i].word);
    }
    if (num_words > 10) {
        printf("  ... (%d palavras não mostradas)\n", num_words - 10);
    }
    print_separator('-', 60);
    
    // ============================================================
    // PASSO 2: PREPARAR ARRAY PARA ORDENAÇÃO
    // ============================================================
    
    printf("\nPasso 2: Preparando array para ordenação...\n");
    
    // Extrai apenas as palavras do WordPosition
    char** string_array = malloc(num_words * sizeof(char*));
    if (!string_array) {
        printf(" Erro ao alocar memória.\n");
        destroy_words(words, num_words);
        return 1;
    }
    
    for (int i = 0; i < num_words; i++) {
        string_array[i] = cp_string(words[i].word);
        if (!string_array[i]) {
            printf(" Erro ao copiar palavra.\n");
            for (int j = 0; j < i; j++) {
                free(string_array[j]);
            }
            free(string_array);
            destroy_words(words, num_words);
            return 1;
        }
    }
    
    printf("Array preparado com %d palavras.\n", num_words);
    
    // ============================================================
    // PASSO 3: ORDENAÇÃO
    // ============================================================
    
    printf("\nPasso 3: Ordenando palavras com Quick Sort paralelo...\n");
    print_separator('-', 60);
    
    // Configura o Quick Sort
    set_max_depth(4);           // Profundidade máxima para threads
    set_fallback_limit(100);    // Fallback para arrays pequenos
    
    // Executa ordenação com 4 threads
    SortResult sort_result = quick_sort_parallel(
        string_array,
        num_words,
        4,                      // 4 threads
        compare_strings
    );
    
    // Verifica se está ordenado
    bool is_sorted = verify_sort(string_array, num_words, compare_strings);
    
    printf(" Ordenação concluída: %s\n", is_sorted ? "SUCESSO" : "FALHA");
    
    // Mostra estatísticas
    print_sort_stats(&sort_result);
    
    // Mostra amostra das palavras ordenadas
    print_word_sample(string_array, num_words);
    
    printf("\nPasso 4: Busca interativa de palavras...\n");
    print_separator('=', 60);
    
    printf("\nO array está ordenado e pronto para buscas\n");

    // Loop principal de busca
    process_search(string_array, num_words);
    

    printf("\n");
    print_separator('=', 60);
    printf("  Programa finalizado!\n");
    print_separator('=', 60);
    printf("\n");
    
    // Libera memória
    for (int i = 0; i < num_words; i++) {
        free(string_array[i]);
    }
    free(string_array);
    destroy_words(words, num_words);
    
    return 0;
}