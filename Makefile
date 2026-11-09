all: obj/ build/ obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/shapes.c.o obj/material.c.o obj/randoms.c.o obj/sdl3webgpu.c.o build/pathtrace 
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
	${CC} src/main.c -c -o obj/main.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/main.c -c -o obj/main.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/vectors.c.o: src/vectors.c 
ifeq (${OSMODE}, l)
	${CC} src/vectors.c -c -o obj/vectors.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/vectors.c -c -o obj/vectors.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/transformations.c.o: src/transformations.c 
ifeq (${OSMODE}, l)
	${CC} src/transformations.c -c -o obj/transformations.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/transformations.c -c -o obj/transformations.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/shapes.c.o: src/shapes.c 
ifeq (${OSMODE}, l)
	${CC} src/shapes.c -c -o obj/shapes.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/shapes.c -c -o obj/shapes.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/material.c.o: src/material.c 
ifeq (${OSMODE}, l)
	${CC} src/material.c -c -o obj/material.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/material.c -c -o obj/material.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/randoms.c.o: src/randoms.c 
ifeq (${OSMODE}, l)
	${CC} src/randoms.c -c -o obj/randoms.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/randoms.c -c -o obj/randoms.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


obj/sdl3webgpu.c.o: src/sdl3webgpu.c 
ifeq (${OSMODE}, l)
	${CC} src/sdl3webgpu.c -c -o obj/sdl3webgpu.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
else
	${CC} src/sdl3webgpu.c -c -o obj/sdl3webgpu.c.o ${CFLAGS} -O3 -fopenmp -Iinclude
endif


build/pathtrace: obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/shapes.c.o obj/material.c.o obj/randoms.c.o obj/sdl3webgpu.c.o 
ifeq (${OSMODE}, l)
	${CXX} obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/shapes.c.o obj/material.c.o obj/randoms.c.o obj/sdl3webgpu.c.o -o build/pathtrace ${LDFLAGS} -fopenmp -lSDL3 -L. -lwgpu_native
else
	${CXX} obj/main.c.o obj/vectors.c.o obj/transformations.c.o obj/shapes.c.o obj/material.c.o obj/randoms.c.o obj/sdl3webgpu.c.o -o build/pathtrace ${LDFLAGS} -fopenmp -lSDL3 -L. -lwgpu_native
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
	clang src/shapes.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/3.json -fsyntax-only
else
	clang src/shapes.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/3.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/material.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/4.json -fsyntax-only
else
	clang src/material.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/4.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/randoms.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/5.json -fsyntax-only
else
	clang src/randoms.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/5.json -fsyntax-only
endif
ifeq (${OSMODE}, l)
	clang src/sdl3webgpu.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/6.json -fsyntax-only
else
	clang src/sdl3webgpu.c  ${CFLAGS} -O3 -fopenmp -Iinclude -MJ emmgtemp/6.json -fsyntax-only
endif
# not cross platform here sad i think
	echo [ > emmgtemp/[
	echo ] > emmgtemp/]
	cat emmgtemp/[ emmgtemp/*.json emmgtemp/] > compile_commands.json
	rm -r emmgtemp
