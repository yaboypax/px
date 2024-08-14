#include <tgmath.h>

// Math Constants
// ---------------------------------
#define DC_OFFSET 1.0E-25
#define PI 3.141592653589793

// ---------------------------------
// Gain

// linear -> dB conversion
static inline float lin2dB(float lin) {
    static const float LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
    return log(lin) * LOG_2_DB;
}

// dB -> linear conversion
static inline float dB2lin(float dB) {
    static const float DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
    return exp(dB * DB_2_LOG);
}
// -------------------------------------


