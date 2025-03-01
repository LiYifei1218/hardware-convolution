#include <iostream>
#include <cstring>  // For memcpy
#include "filter_kernel.h"

#define TEST_WIDTH 16
#define TEST_HEIGHT 16
#define TEST_CHANNELS 1  // Single-channel grayscale image
#define NUM_AXIE4_WORDS ((TEST_WIDTH * TEST_HEIGHT * TEST_CHANNELS + 15) / 16)

typedef ap_uint<128> axie4_t;  // Match kernel data type

// 📌 **Test Cases**
unsigned char test_image_zeros[TEST_HEIGHT][TEST_WIDTH] = {0};

unsigned char test_image_ones[TEST_HEIGHT][TEST_WIDTH];
unsigned char test_image_gradient[TEST_HEIGHT][TEST_WIDTH];
unsigned char test_image_checkerboard[TEST_HEIGHT][TEST_WIDTH];
unsigned char test_image_random[TEST_HEIGHT][TEST_WIDTH];

// 📌 **Function to Convert Image to AXI4 Format**
void image_to_axi4(unsigned char img[TEST_HEIGHT][TEST_WIDTH], axie4_t *axi_buffer) {
    int idx = 0;
    for (int i = 0; i < TEST_HEIGHT; i++) {
        for (int j = 0; j < TEST_WIDTH; j++) {
            int byte_offset = (idx % 16) * 8;
            axi_buffer[idx / 16] |= ((axie4_t)img[i][j] << byte_offset);
            idx++;
        }
    }
}

// 📌 **Function to Convert AXI4 Format Back to Image**
void axi4_to_image(axie4_t *axi_buffer, unsigned char img[TEST_HEIGHT][TEST_WIDTH]) {
    int idx = 0;
    for (int i = 0; i < TEST_HEIGHT; i++) {
        for (int j = 0; j < TEST_WIDTH; j++) {
            int byte_offset = (idx % 16) * 8;
            img[i][j] = (axi_buffer[idx / 16] >> byte_offset) & 0xFF;
            idx++;
        }
    }
}

// 📌 **Function to Print Image Data**
void print_image(const unsigned char img[TEST_HEIGHT][TEST_WIDTH], const std::string &label) {
    std::cout << "---- " << label << " ----\n";
    for (int i = 0; i < TEST_HEIGHT; i++) {
        for (int j = 0; j < TEST_WIDTH; j++) {
            std::cout << (int)img[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    // 📌 **Initialize Test Images**
    memset(test_image_ones, 255, sizeof(test_image_ones));  // All 255 (white)

    // Generate gradient image
    for (int i = 0; i < TEST_HEIGHT; i++) {
        for (int j = 0; j < TEST_WIDTH; j++) {
            test_image_gradient[i][j] = (j * 255) / (TEST_WIDTH - 1);
        }
    }

    // Generate checkerboard pattern
    for (int i = 0; i < TEST_HEIGHT; i++) {
        for (int j = 0; j < TEST_WIDTH; j++) {
            test_image_checkerboard[i][j] = (i + j) % 2 == 0 ? 255 : 0;
        }
    }

    // Generate random image
    srand(42);
    for (int i = 0; i < TEST_HEIGHT; i++) {
        for (int j = 0; j < TEST_WIDTH; j++) {
            test_image_random[i][j] = rand() % 256;
        }
    }

    // 📌 **Run Tests**
    unsigned char *test_cases[] = {
        (unsigned char *)test_image_zeros,
        (unsigned char *)test_image_ones,
        (unsigned char *)test_image_gradient,
        (unsigned char *)test_image_checkerboard,
        (unsigned char *)test_image_random
    };
    const char *test_labels[] = {
        "All Zeros", "All Ones", "Gradient", "Checkerboard", "Random"
    };

    // 📌 **Define Filter**
    float test_filter[3][3] = {
        {1.0, 2.0, 1.0},
        {2.0, 4.0, 2.0},
        {1.0, 2.0, 1.0}
    };
    float filter_divisor = 16.0;

    // 📌 **Run Tests**
    for (int test_id = 0; test_id < 5; test_id++) {
        std::cout << "\n==== Running Test: " << test_labels[test_id] << " ====\n";

        // Allocate memory
        axie4_t *input_image = new axie4_t[NUM_AXIE4_WORDS]();
        axie4_t *output_image = new axie4_t[NUM_AXIE4_WORDS]();
        unsigned char output_buffer[TEST_HEIGHT][TEST_WIDTH];

        // Convert input image to AXI4 format
        image_to_axi4((unsigned char(*)[TEST_WIDTH])test_cases[test_id], input_image);

        // Print input
        print_image((unsigned char(*)[TEST_WIDTH])test_cases[test_id], "Input Image");

        // Run the FPGA function
        filter_kernel(input_image, output_image, test_filter, filter_divisor, TEST_WIDTH, TEST_HEIGHT, TEST_CHANNELS);

        // Convert output back to normal image
        axi4_to_image(output_image, output_buffer);

        // Print output
        print_image(output_buffer, "Filtered Output");

        // Cleanup
        delete[] input_image;
        delete[] output_image;
    }

    return 0;
}
