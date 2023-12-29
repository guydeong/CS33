#include <stdio.h>

typedef unsigned packed_t;

int xbyte(packed_t word, int bytenum) {
	//Holds amount needed to push the desired byte to the end of the 32-bit series
	int arith_left_shift = (3-bytenum) << 3;
	//Holds amount needed to push the byte that is at the end of the series to the least significant placement of 0
	int arith_right_shift = 24;
	//Explicit casting to signed encoding
	int output = (int)(word << arith_left_shift) >> arith_right_shift;
	return output;
	
}
