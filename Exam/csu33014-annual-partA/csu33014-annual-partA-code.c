//
// CSU33014 Summer 2020 Additional Assignment
// Part A of a two-part assignment
//

// Please examine version each of the following routines with names
// starting partA. Where the routine can be vectorized, please
// complete the corresponding vectorized routine using SSE vector
// intrinsics. Where is cannot be vectorized...

// Note the restrict qualifier in C indicates that "only the pointer
// itself or a value directly derived from it (such as pointer + 1)
// will be used to access the object to which it points".


#include <immintrin.h>
#include <stdio.h>

#include "csu33014-annual-partA-code.h"

// Can Zhou 19324118 zhouc@tcd.ie
// There are some in-depth overall explanations between rountine 0 and 1.
// Please have a look :)

/****************  routine 0 *******************/

// Here is an example routine that should be vectorized
void partA_routine0(float * restrict a, float * restrict b,
		    float * restrict c) {
  for (int i = 0; i < 1024; i++ ) {
    a[i] = b[i] * c[i];
  }
}

// here is a vectorized solution for the example above
void partA_vectorized0(float * restrict a, float * restrict b,
		    float * restrict c) {
  __m128 a4, b4, c4;
  
  for (int i = 0; i < 1024; i = i+4 ) {
    b4 = _mm_loadu_ps(&b[i]);
    c4 = _mm_loadu_ps(&c[i]);
    a4 = _mm_mul_ps(b4, c4);
    _mm_storeu_ps(&a[i], a4);
  }
}


/********************************* Some In-depth Overall Explanations ****************************************/
/*
Explanation 0 (suitable for routine 1 to 6):
  The reason why I use loadu/storeu instead of using load/store.
  If using load/store, the input data needs to be aligned with a memory boundary the same as the register size (so 4 float * 4 byte => 16 byte alignment requirement).
  Since we cannot be sure the memory is aligned, if we use load/store, the instruction may cause a hardware fault.
  However, using loadu/storeu actually will cause 4 times slower than using load/store.
  In conclusion, although using load/store will faster, in order to guarantee the accuracy, I will use loadu/storeu.

Explanation 1 (suitable for routine 1):
  Fast way to do horizontal SSE vector sum.
  There are mainly two ways to do horizontal SSE vector sum.
  one is firstly storing the vector into an array, then add them together.
  Here is the sample code:
    float result4[4];
    _mm_storeu_ps(&result4[0], sum4);
    float result = result4[0] + result4[1] + result4[2] + result4[3];
  The another one is using hadd two times.
  Here is the sample code:
    sum4 = _mm_hadd_ps(sum4, sum4);
    sum4 = _mm_hadd_ps(sum4, sum4);
    float result4[4];
    _mm_storeu_ps(&result4[0], sum4);
    sum = result4[0];
  Actually from the reference: https://stackoverflow.com/questions/6996764/fastest-way-to-do-horizontal-sse-vector-sum-or-other-reduction
  It said "The common SSE3 / SSSE3 2x hadd idiom is only good for code-size, not speed on any existing CPUs. 
           There are use-cases for it (like transpose and add, see below), but a single vector isn't one of them."
  So, I think these two approaches' efficiency are similar.
  Moreover from the reference: https://www.cs.fsu.edu/~engelen/courses/HPC-adv/MMXandSSEexamples.txt
  The example in this reference shows that the two methods are interchangeable.
  In conclusion, I think both methods are acceptable.

Explanation 2 (suitable for routine 1):
  The reason why there is difference between vectorized code and the original code in routine 1.
  In short, because of the precision for floating-point arithmetic.
  From this reference: https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html#680
  We can know that rounding error exists in floating-point arithmetic which means
  floating-point arithmetic in different operating sequences will get different answer.
  For example, there has float a, b, c and we want to get their sum.
  The result of  a + (b+c) may different with (a+b) + c because of rounding error in floating-point arithmetic.
  So, as vectorized code has different floating-point arithmetic order with the original code,
  sometimes, there may have small differences between them.
  From the reference I mentioned above, we can decrease this difference by using more precise data type, like double (_m128d).

Explanation 3 (suitable for routine 2):
  Division's precision and speed (using _mm_div_ps or _mm_rcp_ps).
  When calculating the division like 1/x in routine 2, there are two ways.
  One is using _mm_div_ps and the other one is using _mm_rcp_ps.
  Actually using _mm_rcp_ps is much faster than _mm_div_ps, it is fast as an add.
  But, rcp instruction will reduce accuracy especially when taking the reciprocal of very large numbers.
  However, we can use a Newton Rhapson Iteration to increase the precision (but still may have some small errors).
  For the routine 2, I think accuracy is more important and b[i] could be a very large number.
  In conclusion, although using _mm_rcp_ps is faster and can use a Newton Rhapson Iteration to increase the precision,
  there still has a tiny difference with the original code and I think accuracy is more important.
  So, I will use _mm_div_ps instead of _mm_rcp_ps.
*/

