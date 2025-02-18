/* Test and timing harness program for developing a multichannel
   multikernel convolution (as used in deep learning networks)

   Note there are some simplifications around this implementation,
   in particular with respect to computing the convolution at edge
   pixels of the image.

   Author: David Gregg
   Date:   March 2019

   Version 1.7 : Modified the code so that the kernel is sparse

   Version 1.6 : Modified the code so that the input tensor is float

   Version 1.5 : Modified the code so that the input and kernel
                 are tensors of 16-bit integer values

   Version 1.4 : Modified the random generator to reduce the range
                 of generated values;

   Version 1.3 : Fixed which loop variables were being incremented
                 in write_out();
                 Fixed dimensions of output and control_output 
                 matrices in main function

   Version 1.2 : Changed distribution of test data to (hopefully) 
                 eliminate random walk of floating point error;
                 Also introduced checks to restrict kernel-order to
                 a small set of values

   Version 1.1 : Fixed bug in code to create 4d matrix
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <omp.h>
#include <math.h>
#include <stdint.h>
#include <xmmintrin.h>

/* the following two definitions of DEBUGGING control whether or not
   debugging information is written out. To put the program into
   debugging mode, uncomment the following line: */
//#define DEBUGGING(_x) _x 
/* to stop the printing of debugging information, use the following line: */
#define DEBUGGING(_x)


// data structure to represent 2D arrays, where many values are zero
// and only non-zero values are represented
struct sparse_matrix {
  int nkernels;
  int nchannels;
  int non_zeros;
  int * kernel_starts;
  float * values;
  int * channel_numbers;
};

// return a new sparse matrix with the provided dimensions 
struct sparse_matrix * sparse_matrix_new(int nkernels, int nchannels, int nvalues)
{
  struct sparse_matrix * result;

  DEBUGGING(fprintf(stderr, "Entering sparse matrix new %d %d %d\n", nkernels, nchannels, nvalues));
  
  result = malloc(sizeof(struct sparse_matrix));
  DEBUGGING(fprintf(stderr, "  %p\n", result));

  result->nkernels = nkernels;
  result->nchannels = nchannels;
  result->non_zeros = nvalues;
  
  result->kernel_starts = malloc(sizeof(int)*(nkernels+1));
  DEBUGGING(fprintf(stderr, "  %p\n", result->kernel_starts));
  result->values = malloc(sizeof(float) * nvalues);
  DEBUGGING(fprintf(stderr, "  %p\n", result->values));
  result->channel_numbers = malloc(sizeof(float) * nvalues);
  DEBUGGING(fprintf(stderr, "  %p\n", result->channel_numbers));

  DEBUGGING(fprintf(stderr, "Exiting sparse matrix new %d %d %d\n", nkernels, nchannels, nvalues));
  
  return result;
}

// create a copy of a dense matrix in a new sparse matrix
struct sparse_matrix * sparse_matrix_dense2sparse(float ** matrix, int nkernels, int nchannels)
{
  int i, j;
  int non_zeros = 0;
  struct sparse_matrix * result;
  int nvalues;
  
  // find the number of non-zero values in the dense matrix
  for ( i = 0; i < nkernels; i++ ) {
    for ( j = 0; j < nchannels; j++ ) {
      if ( abs(matrix[i][j]) != 0.0 ) {
	non_zeros++;
      }
    }
  }

  // create the new unpopulated sparse matrix
  result = sparse_matrix_new(nkernels, nchannels, non_zeros);
  
  // now copy the values from the dense matrix to the sparse matrix
  nvalues = 0;
  for ( i = 0; i < nkernels; i++ ) {
    result->kernel_starts[i] = nvalues;
    for ( j = 0; j < nchannels; j++ ) {
      if ( abs(matrix[i][j]) != 0.0 ) {
	// record non-zero value and its channel number
	result->values[nvalues] = matrix[i][j];
	result->channel_numbers[nvalues] = j;
	nvalues++;
	assert( nvalues <= non_zeros );
      }
    }
  }
  // finally, record the end of the last kernel
  result->kernel_starts[nkernels] = nvalues;
  return result;
}

struct sparse_matrix *** kernels_dense2sparse(float **** kernels, int kernel_order, int nkernels, int nchannels)
{

