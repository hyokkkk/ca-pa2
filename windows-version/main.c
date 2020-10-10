typedef unsigned short int fp12;

static fp12 float_fp12(float f);    // convert float -> fp12
#define BIAS 31

typedef enum {false, true} bool; //#include <stdbool.h> 해야하지만 여기선 사용불가하므로.

typedef struct {
    unsigned short lower; //little endian 이라서 lower먼저 선언해줘야 뒷부분이 lower에 할당됨
    unsigned short upper;
} Struct;

typedef union {
    float input;
    unsigned int wholefrac;
    Struct twoShort;
} Union;

static fp12 float_fp12(float f)
{
//
// 0. float's sign, exp, frac extraction
//
    // 0) input float 값을 공용체 전체공간에 저장
    Union uni;
    uni.input = f;

    // 1) float sign : +0, -0도 커버된다. 단순히 float 입력값을 0을 기준으로 비교하면 NaN 은 숫자가 아니라 비교가 안 됨. -2점이었음.
    char fsign = uni.wholefrac < 0x80000000 ? 0 : 1;

    // 2) exp : uni.twoshort.upper 값 읽어와서 필요한 부분만 추출
    unsigned short fexp = uni.twoShort.upper << 1; // remove sign bit
    fexp >>= 8; // remove frac bits

    // 3) frac
    unsigned int wholefrac = uni.wholefrac << 9; // 32bit에 전체 frac을 담아 앞에서부터 채움.

//
// 1. special forms : INF, NaN, 0
//
    // 원래 지수
    int e = fexp -127;

    // 1) +0, -0 : fexp = 0000 0000
    // -> fp denorm은 무조건 0으로 변환된다.
    // -> e <= -37도 무조건 0으로 변환된다. -36 <= e <= -31 은 fp normal -> fp12 denormal이라서 따로 다룸
    // -> 이 범위에 fexp == 0도 다 포함됨.
    if (e <= -37)
        return fsign == 0 ? 0 : 0xf800;

    // 2) INF : fexp = 1111 1111, frac = 0
    if (fexp == 0xff && wholefrac == 0)
        return fsign == 0 ? 0x07e0 : 0xffe0;

    // 3) Nan : fexp = 1111 1111, frac != 0
    if (fexp == 0xff && (wholefrac != 0))
        return fsign == 0 ? 0x07f1 : 0xfff1;

    // 3) Rounding 전부터 크기가 너무 커서 INF가 명백한 수 거르기
    // -> NaN까지 다 한 후에 e > 31 인 것들 마저 걸러낸다. (그 전에 하면 nan까지 inf로 처리됨)
    // -> fp12 Max: e=31. fexp = e + 127. fexp Max: 158. ==> 158 < fexp 는 INF이다
    if (e > 31) return fsign == 0 ? 0x07e0 : 0xffe0;

//
//2. 1) fp norm -> fp12 norm (e >= -30): 원래 짜던대로 진행
//   2) fp norm -> fp12 denorm (1.00.....01 * 2^-36 ~ 1.11....11 * 2^-31) : special check is needed
    bool denormflag = (-36 <= e && e <= -31) ? true : false; // 나중에 exp encoding, e== -31에서 rounding될 때 사용.
    if (denormflag) {
        wholefrac >>= 1; // denorm으로 만들기 위해 정수부에 있는 1을 frac부분에 넣는 과정.
        wholefrac |= 0x80000000;
        wholefrac >>= -30-e-1;
        e = -30; // denorm으로 만들어야하니 e==-30으로 통일시킴
    }

//
// 3. Rounding : LRS = 011, 111, 110 일 때만 +1하고 나머지는 truncate한다.
//
    // 1) RS == 11 check
    unsigned int rs = wholefrac << 5;
    bool RS = rs > 0x80000000 ? true : false;

    // 2) LR == 11 check
    unsigned int lr = wholefrac << 4;
    bool LR = lr >= 0xc0000000 ? true : false;

    // 3) truncate 후 LRS 조건에 맞는 것만 +1
    unsigned short frac = wholefrac >> 27;
    if (RS || LR) frac += 1;

//
// 4. Renormalization : frac이 정상이라면 100000 보다 작음
//
    if ((unsigned short)frac >= 0x0020) {
        frac = 0;
        //denorm 켜진 상태에서 frac == 100000 된 거는 1.00000 * 2^-30 된거임
        if (denormflag == true) denormflag = false; // exp encoding 위해 flag 끔.
        else e++;
    }

//
// 4-1. special case after rounding
//
    // fp12 Max = 00000 111110 11111 = 1.11111 * 2^31
    // +INF = 00000 111111 00000 = 0x07e0; -INF = 11111 111111 00000 = 0xffe0;
    if (e >= 32)    return fsign == 1 ? 0xffe0 : 0x07e0;

//
// 5. encoding
//
    fp12 exp = 0;
    fp12 sign = fsign == 0 ? 0 : 0xf800; // 음수면 11111 000000 00000;

    // 1) exp
    // -> denorm 아닌 경우는 e + BIAS
    // -> denorm 인 경우는 0
    if (!denormflag) exp = (e + BIAS) << 5;

    fp12 result = 0;
    result |= (sign | exp | frac);

    return result;

}


