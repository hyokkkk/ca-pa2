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

typedef enum {false, true} bool; //#include <stdbool.h> 해야하지만 여기선 사용불가하므로.

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




/*------------- Convert 32-bit signed integer to 12-bit floating point ---------------*/
fp12 int_fp12(int n)
{

// -1. n == 0 일 때 처리
    if (n == 0) return 0;

//
// 0. sign 받아놓기
//
    unsigned short int sign = n < 0 ? 0xf800 : 0; // -: 1111 1000 0000 0000, 0 & +: 00000000 00000000
    unsigned int un= n < 0 ? (unsigned int) ~n+1 : n; //음수면 양수로 바꿈. -2147483648같은 경계값은 -붙여도 자기 자신임. 

    //이렇게 쓰면 MSB를 sign이 아닌 그냥 숫자로 읽어서 -101이 101이 되는 게 아니라 엄청 큰 수로 인식됨.
    //unsigned int un =(unsigned int)n ;


//
// 1. Normalizing & E 구하기
//
    int cnt = 0;
    
    // un의 MSB == 1일 때까지 shift
    while (un < (unsigned int)0x80000000){ un<<=1; cnt++; } 
    
    int e = 31 - cnt;



//
// 2. Rounding : LRS = 011, 111, 110 일 때만 +1하고 나머지는 truncate한다. (2점)
//
    // 1) RS == 11 check
    unsigned int rs = 0xffffffff; // int 1bit + frac 5 bit -> 11111L|R111111111...
    // RS == 11이려면, R==1, S이하에는 하나의 1만 있으면 됨.
    // rs & un 한 게 Ixxxxx10 00000000 00000000 00000001 이상이면 s==1 이라는 의미다. 
    // --> (rs & un) << 6 한 게 10000000 00000000 00000000 01000000 이상이면 됨.
    bool RS = (rs & un) <<6 >= 0x80000040 ? true : false;
    

    // 2) LR == 11 check
    unsigned int lr = 0xffffffff; // int 1bit + frac 5bit 중 LSB니까 5bit shift
    // lr & un 한 게 Ixxxx11x xxxxxx이면 s에 상관 없이 LR == 11임.
    // --> (lr & un) << 5 한 게 11000000 00000000 0~ 0~ 이상이면 됨.
    bool LR = (lr & un) << 5 >= 0xc0000000 ? true : false;
    

    // 3) truncate 후 LRS 조건에 맞는 것만 +1
    un >>= 32-6; // int 1bit + frac 5bit = 6bit만 남게 shift 
    if (RS || LR) un += 1; 


// 3. Renormalization 
// -> 정상이라면 1xxxxx처럼 6bit 숫자임. but 10.00000처럼 7bit로 넘어가는 경우도 있다.
// -> 01000000 이상이면 >>1 하고 e += 1;
    
    //TODO : delete
    //printf("un: %x ", un);

    if (un >= 0x40) {
        un >>= 1;
        e += 1;
    }
    
    //TODO : delete
    //printf("e: %d ", e);

// TODO: 2점이 깎이는데 여기가 문제가 아닌가... 이 문단은 없어도 점수 동일함.
// 3-1. special case
// -> 라운딩 후 fp12로 나타낼 수 있는 범위를 넘어가는 int는 부호에 따라 +INF, -INF 로 나타냄

    // fp12 Max = 00000 111110 11111 = 1.11111 * 2^31
    // +INF = 00000 111111 00000 = 0x07e0;
    // -INF = 11111 111111 00000 = 0xffe0;
    if (e == 32){
       if (sign == 0xf800) return 0xffe0;
       else if (sign == 0) return 0x07e0;
    }




// 4. Encoding
// -> sign, exp, frac을 애초에 16bit로 extend해서 선언해놓고 result에 넣을 때 shift 없이 OR만 한다.
 
    // 1) sign은 위에서 구해놓음

    // 2) exp : 6bit 니까 <<5.
    unsigned short int exp = e + BIAS;
    exp <<= 5;

    // 3) frac만 남기기 : << 32-5 한 후에 ORing 위해 다시 >>32-5
    un <<= 32-5;
    un >>= 32-5;
    
    // 4) result에 넣기
    fp12 result = 0; // 결과 저장할 fp12형 16bit를 0으로 초기화
    result |= sign | exp | un; // 애초에 16bit로 extend하면 1byte로 선언해놓고 result에 넣을 때 shift 연산 안 해도 돼서 이렇게 함.

    return result;
}








