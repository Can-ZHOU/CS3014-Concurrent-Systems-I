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
#include <x86intrin.h>

/* the following two definitions of DEBUGGING control whether or not
   debugging information is written out. To put the program into
   debugging mode, uncomment the following line: */
//#define DEBUGGING(_x) _x
/* to stop the printing of debugging information, use the following line: */
#define DEBUGGING(_x)

// data structure to represent 2D arrays, where many values are zero
// and only non-zero values are represented
struct sparse_matrix
{
  int nkernels;
  int nchannels;
  int non_zeros;
  int *kernel_starts;
  float *values;
  int *channel_numbers;
};

// return a new sparse matrix with the provided dimensions
struct sparse_matrix *sparse_matrix_new(int nkernels, int nchannels, int nvalues)
{
  struct sparse_matrix *result;

  DEBUGGING(fprintf(stderr, "Entering sparse matrix new %d %d %d\n", nkernels, nchannels, nvalues));

  result = malloc(sizeof(struct sparse_matrix));
  DEBUGGING(fprintf(stderr, "  %p\n", result));

  result->nkernels = nkernels;
  result->nchannels = nchannels;
  result->non_zeros = nvalues;

  result->kernel_starts = malloc(sizeof(int) * (nkernels + 1));
  DEBUGGING(fprintf(stderr, "  %p\n", result->kernel_starts));
  result->values = malloc(sizeof(float) * nvalues);
  DEBUGGING(fprintf(stderr, "  %p\n", result->values));
  result->channel_numbers = malloc(sizeof(float) * nvalues);
  DEBUGGING(fprintf(stderr, "  %p\n", result->channel_numbers));

  DEBUGGING(fprintf(stderr, "Exiting sparse matrix new %d %d %d\n", nkernels, nchannels, nvalues));

  return result;
}

// create a copy of a dense matrix in a new sparse matrix
struct sparse_matrix *sparse_matrix_dense2sparse(float **matrix, int nkernels, int nchannels)
{
  int i, j;
  int non_zeros = 0;
  struct sparse_matrix *result;
  int nvalues;

  // find the number of non-zero values in the dense matrix
  for (i = 0; i < nkernels; i++)
  {
    for (j = 0; j < nchannels; j++)
    {
      if (abs(matrix[i][j]) != 0.0)
      {
        non_zeros++;
      }
    }
  }

  // create the new unpopulated sparse matrix
  result = sparse_matrix_new(nkernels, nchannels, non_zeros);

  // now copy the values from the dense matrix to the sparse matrix
  nvalues = 0;
  for (i = 0; i < nkernels; i++)
  {
    result->kernel_starts[i] = nvalues;
    for (j = 0; j < nchannels; j++)
    {
      if (abs(matrix[i][j]) != 0.0)
      {
        // record non-zero value and its channel number
        result->values[nvalues] = matrix[i][j];
        result->channel_numbers[nvalues] = j;
        nvalues++;
        assert(nvalues <= non_zeros);
      }
    }
  }
  // finally, record the end of the last kernel
  result->kernel_starts[nkernels] = nvalues;
  return result;
}

struct sparse_matrix ***kernels_dense2sparse(float ****kernels, int kernel_order, int nkernels, int nchannels)
{

  int i, j;
  struct sparse_matrix ***result;
  struct sparse_matrix **temp;

  result = malloc(sizeof(struct sparse_matrix **) * kernel_order);
  temp = malloc(sizeof(struct sparse_matrix *) * kernel_order * kernel_order);

  for (i = 0; i < kernel_order; i++)
  {
    result[i] = &(temp[i * kernel_order]);
    for (j = 0; j < kernel_order; j++)
    {
      result[i][j] = sparse_matrix_dense2sparse(kernels[i][j], nkernels, nchannels);
    }
  }
  DEBUGGING(fprintf(stderr, "exiting dense2sparse\n"));
  return result;
}

