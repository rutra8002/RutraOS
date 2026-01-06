#include "lolek.h"
#include "memory.h"

// Lookup tables for performance
#define TABLE_SIZE 360
static float sin_table[TABLE_SIZE];
static float cos_table[TABLE_SIZE];
static int tables_initialized = 0;

void math_init() {
    if (tables_initialized) return;
    
    // We don't have math.h, so we need a Taylor series or similar for init?
    // Actually, let's hardcode a small approximation or simple Taylor series for initialization
    // For a real OS, you'd want a proper math library.
    // Here we will use a very simple Taylor series approximation for initialization
    
    // Implementation note: Since we don't have standard math library, 
    // we will implement sin/cos via Taylor series dynamically if not pre-calculated.
    // But better yet, I will implement a basic sin/cos function using Taylor series 
    // and populate the table.
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        float angle = RAD((float)i);
        // Taylor series for sin(x) = x - x^3/3! + x^5/5! - ...
        // This is slow but only runs once.
        float x = angle;
        float res = x;
        float term = x;
        
        // Loop 10 times for precision
        for (int k = 1; k <= 10; k++) {
            term = -term * x * x / ((2 * k) * (2 * k + 1));
            res += term;
        }
        sin_table[i] = res;
        
        // Taylor series for cos(x) = 1 - x^2/2! + x^4/4! - ...
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