/***************** routine 1 *********************/

// in the following, size can have any positive value
float partA_routine1(float * restrict a, float * restrict b,
		     int size) {
  float sum = 0.0;
  
  for ( int i = 0; i < size; i++ ) {
    sum = sum + a[i] * b[i];
  }
  return sum;
}

// Sometimes, there may have small differences between vectorized code and the original code in routine 1.
// Here are some explanations (as same as mentioned in Explanation 2 above):
//  In short, because of the precision for floating-point arithmetic.
//  The rounding error exists in floating-point arithmetic which means
//  floating-point arithmetic in different operating sequences will get different answer.
//  For example, there has float a, b, c and we want to get their sum.
//  The result of  a + (b+c) may different with (a+b) + c because of rounding error in floating-point arithmetic.
//  So, as vectorized code has different floating-point arithmetic order with the original code,
//  sometimes, there may have small differences between them.
// Here is actually few thing I can do to decrease the difference.
// If the array a and b are not restrict to float, 
// we can use more precise data type, like double (_m128d in SSE).
// This method can increase the precision.
float partA_vectorized1(float * restrict a, float * restrict b,
		     int size) {
  // replace the following code with vectorized code
  float sum = 0.0;
  __m128 a4, b4, mul4, sum4;

  // initialize 4 sum to 0
  sum4 = _mm_setzero_ps();

  // vectorize the part that is divisible by 4
  for ( int i = 0; i < size - size%4; i+=4 ) {
    // for the reason why using loadu/storeu, see the Explanation 0 above
    a4 = _mm_loadu_ps(&a[i]);      // load 4 a
    b4 = _mm_loadu_ps(&b[i]);      // load 4 b
    mul4 = _mm_mul_ps(a4, b4);     // a[i] * b[i]
    sum4 = _mm_add_ps(sum4, mul4); // sum = sum + a[i] * b[i]
  }

  // Sum vector
  // There are mainly two ways to do horizontal SSE vector sum.
  // one is firstly storing the vector into an array, then add them together.
  // Here is the sample code:
  //  float mysum[4];
  //  _mm_storeu_ps(&mysum[0], sum4);
  //  sum = mysum[0] + mysum[1] + mysum[2] + mysum[3];
  // The another one is using hadd two times which is below:
  sum4 = _mm_hadd_ps(sum4, sum4);
  sum4 = _mm_hadd_ps(sum4, sum4);
  float mysum[4];
  _mm_storeu_ps(&mysum[0], sum4);
  sum = mysum[0];
  // See more discussion/reference about this on Explanation 1 above
  // I think the two methods are interchangeable and their efficiency are similar.

  // handle the rest part
  for ( int i = size - size%4; i < size; i++ ) {
    sum = sum + a[i] * b[i];
  }
  return sum;
}

/******************* routine 2 ***********************/

// in the following, size can have any positive value
void partA_routine2(float * restrict a, float * restrict b, int size) {
  for ( int i = 0; i < size; i++ ) {
    a[i] = 1 - (1.0/(b[i]+1.0));
  }
}

