#!/usr/bin/env make -f
# Copyright © 2022 OOTA, Masato
# This is published under CC0 1.0.
# For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.

#DMACROS=-DSERDES_TURN_DEG_DEBUG_FORCE_32BIT
CFLAGS=-O3 -fanalyzer -pedantic -Wall -fsanitize=address -fsanitize=undefined $(DMACROS) -I../source
CFLAGS_LIB=-std=c89 $(CFLAGS)
CFLAGS_TEST=-std=c99 $(CFLAGS)

SRCS=../source/serdes_turn_deg.c
TESTS=test_serdes_turn_deg.c
OBJS=test_serdes_turn_deg.o serdes_turn_deg.o

all: test

test: test_serdes_turn_deg
	./test_serdes_turn_deg

test_serdes_turn_deg: $(OBJS)
	gcc $(CFLAGS_TEST) -o $@ $^ -lm

test_serdes_turn_deg.o: test_serdes_turn_deg.c ../source/serdes_turn_deg.h
	gcc $(CFLAGS_TEST) -c test_serdes_turn_deg.c

serdes_turn_deg.o: ../source/serdes_turn_deg.c ../source/serdes_turn_deg.h
	gcc $(CFLAGS_LIB) -c ../source/serdes_turn_deg.c

clean:
	rm test_serdes_turn_deg $(OBJS)
