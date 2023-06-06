CFLAGS = -Wall -D_POSIX_C_SOURCE=199309L
CFLAGS_OBFS = -Wall -s -funroll-loops -O2

all: obfs
	#gcc main.c -o main ${CFLAGS_OBFS}

obfs:
	gcc main.obfs.c -o main ${CFLAGS_OBFS}

overkill:
	gcc main.obfs.c -o main -funroll-loops -Wshadow -s -fvisibility=hidden -static

docker:
	docker build -t haklu:sigs .

pack:
	tar czf signals.tar.xz Dockerfile main.obfs.c Makefile run.sh test.flag README.md

clean:
	-rm main signals.tar.xz