// in the following, size can have any positive value
void partA_vectorized2(float * restrict a, float * restrict b, int size) {
  // replace the following code with vectorized code
  __m128 a4, b4, add4, div4;
  __m128 one = _mm_set1_ps(1.0f);

  // vectorize the part that is divisible by 4
  for ( int i = 0; i < size - size%4; i+=4 ) {
    // for the reason why using loadu/storeu, see the Explanation 0 above
    b4 = _mm_loadu_ps(&b[i]);     // load 4 b
    add4 = _mm_add_ps(b4, one);   // b[i]+1.0

    // There are two method to do this step
    // One is using _mm_div_ps and the other one is using _mm_rcp_ps.
    // div version is on below and rcp version is
    // div4 = _mm_rcp_ps(add4);
    // Although using _mm_rcp_ps is faster and can use a Newton Rhapson Iteration to increase the precision,
    // there still has a tiny difference with the original code and I think accuracy is more important.
    // So, I will use _mm_div_ps instead of _mm_rcp_ps.
    // See more details on explanation 3 above.
    div4 = _mm_div_ps(one, add4); // 1.0/(b[i]+1.0) and also could be div4 = _mm_rcp_ps(add4);
    a4 = _mm_sub_ps(one, div4);   // 1 - (1.0/(b[i]+1.0))
    _mm_storeu_ps(&a[i], a4);     // store the result back
  }

  // handle the rest part
  for ( int i = size - size%4; i < size; i++ ) {
    a[i] = 1 - (1.0/(b[i]+1.0));
  }
}

/******************** routine 3 ************************/

// in the following, size can have any positive value
void partA_routine3(float * restrict a, float * restrict b, int size) {
  for ( int i = 0; i < size; i++ ) {
    if ( a[i] < 0.0 ) {
      a[i] = b[i];
    }
  }
}

// in the following, size can have any positive value
void partA_vectorized3(float * restrict a, float * restrict b, int size) {
  // replace the following code with vectorized code
  __m128 a4, b4;
  // set zeros
  __m128 zeros = _mm_setzero_ps();

  // vectorize the part that is divisible by 4
  for ( int i = 0; i < size - size/4; i+=4 ) {
    // for the reason why using loadu/storeu, see the Explanation 0 above
    a4 = _mm_loadu_ps(&a[i]);                         // load 4 a
    b4 = _mm_loadu_ps(&b[i]);                         // load 4 b
    __m128 lt = _mm_cmplt_ps(a4, zeros);              // get mask(1 or 0) about condition: a[i] < 0.0
    __m128 true_part = _mm_and_ps(lt, b4);            // if a[i] < 0.0 (this position's mask is 1),  a[i] = b[i]
    __m128 false_part = _mm_andnot_ps(lt, a4);        // if a[i] >= 0.0 (this position's mask is 0),  a[i] = a[i]
    __m128 result = _mm_or_ps(true_part, false_part); // combine two parts to get the result
    _mm_storeu_ps(&a[i], result);                     // store the result back to a
  }

  // handle the rest part
  for ( int i = size - size/4; i < size; i++ ) {
    if ( a[i] < 0.0 ) {
      a[i] = b[i];
    }
  }
}

/********************* routine 4 ***********************/

// hint: one way to vectorize the following code might use
// vector shuffle operations
void partA_routine4(float * restrict a, float * restrict b,
		       float * restrict c) {
  for ( int i = 0; i < 2048; i = i+2  ) {
    a[i] = b[i]*c[i] - b[i+1]*c[i+1];
    a[i+1] = b[i]*c[i+1] + b[i+1]*c[i];
  }
}

