# defector

Motivated by DBN approach for defect prediction proposed by Wang, Liu, and Tan (https://ece.uwaterloo.ca/%7Elintan/publications/deeplearn-icse16.pdf), defector targets C files using llvm to obtain tokens.

Token vectors are still listed in topographic ordering. However, unlike Java, C files can consist of many times more lines of code which makes defect targeting difficult should we choose per file labelling.

Instead, good and bad code regions are labelled per method. That is, a token vector is constructed for each method.
