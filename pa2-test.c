// vim: noexpandtab ts=8 sw=8 sts=8
//---------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2020)
//
//  Project #2: FP12 (12-bit floating point) Representation
//
//  September 28, 2020
//
//  Injae Kang (abcinje@snu.ac.kr)
//  Sunmin Jeong (sunnyday0208@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//---------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#define RED   "\033[0;31m"
#define GREEN "\033[0;32m"
#define CYAN  "\033[0;36m"
#define RESET "\033[0m"

#include "pa2.h"

#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BINARY(BYTE)			\
	(BYTE & 0x80 ? '1' : '0'),	\
	(BYTE & 0x40 ? '1' : '0'),	\
	(BYTE & 0x20 ? '1' : '0'),	\
	(BYTE & 0x10 ? '1' : '0'),	\
	(BYTE & 0x08 ? '1' : '0'),	\
	(BYTE & 0x04 ? '1' : '0'),	\
	(BYTE & 0x02 ? '1' : '0'),	\
	(BYTE & 0x01 ? '1' : '0')

#define PRINT_BYTE(BYTE) printf(BINARY_PATTERN, BINARY(BYTE))
#define PRINT(DATATYPE, TYPENAME, NUM)				\
	do {							\
		size_t typesize = sizeof(DATATYPE);		\
		DATATYPE data = NUM;				\
		uint8_t *ptr = (uint8_t *)&data;		\
								\
		printf("%s(", TYPENAME);			\
		PRINT_BYTE(*(ptr + typesize - 1));		\
		for (ssize_t i = typesize - 2; i >= 0; i--) {	\
			printf(" ");				\
			PRINT_BYTE(*(ptr + i));			\
		}						\
		printf(")");					\
	} while (0)

#define CHECK(RES, ANS) printf("%s"RESET, (RES) == (ANS) ? GREEN"CORRECT" : RED"WRONG")
#define COMP(RES, ANS, TYPENAME) comp_##TYPENAME(RES, ANS)

static void comp_int(uint32_t result, uint32_t answer)
{
	CHECK(result, answer);
}

static void comp_fp12(uint16_t result, uint16_t answer)
{
	uint16_t exp = 0x7e0 & result;
	uint16_t frac = 0x1f & result;
	if (exp == 0x7e0 && frac != 0) {
		result &= 0xffe0;
		result++;
	}
	CHECK(result, answer);
}

static void comp_float(uint32_t result, uint32_t answer)
{
	uint32_t exp = 0x7f800000 & result;
	uint32_t frac = 0x7fffff & result;
	if (exp == 0x7f800000 && frac != 0) {
		result &= 0xff800000;
		result++;
	}
	CHECK(result, answer);
}

#define N 6

/* int -> fp12 */
uint32_t test1[N] =	{0x00000000, 0x00000001, 0x000007e4, 0xffffff9b, 0x7fffffff, 0x80000000};
uint16_t ans1[N] =	{0x0000, 0x03e0, 0x053f, 0xfcb2, 0x07c0, 0xffc0};

/* fp12 -> int */
uint16_t test2[N] =	{0xf800, 0x04d6, 0x044c, 0xf81f, 0x07e0, 0xffff};
uint32_t ans2[N] =	{0x00000000, 0x000000d8, 0x0000000b, 0x00000000, 0x80000000, 0x80000000};

/* float -> fp12 */
uint32_t test3[] =	{0x00000001, 0x3fe00000, 0xe0000000, 0x80b5840c, 0x4f7a0001, 0x4f7e0001, 0x2d800001, 0x307fffff, 0x7f800000, 0xffffffff};
			// 00000111 11011111 = 1.11111 * 2^31
			// 00000111 11100000 = 10.11110 * 2^31 이라 무한대
			// 00000000 00000001 = 1.00000.....1 * 2^-36 = 0.00001 * 2^-30
			// 00000000 00100000 = 1.11111000....01 * 2^-31 = 1.00000 * 2^-30
uint16_t ans3[sizeof test3/sizeof test3[0]] =	{0x0000, 0x03f8, 0xffe0, 0xf800, 0x07df, 0x07e0, 0x0001, 0x0020, 0x07e0, 0xffe1};

/* fp12 -> float */
uint16_t test4[N] =	{0x0000, 0xf801, 0x0555, 0x07e0, 0xffe0, 0x07e1};
uint32_t ans4[N] =	{0x00000000, 0xae000000, 0x45540000, 0x7f800000, 0xff800000, 0x7f800001};

int main(void)
{
	printf("\n%sTest 1: Casting from int to fp12%s\n", CYAN, RESET);
	for (int i = 0; i < N; i++) {
		uint16_t result = (uint16_t)int_fp12(test1[i]);

		PRINT(uint32_t, "int", test1[i]);
		printf(" => ");
		PRINT(uint16_t, "fp12", result);
		printf(", ");
		PRINT(uint16_t, "ans", ans1[i]);
		printf(", ");
		COMP(result, ans1[i], fp12);
		printf("\n");
	}

	printf("\n%sTest 2: Casting from fp12 to int%s\n", CYAN, RESET);
	for (int i = 0; i < N; i++) {
		uint32_t result = (uint32_t)fp12_int(test2[i]);

		PRINT(uint16_t, "fp12", test2[i]);
		printf(" => ");
		PRINT(uint32_t, "int", result);
		printf(", ");
		PRINT(uint32_t, "ans", ans2[i]);
		printf(", ");
		COMP(result, ans2[i], int);
		printf("\n");
	}

	printf("\n%sTest 3: Casting from float to fp12%s\n", CYAN, RESET);
	for (int i = 0; i < sizeof test3/sizeof test3[0]; i++) {
		float *p = (float *)&test3[i];
		float f = *p;
		uint16_t result = (uint16_t)float_fp12(f);

		PRINT(uint32_t, "float", test3[i]);
		printf(" => ");
		PRINT(uint16_t, "fp12", result);
		printf(", ");
		PRINT(uint16_t, "ans", ans3[i]);
		printf(", ");
		COMP(result, ans3[i], fp12);
		printf("\n");
	}

	printf("\n%sTest 4: Casting from fp12 to float%s\n", CYAN, RESET);
	for (int i = 0; i < N; i++) {
		float f = fp12_float(test4[i]);
		uint32_t *p = (uint32_t *)&f;
		uint32_t result = *p;

		PRINT(uint16_t, "fp12", test4[i]);
		printf(" => ");
		PRINT(uint32_t, "float", result);
		printf(", ");
		PRINT(uint32_t, "ans", ans4[i]);
		printf(", ");
		COMP(result, ans4[i], float);
		printf("\n");
	}

	printf("\n");

	printf("\n%sBenchmark: Running float_fp12 for 2^28 times%s\n", CYAN, RESET);
	struct timespec begin, end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
	for (int i = 0; i < (1<<28); ++i) {
		// Worst case input
		float_fp12(0.000000000916770714898262895076186396181583404541015625);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	double elapsed =
		(double)(end.tv_sec - begin.tv_sec) * 1000.0 +
		(double)(end.tv_nsec - begin.tv_nsec) / 1000000.0;
	printf("%lfms\n", elapsed);
	return 0;
}
