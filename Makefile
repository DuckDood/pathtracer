all: obj/ build/ obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/ray.c.o build/pathtrace 
OSMODE := l

obj/: 
ifeq (${OSMODE}, l)
	mkdir -p obj
else
	mkdir obj
endif

build/: 
ifeq (${OSMODE}, l)
	mkdir -p build
else
	mkdir build
endif

obj/main.c.o: src/main.c 
ifeq (${OSMODE}, l)
	gcc src/main.c -c -o obj/main.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	gcc src/main.c -c -o obj/main.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/vectors.c.o: src/vectors.c 
ifeq (${OSMODE}, l)
	gcc src/vectors.c -c -o obj/vectors.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	gcc src/vectors.c -c -o obj/vectors.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/transformations.c.o: src/transformations.c 
ifeq (${OSMODE}, l)
	gcc src/transformations.c -c -o obj/transformations.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	gcc src/transformations.c -c -o obj/transformations.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/ray.c.o: src/ray.c 
ifeq (${OSMODE}, l)
	gcc src/ray.c -c -o obj/ray.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	gcc src/ray.c -c -o obj/ray.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


build/pathtrace: obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/ray.c.o 
ifeq (${OSMODE}, l)
	${CXX} obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/ray.c.o -o build/pathtrace ${LDFLAGS} -fopenmp -lSDL3
else
	${CXX} obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/ray.c.o -o build/pathtrace ${LDFLAGS} -fopenmp -lSDL3
endif


clean:
	rm -r obj
	rm -r build
.PHONY: clean


gencommands:
	mkdir emmgtemp
ifeq (${OSMODE}, l)
	clang src/main.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/0.json -fsyntax-only
else
	clang src/main.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/0.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/vectors.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/1.json -fsyntax-only
else
	clang src/vectors.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/1.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/transformations.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/2.json -fsyntax-only
else
	clang src/transformations.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/2.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/ray.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/3.json -fsyntax-only
else
	clang src/ray.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/3.json -fsyntax-only
endif
# not cross platform here sad i think
	echo [ > emmgtemp/[
	echo ] > emmgtemp/]
	cat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json
	rm -r emmgtemp
