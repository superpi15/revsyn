# revsyn
Reversible circuit synthesis

# compile
sh compile.sh

# run
* Format: ./a.out [BENCHMARK] [MODE] [OUTPUT]
* For example: ./a.out test/cm152.pla 1 cm152.real 
+ [BENCHMARK]:
+         in path: test/
+ [MODE]: 
+          0=Proposed GBD   (Generalized Bidirectional method)
+          1=Proposed qGBD  (Generalized Bidirectional method with quantum cost optimization)
