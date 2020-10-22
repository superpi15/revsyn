#MINISAT=../../tools/minisat/
#MINISAT_FLAG="-D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS"

FLAGS="$MINISAT_FLAG"

g++ -I$MINISAT $FLAGS -g -O3 -c *.cpp
#g++ -L$MINISAT -g -O3 -o revsyn *.o -lminisat
g++ -L$MINISAT -g -O3 -o revsyn *.o 
