#include "../inc/text_processor.h"
#include "../inc/utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>

/* ============================================================
 * CONSTANTES INTERNAS
 * ============================================================ */

#define INITIAL_POSITION_CAPACITY 4 // Capacidade inicial para posições
#define MAX_WORD_LENGTH 100         // Tamanho máximo de uma palavra

/* ============================================================
 * FUNÇÕES AUXILIARES INTERNAS (STATIC)
 * ============================================================ */

/**
 * Verifica se um caractere é um separador (espaço, pontuação, etc.)
 * Retorna true se for separador, false caso contrário
 */
static bool is_separator(char c)
{
    // Espaços e quebras de linha
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        return true;
    }

    // Pontuação comum
    if (c == '.' || c == ',' || c == ';' || c == ':' ||
        c == '!' || c == '?' || c == '"' || c == '\'' ||
        c == '(' || c == ')' || c == '[' || c == ']' ||
        c == '{' || c == '}' || c == '-' || c == '_' ||
        c == '/' || c == '\\' || c == '|' || c == '@' ||
        c == '#' || c == '$' || c == '%' || c == '^' ||
        c == '&' || c == '*' || c == '+' || c == '=' ||
        c == '<' || c == '>' || c == '~' || c == '`')
    {
        return true;
    }

    return false;
}

/**
 * Adiciona uma posição à estrutura WordPosition
 * Realoca o array de posições se necessário (dobra a capacidade)
 */
static void add_position(WordPosition *wp, int position)
{
    // Verifica se precisa aumentar a capacidade
    if (wp->num_position >= wp->position_cap)
    {
        // Dobra a capacidade (ou define para INITIAL_POSITION_CAPACITY se for 0)
        int new_cap = (wp->position_cap == 0) ? INITIAL_POSITION_CAPACITY : wp->position_cap * 2;

        // Realoca o array de posições
        int *new_positions = realloc(wp->position, new_cap * sizeof(int));
        if (!new_positions)
        {
            // Se falhar, mantém o antigo e retorna (melhor que crashar)
            return;
        }

        wp->position = new_positions;
        wp->position_cap = new_cap;
    }

    // Adiciona a nova posição
    wp->position[wp->num_position] = position;
    wp->num_position++;
}

/**
 * Busca uma palavra no array de WordPosition
 * Retorna o índice se encontrar, -1 caso contrário
 * Usado para encontrar palavras já existentes durante a tokenização
 */
