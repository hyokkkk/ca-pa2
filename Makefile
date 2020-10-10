#----------------------------------------------------------------
#
#  4190.308 Computer Architecture (Fall 2020)
#
#  Project #2: FP12 (12-bit floating point) Representation
#
#  September 28, 2020
#
#  Injae Kang (abcinje@snu.ac.kr)
#  Sunmin Jeong (sunnyday0208@snu.ac.kr)
#  Systems Software & Architecture Laboratory
#  Dept. of Computer Science and Engineering
#  Seoul National University
#
#----------------------------------------------------------------

TARGET	= pa2
SRCS	= pa2.c pa2-test.c
CC	= gcc
CFLAGS	= --coverage -g -O2 -Wall -Wextra -Wpedantic
LDFLAGS = --coverage
OBJS	= $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)

.PHONY: bench
bench: $(TARGET)
	./pa2
	gcov pa2.c
