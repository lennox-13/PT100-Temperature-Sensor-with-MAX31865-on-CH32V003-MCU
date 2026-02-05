#ifndef __MAX31865_H
#define __MAX31865_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// PT100 reference and ADC parameters
// -----------------------------------------------------------------------------
#define MAX31865_R_REF   426L     // Precise reference resistor value [ohms]
#define MAX31865_ADC_MAX 32768L   // 15-bit ADC full scale

// -----------------------------------------------------------------------------
// Callendar-Van Dusen coefficients (IEC 60751)
// Used for negative temperatures (full equation including C term)
// Scaling is applied for fixed-point integer math
// -----------------------------------------------------------------------------
#define MAX31865_A   39083L   // 3.9083e-3  scaled °¡1e7
#define MAX31865_B  -57750L   // ?5.775e-7  scaled °¡1e10
#define MAX31865_C  -41830L   // ?4.183e-12 scaled °¡1e16

// -----------------------------------------------------------------------------
// Fixed-point CVD constants for positive temperatures
// Simplified equation without C term + Newton iteration
// -----------------------------------------------------------------------------
#define R0_X100   10000LL   // R0 = 100.00 R scaled °¡100
#define A_S       3908300LL // A coefficient scaled °¡1e9
#define B_S       -577LL    // B coefficient scaled °¡1e9

// -----------------------------------------------------------------------------
// Global measurement variables 
// -----------------------------------------------------------------------------
extern uint16_t Max31865_resistance_raw;      // Raw RTD ADC value
extern int32_t  Max31865_resistance_x100;     // Resistance scaled °¡100
extern int16_t  Max31865_temperature_x10;     // Temperature in 0.1 °„C
extern uint8_t  Max31865_reg1, Max31865_reg2; // RTD register bytes

// -----------------------------------------------------------------------------
// Public
// -----------------------------------------------------------------------------
void    Max31865_Init(void);                 // Initialize MAX31865
uint8_t Max31865_ReadRegister(void);         // Read RTD register, return fault flag
void    Max31865_ConvertToTemperature(void); // Convert resistance to temperature

#endif // __MAX31865_H