  int i, j;
  struct sparse_matrix *** result;
  struct sparse_matrix ** temp;

  result = malloc(sizeof(struct sparse_matrix**) * kernel_order);
  temp = malloc(sizeof(struct sparse_matrix*) * kernel_order * kernel_order);
  
  for ( i = 0; i < kernel_order; i++ ) {
    result[i] = &(temp[i * kernel_order]);
    for ( j = 0; j < kernel_order; j++ ) {
      result[i][j] = sparse_matrix_dense2sparse(kernels[i][j], nkernels, nchannels);
    }
  }
  DEBUGGING(fprintf(stderr, "exiting dense2sparse\n"));
  return result;
}


/* write 3d matrix to stdout */
void write_out(float *** a, int dim0, int dim1, int dim2)
{
  int i, j, k;

  for ( i = 0; i < dim0; i++ ) {
    printf("Outer dimension number %d\n", i);
    for ( j = 0; j < dim1; j++ ) {
      for ( k = 0; k < dim2 - 1; k++ ) {
        printf("%f, ", a[i][j][k]);
      }
      // print end of line
      printf("%f\n", a[i][j][dim2-1]);
    }
  }
}


/* create new empty 4d float matrix */
float **** new_empty_4d_matrix(int dim0, int dim1, int dim2, int dim3)
{
  float **** result;
  float *** mat1;
  float ** mat2;
  float * mat3;
  int i, j, k;

  assert ( (dim0 > 0)  && (dim1 > 0) && (dim2 > 0) && (dim3 > 0) );
  
  // allocate memory for the 4D data structure
  result = malloc(dim0 * sizeof(float***));
  mat1 = malloc(dim0 * dim1 * sizeof(float**));
  mat2 = malloc(dim0 * dim1 * dim2 * sizeof(float*));
  mat3 = malloc(dim0 * dim1 * dim2 *dim3 * sizeof(float));

  // now check the memory allocations were successful
  assert ( result != NULL );
  assert ( mat1 != NULL );
  assert ( mat2 != NULL );
  assert ( mat3 != NULL );
  
  for ( i = 0; i < dim0; i++ ) {
    result[i] = &(mat1[i*dim1]);
    for ( j = 0; j < dim1; j++ ) {
      result[i][j] = &(mat2[i*dim1*dim2 + j*dim2]);
      for ( k = 0; k < dim2; k++ ) {
        result[i][j][k] = &(mat3[i*dim1*dim2*dim3+j*dim2*dim3+k*dim3]);
      }
    }
  }
  
  return result;
}

/* create new empty 3d matrix */
float *** new_empty_3d_matrix(int dim0, int dim1, int dim2)
{
  float **** mat4d;
  float *** mat3d;

  // create a 4d matrix with a length 1 first dimension
  mat4d = new_empty_4d_matrix(1, dim0, dim1, dim2);

  // now throw away out first dimension so that we have a 3D matrix
  mat3d = mat4d[0];
  
  free(mat4d);

  return mat3d;
}

/* take a copy of the matrix and return in a newly allocated matrix */
float **** copy_4d_matrix(float **** source_matrix, int dim0,
                            int dim1, int dim2, int dim3)
{
  int i, j, k, l;
  float **** result = new_empty_4d_matrix(dim0, dim1, dim2, dim3);

  for ( i = 0; i < dim0; i++ ) {
    for ( j = 0; j < dim1; j++ ) {
      for ( k = 0; k < dim2; k++ ) {
        for ( l = 0; l < dim3; l++ ) {
          result[i][j][k][l] = source_matrix[i][j][k][l];
        }
      }
    }
  }
  return result;
}

