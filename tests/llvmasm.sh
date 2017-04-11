#!/usr/bin/env bash

LLVM_DIR=/Users/cmk/llvm/bin
CLANG=$LLVM_DIR/clang-3.9
OPT=$LLVM_DIR/opt

while IFS=';' read -r src_path include_path || [[ -n "$src_path" ]]; do
    for file in ./$src_path/*.c**
    do
        base=${file##*/}
        $CLANG -g -I $include_path -O0 -emit-llvm -S $file -o - | $OPT -mem2reg -S -o ./ll/${base%.c**}.ll
    done
done < "project_sources.txt"