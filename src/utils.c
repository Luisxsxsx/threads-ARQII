#include "../inc/utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

/* ============================================================
 * CONSTANTES INTERNAS
 * ============================================================ */

#define MAX_LOG_MESSAGE 1024 // Tamanho máximo para mensagens de log
#define LOG_TIME_BUFFER 64   // Tamanho do buffer para timestamp

/* ============================================================
 * FUNÇÕES AUXILIARES INTERNAS (STATIC)
 * ============================================================ */

/**
 * Obtém timestamp atual formatado para logs
 * Retorna uma string com data e hora no formato: "YYYY-MM-DD HH:MM:SS"
 * Usa buffer estático, não thread-safe (mas suficiente para nosso uso)
 */
static const char *get_timestamp(void)
{
    static char buffer[LOG_TIME_BUFFER];
    time_t now;
    struct tm *tm_info;

    time(&now);
    tm_info = localtime(&now);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

/* ============================================================
 * GERENCIAMENTO DE MEMÓRIA
 * ============================================================ */

/**
 * Aloca um array de strings com tamanho fixo para cada string
 *
 * Parâmetros:
 *   - num_strings: número de strings a alocar
 *   - max_length: tamanho máximo de cada string (incluindo '\0')
 *
 * Retorna: ponteiro para array de strings, ou NULL em caso de erro
 *
 * Exemplo de uso:
 *   char** words = allocate_array_strings(100, 50);
 *   // Cria 100 strings, cada uma com capacidade para 50 caracteres
 */
char **allocate_array_strings(int num_strings, int max_length)
{
    // Verifica parâmetros
    if (num_strings <= 0 || max_length <= 0)
    {
        log_message("ERROR", "allocate_array_strings: parâmetros inválidos");
        return NULL;
    }

    // Aloca array de ponteiros para strings
    char **array = malloc(num_strings * sizeof(char *));
    if (!array)
    {
        log_message("ERROR", "allocate_array_strings: falha ao alocar array de ponteiros");
        return NULL;
    }

    // Aloca cada string individualmente
    for (int i = 0; i < num_strings; i++)
    {
        array[i] = malloc((max_length + 1) * sizeof(char)); // +1 para o '\0'
        if (!array[i])
        {
            // Em caso de erro, libera tudo que já foi alocado
            log_message("ERROR", "allocate_array_strings: falha ao alocar string %d", i);
            for (int j = 0; j < i; j++)
            {
                free(array[j]);
            }
            free(array);
            return NULL;
        }
        // Inicializa com string vazia
        array[i][0] = '\0';
    }

    return array;
}

/**
 * Libera um array de strings alocado com allocate_array_strings
 *
 * Parâmetros:
 *   - array: array de strings a ser liberado
 *   - num_strings: número de strings no array
 */
void free_array_strings(char **array, int num_strings)
{
    if (!array)
        return;

    // Libera cada string individualmente
    for (int i = 0; i < num_strings; i++)
    {
        if (array[i])
        {
            free(array[i]);
            array[i] = NULL; // Boa prática: evitar ponteiros pendurados
        }
    }

    // Libera o array de ponteiros
    free(array);
}

/**
 * Cria uma cópia de uma string (equivalente a strdup)
 *
 * Parâmetros:
 *   - origem: string a ser copiada
 *
 * Retorna: nova string alocada dinamicamente, ou NULL em caso de erro
 *
 * Nota: O caller é responsável por liberar a memória com free()
 */
char *cp_string(const char *origem)
{
    if (!origem)
    {
        log_message("WARN", "cp_string: origem NULL");
        return NULL;
    }

    // Calcula tamanho da string
    size_t len = strlen(origem);

    // Aloca memória para a cópia (+1 para o '\0')
    char *copia = malloc((len + 1) * sizeof(char));
    if (!copia)
    {
        log_message("ERROR", "cp_string: falha ao alocar memória para cópia");
        return NULL;
    }

    // Copia a string
    strcpy(copia, origem);
    return copia;
}

/* ============================================================
 * MANIPULAÇÃO DE STRINGS
 * ============================================================ */

/**
 * Converte todos os caracteres de uma string para minúsculas
 *
 * Parâmetros:
 *   - str: string a ser convertida (modifica in-place)
 *
 * Exemplo: "Hello World" → "hello world"
 */
void to_lowercase(char *str)
{
    if (!str)
        return;

    for (int i = 0; str[i] != '\0'; i++)
    {
        str[i] = tolower(str[i]);
    }
}

/**
 * Remove todos os espaços em branco de uma string
 *
 * Parâmetros:
 *   - str: string a ser processada (modifica in-place)
 *
 * Exemplo: "Hello World" → "HelloWorld"
 */
void rm_spaces(char *str)
{
    if (!str)
        return;

    int i, j = 0;
    for (i = 0; str[i] != '\0'; i++)
    {
        // Mantém apenas caracteres que NÃO são espaços
        if (!isspace(str[i]))
        {
            str[j++] = str[i];
        }
    }
    str[j] = '\0'; // Finaliza a string
}

/**
 * Compara duas strings ignorando diferenças entre maiúsculas/minúsculas
 *
 * Parâmetros:
 *   - a: primeira string
 *   - b: segunda string
 *
 * Retorna: 0 se iguais, negativo se a < b, positivo se a > b
 *
 * Exemplo: compare_ignore_case("Hello", "hello") → 0
 */
int compare_ignore_case(const char *a, const char *b)
{
    if (!a && !b)
        return 0;
    if (!a)
        return -1;
    if (!b)
        return 1;

    // Cria cópias em minúsculas para comparação
    // (Alocação dinâmica para não modificar as strings originais)
    char *a_lower = cp_string(a);
    char *b_lower = cp_string(b);

    if (!a_lower || !b_lower)
    {
        // Em caso de erro, libera e faz comparação normal
        free(a_lower);
        free(b_lower);
        return strcmp(a, b);
    }

    to_lowercase(a_lower);
    to_lowercase(b_lower);

    int result = strcmp(a_lower, b_lower);

    free(a_lower);
    free(b_lower);

    return result;
}

/**
 * Remove espaços em branco do início e fim de uma string
 *
 * Parâmetros:
 *   - str: string a ser processada (modifica in-place)
 *
 * Retorna: ponteiro para a string processada (mesmo ponteiro)
 *
 * Exemplo: "  Hello World  " → "Hello World"
 */
char *trim_string(char *str)
{
    if (!str)
        return NULL;

    // Encontra o primeiro caractere não-espaço
    char *start = str;
    while (*start && isspace(*start))
    {
        start++;
    }

    // Se a string só tem espaços
    if (*start == '\0')
    {
        str[0] = '\0';
        return str;
    }

    // Encontra o último caractere não-espaço
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
    {
        end--;
    }

    // Move a string para o início
    if (start != str)
    {
        memmove(str, start, (end - start + 2)); // +2 para incluir o '\0'
    }
    else
    {
        *(end + 1) = '\0'; // Coloca o terminador após o último caractere
    }

    return str;
}

/* ============================================================
 * VALIDAÇÃO E LOGGING
 * ============================================================ */

/**
 * Verifica se um ponteiro é válido (não NULL) e registra mensagem
 *
 * Parâmetros:
 *   - ptr: ponteiro a ser verificado
 *   - mensagem: mensagem a ser registrada se ptr for NULL
 *
 * Retorna: true se ptr não for NULL, false caso contrário
 *
 * Uso típico:
 *   if (!verify_mem(ptr, "Falha ao alocar memória para dados")) {
 *       return NULL;
 *   }
 */
bool verify_mem(void *ptr, const char *mensagem)
{
    if (!ptr)
    {
        log_message("ERROR", mensagem);
        return false;
    }
    return true;
}

/**
 * Registra uma mensagem de log com timestamp e nível
 *
 * Parâmetros:
 *   - nivel: nível da mensagem ("INFO", "WARN", "ERROR", "DEBUG")
 *   - mensagem: mensagem a ser registrada
 *   - ...: argumentos adicionais (formato printf)
 *
 * Exemplo: log_message("INFO", "Processando arquivo %s", filename);
 */
void log_message(const char *nivel, const char *mensagem, ...)
{
    if (!nivel || !mensagem)
        return;

    // Obtém timestamp
    const char *timestamp = get_timestamp();

    // Prepara buffer para a mensagem formatada
    char buffer[MAX_LOG_MESSAGE];

    // Processa argumentos variáveis (printf-style)
    va_list args;
    va_start(args, mensagem);
    vsnprintf(buffer, sizeof(buffer), mensagem, args);
    va_end(args);

    // Imprime no formato: [TIMESTAMP] [NIVEL] mensagem
    fprintf(stderr, "[%s] [%s] %s\n", timestamp, nivel, buffer);
    fflush(stderr); // Garante que a mensagem seja escrita imediatamente
}

/**
 * Registra tempo de execução de uma operação
 *
 * Parâmetros:
 *   - operacao: nome da operação
 *   - tempo: tempo em segundos
 *
 * Exemplo: log_time("Ordenação", 2.345);
 */
void log_time(const char *operacao, double tempo)
{
    if (!operacao)
        return;

    const char *timestamp = get_timestamp();
    fprintf(stderr, "[%s] [TIME] %s: %.6f segundos\n",
            timestamp, operacao, tempo);
    fflush(stderr);
}

/* ============================================================
 * UTILITÁRIOS DE ARRAY
 * ============================================================ */

/**
 * Encontra o valor máximo em um array de inteiros
 *
 * Parâmetros:
 *   - array: array de inteiros
 *   - tamanho: número de elementos no array
 *
 * Retorna: valor máximo encontrado, ou 0 se array for NULL ou vazio
 */
int find_max(int *array, int tamanho)
{
    if (!array || tamanho <= 0)
    {
        log_message("WARN", "find_max: array vazio ou NULL");
        return 0;
    }

    int max = array[0];
    for (int i = 1; i < tamanho; i++)
    {
        if (array[i] > max)
        {
            max = array[i];
        }
    }
    return max;
}

/**
 * Encontra o valor mínimo em um array de inteiros
 *
 * Parâmetros:
 *   - array: array de inteiros
 *   - tamanho: número de elementos no array
 *
 * Retorna: valor mínimo encontrado, ou 0 se array for NULL ou vazio
 */
int find_min(int *array, int tamanho)
{
    if (!array || tamanho <= 0)
    {
        log_message("WARN", "find_min: array vazio ou NULL");
        return 0;
    }

    int min = array[0];
    for (int i = 1; i < tamanho; i++)
    {
        if (array[i] < min)
        {
            min = array[i];
        }
    }
    return min;
}

/**
 * Calcula a média de um array de inteiros
 *
 * Parâmetros:
 *   - array: array de inteiros
 *   - tamanho: número de elementos no array
 *
 * Retorna: média como double, ou 0.0 se array for NULL ou vazio
 */
double calc_med(int *array, int tamanho)
{
    if (!array || tamanho <= 0)
    {
        log_message("WARN", "calc_med: array vazio ou NULL");
        return 0.0;
    }

    long long soma = 0; // Usa long long para evitar overflow
    for (int i = 0; i < tamanho; i++)
    {
        soma += array[i];
    }

    return (double)soma / tamanho;
}

/**
 * Imprime um array de strings de forma formatada
 * Útil para debug e visualização
 *
 * Parâmetros:
 *   - array: array de strings
 *   - tamanho: número de strings no array
 *   - limite: número máximo de strings a imprimir (-1 para imprimir todas)
 *
 * Exemplo de saída:
 *   Array de strings (5 de 10 mostradas):
 *   [0] "hello"
 *   [1] "world"
 *   [2] "test"
 *   ...
 */
void print_array_strings(char **array, int tamanho, int limite)
{
    if (!array || tamanho <= 0)
    {
        log_message("WARN", "print_array_strings: array vazio ou NULL");
        return;
    }

    // Determina quantos elementos imprimir
    int imprimir = (limite < 0 || limite >= tamanho) ? tamanho : limite;

    printf("Array de strings (%d de %d mostrados):\n", imprimir, tamanho);

    for (int i = 0; i < imprimir; i++)
    {
        if (array[i])
        {
            printf("  [%d] \"%s\"\n", i, array[i]);
        }
        else
        {
            printf("  [%d] (NULL)\n", i);
        }
    }

    // Se limite foi menor que tamanho, mostra reticências
    if (limite >= 0 && limite < tamanho)
    {
        printf("  ... (%d elementos não mostrados)\n", tamanho - limite);
    }

    fflush(stdout);
}

/* ============================================================
 * FUNÇÕES UTILITÁRIAS ADICIONAIS
 * ============================================================ */

/**
 * Converte um double para string com precisão controlada
 * Útil para formatação de saída
 *
 * Parâmetros:
 *   - valor: valor a ser convertido
 *   - precisao: número de casas decimais
 *   - buffer: buffer para armazenar a string
 *   - buffer_size: tamanho do buffer
 */
void double_to_string(double valor, int precisao, char *buffer, int buffer_size)
{
    if (!buffer || buffer_size <= 0)
        return;

    snprintf(buffer, buffer_size, "%.*f", precisao, valor);
}

/**
 * Verifica se uma string contém apenas caracteres alfanuméricos
 *
 * Parâmetros:
 *   - str: string a ser verificada
 *
 * Retorna: true se a string for alfanumérica, false caso contrário
 */
bool is_alphanumeric(const char *str)
{
    if (!str || strlen(str) == 0)
        return false;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (!isalnum(str[i]))
        {
            return false;
        }
    }
    return true;
}