/* create a matrix and fill it with random numbers */
float **** gen_random_4d_matrix(int dim0, int dim1, int dim2, int dim3, int nz_ratio)
{
  float **** result;
  int i, j, k, l;
  struct timeval seedtime;
  int seed;

  assert( nz_ratio >= 1 );
  
  result = new_empty_4d_matrix(dim0, dim1, dim2, dim3);

  /* use the microsecond part of the current time as a pseudorandom seed */
  gettimeofday(&seedtime, NULL);
  seed = seedtime.tv_usec;
  srandom(seed);

  /* fill the matrix with random numbers */
  const int range = 1 << 10; // 2^10
  //const int bias = 1 << 16; // 2^16
  float offset = 0.0;
  for ( i = 0; i < dim0; i++ ) {
    for ( j = 0; j < dim1; j++ ) {
      for ( k = 0; k < dim2; k++ ) {
        for ( l = 0; l < dim3; l++ ) {
	  // generated a random number to decide if the value should be zero
	  long long rand = random();
	  // nz ratio is the reciprocal of the proportion of values that
	  // are non-zero; a nz ratio of 1 means all values are non-zero.
	  // a nz ratio of 3 means that one in three values is non-zero
	  if ( (rand % nz_ratio) == 0 ) {
	    // now use the random number to set a useful non-zero value
	    // cut down the range and bias the mean to reduce
	    // the likelihood of large floating point round-off errors
	    int reduced_range = (rand % range);
	    // but make sure that cutting down the range does not give us
	    // a zero value; this loop might never terminate, but probably will
	    while ( reduced_range == 0 ) {
	      reduced_range = random() % range;
	    }
	    result[i][j][k][l] = reduced_range;
	  }
	  else {
	    // the nz ratio tells us that this value must be zero
	    result[i][j][k][l] = 0;
	  }
        }
      }
    }
  }

  return result;
}

/* create a matrix and fill it with random numbers */
float *** gen_random_3d_matrix(int dim0, int dim1, int dim2, int nz_ratio)
{
  float **** mat4d;
  float *** mat3d;

  // create a 4d matrix with single first dimension
  mat4d = gen_random_4d_matrix(1, dim0, dim1, dim2, nz_ratio);
  // now throw away out first dimension
  mat3d = mat4d[0];
  free(mat4d);
  return mat3d;
}



/* check the sum of absolute differences is within reasonable epsilon */
void check_result(float *** result, float *** control,
                  int dim0, int dim1, int dim2)
{
  int i, j, k;
  double sum_abs_diff = 0.0;
  const double EPSILON = 0.0625;

  DEBUGGING(printf("SAD\n"));
  
  for ( i = 0; i < dim0; i++ ) {
    for ( j = 0; j < dim1; j++ ) {
      for ( k = 0; k < dim2; k++ ) {
        double diff = fabs(control[i][j][k] - result[i][j][k]);
        assert( diff >= 0.0 );
        sum_abs_diff = sum_abs_diff + diff;
      }
    }
  }

  if ( sum_abs_diff > EPSILON ) {
    fprintf(stderr, "WARNING: sum of absolute differences (%f) > EPSILON (%f)\n",
            sum_abs_diff, EPSILON);
  }
  else {
    printf("COMMENT: sum of absolute differences (%f)  within acceptable range (%f)\n", sum_abs_diff, EPSILON);
  }
}

/* a slow but correct version of dense convolution written by David */
void multichannel_conv_dense(float *** image, float **** kernels,
		       float *** output, int width, int height,
		       int nchannels, int nkernels, int kernel_order)
{
  int h, w, x, y, c, m;

 // initialize the output matrix to zero
  for ( m = 0; m < nkernels; m++ ) {
    for ( h = 0; h < height; h++ ) {
      for ( w = 0; w < width; w++ ) {
	output[m][h][w] = 0.0;
      }
    }
  }

  for ( m = 0; m < nkernels; m++ ) {
    for ( w = 0; w < width; w++ ) {
      for ( h = 0; h < height; h++ ) {
	for ( x = 0; x < kernel_order; x++) {
	  for ( y = 0; y < kernel_order; y++ ) {
	    for ( c = 0; c < nchannels; c++ ) {
               output[m][h][w] += image[w+x][h+y][c] * kernels[x][y][m][c];
            }
          }
        }
      }
    }
  }
}


