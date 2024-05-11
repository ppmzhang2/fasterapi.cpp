#include "common.hpp"
#include <cstring>

// ARM NEON implementation
#if defined(__aarch64__) || defined(__ARM_NEON)
void toupper_ascii_neon(char *str) {
    int length = strlen(str);
    int i = 0;

    // Define constants for vector operations
    uint8x16_t lo = vdupq_n_u8('a');
    uint8x16_t hi = vdupq_n_u8('z');
    uint8x16_t diff = vdupq_n_u8(32);

    // Process 16 characters at a time
    for (; i <= length - 16; i += 16) {
        uint8x16_t data = vld1q_u8((const uint8_t *)(str + i));

        // Check if each character is lowercase
        uint8x16_t is_lower = vandq_u8(vcgeq_u8(data, lo), vcleq_u8(data, hi));

        // Convert to uppercase by subtracting 32
        data = vsubq_u8(data, vandq_u8(diff, is_lower));

        // Store result back
        vst1q_u8((uint8_t *)(str + i), data);
    }

    // Process any remaining characters
    for (; i < length; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

#elif defined(__AVX2__)
void toupper_ascii_avx2(char *str) {
    int length = std::strlen(str);
    int i = 0;

    // Define constants for vector operations
    __m256i lo = _mm256_set1_epi8('a');
    __m256i hi = _mm256_set1_epi8('z');
    __m256i diff = _mm256_set1_epi8(32);

    // Process 32 characters at a time with AVX2
    for (; i <= length - 32; i += 32) {
        // Load 32 bytes into a 256-bit vector
        __m256i data = _mm256_loadu_si256((__m256i *)(str + i));

        // Compare if the characters are within the lowercase range
        __m256i is_lower = _mm256_and_si256(
            _mm256_cmpgt_epi8(data, _mm256_sub_epi8(lo, _mm256_set1_epi8(1))),
            _mm256_cmpgt_epi8(_mm256_add_epi8(hi, _mm256_set1_epi8(1)), data));

        // Subtract 32 (the difference between 'a' and 'A') where applicable
        data = _mm256_sub_epi8(data, _mm256_and_si256(diff, is_lower));

        // Store the modified vector back to the string
        _mm256_storeu_si256((__m256i *)(str + i), data);
    }

    // Process any remaining characters one by one
    for (; i < length; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

#else
void toupper_ascii_scalar(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

#endif