// -----------------------------------------------------------------------------------------------------


#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define RED   ""
#define GREEN ""
#define CYAN  ""
#define RESET ""

#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BINARY(BYTE)            \
    (BYTE & 0x80 ? '1' : '0'),    \
    (BYTE & 0x40 ? '1' : '0'),    \
    (BYTE & 0x20 ? '1' : '0'),    \
    (BYTE & 0x10 ? '1' : '0'),    \
    (BYTE & 0x08 ? '1' : '0'),    \
    (BYTE & 0x04 ? '1' : '0'),    \
    (BYTE & 0x02 ? '1' : '0'),    \
    (BYTE & 0x01 ? '1' : '0')

#define PRINT_BYTE(BYTE) printf(BINARY_PATTERN, BINARY(BYTE))
#define PRINT(DATATYPE, TYPENAME, NUM)                \
    do {                            \
        size_t typesize = sizeof(DATATYPE);        \
        DATATYPE data = NUM;                \
        uint8_t *ptr = (uint8_t *)&data;        \
                                \
        printf("%s(", TYPENAME);            \
        PRINT_BYTE(*(ptr + typesize - 1));        \
        for (ssize_t i = typesize - 2; i >= 0; i--) {    \
            printf(" ");                \
            PRINT_BYTE(*(ptr + i));            \
        }                        \
        printf(")");                    \
    } while (0)

#define CHECK(RES, ANS) printf("%s"RESET, (RES) == (ANS) ? GREEN"CORRECT" : RED"WRONG")
#define COMP(RES, ANS, TYPENAME) comp_##TYPENAME(RES, ANS)

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

static uint32_t test3[] = {0x00000001, 0x3fe00000, 0xe0000000, 0x80b5840c, 0x4f7a0001, 0x4f7e0001, 0x2d800001, 0x307fffff, 0x7f800000, 0xffffffff};
static uint16_t ans3[sizeof test3/sizeof test3[0]] =    {0x0000, 0x03f8, 0xffe0, 0xf800, 0x07df, 0x07e0, 0x0001, 0x0020, 0x07e0, 0xffe1};

int main(void)
{
    printf("\n%sTest: Casting from float to fp12%s\n", CYAN, RESET);
    for (unsigned int i = 0; i < sizeof test3/sizeof test3[0]; i++) {
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
    printf("\n");

    int count = 1<<26; // 1<<28;
    printf("\n%sBenchmark: Running float_fp12 for %d times%s\n", CYAN, count, RESET);
    struct timespec begin, end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
    for (int i = 0; i < count; ++i) {
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
