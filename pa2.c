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

#include "pa2.h"
#define BIAS 31

//TODO : delete
#include <stdio.h>

/********* How to normalize *********/

// 0. MSB로 sign 받아놓음.
//      1) 양수면 1로 넘어감
//      2) 음수면 2's complement 해서 양수로 만든다.
// 1. 1이 나올 때까지 어떤 연산을 한다. (아마 어떤 bit 연산)
//      1) 32bit 중에서 n번째에 1이 나왔다고 치면, 32-n이 E가 된다.
//      2) 00000000 00000000 00000111 11100100 이라면 MSB쪽 가장 먼저 나오는 1은 22번째임.
//          -> E = 32-22 = 10 
//          -> 1.1111100100 * 2^10
// 2. ROUNDING 
//      1) frac bit = 5bit. 
//      2) LSB = 101이므로 걍 truncate한다.
//          -> 1.11111 * 2^10
// 3. renormalize : 만일 10.00000 * 2^31 과 같이 노멀이 아닌 값이 나오면 1.00000 * 2^32로 normalize.
// 4. encoding
//      1) sign bit : 5bit
//          -> 0에서 받아놓은 sign extension
//      2) exp : 6bit
//          -> exp = e + BIAS
//          -> exp = 10 + 31 = 41 = 32+8+1 = 101001
//      3) frac : 5bit
//          -> 11111
//      4) result : 00000 101001 11111 = 00000101 00111111




/* Convert 32-bit signed integer to 12-bit floating point */
fp12 int_fp12(int n)
{
    fp12 result = 0; // 결과 저장할 fp12형 16bit를 0으로 초기화


	return result;
}

/* Convert 12-bit floating point to 32-bit signed integer */
int fp12_int(fp12 x)
{
	/* TODO */
	return 1;
}

/* Convert 32-bit single-precision floating point to 12-bit floating point */
fp12 float_fp12(float f)
{
	/* TODO */
	return 1;
}

/* Convert 12-bit floating point to 32-bit single-precision floating point */
float fp12_float(fp12 x)
{
	/* TODO */
	return 1.0;
}
