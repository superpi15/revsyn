# Revsyn
Reversible circuit synthesis

# Compile
sh compile.sh

# Run
* Format: ./a.out [BENCHMARK] [MODE] [OUTPUT]
* For example: ./a.out test/cm152.pla 1 cm152.real 
+ [BENCHMARK]:
+         in path: test/
+ [MODE]: 
+          0=  GBD   (Generalized Bidirectional method)
+          1= qGBD  (Generalized Bidirectional method with quantum cost optimization)