/* a slow but correct version of sparse convolution written by David */
void multichannel_conv_sparse(float *** image, struct sparse_matrix *** kernels,
		       float *** output, int width, int height,
		       int nchannels, int nkernels, int kernel_order) {
  int h, w, x, y, c, m, index;
  float value;

  // initialize the output matrix to zero
  for ( m = 0; m < nkernels; m++ ) {
    for ( h = 0; h < height; h++ ) {
      for ( w = 0; w < width; w++ ) {
	output[m][h][w] = 0.0;
      }
    }
  }

  DEBUGGING(fprintf(stderr, "w=%d, h=%d, c=%d\n", w, h, c));

  // now compute multichannel, multikernel convolution
  for ( w = 0; w < width; w++ ) {
    for ( h = 0; h < height; h++ ) { 
      double sum = 0.0;
      for ( x = 0; x < kernel_order; x++) {
	      for ( y = 0; y < kernel_order; y++ ) {
	        struct sparse_matrix * kernel = kernels[x][y];
	        for ( m = 0; m < nkernels; m++ ) {
	          for ( index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m+1]; index++ ) {
	            int this_c = kernel->channel_numbers[index];
	            assert( (this_c >= 0) && (this_c < nchannels) );
	            value = kernel->values[index];
	            output[m][h][w] += image[w+x][h+y][this_c] * value;
	    }
	  } // m
	} // y
      } // x
    } // h
  }// w
}

/*
This additional method is used to calculate the remaining part of the image. Since SSE calculate four (pixel) at one time, there will be possibility that the width and height are not the multiple of 4. So there will be extra bit to calculate. This function is to handle the extra bit.
*/

void remaining_calculation(float *** image, struct sparse_matrix *** kernels,
		       float *** output, int width, int height,
		       int nchannels, int nkernels, int kernel_order, int hi, int wi) {
  /*the additional parameter about is hi and wi, which represent the remainder height first index and the remainder width first index
*/
  int h, w, x, y, c, m, index, tmpwidth;
  float value;
  
  DEBUGGING(fprintf(stderr, "w=%d, h=%d, c=%d\n", w, h, c));

  /*  _______________  In the rough graph on the left, there is extra part for both height and 
     |             | | width. And both parts have an overlap are on the bottom right corner. 
     |             | | To avoid calculating that part twice, our program will calculate the bottom
     |             | | part first without the overlapped square, which is the first if. And then 
     |             | | calcualte the right part, which is the second if. 
     |_____________|_| If there is no extra part in the image, the parameter will be 0. 
     |_____________|_|
  */

  if(wi!=width) tmpwidth = wi; //if there is extra part in the right, avoid the overlapped part.
  else tmpwidth=width; //if no extra part in the right, calculate the bottom part only.

  if(hi != 0){
    for ( w = 0; w < tmpwidth; w++ ) {
      for ( h = hi; h < height; h++ ) { 
        for ( m = 0; m < nkernels; m++ ) {
        double sum = 0.0;
          for ( x = 0; x < kernel_order; x++) {
	    for ( y = 0; y < kernel_order; y++ ) {
	    struct sparse_matrix * kernel = kernels[x][y];
	      for ( index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m+1]; index++ ) {
	        int this_c = kernel->channel_numbers[index];
	        assert( (this_c >= 0) && (this_c < nchannels) );
	        value = kernel->values[index];
	        output[m][h][w] += image[w+x][h+y][this_c] * value;
	      }
	    } // m
	  } // y
        } // x
      } // h
    }// w
  }

  //the if statement below is used to calculate the bottom part

  if(wi != 0){
    for ( w = wi; w < width; w++ ) {
      for ( h = 0; h < height; h++ ) {
	for ( m = 0; m < nkernels; m++ ) { 
          double sum = 0.0;
          for ( x = 0; x < kernel_order; x++) {
	    for ( y = 0; y < kernel_order; y++ ) {
	      struct sparse_matrix * kernel = kernels[x][y];
	      for ( index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m+1]; index++ ) {
	        int this_c = kernel->channel_numbers[index];
	        assert( (this_c >= 0) && (this_c < nchannels) );
	        value = kernel->values[index];
	        output[m][h][w] += image[w+x][h+y][this_c] * value;
	      }
	    } // m
	  } // y
        } // x
      } // h
    }// w
  }
}


