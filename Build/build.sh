#!/bin/sh

SRC_UNITTEST_LATTICE="genLE_Init.c p_alloc.c u_list.c Test/unittest_lattice.c Test/unittest_u_list.c" 
DIR_UNITTEST_LATTICE="../Source/Optimizer/BuildPath"
#FLAGS="-fsanitize=memory"
FLAGS="-fsanitize=integer -fsanitize=undefined -fsanitize=unsigned-integer-overflow"

(cd $DIR_UNITTEST_LATTICE && clang -std=c11 $FLAGS $SRC_UNITTEST_LATTICE -o ../../../Build/unittest_lattice)

