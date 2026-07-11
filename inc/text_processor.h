#ifndef TEXT_PROCESSOR_H
#define TEXT_PROCESSOR_H

#include "utils.h"
#include <time.h>

/**
 * Estruturas de Dados
 */

// Estrutura para armazenar uma palavra e suas posições no text
typedef struct
{
    char *word;       // A palavra em si
    int *position;    // Array de posições onde aparece
    int num_position; // Quantas ocorrências
    int position_cap; // Capacidade atual do array
} WordPosition;

// Estatísticas do text processado
typedef struct
{
    int total_words;
    int uniq_words;
    int total_chars;
    int chars_woutSpaces;
    double tokenization_time;
    double normalization_time;
} TxtStatistic;

// Opções de tokenização
typedef struct
{
    bool ignore_uppercase;       // Converter para minúsculas
    bool rm_punctuation;         // Remover sinais de pontuação
    bool ignore_stopwords;       // Remover words comuns
    int min_size;                // Ignorar words mais curtas
    const char **stopwords_list; // Lista de stopwords personalizada
    int num_stopwords;
} TokenizationOpt;

/**
 * Funções de Tokenização
 */

// Tokeniza um text, retornando array de WordPosition
WordPosition *tokenize_txt(const char *text,
                           int *num_words,
                           TokenizationOpt *options);

// Versão simplificada (usa opções padrão)
WordPosition *tokenize_txt_simple(const char *text,
                                  int *num_words);

/**
 * Normalização de Texto
 */

// Converte text para minúsculas e remove acentos básicos
void normalize_txt(char *text);

// Normaliza uma única palavra
void normalize_word(char *palavra);

/**
 * Remoção de Duplicatas e Stopwords
 */

// Remove words duplicadas e conta frequências
int rm_duplicate(WordPosition *words,
                 int num_words,
                 WordPosition **resultado);

// Filtra stopwords de um array de words
int filter_stopwords(WordPosition *words,
                     int num_words,
                     const char **stopwords,
                     int num_stopwords,
                     WordPosition **resultado);

/**
 * Análise e Estatísticas
 */

// Calcula estatísticas do text processado
TxtStatistic calc_statistics(const char *text,
                             WordPosition *words,
                             int num_words);

// Encontra as words mais frequentes
WordPosition *find_frequent(WordPosition *words,
                            int num_words,
                            int top_n,
                            int *resultado_count);

/**
 * Liberação de Memória
 */

// Libera toda a estrutura WordPosition
void destroy_words(WordPosition *words, int num_words);

#endif /* TEXT_PROCESSOR_H */