/* the fast version of sparse convolution written by the team */
void team_conv_sparse(float *** image, struct sparse_matrix *** kernels,
		       float *** output, int width, int height,
		       int nchannels, int nkernels, int kernel_order, int nz) {
  int h, w, x, y, c, m;
  //Flag variable to determine if the calculation needs the use OpenMP
  int useOpenMP = 0;
  /*
    Calculate the approximate iteration amount.
    The data here are all based on the experiments.
    The iteration amount is separated into three intervals.
    The first interval is if amount is less than 232*232*32*32*3*3*nz_ratio(no matter how small nz_ratio is),
    only using sse is faster.
    The third interval is if amount is greater than 340*340*32*32*3*3*nz_ratio(no matter how large nz_ratio is),
    using OpenMP + sse is faster.
    The second interval is if the amount is between 232*232*32*32*3*3*nz_ratio and 340*340*32*32*3*3*nz_ratio, the operation needs to
    depend on the relation between (width, height, kernels ...) and nz_ratio.
    Since the influence of nz_ratio is not so much big as its value, the nz_ratio is translated into ln(nz_ratio)
    In the second interval, here's a avarage value which is 288*288*32*32*3*3 / ln(500.0), so if the calculated value is 
    larger than it, then it should use sse + OpenMP, if it less than it, it should only use sse.
  */
  long long useNumber = width*height*nchannels*nkernels*kernel_order*kernel_order;
  //The upper bound
  long long useNumberUpper = 340*340*32*32*3*3;
  //The lower bound
  long long useNumberLower = 232*232*32*32*3*3;
  // If it larger than the upper bound than just use it.
  if(useNumber >= useNumberUpper){
    useOpenMP = 1;
  }
  //If it larger than the lower bound, then determine if OpenMP should be used
  else if(useNumber >= useNumberLower){
    long long useNumberMid = (long long)(288*288*32*32*3*3 / log(500.0));
    useNumber = useNumber / (int)(log((double)nz));
    //If the calculated value larger than the average value then use OpenMP
    if(useNumber > useNumberMid){
      useOpenMP = 1;
    }
  }

  // initialize the output matrix to zero

    #pragma omp parallel for if(useOpenMP == 1)
    for ( m = 0; m < nkernels; m++ ) {
      for ( h = 0; h < height; h++ ) {
        for ( w = 0; w < width; w++ ) {
	  output[m][h][w] = 0.0;
        }
      }
    }
  DEBUGGING(fprintf(stderr, "w=%d, h=%d, c=%d\n", w, h, c));

  //check if the height and width are multiple of 4, if not, calcualte the extra part.

  int checkHeight = height % 4;
  int checkWidth = width % 2;
  remaining_calculation(image,kernels,output,width,height,nchannels,nkernels,kernel_order,height-checkHeight,width-checkWidth);

  // now compute multichannel, multikernel convolution 

  //The output is using variable w, h, m, put them in the outer loop instead of put any of them in the inner loop 
  //and do it again and again, will make the code more efficient
    #pragma omp parallel for private(w,h,m,x,y) if(useOpenMP == 1)
    for ( w = 0; w < width-checkWidth; w+=2 ) {  //calculate two column at one time to improve the calculating speed
      for ( h = 0; h < height-checkHeight; h+=4) { //calculate four rows together at one time using sse to improve the speed
        for ( m = 0; m < nkernels; m++ ) {
        __m128 sum1 = _mm_setzero_ps(); //each sum for each w (column/width)
        __m128 sum2 = _mm_setzero_ps();
        for ( x = 0; x < kernel_order; x++) {
	  for ( y = 0; y < kernel_order; y++ ) {     
            struct sparse_matrix * kernel = kernels[x][y];
	    for (int index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m+1]; index++ ) {
	      int this_c = kernel->channel_numbers[index];
	      assert( (this_c >= 0) && (this_c < nchannels) );
              __m128 value = _mm_set1_ps(kernel->values[index]);  //load four copies of value to __m128 value to calcualte
	      //load four values of image (with the increasing of h) to calculate into __m128 to calculate together
              __m128 tmp1 = _mm_setr_ps(image[w+x][h+y][this_c],image[w+x][h+y+1][this_c],image[w+x][h+y+2][this_c],image[w+x][h+y+3][this_c]); 
              tmp1 = _mm_mul_ps(tmp1,value); //image[w+x][h+y][this_c]*value but four multiplication at the same time

              __m128 tmp2 = _mm_setr_ps(image[w+x+1][h+y][this_c],image[w+x+1][h+y+1][this_c],image[w+x+1][h+y+2][this_c],image[w+x+1][h+y+3][this_c]);
              tmp2 = _mm_mul_ps(tmp2,value);
                 
              sum1 = _mm_add_ps(sum1,tmp1);//sum1 = sum1 + image[w+x][h+y][this_c] but four addition at the same time
              sum2 = _mm_add_ps(sum2,tmp2);
            }      
          } // y
	} // x
        float sum[4];
        _mm_storeu_ps(sum,sum1);  //store the sum1 to array sum
        for(int i=0;i<4;i++){  // load the sum to the output
          output[m][h+i][w] = sum[i];
        }
        _mm_storeu_ps(sum,sum2);
        for(int i=0;i<4;i++){
          output[m][h+i][w+1] = sum[i];
        }
      } // m
    } // h
  }// w
  
}

