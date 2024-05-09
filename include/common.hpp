#pragma once

// HTTP header delimiter
#define CRLF "\r\n"
#define CRLF2 "\r\n\r\n"

// Conditionally include the appropriate SIMD headers,
// prototypes, and macro definitions based on the target
#if defined(__aarch64__) || defined(__ARM_NEON)
#include <arm_neon.h>
void toupper_ascii_neon(char *str);
#define TOUPPER_ASCII(str) toupper_ascii_neon(str)
#elif defined(__AVX2__)
#include <immintrin.h>
void toupper_ascii_avx2(char *str);
#define TOUPPER_ASCII(str) toupper_ascii_avx2(str)
#else
void toupper_ascii_scalar(char *str);
#define TOUPPER_ASCII(str) toupper_ascii_scalar(str)
#endif