void partA_vectorized4(float * restrict a, float * restrict b,
		       float * restrict  c) {
  // replace the following code with vectorized code
  float test[4];
  __m128 a4, b4, c4, bc, sh_b, sh_bc, sub_bc, add_sh_bc, mask, left, right, combine;

  for ( int i = 0; i < 2048; i = i+4  ) {
    // for the reason why using loadu/storeu, see the Explanation 0 above
    b4 = _mm_loadu_ps(&b[i]); // load 4 b
    c4 = _mm_loadu_ps(&c[i]); // load 4c
    bc = _mm_mul_ps(b4,c4);   // get 4 b*c

    // shuffle b, so sh_b is [ b[i+1], b[i], b[i+1], b[i+2] ]
    sh_b = _mm_shuffle_ps(b4, b4, _MM_SHUFFLE(2, 3, 0, 1));

    // got shuffle b multiply c, so sh_bc is [ b[i+1]c[i], b[i]c[i+1], b[i+1]c[i+2], b[i+2]c[i+3] ]
    sh_bc = _mm_mul_ps(sh_b,c4);

    // got sub_bc: [ (b[i]*c[i]-b[i+1]*c[i+1]), (b[i+2]*c[i+2]-b[i+3]*c[i+3]), (b[i]*c[i]-b[i+1]*c[i+1]), (b[i+2]*c[i+2]-b[i+3]*c[i+3]) ]
    sub_bc = _mm_hsub_ps(bc, bc);

    // got add_sh_bc: [ (b[i]*c[i+1]+b[i+1]*c[i]), (b[i+2]*c[i+3]+b[i+3]*c[i+2]), (b[i]*c[i+1]+b[i+1]*c[i]), (b[i+2]*c[i+3]+b[i+3]*c[i+2]) ]
    add_sh_bc = _mm_hadd_ps(sh_bc, sh_bc);

    // set mask [1,1,0,0] (the register's address is from right to left and -1 in binary is all 1, so it actually is [0,0,-1,-1] )
    mask = _mm_castsi128_ps(_mm_set_epi32(0, 0, -1, -1));

    // got left: [ (b[i]*c[i]-b[i+1]*c[i+1]), (b[i+2]*c[i+2]-b[i+3]*c[i+3]), 0, 0 ]
    left = _mm_and_ps(mask, sub_bc);

    // got right: [ 0, 0, (b[i]*c[i+1]+b[i+1]*c[i]), (b[i+2]*c[i+3]+b[i+3]*c[i+2]) ]
    right = _mm_andnot_ps(mask, add_sh_bc);

    // combine two parts: [ (b[i]*c[i]-b[i+1]*c[i+1]), (b[i+2]*c[i+2]-b[i+3]*c[i+3]), (b[i]*c[i+1]+b[i+1]*c[i]), (b[i+2]*c[i+3]+b[i+3]*c[i+2]) ]
    combine = _mm_or_ps(left, right);

    // shuffle vector to right position: [ (b[i]*c[i]-b[i+1]*c[i+1]), (b[i]*c[i+1]+b[i+1]*c[i]), (b[i+2]*c[i+2]-b[i+3]*c[i+3]), (b[i+2]*c[i+3]+b[i+3]*c[i+2]) ]
    a4 = _mm_shuffle_ps(combine, combine, _MM_SHUFFLE(3, 1, 2, 0));

    // store the result back to a
    _mm_storeu_ps(&a[i], a4);
  }
}

/********************* routine 5 ***********************/

// in the following, size can have any positive value
void partA_routine5(unsigned char * restrict a,
		    unsigned char * restrict b, int size) {
  for ( int i = 0; i < size; i++ ) {
    a[i] = b[i];
  }
}

void partA_vectorized5(unsigned char * restrict a,
		       unsigned char * restrict b, int size) {
  // replace the following code with vectorized code
  __m128i b16;

  // as one char type takes 8 bits, so we can take 128/8 = 16 chars in one operation
  // vectorize the part that is divisible by 16
  for ( int i = 0; i < size - size%16; i+=16 ) {
    b16 = _mm_loadu_si128((__m128i*)&b[i]); // load 16 b
    _mm_storeu_si128((__m128i*)&a[i], b16); // store 16 b to 16 a
  }

  // handle the rest part
  for ( int i = size - size%16; i < size; i++ ) {
    a[i] = b[i];
  }
}

/********************* routine 6 ***********************/

void partA_routine6(float * restrict a, float * restrict b,
		       float * restrict c) {
  a[0] = 0.0;
  for ( int i = 1; i < 1023; i++ ) {
    float sum = 0.0;
    for ( int j = 0; j < 3; j++ ) {
      sum = sum +  b[i+j-1] * c[j];
    }
    a[i] = sum;
  }
  a[1023] = 0.0;
}

