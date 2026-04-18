#include "lolek.h"
#include "memory.h"

// Lookup tables for performance
#define TABLE_SIZE 360
static float sin_table[TABLE_SIZE];
static float cos_table[TABLE_SIZE];
static int tables_initialized = 0;

void math_init() {
    if (tables_initialized) return;
    
    // taylor swift lets go
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        float angle = RAD((float)i);
        float x = angle;
        float res = x;
        float term = x;
        
        // loop 10 times for precision
        for (int k = 1; k <= 10; k++) {
            term = -term * x * x / ((2 * k) * (2 * k + 1));
            res += term;
        }
        sin_table[i] = res;
        
        res = 1.0f;
        term = 1.0f;
        for (int k = 1; k <= 10; k++) {
            term = -term * x * x / ((2 * k - 1) * (2 * k));
            res += term;
        }
        cos_table[i] = res;
    }
    
    tables_initialized = 1;
}

static float normalize_angle(float angle) {
    while (angle < 0) angle += TAU;
    while (angle >= TAU) angle -= TAU;
    return angle;
}

float sin(float angle) {
    if (!tables_initialized) math_init();
    angle = normalize_angle(angle);
    int idx = (int)(angle * (180.0f / PI));
    if (idx < 0) idx = 0;
    if (idx >= TABLE_SIZE) idx = TABLE_SIZE - 1;
    return sin_table[idx];
}

float cos(float angle) {
    if (!tables_initialized) math_init();
    angle = normalize_angle(angle);
    int idx = (int)(angle * (180.0f / PI));
    if (idx < 0) idx = 0;
    if (idx >= TABLE_SIZE) idx = TABLE_SIZE - 1;
    return cos_table[idx];
}

int abs(int x) {
    return (x < 0) ? -x : x;
}

float fabs(float x) {
    return (x < 0) ? -x : x;
}

