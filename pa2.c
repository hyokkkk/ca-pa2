#pragma GCC optimize("O3")
#pragma GCC target("arch=ivybridge")

typedef unsigned short int fp12;
#define BIAS 31

#define FP12_NAN 0x07f1

#define FP12_NEG_NAN 0xfff1

#define FP12_INF 0x07e0

#define FP12_NEG_INF 0xffe0

typedef enum {false, true} bool;

fp12 int_fp12(int n)
{

    if (n == 0) return 0;

    unsigned short sign = n < 0 ? 0xf800 : 0;
    unsigned int un = n < 0 ? (unsigned int)~n+1 : (unsigned int)n;

    int cnt = 0;
    while (un < (unsigned int)0x80000000){ un<<=1; cnt++; }
    int e = 31 - cnt;

    bool RS = un <<6 >= 0x80000040 ? true : false;

    bool LR = un << 5 >= 0xc0000000 ? true : false;

    un >>= 32-6;
    if (RS || LR) un += 1;

    if (un >= 0x40) {
        un >>= 1;
        e += 1;
    }

    if (e == 32){
       if (sign == 0xf800) return FP12_NEG_INF;
       else if (sign == 0) return FP12_INF;
    }

    unsigned short int exp = e + BIAS;
    exp <<= 5;

    un <<= 32-5;
    un >>= 32-5;

    fp12 result = 0;
    result |= sign | exp | un;

    return result;
}

int fp12_int(fp12 x)
{

    fp12 exp = x << 5;
    exp >>= 10;

    if (exp == 0) return 0;

    if (exp == 0x3f) return 0x80000000;

    int sign = x >= 0xf800 ? 1 : 0;

    int e = exp - BIAS;

    fp12 frac = x << 11;
    frac >>= 11;

    int mantissa = frac | 0x20;

    int unsignedResult = 0;
    if (0 < e-5 && e-5 <= 32) unsignedResult |= (mantissa <<= (e-5));
    else if (0 <= 5-e && 5-e <= 32) unsignedResult |= (mantissa >>= (5-e));

    if (sign == 0) {
        if ((unsigned int)unsignedResult > 0x7fffffff) return 0x80000000;
    }

    else {
        if ((unsigned int)unsignedResult > (unsigned int)0x80000000) return 0x80000000;
    }

    int result = unsignedResult;
    if (sign == 1) result = ~unsignedResult +1;

    return result;

}

fp12 float_fp12(float f)
{
    const union { float ieee754; unsigned int binary; } uni = { .ieee754 = f };
    const unsigned int fsign = uni.binary & 0x80000000;
    unsigned short fexp = (uni.binary >> 23) & 0xff;

    if (fexp <= 127-37) { return fsign ? 0xf800 : 0x0000; }

    const unsigned int ffrac = uni.binary & 0b00000000011111111111111111111111;

    if (fexp > 127+31) {
        if (fexp == 0xff) {
            if (ffrac) {
                return fsign ? FP12_NEG_NAN : FP12_NAN;
            } else {
                return fsign ? FP12_NEG_INF : FP12_INF;
            }
        }
        return fsign ? FP12_NEG_INF : FP12_INF;
    }

    bool denormflag = fexp <= 127-31;

    unsigned short frac;
    if (denormflag) {
        const unsigned int wholefrac = ((ffrac | 0x00800000) << (9 + 30 + fexp - 127));
        fexp = 127-30;
        const unsigned int R =    wholefrac & 0b00000100000000000000000000000000;
        const unsigned int LorS = wholefrac & 0b00001011111111111111111111111000;
        const int tmp = wholefrac >> 27;
        frac = R && LorS ? tmp + 1 : tmp;
    } else {
        const unsigned int R =    ffrac & 0b00000000000000100000000000000000;
        const unsigned int LorS = ffrac & 0b00000000000001011111111111111111;
        const int tmp = ffrac >> 18;
        frac = R && LorS ? tmp + 1 : tmp;
    }

    if (frac == 0b100000) {
        frac = 0;
        if (!denormflag) { ++fexp; }
        if (fexp == 127+32) { return fsign ? FP12_NEG_INF : FP12_INF; }
        denormflag = false;
    }

    const fp12 sign = fsign ? 0xf800 : 0;
    if (denormflag) { return sign | frac; }
    const fp12 exp = (fexp - 127 + BIAS) << 5;
    return sign | frac | exp;
}

float fp12_float(fp12 x)
{

    char fpSign = x >= 0xf800 ? 1 : 0;

    fp12 fpExp = x << 5;
    fpExp >>= 10;

    fp12 fpFrac = x << 11;
    fpFrac >>= 11;

    if (fpExp == 0x3f && fpFrac == 0)
       return fpSign == 0 ? 1/0.0 : -1/0.0;

    if (fpExp == 0x3f && fpFrac != 0) {
        return fpSign == 0 ? -(0.0/0.0): 0.0/0.0; }

    if (fpExp==0 && fpFrac ==0)
        return fpSign == 0 ? 0.0 : -0.0;

    int e = fpExp == 0 ? 1-BIAS : fpExp - BIAS;

    float mantissa = fpExp == 0 ? fpFrac/32.0f: 1+fpFrac/32.0f;

    int twoPowE = 1;
    twoPowE = e >= 0 ? twoPowE << e : twoPowE << -e;

    float twoPowNegE = 1.0f/twoPowE;

    float unsigned_result = e >= 0 ? mantissa * twoPowE : mantissa * twoPowNegE;

    if (e == 31) return -unsigned_result;

    float result = fpSign == 0 ? unsigned_result : -unsigned_result;
    return result;

}