/* write 3d matrix to stdout */
void write_out(float ***a, int dim0, int dim1, int dim2)
{
  int i, j, k;

  for (i = 0; i < dim0; i++)
  {
    printf("Outer dimension number %d\n", i);
    for (j = 0; j < dim1; j++)
    {
      for (k = 0; k < dim2 - 1; k++)
      {
        printf("%f, ", a[i][j][k]);
      }
      // print end of line
      printf("%f\n", a[i][j][dim2 - 1]);
    }
  }
}

/* create new empty 4d float matrix */
float ****new_empty_4d_matrix(int dim0, int dim1, int dim2, int dim3)
{
  float ****result;
  float ***mat1;
  float **mat2;
  float *mat3;
  int i, j, k;

  assert((dim0 > 0) && (dim1 > 0) && (dim2 > 0) && (dim3 > 0));

  // allocate memory for the 4D data structure
  result = malloc(dim0 * sizeof(float ***));
  mat1 = malloc(dim0 * dim1 * sizeof(float **));
  mat2 = malloc(dim0 * dim1 * dim2 * sizeof(float *));
  mat3 = malloc(dim0 * dim1 * dim2 * dim3 * sizeof(float));

  // now check the memory allocations were successful
  assert(result != NULL);
  assert(mat1 != NULL);
  assert(mat2 != NULL);
  assert(mat3 != NULL);

  for (i = 0; i < dim0; i++)
  {
    result[i] = &(mat1[i * dim1]);
    for (j = 0; j < dim1; j++)
    {
      result[i][j] = &(mat2[i * dim1 * dim2 + j * dim2]);
      for (k = 0; k < dim2; k++)
      {
        result[i][j][k] = &(mat3[i * dim1 * dim2 * dim3 + j * dim2 * dim3 + k * dim3]);
      }
    }
  }

  return result;
}

/* create new empty 3d matrix */
float ***new_empty_3d_matrix(int dim0, int dim1, int dim2)
{
  float ****mat4d;
  float ***mat3d;

  // create a 4d matrix with a length 1 first dimension
  mat4d = new_empty_4d_matrix(1, dim0, dim1, dim2);

  // now throw away out first dimension so that we have a 3D matrix
  mat3d = mat4d[0];

  free(mat4d);

  return mat3d;
}

/* take a copy of the matrix and return in a newly allocated matrix */
float ****copy_4d_matrix(float ****source_matrix, int dim0,
                         int dim1, int dim2, int dim3)
{
  int i, j, k, l;
  float ****result = new_empty_4d_matrix(dim0, dim1, dim2, dim3);

  for (i = 0; i < dim0; i++)
  {
    for (j = 0; j < dim1; j++)
    {
      for (k = 0; k < dim2; k++)
      {
        for (l = 0; l < dim3; l++)
        {
          result[i][j][k][l] = source_matrix[i][j][k][l];
        }
      }
    }
  }
  return result;
}

