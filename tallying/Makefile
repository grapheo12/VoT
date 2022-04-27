TFHE_PREFIX = /usr/local
TFHE_OPTIONS = C_INCLUDE_PATH=$$C_INCLUDE_PATH:${TFHE_PREFIX}/include CPLUS_INCLUDE_PATH=$$CPLUS_INCLUDE_PATH:${TFHE_PREFIX}/include LIBRARY_PATH=$$LIBRARY_PATH:${TFHE_PREFIX}/lib LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:${TFHE_PREFIX}/lib
TFHE_LIB = -ltfhe-spqlios-fma
BLAS_LIB = -lopenblas


all: dump bin/main

dump:
	mkdir -p dump

bin/main: main.cpp
	mkdir -p bin
	mkdir -p results
	${TFHE_OPTIONS} g++ -g -fopenmp main.cpp -o bin/main -lthfhe -lcrypto -lssl ${TFHE_LIB} ${BLAS_LIB}


.PHONY: clean
clean:
	rm -rf bin dump results *.vote