/*--------------- Convert 12-bit floating point to 32-bit signed integer ------------------*/


int fp12_int(fp12 x)
{

//
// 0. exp = 000000, 111111 처리하기
//
    fp12 exp = x << 5; // remove sign
    exp >>= 10; // remove frac

    // 1) exp = 000000 : +0, -0, denormalized form 은 다 0으로 처리
    if (exp == 0) return 0;

    // 2) exp = 111111 은 -inf, +inf, +Nan, -Nan 이다.
    if (exp == 0x3f) return 0x80000000;

//
// 1. encoding
//
    // 1) sign  
    // -> input x >= 11111000~~~ 이라면 sign은 1, 작으면 걍 0.
    int sign = x >= 0xf800 ? 1 : 0; // 여기가 문제였어!!!! 갹~~~~ (3점) 

    // 2) exp : 위에서 구해놓음. 실제 지수 e는 exp - BIAS. e는 ORing이 필요 없다.
    int e = exp - BIAS;

    // 3) frac : LSB 5bit에 넣어놓기
    fp12 frac = x << 11;
    frac >>= 11;

    // 4) mantissa : 1.frac
    // -> 5bit frac | 100000 
    int mantissa = frac | 0x20;

    //TODO : delete
//    printf("man: %x , e: %d ", mantissa, e);

    // 5) sign 붙이기 전 : mantissa를 e-5만큼 <<. 혹시 shift를 bit수 이상으로 하면 비트가 순환하나?
    // -> 가상의 소숫점이 frac 5bit를 가진다고 봐야하니까 실제 지수가 7이면 << 2 해야 함.
    // -> e-5가 음수면 >>해야 함. 32bit shift 이상이면 비트가 순환해서 오류가 생기므로 초기값인 0으로 놔둔다.
    int unsignedResult = 0;
    if (0 < e-5 && e-5 <= 32) unsignedResult |= (mantissa <<= (e-5));
    else if (0 <= 5-e && 5-e <= 32) unsignedResult |= (mantissa >>= (5-e));
    

    //TODO : delete
  //  printf("unRes: %x ", unsignedResult);

//
// 2. Range overflow check
//    fp12가 의미하는 값이 int의 범위를 넘어서면 0x80000000 으로 표현. (4점)
    

    // 1) Pos num
    // -> fp12 positive Max : 00000 111110 11111 = 1.11111 * 2^31 = 11111100 0~ 0~ 0~
    // -> int pos Max : 01111111 1~ 1~ 1~
    // -> int가 나타낼 수 있는 범위를 넘어섰기에 overflow가 나타난다.
    if (sign == 0) {
        if ((unsigned int)unsignedResult > 0x7fffffff) return 0x80000000;
    } 
    // 2) Neg num
    // fp12 negative Max(abs) : 11111 111110 11111 = -1.11111 * 2^31 -> int로 나타낼 수 없다.
    // int neg Max(abs) : 10000000 0~ 0~ 0~ (절대값으로 생각해보자)
    else {
        if ((unsigned int)unsignedResult > (unsigned int)0x80000000) return 0x80000000;
    }

//
// 3. result : +면 그대로, -면 2's complement 해서 내보냄
//
    int result = unsignedResult;
    if (sign == 1) result = ~unsignedResult +1;

    return result;
    
}








/*------- Convert 32-bit single-precision floating point to 12-bit floating point -------*/

//
// 공용체의 존재사실을 망각하고 있었다!
//
// <-------------union---------------->
//
// +-+--------+-----------------------+
// |S|exp 8bit|      frac 23bit       | <- float 
// +-+--------+-----------------------+
// +----------------+----------------+
// |  upper 16bit   |  lower 16bit   | <- struct
// +----------------+----------------+
//
// 이렇게 메모리를 공유하도록 Union을 만든다.
//

typedef struct {
    unsigned short lower; //stack에 쌓이니까 lower먼저 선언해줘야 뒷부분이 lower에 할당됨
    unsigned short upper;
} Struct;

typedef union {
    float input;
    unsigned int wholefrac;
    Struct twoShort;
} Union;