int main(int argc, char ** argv) {
  //float image[W][H][C];
  //float kernels[M][C][K][K];
  //float output[M][W][H];
  
  float *** image;
  float **** kernels;
  struct sparse_matrix *** sparse_kernels = NULL;
  float *** control_output, *** output;
  long long mul_time;
  int width, height, kernel_order, nchannels, nkernels;
  struct timeval start_time;
  struct timeval stop_time;
  int nz_ratio = 1; // by default we just have a dense matrix

  if ( argc != 7 ) {
    fprintf(stderr, "Usage: conv-harness <image_width> <image_height> <kernel_order> <number of channels> <number of kernels> <non-zero ratio>\n");
    exit(1);
  }
  else {
    width = atoi(argv[1]);
    height = atoi(argv[2]);
    kernel_order = atoi(argv[3]);
    nchannels = atoi(argv[4]);
    nkernels = atoi(argv[5]);
    nz_ratio = atoi(argv[6]);
  }
  switch ( kernel_order ) {
  case 1:
  case 3:
  case 5:
  case 7: break;
  default:
    fprintf(stderr, "FATAL: kernel_order must be 1, 3, 5 or 7, not %d\n",
            kernel_order);
    exit(1);
  }

  assert( width >= 1 );
  assert( height >= 1 );
  assert( nchannels >= 1 );
  assert( nkernels >= 1 );
  assert( nz_ratio >= 1 );

  /* allocate the matrices */
  image = gen_random_3d_matrix(width+kernel_order, height + kernel_order,
                               nchannels, 1); // nz_ratio == 1, ie no sparsity
  kernels = gen_random_4d_matrix(kernel_order, kernel_order, nkernels, nchannels, nz_ratio);
  if ( nz_ratio > 1 ) { // we have sparsity
    sparse_kernels = kernels_dense2sparse(kernels, kernel_order, nkernels, nchannels);
  }

  output = new_empty_3d_matrix(nkernels, width, height);

  control_output = new_empty_3d_matrix(nkernels, width, height);

  /* use a simple multichannel convolution routine to produce control result */
  multichannel_conv_dense(image, kernels, control_output, width,
                    height, nchannels, nkernels, kernel_order);

  /* record starting time of team's code*/
  gettimeofday(&start_time, NULL);
  
  if ( nz_ratio > 1 ) { // we're working on a sparse matrix
    /* perform student team's sparse multichannel convolution */
    team_conv_sparse(image, sparse_kernels, output, width,
                    height, nchannels, nkernels, kernel_order,nz_ratio);
  }
  else { // we're working on a dense matrix
    multichannel_conv_dense(image, kernels, output, width,
                    height, nchannels, nkernels, kernel_order);
  }
  /* record finishing time */
  gettimeofday(&stop_time, NULL);

  mul_time = (stop_time.tv_sec - start_time.tv_sec) * 1000000L +
    (stop_time.tv_usec - start_time.tv_usec);
  printf("Team conv time: %lld microseconds\n", mul_time);

  DEBUGGING(write_out(output, nkernels, width, height));

  /* now check that the team's multichannel convolution routine
     gives the same answer as the known working version */
  check_result(output, control_output, nkernels, width, height);

  return 0;
}
