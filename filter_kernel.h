#ifndef FILTER_KERNEL_H
#define FILTER_KERNEL_H

#include "ap_int.h"

typedef ap_uint<128> axie4_t; // Match AXI4 interface type

extern "C" { 
    void filter_kernel(
        axie4_t *input_image,   
        axie4_t *output_image,
        float filter[3][3],    
        float filter_divisor,  
        int width, 
        int height,
        int channels
    );
}

#endif // FILTER_KERNEL_H