/* create a matrix and fill it with random numbers */
float ****gen_random_4d_matrix(int dim0, int dim1, int dim2, int dim3, int nz_ratio)
{
  float ****result;
  int i, j, k, l;
  struct timeval seedtime;
  int seed;

  assert(nz_ratio >= 1);

  result = new_empty_4d_matrix(dim0, dim1, dim2, dim3);

  /* use the microsecond part of the current time as a pseudorandom seed */
  gettimeofday(&seedtime, NULL);
  seed = seedtime.tv_usec;
  srandom(seed);

  /* fill the matrix with random numbers */
  const int range = 1 << 10; // 2^10
  //const int bias = 1 << 16; // 2^16
  float offset = 0.0;
  for (i = 0; i < dim0; i++)
  {
    for (j = 0; j < dim1; j++)
    {
      for (k = 0; k < dim2; k++)
      {
        for (l = 0; l < dim3; l++)
        {
          // generated a random number to decide if the value should be zero
          long long rand = random();
          // nz ratio is the reciprocal of the proportion of values that
          // are non-zero; a nz ratio of 1 means all values are non-zero.
          // a nz ratio of 3 means that one in three values is non-zero
          if ((rand % nz_ratio) == 0)
          {
            // now use the random number to set a useful non-zero value
            // cut down the range and bias the mean to reduce
            // the likelihood of large floating point round-off errors
            int reduced_range = (rand % range);
            // but make sure that cutting down the range does not give us
            // a zero value; this loop might never terminate, but probably will
            while (reduced_range == 0)
            {
              reduced_range = random() % range;
            }
            result[i][j][k][l] = reduced_range;
          }
          else
          {
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
float ***gen_random_3d_matrix(int dim0, int dim1, int dim2, int nz_ratio)
{
  float ****mat4d;
  float ***mat3d;

  // create a 4d matrix with single first dimension
  mat4d = gen_random_4d_matrix(1, dim0, dim1, dim2, nz_ratio);
  // now throw away out first dimension
  mat3d = mat4d[0];
  free(mat4d);
  return mat3d;
}

/* check the sum of absolute differences is within reasonable epsilon */
void check_result(float ***result, float ***control,
                  int dim0, int dim1, int dim2)
{
  int i, j, k;
  double sum_abs_diff = 0.0;
  const double EPSILON = 0.0625;

  DEBUGGING(printf("SAD\n"));

  for (i = 0; i < dim0; i++)
  {
    for (j = 0; j < dim1; j++)
    {
      for (k = 0; k < dim2; k++)
      {
        double diff = fabs(control[i][j][k] - result[i][j][k]);
        assert(diff >= 0.0);
        sum_abs_diff = sum_abs_diff + diff;
      }
    }
  }

  if (sum_abs_diff > EPSILON)
  {
    fprintf(stderr, "WARNING: sum of absolute differences (%f) > EPSILON (%f)\n",
            sum_abs_diff, EPSILON);
  }
  else
  {
    printf("COMMENT: sum of absolute differences (%f)  within acceptable range (%f)\n", sum_abs_diff, EPSILON);
  }
}

/* a slow but correct version of dense convolution written by David */
void multichannel_conv_dense(float ***image, float ****kernels,
                             float ***output, int width, int height,
                             int nchannels, int nkernels, int kernel_order)
{
  int h, w, x, y, c, m;

  // initialize the output matrix to zero
  for (m = 0; m < nkernels; m++)
  {
    for (h = 0; h < height; h++)
    {
      for (w = 0; w < width; w++)
      {
        output[m][h][w] = 0.0;
      }
    }
  }

  for (m = 0; m < nkernels; m++)
  {
    for (w = 0; w < width; w++)
    {
      for (h = 0; h < height; h++)
      {
        for (x = 0; x < kernel_order; x++)
        {
          for (y = 0; y < kernel_order; y++)
          {
            for (c = 0; c < nchannels; c++)
            {
              output[m][h][w] += image[w + x][h + y][c] * kernels[x][y][m][c];
            }
          }
        }
      }
    }
  }
}

/* a slow but correct version of sparse convolution written by David */
void multichannel_conv_sparse(float ***image, struct sparse_matrix ***kernels,
                              float ***output, int width, int height,
                              int nchannels, int nkernels, int kernel_order)
{
  int h, w, x, y, c, m, index;
  float value;

  // initialize the output matrix to zero
  for (m = 0; m < nkernels; m++)
  {
    for (h = 0; h < height; h++)
    {
      for (w = 0; w < width; w++)
      {
        output[m][h][w] = 0.0;
      }
    }
  }

  DEBUGGING(fprintf(stderr, "w=%d, h=%d, c=%d\n", w, h, c));

  // now compute multichannel, multikernel convolution
  for (w = 0; w < width; w++)
  {
    for (h = 0; h < height; h++)
    {
      double sum = 0.0;
      for (x = 0; x < kernel_order; x++)
      {
        for (y = 0; y < kernel_order; y++)
        {
          struct sparse_matrix *kernel = kernels[x][y];
          for (m = 0; m < nkernels; m++)
          {
            for (index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m + 1]; index++)
            {
              int this_c = kernel->channel_numbers[index];
              assert((this_c >= 0) && (this_c < nchannels));
              value = kernel->values[index];
              output[m][h][w] += image[w + x][h + y][this_c] * value;
            }
          } // m
        }   // y
      }     // x
    }       // h
  }         // w
}

/* the fast version of sparse convolution written by the team */
void team_conv_sparse(float ***image, struct sparse_matrix ***kernels,
                      float ***output, int width, int height,
                      int nchannels, int nkernels, int kernel_order)
{
  int h, w, x, y, c, m, index;
  float value;
  int OpenMP_flag = 0;

  /* the threshold to use OpenMP,
     if the inputs width * nchannels * nkernels * kernel_order
     are greater than 270 * 270 * 32 * 64 * 3,
     then the program will use OpenMp to speed up and let OpenMP_flag = 1. 
     this threshold is gained from multiple different inputs tests, so this threshold may be not very accurate. */
  long long check = width * nchannels * nkernels * kernel_order;
  long long threshold = 270 * 32 * 64 * 3;

  if (check >= threshold)
  {
    OpenMP_flag = 1;
  }

  /*
    ______________
    |          | |
    |     1    | |
    |          |2|
    |__________| |
    |_____3____|_|
  
    The output matrix in initialization and convolution parts is divided to 3 parts.
    Part 1 size is (height - height%4) * (width - width%4), so both part 1's height and width can be exactly divided by 4.
    Part 2 size is height * (width%4).
    Part 3 size is (height%4) * (width - width%4).
    If image's height and width can be exactly divided by 4, then part 2 and 3 won't exist.
    The loop unrolling and SSE are used in part 1 to speed up.
    Because of small values of height and width in part 2 and 3, I haven't use SSE to speed up these part.
  */

  // initialize the output matrix to zero
  float init = 0.0;
  __m128 initValue = _mm_set1_ps(init);
  for (m = 0; m < nkernels; m++)
  {
    // Using loop unrolling to speedup and assign four rows in one iteration.
    for (h = 0; h < height - height % 4; h += 4)
    {
      // Using SSE to speedup and assign four colums each time.
      for (w = 0; w < width - width % 4; w += 4)
      {
        // output[i][j][k] = 0.0;
        // Using SSE will assign four colums each time, so four elements in output will be initialized to 0.
        // And because of loop unrolling, four rows will be operated in one interation. 
        _mm_storeu_ps(&output[m][h][w], initValue);
        _mm_storeu_ps(&output[m][h + 1][w], initValue);
        _mm_storeu_ps(&output[m][h + 2][w], initValue);
        _mm_storeu_ps(&output[m][h + 3][w], initValue);
      }
    }
  }

  // Handle the rest parts.
  // I put m (nkernels) as the innermost loop to avoid unnecessary for loop when the width or height is exactly divided by 4.
  // Handle part 2
  for (h = height - height % 4; h < height; h++)
  {
    for (w = 0; w < width; w++)
    {
      for (m = 0; m < nkernels; m++)
      {
        output[m][h][w] = 0.0;
      }
    }
  }
  // Handle part 3
  for (h = 0; h < height - height % 4 && (width % 4 != 0); h++)
  {
    for (w = width - width % 4; w < width; w++)
    {
      for (m = 0; m < nkernels; m++)
      {
        output[m][h][w] = 0.0;
      }
    }
  }


// now compute multichannel, multikernel convolution

// First handle the part 1 that both the length and width exactly divisible by 4.
// If input dataset reached threshold then OpenMP_flag = 1 and program will use OpenMP to speedup.
#pragma omp parallel for if (OpenMP_flag) private(w, h, m, x, y) shared(kernels, image, output)
  // I got some ideas when I read multichannel_conv_dense(), so I put m’s for loop as the outermost loop in convolution part. 
  // In this order, I can implement the SSE on h (height).
  for (m = 0; m < nkernels; m++)
  {
    // Using loop unrolling to speedup and calculate four colums in one iteration.
    for (w = 0; w < width - width % 4; w += 4)
    {
      // Using SSE to speedup and calculate four rows each time.
      for (h = 0; h < height - height % 4; h += 4)
      {
        // double sum = 0.0;
        __m128 sum1 = _mm_setzero_ps();
        __m128 sum2 = _mm_setzero_ps();
        __m128 sum3 = _mm_setzero_ps();
        __m128 sum4 = _mm_setzero_ps();
        for (x = 0; x < kernel_order; x++)
        {
          for (y = 0; y < kernel_order; y++)
          {
            struct sparse_matrix *kernel = kernels[x][y];
            for (index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m + 1]; index++)
            {
              int this_c = kernel->channel_numbers[index];
              assert((this_c >= 0) && (this_c < nchannels));

              // value = kernel->values[index];
              // Load four copies of value.
              __m128 value = _mm_set1_ps(kernel->values[index]);

              // output[m][h][w] += image[w + x][h + y][this_c] * value;
              // Load four elements in height and calculate four multiplication at same time.
              // Becasue of the loop unrolling, four elements in width will be assigned in one iteration.
              __m128 value1 = _mm_setr_ps(image[w + x][h + y][this_c], image[w + x][h + y + 1][this_c], image[w + x][h + y + 2][this_c], image[w + x][h + y + 3][this_c]);
              value1 = _mm_mul_ps(value1, value);

              __m128 value2 = _mm_setr_ps(image[w + x + 1][h + y][this_c], image[w + x + 1][h + y + 1][this_c], image[w + x + 1][h + y + 2][this_c], image[w + x + 1][h + y + 3][this_c]);
              value2 = _mm_mul_ps(value2, value);

              __m128 value3 = _mm_setr_ps(image[w + x + 2][h + y][this_c], image[w + x + 2][h + y + 1][this_c], image[w + x + 2][h + y + 2][this_c], image[w + x + 2][h + y + 3][this_c]);
              value3 = _mm_mul_ps(value3, value);

              __m128 value4 = _mm_setr_ps(image[w + x + 3][h + y][this_c], image[w + x + 3][h + y + 1][this_c], image[w + x + 3][h + y + 2][this_c], image[w + x + 3][h + y + 3][this_c]);
              value4 = _mm_mul_ps(value4, value);

              // Four additions each time and four loop unrolling in one iteration.
              sum1 = _mm_add_ps(sum1, value1);
              sum2 = _mm_add_ps(sum2, value2);
              sum3 = _mm_add_ps(sum3, value3);
              sum4 = _mm_add_ps(sum4, value4);
            }
          } // y
        }   // x

        // Load to result sum to output
        float sum[4];
        _mm_storeu_ps(sum, sum1);
        output[m][h][w] = sum[0];
        output[m][h + 1][w] = sum[1];
        output[m][h + 2][w] = sum[2];
        output[m][h + 3][w] = sum[3];

        _mm_storeu_ps(sum, sum2);
        output[m][h][w + 1] = sum[0];
        output[m][h + 1][w + 1] = sum[1];
        output[m][h + 2][w + 1] = sum[2];
        output[m][h + 3][w + 1] = sum[3];

        _mm_storeu_ps(sum, sum3);
        output[m][h][w + 2] = sum[0];
        output[m][h + 1][w + 2] = sum[1];
        output[m][h + 2][w + 2] = sum[2];
        output[m][h + 3][w + 2] = sum[3];

        _mm_storeu_ps(sum, sum4);
        output[m][h][w + 3] = sum[0];
        output[m][h + 1][w + 3] = sum[1];
        output[m][h + 2][w + 3] = sum[2];
        output[m][h + 3][w + 3] = sum[3];
      } // h
    }   // w
  }     // m

  // Then handle the part 2 that leaves in right.
  for (w = width - width % 4; w < width; w++)
  {
    for (h = 0; h < height; h++)
    {
      for (x = 0; x < kernel_order; x++)
      {
        for (y = 0; y < kernel_order; y++)
        {
          struct sparse_matrix *kernel = kernels[x][y];
          for (m = 0; m < nkernels; m++)
          {
            for (index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m + 1]; index++)
            {
              int this_c = kernel->channel_numbers[index];
              assert((this_c >= 0) && (this_c < nchannels));
              value = kernel->values[index];
              output[m][h][w] += image[w + x][h + y][this_c] * value;
            }
          } // m
        }   // y
      }     // x
    }       // h
  }         // w

  // Then handle the part 3 that leaves in bottom.
  for (w = 0; w < width - width % 4 && (height % 4 != 0); w++)
  {
    for (h = height - height % 4; h < height; h++)
    {
      for (x = 0; x < kernel_order; x++)
      {
        for (y = 0; y < kernel_order; y++)
        {
          struct sparse_matrix *kernel = kernels[x][y];
          for (m = 0; m < nkernels; m++)
          {
            for (index = kernel->kernel_starts[m]; index < kernel->kernel_starts[m + 1]; index++)
            {
              int this_c = kernel->channel_numbers[index];
              assert((this_c >= 0) && (this_c < nchannels));
              value = kernel->values[index];
              output[m][h][w] += image[w + x][h + y][this_c] * value;
            }
          } // m
        }   // y
      }     // x
    }       // h
  }         // w
}

int main(int argc, char **argv)
{
  //float image[W][H][C];
  //float kernels[M][C][K][K];
  //float output[M][W][H];

  float ***image;
  float ****kernels;
  struct sparse_matrix ***sparse_kernels = NULL;
  float ***control_output, ***output;
  long long mul_time;
  int width, height, kernel_order, nchannels, nkernels;
  struct timeval start_time;
  struct timeval stop_time;
  int nz_ratio = 1; // by default we just have a dense matrix

  if (argc != 7)
  {
    fprintf(stderr, "Usage: conv-harness <image_width> <image_height> <kernel_order> <number of channels> <number of kernels> <non-zero ratio>\n");
    exit(1);
  }
  else
  {
    width = atoi(argv[1]);
    height = atoi(argv[2]);
    kernel_order = atoi(argv[3]);
    nchannels = atoi(argv[4]);
    nkernels = atoi(argv[5]);
    nz_ratio = atoi(argv[6]);
  }
  switch (kernel_order)
  {
  case 1:
  case 3:
  case 5:
  case 7:
    break;
  default:
    fprintf(stderr, "FATAL: kernel_order must be 1, 3, 5 or 7, not %d\n",
            kernel_order);
    exit(1);
  }

  assert(width >= 1);
  assert(height >= 1);
  assert(nchannels >= 1);
  assert(nkernels >= 1);
  assert(nz_ratio >= 1);

  /* allocate the matrices */
  image = gen_random_3d_matrix(width + kernel_order, height + kernel_order,
                               nchannels, 1); // nz_ratio == 1, ie no sparsity
  kernels = gen_random_4d_matrix(kernel_order, kernel_order, nkernels, nchannels, nz_ratio);
  if (nz_ratio > 1)
  { // we have sparsity
    sparse_kernels = kernels_dense2sparse(kernels, kernel_order, nkernels, nchannels);
  }

  output = new_empty_3d_matrix(nkernels, width, height);

  control_output = new_empty_3d_matrix(nkernels, width, height);

  /* use a simple multichannel convolution routine to produce control result */
  multichannel_conv_dense(image, kernels, control_output, width,
                          height, nchannels, nkernels, kernel_order);

  /* record starting time of team's code*/
  gettimeofday(&start_time, NULL);

  if (nz_ratio > 1)
  { // we're working on a sparse matrix
    /* perform student team's sparse multichannel convolution */
    team_conv_sparse(image, sparse_kernels, output, width,
                     height, nchannels, nkernels, kernel_order);
  }
  else
  { // we're working on a dense matrix
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
