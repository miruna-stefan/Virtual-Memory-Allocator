# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

SRCS=$(wildcard *.c)
OBJS=$(SRCS:%.c=%.o)

all: build

build: $(OBJS)
	$(CC) $(CFLAGS) $^ -o vma

run_vma: build
	./vma

pack:
	zip -FSr 314CA_StefanMiruna-Andreea_Tema1.zip README Makefile *.c *.h

clean:
	rm -f $(OBJS) vma

.PHONY: all build run_vma clean
