#!/bin/bash

CC="gcc"
CFLAGS="-Wall -O2 -pthread"
LDFLAGS="-lm -pthread"
OUTPUT="programa"
FILE="main.c"

SRC=(
    "src/utils.c"
    "src/text_processor.c"
    "src/quick_sort.c"
    "src/binary_search.c"
)

$CC $CFLAGS -o $OUTPUT ${SRC[@]} $FILE $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Compilado: ./$OUTPUT"
else
    echo "Erro na compilação"
    exit 1
fi