void partA_vectorized6(float * restrict a, float * restrict b,
		       float * restrict c) {
  // replace the following code with vectorized code
  a[0] = 0.0;
  __m128 b4_1, b4_2, b4_3, b4_4, sum3_1, sum3_2, sum3_3, sum3_4, bc3_1, bc3_2, bc3_3, bc3_4, sum3_12, sum3_34, a4;

  // load 3 elements in c and set the last element to 0
  __m128 c3 = _mm_set_ps(0, c[2], c[1], c[0]);

  // vectorize the part that is divisible by 4
  for ( int i = 1; i < 1021; i+=4 ) {
    // for the reason why using loadu/storeu, see the Explanation 0 above
    // load four groups b4
    b4_1 = _mm_loadu_ps(&b[i-1]); // load (b[i-1], b[i], b[i+1], b[i+2])
    b4_2 = _mm_loadu_ps(&b[i]);   // load (b[i], b[i+1], b[i+2], b[i+3])
    b4_3 = _mm_loadu_ps(&b[i+1]); // load (b[i+1], b[i+2], b[i+3], b[i+4])
    b4_4 = _mm_loadu_ps(&b[i+2]); // load (b[i+2], b[i+3], b[i+4], b[i+5])

    // for the inner loop:
    // for ( int j = 0; j < 3; j++ ) {
    //   sum = sum +  b[i+j-1] * c[j];
    // }
    // we can find that actually it is the sum of
    // b[i-1]*c[0] + b[i]*c[1] + b[i+1]*c[2];
    // although I got four b each time, I set the 4th element in c to 0,
    // so b4*c3 actually is ( b[i-1]c[0], b[i]c[1], b[i+1]c[2], 0 )
    // in this way, we can remove the inner loop

    bc3_1 = _mm_mul_ps(b4_1, c3); // got ( b[i-1]c[0], b[i]c[1], b[i+1]c[2], 0 )
    bc3_2 = _mm_mul_ps(b4_2, c3); // got ( b[i]c[0], b[i+1]c[1], b[i+2]c[2], 0 )
    bc3_3 = _mm_mul_ps(b4_3, c3); // got ( b[i+1]c[0], b[i+2]c[1], b[i+3]c[2], 0 )
    bc3_4 = _mm_mul_ps(b4_4, c3); // got ( b[i+2]c[0], b[i+3]c[1], b[i+4]c[2], 0 )

    // got the sum of vector bc3_1
    sum3_1 = _mm_hadd_ps(bc3_1, bc3_1);
    sum3_1 = _mm_hadd_ps(sum3_1, sum3_1);

    // got the sum of vector bc3_2
    sum3_2 = _mm_hadd_ps(bc3_2, bc3_2);
    sum3_2 = _mm_hadd_ps(sum3_2, sum3_2);

    // combine sum3_1 and sum3_2 by using shuffle, so it is:
    // [ (b[i-1]c[0]+b[i]c[1]+b[i+1]c[2]), (b[i-1]c[0]+b[i]c[1]+b[i+1]c[2]), (b[i]c[0]+b[i+1]c[1]+b[i+2]c[2]), (b[i]c[0]+b[i+1]c[1]+b[i+2]c[2]) ]
    sum3_12 = _mm_shuffle_ps(sum3_1, sum3_2, _MM_SHUFFLE(1, 0, 1, 0));

    // got the sum of vector bc3_3
    sum3_3 = _mm_hadd_ps(bc3_3, bc3_3);
    sum3_3 = _mm_hadd_ps(sum3_3, sum3_3);

    // got the sum of vector bc3_4
    sum3_4 = _mm_hadd_ps(bc3_4, bc3_4);
    sum3_4 = _mm_hadd_ps(sum3_4, sum3_4);

    // combine sum3_3 and sum3_4 by using shuffle, so it is:
    // [ (b[i+1]c[0]+b[i+2]c[1]+b[i+3]c[2]), (b[i+1]c[0]+b[i+2]c[1]+b[i+3]c[2])), (b[i+2]c[0]+b[i+3]c[1]+b[i+4]c[2]), (b[i+2]c[0]+b[i+3]c[1]+b[i+4]c[2]) ]
    sum3_34 = _mm_shuffle_ps(sum3_3, sum3_4, _MM_SHUFFLE(1, 0, 1, 0));

    // combine 4 sum to a4 by using shuffle, so we can store 4 a value in one iteration
    a4 = _mm_shuffle_ps(sum3_12, sum3_34, _MM_SHUFFLE(3, 1, 3, 1));

    // store the result back to a
    _mm_storeu_ps(&a[i], a4);
  }

  // handle the rest part
  for ( int i = 1021; i < 1023; i++ ) {
    float sum = 0.0;
    for ( int j = 0; j < 3; j++ ) {
      sum = sum +  b[i+j-1] * c[j];
    }
    a[i] = sum;
  }

  a[1023] = 0.0;
}



