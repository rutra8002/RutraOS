#ifndef LOLEK_H
#define LOLEK_H

#include "types.h"

#define PI 3.14159265359f
#define TAU 6.28318530718f
#define RAD(d) ((d) * (PI / 180.0f))

// Fixed-point math constants (16.16)
#define FIXED_SHIFT 16
#define ONE (1 << FIXED_SHIFT)
#define TO_FIXED(x) ((int)((x) * ONE))
#define TO_FLOAT(x) ((float)(x) / ONE)
#define FIXED_MUL(x, y) ((int)(((int64_t)(x) * (y)) >> FIXED_SHIFT))
#define FIXED_DIV(x, y) ((int)(((int64_t)(x) << FIXED_SHIFT) / (y)))

// Trigonometry lookup tables
void math_init();
float sin(float angle);
float cos(float angle);

// Utility
int abs(int x);
float fabs(float x);

#endif