fp12 float_fp12(float f)
{
//
// 0. float's sign, exp, frac extraction
//
    // 0) input float 값을 공용체 전체공간에 저장
    Union uni;
    uni.input = f;
    
    // 1) float sign : +0, -0도 커버된다.
    char fsign = uni.wholefrac < 0x80000000 ? 0 : 1;

    // 2) exp : uni.twoshort.upper 값 읽어와서 필요한 부분만 추출
    unsigned short fexp = uni.twoShort.upper << 1; // remove sign bit
    fexp >>= 8; // remove frac bits 

    // TODO : delete
//    printf("fsign: %d, fexp: %d ", fsign, fexp);

    // 3) frac : upper lsb 7bit + lower 16bit.
    unsigned int wholefrac = uni.wholefrac << 9; // 32bit에 전체 frac을 담아 앞에서부터 채움.
    
//
// 1. special forms : INF, NaN, 0
//
    // 원래 지수 
    int e = fexp -127;
    
    //TODO : delete
//    printf("brf e: %d \n", e);


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
    // -> NaN까지 다 한 후에 e > 31 인 것들 마저 걸러낸다. (그 전에 하면 nan까지 inf로 됨)
    // -> fp12 Max: e=31. fexp = e + 127. fexp Max: 158. ==> 158 < fexp 는 INF이다
    if (e > 31) return fsign == 0 ? 0x07e0 : 0xffe0;

// TODO : delete
// printf("bfr wholefrac: %x ", wholefrac);


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

    // TODO : delete
  //  printf("bool: %d , boolwholefrac: %x ", denormflag, wholefrac);

//
// 3. Rounding : LRS = 011, 111, 110 일 때만 +1하고 나머지는 truncate한다.
//
    // 1) RS == 11 check
    // lower에 1이 있으면 R만 1이어도 됨. 없으면 RS 다 1이어야 함.
    
    unsigned int rs = wholefrac << 5;
    bool RS = rs > 0x80000000 ? true : false;
    
    //TODO : delete
   // printf("rs: %x ", rs);


// TODO : delete
//    printf("<<한거: %x ", (rs&fracMSB7) <<13);

    // 2) LR == 11 check
   
    unsigned int lr = wholefrac << 4;
    bool LR = lr >= 0xc0000000 ? true : false;

    //TODO : delete
// printf("RS: %d, LR: %d , frac7: %x ",RS, LR, fracMSB7);

    // 3) truncate 후 LRS 조건에 맞는 것만 +1
    unsigned short frac = wholefrac >> 27; // fracMSB7 = xxxxLRS 니까 RS 날림. >>1한게 문제엿어...ㅅㅂ
    if (RS || LR) frac += 1; 
   // TODO : delete
//    printf("wholefrac: %x frac: %x ", wholefrac, frac);

//
// 3. Renormalization : frac이 정상이라면 100000 보다 작음 
//
    if ((unsigned short)frac >= 0x0020) {
        frac = 0;
        //denorm 켜진 상태에서 frac == 100000 된 거는 1.00000 * 2^-30 된거임
        if (denormflag == true) denormflag = false; // exp encoding 위해 flag 끔. 
        else e++;
    }
    

//TODO : delete
// printf("aft e: %d \n", e);


//
// 3-1. special case after rounding 
//
    // 1) fp12 Max = 00000 111110 11111 = 1.11111 * 2^31
    // +INF = 00000 111111 00000 = 0x07e0;
    // -INF = 11111 111111 00000 = 0xffe0;
    if (e >= 32)    return fsign == 1 ? 0xffe0 : 0x07e0;

    // 2) denorm의 frac이 all 1이어도 fexp = 0 -> e = -126이다. 절대 fp12로 못나타냄 
    // -> e < -35 이어도 0 돼야 함. 
//    if (e < -35)     return fsign == 0 ? 0 : 0xf800;
  


//
// 4. encoding (2점 오름 ㅠㅠㅠㅠㅠ 나머지는 어디가 문제인거냐)
//
 
    fp12 exp = 0;
    fp12 sign = fsign == 0 ? 0 : 0xf800; // 음수면 11111 000000 00000;

    // 1) exp
    // -> denorm 아닌 경우는 e + BIAS
    // -> denorm 인 경우는 0
    if (!denormflag) exp = (e + BIAS) << 5;


    // 1) fp12가 normalized form 으로 표현되는 경우.
    // TODO : 여기 고쳐야함!!!! e= -30일 때 왜 노말로 해놨어.... 디노말도 -30일 때 있잖아....
    // 일단 e는 normal로 가정? ㅇㅇ 디노말로 표현되는 건 2)밖에 없음.
//    if (-30 <= e && e < 32)     exp = (e + BIAS) << 5; 



    // 2) float normal중 rounding 후에 fp12 denormal로 표현되는 수가 있다. 
    // -> 1.00000 * 2^-35 == 0.00001 * 2^-30;
    // -> 1.x0000   2^-34 == 0.0001x * 2^-30;
    // -> 1.xx000   2^-33 == 0.001xx * 2^-30;
    // -> 1.xxx00   2^-32 == 0.01xxx * 2^-30;
    // -> 1.xxxx0   2^-31 == 0.1xxxx * 2^-30;
    //      denorm 이니까 exp=0인 상태로 놔둠.
//    if (-35 <= e && e <= -31) {
  //      frac |= 0x20;
    //    frac >>= -30 -e;
    //}




// printf("sign: %x, exp: %x\n", sign, exp);
// TODO : delete
//    printf("e: %d, exp: %x ",e, exp);

    fp12 result = 0;
    result |= (sign | exp | frac);  

    return result;


}










