#!/usr/bin/env bash

TOKENPROFILER=../bin/bin/tokenprofiler

llfiles=""

for file in ./ll/*.ll
do
    llfiles="$llfiles $file"
done
$TOKENPROFILER $llfiles
