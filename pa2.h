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

#ifndef _PA2_H_
#define _PA2_H_

typedef unsigned short int fp12;

fp12 int_fp12(int n);		// convert int -> fp12
int fp12_int(fp12 x);		// convert fp12 -> int
fp12 float_fp12(float f);	// convert float -> fp12
float fp12_float(fp12 x);	// convert fp12 -> float

#endif /* _PA2_H_ */