/*--------- Convert 12-bit floating point to 32-bit single-precision floating point ----*/

float fp12_float(fp12 x)
{
/* float : sign = 1bit     exp = 8bit      frac = 23bit */
/* float 지수 표기법
 *      ex) 31.3324 = 3.13324e1f
 *      ex) 0.01327 = 1.327e-2f
 */

//
// 0. fp12's sign, exp, frac extraction
//
    // 1) fp12 sign : 1111 1000 0000 0000 보다 크면 sign == 1.
    char fpSign = x >= 0xf800 ? 1 : 0;

    // 2) fp12 exp
    fp12 fpExp = x << 5; //sign remove
    fpExp >>= 10; // frac remove

    // 3) fp12 frac
    fp12 fpFrac = x << 11;
    fpFrac >>= 11;



//
// 1. INF, NaN, 0
//

    // 1) INF : exp == 111111, frac == 00000
    if (fpExp == 0x3f && fpFrac == 0)
       return fpSign == 0 ? 1/0.0 : -1/0.0;

    // 2) NaN : exp == 111111, frac != 00000 (1점)
    if (fpExp == 0x3f && fpFrac != 0) {
        return fpSign == 0 ? -(0.0/0.0): 0.0/0.0; } // 0.0/0.0 : -NaN 

    // 3) 0 : exp == 000000, frac == 00000
    if (fpExp==0 && fpFrac ==0) 
        return fpSign == 0 ? 0.0 : -0.0;
    

//
// 2. General case
// -> float는 bitwise operation이 안 된다. bit에 바로 때려넣는 거 불가.
// -> 입력으로 들어온 sign, exp, frac을 분석해서 실제 값으로 환산하기로 함.
//
    // 1) sign : 위에서 함
    // 2) 실제 지수 구하기 : normal, denormal 따로.
    int e = fpExp == 0 ? 1-BIAS : fpExp - BIAS;

    // TODO : delete
//    printf("\nfsign: %x, fexp: %x, e: %d, ffrac: %x ", fpSign, fpExp, e, fpFrac);

    // 3) mantissa 구하기 : frac 값 + 1
    // -> fp12 frac : 5bit. bit의 값에 /32 하면 실제 frac의 값이 나온다.
    float mantissa = fpExp == 0 ? fpFrac/32.0f: 1+fpFrac/32.0f;

    // 4) 2^e 값 구하기
    int twoPowE = 1;
    twoPowE = e >= 0 ? twoPowE << e : twoPowE << -e; // e >= 0
 
    // TODO : delete
//    printf("2^e: %x\n ", twoPowE);

    float twoPowNegE = 1.0f/twoPowE;

    float unsigned_result = e >= 0 ? mantissa * twoPowE : mantissa * twoPowNegE;
    
    // TODO : 연구 좀 더 하기 (3점)
    // e == 31일 때에는 mantissa에 << 31해버리면 정수부분까지 해서 32bit가 차버림.
    // float는 정수부 나타낼 필요 없으니..... 아 머리 안 돌아간다. 일단 패스.
    if (e == 31) return -unsigned_result;

    float result = fpSign == 0 ? unsigned_result : -unsigned_result;

    return result;


}
