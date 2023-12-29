
/* 
 *  Name: Marc Luzuriaga
 *  UID: 205916585
 */

#include <stdlib.h>
#include <omp.h>

#include "utils.h"
#include "parallel.h"


/*
 *  PHASE 1: compute the mean pixel value
 *  This code is buggy! Find the bug and speed it up.
 */
void mean_pixel_parallel(const uint8_t img[][NUM_CHANNELS], int num_rows, int num_cols, double mean[NUM_CHANNELS]) {
  int row,col,ch;
  long count = 0;
  // int size = num_cols*num_rows;
  double sum1 = 0;
  double sum2 = 0;
  double sum3 = 0;
  //  omp_set_num_threads(20);
#pragma omp parallel for default(shared) schedule(static) reduction (+:sum1,sum2,sum3,count) collapse(2)
  
  for (row = 0; row < num_rows; row++){
    for (col = 0; col < num_cols; col++){
	sum1 += img[row*num_cols+col][0];
	sum2 += img[row*num_cols+col][1];
	sum3 += img[row*num_cols+col][2];
	count+=3;
      }
  }

count /= NUM_CHANNELS;
 mean[0] = sum1;
 mean[1] = sum2;
 mean[2] = sum3;
 //#pragma omp parallel for
    for (ch = 0; ch < NUM_CHANNELS; ch++) {
        mean[ch] /= count;
    }
}



/*
 *  PHASE 2: convert image to grayscale and record the max grayscale value along with the number of times it appears
 *  This code is NOT buggy, just sequential. Speed it up.
 */

void grayscale_parallel(const uint8_t img[][NUM_CHANNELS], int num_rows, int num_cols, uint32_t grayscale_img[][NUM_CHANNELS], uint8_t *max_gray, uint32_t *max_count) {
  int row, ch, col;
    *max_gray = 0;
    *max_count = 0;
    int max_gray_temp = 0;
    int max_count_temp = 0;
    
#pragma omp parallel for schedule(guided) private(row,col,ch) reduction(max:max_gray_temp) collapse(2)
    for (row = 0; row < num_rows; row++) {
      for (col = 0; col < num_cols; col++){
	    int element = row*num_cols+col;
	    
	      int temp_grayscale_img = 0;
		  //Accumulate
	      for (ch = 0; ch < NUM_CHANNELS; ch++){
		temp_grayscale_img += img[element][ch];
	      }
	      temp_grayscale_img = temp_grayscale_img / NUM_CHANNELS;
		grayscale_img[element][0] = temp_grayscale_img;
		grayscale_img[element][1] = temp_grayscale_img;
		grayscale_img[element][2] = temp_grayscale_img;
	      
	      if(temp_grayscale_img > max_gray_temp){
		max_gray_temp = temp_grayscale_img;
	      }
      }
    }  
    
#pragma omp parallel for schedule(guided) private(row,col) reduction(+:max_count_temp) collapse(2)
    for (row = 0; row < num_rows; row++){
      for (col = 0; col < num_cols; col++){
	  if (grayscale_img[row*num_cols+col][0] == max_gray_temp){
	    max_count_temp+=3;
	  }
      }
    }
    
    
    
    *max_gray = max_gray_temp;
    *max_count = max_count_temp;
}


/*
 *  PHASE 3: perform convolution on image
 *  This code is NOT buggy, just sequential. Speed it up.
 */
void convolution_parallel(const uint8_t padded_img[][NUM_CHANNELS], int num_rows, int num_cols, const uint32_t kernel[], int kernel_size, uint32_t convolved_img[][NUM_CHANNELS]) {
    int row, col, ch, kernel_row, kernel_col;
    int kernel_norm, i;
    int conv_rows, conv_cols;

    // compute kernel normalization factor
    kernel_norm = 0;
    //#pragma omp parallel for schedule(static) reduction(+:kernel_norm)
    for(i = 0; i < kernel_size*kernel_size; i++) {
        kernel_norm += kernel[i];
    }
    // compute dimensions of convolved image
    conv_rows = num_rows - kernel_size + 1;
    conv_cols = num_cols - kernel_size + 1;
#pragma omp parallel for schedule(static) private(col,ch,kernel_row,kernel_col)
    // perform convolution
    for (row = 0; row < conv_rows; row++) {
      for (col = 0; col < conv_cols; col++) {
	int element = row*conv_cols+col;
            for (ch = 0; ch < NUM_CHANNELS; ch++) {
                convolved_img[element][ch] = 0;
                for (kernel_row = 0; kernel_row < kernel_size; kernel_row++) {
                    for (kernel_col = 0; kernel_col < kernel_size; kernel_col++) {
                        convolved_img[element][ch] += padded_img[(row+kernel_row)*num_cols + col+kernel_col][ch] * kernel[kernel_row*kernel_size + kernel_col];
                    }
                }
                convolved_img[element][ch] /= kernel_norm;
            }    
        }
  }
}