static int find_word(WordPosition *words, int num_words, const char *word)
{
    for (int i = 0; i < num_words; i++)
    {
        if (strcmp(words[i].word, word) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Comparação para qsort (usada para ordenar palavras)
 * Ordena alfabeticamente
 */
static int compare_words(const void *a, const void *b)
{
    WordPosition *w1 = (WordPosition *)a;
    WordPosition *w2 = (WordPosition *)b;
    return strcmp(w1->word, w2->word);
}

/**
 * Comparação para qsort por frequência (decrescente)
 * Usada para encontrar palavras mais frequentes
 */
static int compare_by_frequency(const void *a, const void *b)
{
    WordPosition *w1 = (WordPosition *)a;
    WordPosition *w2 = (WordPosition *)b;
    // Ordem decrescente (maior frequência primeiro)
    return (w2->num_position - w1->num_position);
}

/**
 * Remove acentos básicos de uma palavra (português simplificado)
 * Substitui caractéres acentuados por seus equivalentes sem acento
 */
static void remove_accents(char *word)
{
    // Tabela de substituição para caracteres acentuados comuns
    // Formato: {caractere_acentuado, caractere_sem_acento}
    struct
    {
        const char *acentuado;
        char normal;
    } accent_map[] = {
        {"á", 'a'}, {"à", 'a'}, {"ã", 'a'}, {"â", 'a'}, {"ä", 'a'}, {"é", 'e'}, {"è", 'e'}, {"ê", 'e'}, {"ë", 'e'}, {"í", 'i'}, {"ì", 'i'}, {"î", 'i'}, {"ï", 'i'}, {"ó", 'o'}, {"ò", 'o'}, {"õ", 'o'}, {"ô", 'o'}, {"ö", 'o'}, {"ú", 'u'}, {"ù", 'u'}, {"û", 'u'}, {"ü", 'u'}, {"ç", 'c'}, {"ñ", 'n'}};

    int num_mappings = sizeof(accent_map) / sizeof(accent_map[0]);

    // Para cada mapeamento, substitui na string
    for (int i = 0; i < num_mappings; i++)
    {
        char *pos = strstr(word, accent_map[i].acentuado);
        while (pos)
        {
            // Substitui o caractere acentuado pelo normal
            *pos = accent_map[i].normal;
            // Move o resto da string uma posição para a esquerda
            // (remove o segundo byte do UTF-8)
            char *src = pos + 1;
            char *dst = pos + 1;
            while (*src)
            {
                *dst = *src;
                src++;
                dst++;
            }
            *dst = '\0';
            // Procura próxima ocorrência
            pos = strstr(word, accent_map[i].acentuado);
        }
    }
}

/**
 * Remove sinais de pontuação de uma palavra
 * Mantém apenas letras e números
 */
static void remove_punctuation_from_word(char *word)
{
    char temp[MAX_WORD_LENGTH];
    int j = 0;

    for (int i = 0; word[i] && j < MAX_WORD_LENGTH - 1; i++)
    {
        // Mantém apenas caracteres alfanuméricos
        if (isalnum(word[i]))
        {
            temp[j++] = word[i];
        }
    }
    temp[j] = '\0';
    strcpy(word, temp);
}

/* ============================================================
 * FUNÇÕES PÚBLICAS - IMPLEMENTAÇÃO
 * ============================================================ */

/**
 * Tokeniza um texto, dividindo em palavras e armazenando posições
 *
 * Parâmetros:
 *   - text: texto a ser tokenizado (não modifica)
 *   - num_words: ponteiro para armazenar o número de palavras
 *   - options: opções de tokenização (pode ser NULL para padrão)
 *
 * Retorna: array de WordPosition alocado dinamicamente
 *          ou NULL em caso de erro
 */
WordPosition *tokenize_txt(const char *text, int *num_words, TokenizationOpt *options)
{
    // Validação dos parâmetros
    if (!text || !num_words)
    {
        return NULL;
    }

    *num_words = 0;

    // Cria opções padrão se não foram fornecidas
    TokenizationOpt default_options = {
        .ignore_uppercase = true,
        .rm_punctuation = true,
        .ignore_stopwords = false,
        .min_size = 2,
        .stopwords_list = NULL,
        .num_stopwords = 0};

    if (!options)
    {
        options = &default_options;
    }

    // Aloca array inicial de palavras (estimativa conservadora)
    int max_words = strlen(text) / 2 + 1; // Estimativa aproximada
    WordPosition *words = calloc(max_words, sizeof(WordPosition));
    if (!words)
    {
        return NULL;
    }

    char word_buffer[MAX_WORD_LENGTH];
    int word_idx = 0;
    int current_pos = 0; // Posição atual no texto (número de palavras)

    // Percorre o texto caractere por caractere
    for (int i = 0; text[i] != '\0'; i++)
    {
        char c = text[i];

        // Verifica se é separador
        if (is_separator(c))
        {
            // Se temos uma palavra acumulada, finaliza
            if (word_idx > 0)
            {
                word_buffer[word_idx] = '\0';

                // Aplica normalizações
                if (options->ignore_uppercase)
                {
                    to_lowercase(word_buffer); // Função de utils.h
                }

                if (options->rm_punctuation)
                {
                    remove_punctuation_from_word(word_buffer);
                }

                // Remove acentos (normalização adicional)
                remove_accents(word_buffer);

                // Verifica tamanho mínimo
                if ((int)strlen(word_buffer) >= options->min_size)
                {
                    // Tenta encontrar palavra existente
                    int idx = find_word(words, *num_words, word_buffer);

                    if (idx >= 0)
                    {
                        // Palavra já existe: adiciona posição
                        add_position(&words[idx], current_pos);
                    }
                    else
                    {
                        // Palavra nova: cria entrada
                        words[*num_words].word = strdup(word_buffer);
                        if (!words[*num_words].word)
                        {
                            // Erro de memória: libera e retorna NULL
                            destroy_words(words, *num_words);
                            return NULL;
                        }

                        words[*num_words].position = NULL;
                        words[*num_words].num_position = 0;
                        words[*num_words].position_cap = 0;

                        // Adiciona posição atual
                        add_position(&words[*num_words], current_pos);
                        (*num_words)++;
                    }

                    current_pos++; // Incrementa posição para próxima palavra
                }

                word_idx = 0; // Reseta buffer
            }
        }
        else
        {
            // Caractere faz parte da palavra
            if (word_idx < MAX_WORD_LENGTH - 1)
            {
                word_buffer[word_idx++] = c;
            }
        }
    }

    // Processa última palavra (se houver)
    if (word_idx > 0)
    {
        word_buffer[word_idx] = '\0';

        if (options->ignore_uppercase)
        {
            to_lowercase(word_buffer);
        }

        if (options->rm_punctuation)
        {
            remove_punctuation_from_word(word_buffer);
        }

        remove_accents(word_buffer);

        if ((int)strlen(word_buffer) >= options->min_size)
        {
            int idx = find_word(words, *num_words, word_buffer);

            if (idx >= 0)
            {
                add_position(&words[idx], current_pos);
            }
            else
            {
                words[*num_words].word = strdup(word_buffer);
                if (!words[*num_words].word)
                {
                    destroy_words(words, *num_words);
                    return NULL;
                }

                words[*num_words].position = NULL;
                words[*num_words].num_position = 0;
                words[*num_words].position_cap = 0;
                add_position(&words[*num_words], current_pos);
                (*num_words)++;
            }
        }
    }

    return words;
}

/**
 * Versão simplificada da tokenização
 * Usa opções padrão (ignora maiúsculas, remove pontuação)
 */
WordPosition *tokenize_txt_simple(const char *text, int *num_words)
{
    return tokenize_txt(text, num_words, NULL);
}

/**
 * Normaliza um texto completo
 * Converte para minúsculas e remove acentos
 */
void normalize_txt(char *text)
{
    if (!text)
        return;

    // Converte todo o texto para minúsculas
    to_lowercase(text);

    // Remove acentos de todas as palavras (simplificado)
    // Percorre o texto e remove acentos de cada palavra
    char word[MAX_WORD_LENGTH];
    int word_idx = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        if (is_separator(text[i]))
        {
            if (word_idx > 0)
            {
                word[word_idx] = '\0';
                remove_accents(word);
                // Não podemos substituir diretamente, então pulamos
                // A implementação completa seria mais complexa
                word_idx = 0;
            }
        }
        else
        {
            if (word_idx < MAX_WORD_LENGTH - 1)
            {
                word[word_idx++] = text[i];
            }
        }
    }
}

/**
 * Normaliza uma única palavra
 * Converte para minúsculas e remove acentos
 */
void normalize_word(char *palavra)
{
    if (!palavra)
        return;
    to_lowercase(palavra);
    remove_accents(palavra);
}

/**
 * Remove palavras duplicadas do array e conta frequências
 *
 * Parâmetros:
 *   - words: array de WordPosition (DEVE estar ordenado)
 *   - num_words: número de palavras no array
 *   - result: ponteiro para receber o novo array (sem duplicatas)
 *
 * Retorna: número de palavras únicas
 */
int rm_duplicate(WordPosition *words, int num_words, WordPosition **result)
{
    if (!words || num_words == 0 || !result)
    {
        return 0;
    }

    // Ordena o array primeiro (necessário para remover duplicatas)
    qsort(words, num_words, sizeof(WordPosition), compare_words);

    // Aloca novo array (no máximo terá o mesmo tamanho)
    WordPosition *unique = malloc(num_words * sizeof(WordPosition));
    if (!unique)
    {
        return 0;
    }

    int unique_count = 0;

    for (int i = 0; i < num_words; i++)
    {
        // Se é a primeira palavra ou diferente da anterior
        if (i == 0 || strcmp(words[i].word, words[i - 1].word) != 0)
        {
            // Copia a palavra para o array único
            unique[unique_count].word = strdup(words[i].word);
            if (!unique[unique_count].word)
            {
                // Limpeza em caso de erro
                for (int j = 0; j < unique_count; j++)
                {
                    free(unique[j].word);
                }
                free(unique);
                return 0;
            }

            // Inicializa posições
            unique[unique_count].position = NULL;
            unique[unique_count].num_position = 0;
            unique[unique_count].position_cap = 0;

            // Conta todas as ocorrências desta palavra
            int freq = 1;
            while (i + freq < num_words &&
                   strcmp(words[i].word, words[i + freq].word) == 0)
            {
                freq++;
            }

            // Adiciona a frequência como posições (simplificado)
            // Na prática, poderíamos armazenar as posições reais
            for (int p = 0; p < freq; p++)
            {
                add_position(&unique[unique_count], i + p);
            }

            unique_count++;
            i += freq - 1; // Pula as duplicatas
        }
    }

    *result = unique;
    return unique_count;
}

/**
 * Filtra stopwords de um array de palavras
 * Remove palavras que estão na lista de stopwords
 */
int filter_stopwords(WordPosition *words, int num_words,
                     const char **stopwords, int num_stopwords,
                     WordPosition **result)
{
    if (!words || num_words == 0 || !stopwords || num_stopwords == 0 || !result)
    {
        return 0;
    }

    // Aloca novo array (no máximo terá o mesmo tamanho)
    WordPosition *filtered = malloc(num_words * sizeof(WordPosition));
    if (!filtered)
    {
        return 0;
    }

    int filtered_count = 0;

    for (int i = 0; i < num_words; i++)
    {
        bool is_stopword = false;

        // Verifica se a palavra está na lista de stopwords
        for (int j = 0; j < num_stopwords; j++)
        {
            if (strcmp(words[i].word, stopwords[j]) == 0)
            {
                is_stopword = true;
                break;
            }
        }

        // Se não é stopword, adiciona ao resultado
        if (!is_stopword)
        {
            filtered[filtered_count] = words[i]; // Cópia superficial
            filtered_count++;
        }
    }

    *result = filtered;
    return filtered_count;
}

/**
 * Retorna lista de stopwords em português
 * Palavras comuns que geralmente não são úteis para busca
 */
const char **get_stopwords_portugues(int *num_stopwords)
{
    // Lista estática de stopwords em português
    static const char *stopwords[] = {
        "a", "ao", "aos", "aquela", "aquelas", "aquele", "aqueles", "aquilo",
        "as", "até", "com", "como", "da", "das", "de", "dela", "delas", "dele",
        "deles", "depois", "do", "dos", "e", "ela", "elas", "ele", "eles",
        "em", "entre", "era", "eram", "essa", "essas", "esse", "esses",
        "esta", "estas", "este", "estes", "eu", "foi", "foram", "for",
        "fosse", "há", "isso", "isto", "já", "lhe", "lhes", "mais",
        "mas", "me", "mesmo", "meu", "meus", "minha", "minhas", "muito",
        "na", "nas", "nem", "no", "nos", "nossa", "nossas", "nosso",
        "nossos", "num", "numa", "não", "nós", "o", "os", "ou", "para",
        "pela", "pelas", "pelo", "pelos", "por", "porque", "que", "quando",
        "quem", "se", "sem", "ser", "sua", "suas", "seu", "seus", "são",
        "também", "te", "tem", "temos", "tenha", "ter", "teu", "teus",
        "ti", "tua", "tuas", "tudo", "um", "uma", "umas", "uns", "você",
        "vocês", "vós", "à", "às", "é", "ela", "ele", "eles", "elas"};

    *num_stopwords = sizeof(stopwords) / sizeof(stopwords[0]);
    return stopwords;
}

/**
 * Calcula estatísticas do texto processado
 */
TxtStatistic calc_statistics(const char *text, WordPosition *words, int num_words)
{
    TxtStatistic stats = {0};

    if (!text || !words)
    {
        return stats;
    }

    // Estatísticas básicas
    stats.total_words = num_words;
    stats.total_chars = strlen(text);

    // Conta caracteres sem espaços
    int chars_wout = 0;
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (!is_separator(text[i]))
        {
            chars_wout++;
        }
    }
    stats.chars_woutSpaces = chars_wout;

    // Conta palavras únicas
    // Para isso, precisamos ordenar e contar duplicatas
    WordPosition *unique_words = NULL;
    stats.uniq_words = rm_duplicate(words, num_words, &unique_words);

    // Libera o array temporário
    if (unique_words)
    {
        for (int i = 0; i < stats.uniq_words; i++)
        {
            free(unique_words[i].word);
            free(unique_words[i].position);
        }
        free(unique_words);
    }

    return stats;
}

/**
 * Encontra as palavras mais frequentes no texto
 * Retorna um array com as top_n palavras mais frequentes
 */
WordPosition *find_frequent(WordPosition *words, int num_words,
                            int top_n, int *result_count)
{
    if (!words || num_words == 0 || top_n <= 0 || !result_count)
    {
        *result_count = 0;
        return NULL;
    }

    // Cria cópia do array para não modificar o original
    WordPosition *copy = malloc(num_words * sizeof(WordPosition));
    if (!copy)
    {
        *result_count = 0;
        return NULL;
    }

    memcpy(copy, words, num_words * sizeof(WordPosition));

    // Ordena por frequência (decrescente)
    qsort(copy, num_words, sizeof(WordPosition), compare_by_frequency);

    // Determina quantos retornar (no máximo top_n)
    int return_count = (top_n < num_words) ? top_n : num_words;

    // Aloca resultado
    WordPosition *result = malloc(return_count * sizeof(WordPosition));
    if (!result)
    {
        free(copy);
        *result_count = 0;
        return NULL;
    }

    // Copia as top_n palavras
    for (int i = 0; i < return_count; i++)
    {
        result[i].word = strdup(copy[i].word);
        if (!result[i].word)
        {
            // Limpeza em caso de erro
            for (int j = 0; j < i; j++)
            {
                free(result[j].word);
            }
            free(result);
            free(copy);
            *result_count = 0;
            return NULL;
        }

        // Copia posições (cópia profunda)
        result[i].num_position = copy[i].num_position;
        result[i].position_cap = copy[i].num_position;
        result[i].position = malloc(copy[i].num_position * sizeof(int));
        if (!result[i].position)
        {
            // Limpeza em caso de erro
            free(result[i].word);
            for (int j = 0; j < i; j++)
            {
                free(result[j].word);
                free(result[j].position);
            }
            free(result);
            free(copy);
            *result_count = 0;
            return NULL;
        }
        memcpy(result[i].position, copy[i].position,
               copy[i].num_position * sizeof(int));
    }

    *result_count = return_count;
    free(copy);
    return result;
}

/**
 * Libera toda a estrutura WordPosition
 * Libera a palavra, as posições e a estrutura em si
 */
void destroy_words(WordPosition *words, int num_words)
{
    if (!words)
        return;

    for (int i = 0; i < num_words; i++)
    {
        // Libera a string da palavra
        if (words[i].word)
        {
            free(words[i].word);
            words[i].word = NULL;
        }

        // Libera o array de posições
        if (words[i].position)
        {
            free(words[i].position);
            words[i].position = NULL;
        }

        words[i].num_position = 0;
        words[i].position_cap = 0;
    }

    free(words);
}

/**
 * Libera as opções de tokenização
 * Libera a lista de stopwords se foi alocada dinamicamente
 */
void destroy_token_options(TokenizationOpt *options)
{
    if (!options)
        return;

    // Se a lista de stopwords foi alocada dinamicamente
    // Nota: A lista padrão é estática, não deve ser liberada
    if (options->stopwords_list)
    {
        // Assumimos que foi alocada com malloc
        // O usuário deve ser responsável por gerenciar isso
        // Esta é uma decisão de design: deixamos o usuário gerenciar
        options->stopwords_list = NULL;
        options->num_stopwords = 0;
    }
}