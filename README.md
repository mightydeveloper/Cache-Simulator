# Cache-Simulator
CS311 Project 4: Cache Design

# Input Usage
You can give input as following:

     Cache-Simulator -c cap:assoc:bsize [-x] input_trace

> -c : cache configuration

> -x : dump the cache content only at the end of simulation

Example Usage)
```sh
Cache-Simulator -c 4096:4:32 real_workload/gcc
```

# Compiler Configuration
* Compiler : LLVM 
* Language : C
* Dialect : GNU99