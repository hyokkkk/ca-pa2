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
//      0) 입력이 signed int n으로 들어오니까 그냥 n<0 만 판단하면 된다.
//      1) 양수면 1.로 넘어감
//      2) 음수면 2's complement 해서 양수로 만든다.
// 1. 1이 나올 때까지 어떤 연산을 한다.
//      0) 0x80000000는 100000....000 이다. 따라서 어떤 수의 msb에 1이 나오면 >= 10000...000 이 될거임. shift연산하면서 몇 번 옮겼는지 세면 됨.
//      1) 32bit 중에서 n번째에 1이 나왔다고 치면, 32-n이 E가 된다.
//      2) 00000000 00000000 00000111 11100100 이라면 MSB쪽 가장 먼저 나오는 1은 22번째임.
//          -> E = 32-22 = 10 
//          -> 1.1111100100 * 2^10
// 2. ROUNDING 
//      1) frac bit = 5bit. 
//      2) LRS = 011, 111, 110 인 경우에만 round up, 나머지는 truncate.
//          -> LRS = 101이므로 걍 truncate한다.
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

// -1. n == 0 일 때 처리
    if (n == 0) return 0;

//
// 0. sign 받아놓기
//
    unsigned short int sign = n < 0 ? 0xf800 : 0; // -: 1111 1000 0000 0000, 0 & +: 00000000 00000000
    unsigned int un= n < 0 ? (unsigned int) ~n+1 : n; //음수면 양수로 바꿈. -2147483648같은 경계값은 -붙여도 자기 자신임. 

    //TODO : 이렇게 쓰면 MSB를 그냥 숫자로 읽어서 -101이 101이 되는 게 아니라 엄청 큰 수로 인식됨.
    //unsigned int un =(unsigned int)n ;


//
// 1. Normalizing & E 구하기
//

    //TODO : delete
    printf("n : %d, un : %u, 맨앞1: %u\n", n, un, (unsigned int) 0x80000000);


    int cnt = 1;
    
    // un의 MSB == 1일 때까지 shift
    while (un < (unsigned int)0x80000000){
        un<<=1; cnt++;
    } 

    int e = 32 - cnt;

    //TODO : delete
    printf("e : %d\n", e);


//
// 2. Rounding : LRS = 011, 111, 110 일 때만 +1하고 나머지는 truncate한다.
//
    //
    // 2-1) RS == 11 check
    //
    unsigned int rs = 0xffffffff >> 6; // int 1bit + frac 5 bit -> 00000LR111111111...
    
    // rs & un 한 게 >= 00000010 00000000 00000000 00000001 이면 s==1 이라는 의미다. 
    bool RS = rs & un > 0x02000000 ? true : false;


    //
    // 2-2) LR == 11 check
    //
    unsigned int lr = 0xffffffff >> 5; // int 1bit + frac 5bit 중 LSB니까 5bit shift -> 00000111.....
    
    // lr & un >= 000001100 00000000 00000000 00000000 이면 s에 상관 없이 LR == 11임.
    bool LR = lr & un >= 0x0c000000 ? true : false;


    //
    // 2-3) truncate 후 LRS 조건에 맞는 것만 +1
    //
    un >>= 32-6; // int 1bit + frac 5bit = 6bit만 남게 shift 
    if (RS || LR) un += 1; 


//
// 3. Renormalization
// -> 정상이라면 1xxxxx처럼 6bit 숫자임. but 10.00000처럼 7bit로 넘어가는 경우도 있다.
// -> 01000000 이상이면 >>1 하고 e += 1;
//
    
    if (un >= 0x40) {
        un >>= 1;
        e += 1;
    }


//
// 4. Encoding
// -> sign, exp, frac을 애초에 16bit로 extend해서 선언해놓고 result에 넣을 때 shift 없이 OR만 한다.
// 
    
    // 4-1) sign은 위에서 구해놓음
    
    // 4-2) exp : 6bit 니까 <<5.
    unsigned short int exp = e + BIAS;
    exp <<= 5;

    // 4-3) frac


    fp12 result = 0; // 결과 저장할 fp12형 16bit를 0으로 초기화

    result |= sign|exp; // 애초에 16bit로 extend하면 1byte로 선언해놓고 result에 넣을 때 shift 연산 안 해도 돼서 이렇게 함.

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
