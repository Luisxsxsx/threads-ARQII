#include "../inc/utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#define MAX_LOG_MESSAGE 1024 // Tamanho máximo para mensagens de log
#define LOG_TIME_BUFFER 64   // Tamanho do buffer para timestamp

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

void to_lowercase(char *str)
{
    if (!str)
        return;

    for (int i = 0; str[i] != '\0'; i++)
    {
        str[i] = tolower(str[i]);
    }
}

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

bool verify_mem(void *ptr, const char *mensagem)
{
    if (!ptr)
    {
        log_message("ERROR", mensagem);
        return false;
    }
    return true;
}

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

void log_time(const char *operacao, double tempo)
{
    if (!operacao)
        return;

    const char *timestamp = get_timestamp();
    fprintf(stderr, "[%s] [TIME] %s: %.6f segundos\n",
            timestamp, operacao, tempo);
    fflush(stderr);
}

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