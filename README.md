# defector
defect prediction using DBN generated semantic features by https://ece.uwaterloo.ca/~lintan/publications/deeplearn-icse16.pdf

Defect prediction extracts token vectors from a project's source code.

For each training file with lines of code labelled buggy or clean, color each token vector by the corresponding state.

Feed the token vectors through Deep Belief Network to obtain the feature vectors.

# Dataset

We choose open source projects and artificially insert defects.

For simplicity, we will directly insert defects into our token vectors.

# Token profiler

Token profiler takes a buggy instruction in csv and a series of files and outputs two csv files:
1. file with each line mapping token integer value to its label
2. file with each line representing token vectors as a series of integers

## Usage

> <path/to/defector>/bin/tokenprofiler -b "bug.csv" <sample>.ll ...

bug.csv is optional but must specify defects in the following format:

<\Filename>, <\Line #>

# Tests

Tests chosen are ideally high quality (high coverage and build passing). 
This criteria is to ensure minimum initial defects which makes it more likely that injected defects are the only defects.

## Jansson

Decoding and manipulating JSON Data - https://github.com/akheron/jansson

How to analyze: 

- include <stdlib.h> to dump.c for non-linux OS

- build to generate include files from their config