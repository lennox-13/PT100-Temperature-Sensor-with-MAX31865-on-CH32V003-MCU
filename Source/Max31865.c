/******************************************************************************
 * Tittle    : CH32V003F4P6 MCU - MAX31865 PT100 Driver
 *
 * Author    : Lennox
 * Date      : 2026-02-05
 * Version   : V1.0
 *
 * Description:
 *   Driver implementation for MAX31865 RTD-to-Digital converter
 *   to interface with PT100 temperature sensors via SPI1.
 *
 * Features:
 *   - Supports full temperature range: -200 C ¡­ +850 C
 *   - Fixed-point Callendar-Van Dusen equation for temperature calculation
 *   - Newton-Raphson iteration for positive temperatures
 *   - Fault detection and automatic clearing
 *
 ******************************************************************************
 */

#include "ch32v00x.h"
#include "spi.h"
#include "Max31865.h"

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
uint16_t Max31865_resistance_raw;      // Raw ADC resistance value
int32_t  Max31865_resistance_x100;     // Resistance scaled by 100
int16_t  Max31865_temperature_x10;     // Temperature in 0.1 C units
uint8_t  Max31865_reg1, Max31865_reg2; // RTD register bytes

// -----------------------------------------------------------------------------
// MAX31865 initialization
// -----------------------------------------------------------------------------
void Max31865_Init(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_4);    // Select MAX31865 (CS low)
    SPI_TransferByte(0x80);               // Write to configuration register
    SPI_TransferByte(0xD2);               // 3-wire mode, auto conversion, Vbias ON
    GPIO_SetBits(GPIOC, GPIO_Pin_4);      // Deselect device

    Delay_Ms(100);                        // Allow bias and conversion to stabilize
}

// -----------------------------------------------------------------------------
// Convert measured resistance to temperature using Callendar-Van Dusen equation
// -----------------------------------------------------------------------------
void Max31865_ConvertToTemperature(void)
{
    // Scale raw ADC value to resistance *100
    Max31865_resistance_x100 =
        ((int32_t)Max31865_resistance_raw * MAX31865_R_REF * 100L) / MAX31865_ADC_MAX;

    // -------------------------------------------------------------------------
    // Positive temperature branch (0 ¡­ 850 C)
    // Uses simplified CVD (without C term) + Newton iteration
    // -------------------------------------------------------------------------
    if (Max31865_resistance_x100 >= 10000L)
    {
        int32_t t;

        // Linear initial estimate using A coefficient only
        t = (int32_t)(((int64_t)(Max31865_resistance_x100 - R0_X100) * 10LL * 1000000000LL) /
                      (R0_X100 * A_S));

        // Clamp to valid PT100 range
        if (t < 0)    t = 0;
        if (t > 8500) t = 8500;

        // Newton¨CRaphson refinement
        for (int i = 0; i < 4; i++)
        {
            int64_t tt = t;

            int64_t termA = (R0_X100 * A_S * tt) / (10LL * 1000000000LL);
            int64_t termB = (R0_X100 * B_S * (tt * tt)) / (100LL * 1000000000LL);
            int64_t Rcalc = R0_X100 + termA + termB;

            int64_t f = Rcalc - (int64_t)Max31865_resistance_x100;

            int64_t dRdt =
                (R0_X100 * A_S) / (10LL * 1000000000LL) +
                (R0_X100 * 2LL * B_S * tt) / (100LL * 1000000000LL);

            if (dRdt == 0) break;

            t -= (int32_t)(f / dRdt);

            // Clamp again after iteration
            if (t < 0)    t = 0;
            if (t > 8500) t = 8500;
        }

        Max31865_temperature_x10 = (int16_t)t;
    }
    // -------------------------------------------------------------------------
    // Negative temperature branch (?200 ¡­ 0 C)
    // Full Callendar-Van Dusen including C coefficient
    // -------------------------------------------------------------------------
    else
    {
        int32_t delta = Max31865_resistance_x100 - 10000L;
        int32_t t = (delta * 10000L) / MAX31865_A;  // Initial estimate

        for (int i = 0; i < 3; i++)
        {
            int64_t t_val = t;
            int64_t t2 = (t_val * t_val) / 10LL;
            int64_t t3 = (t2 * t_val) / 10LL;

            int64_t term_const = 10000000000LL;
            int64_t term_at  = ((int64_t)MAX31865_A * t_val) * 100LL;
            int64_t term_bt2 = ((int64_t)MAX31865_B * t2) / 10LL;

            int64_t t_minus_100 = t_val - 1000LL;
            int64_t term_c =
                ((int64_t)MAX31865_C * t_minus_100 * t3) / 100000000LL;

            int64_t r_calc =
                (100LL * (term_const + term_at + term_bt2 + term_c)) / 1000000000LL;

            int64_t f = r_calc - (int64_t)Max31865_resistance_x100;

            // Derivative for Newton iteration
            int64_t fp_a = (int64_t)MAX31865_A * 100LL;
            int64_t fp_bt = (2LL * (int64_t)MAX31865_B * t_val) / 10LL;
            int64_t fp_c_bracket = 4LL * t3 - 3000LL * t2;
            int64_t fp_c = ((int64_t)MAX31865_C * fp_c_bracket) / 100000000LL;

            int64_t fp =
                (100LL * (fp_a + fp_bt + fp_c)) / 1000000000LL;

            if (fp != 0)
            {
                t -= (int32_t)((f * 10LL) / fp);
            }
        }

        Max31865_temperature_x10 = (int16_t)t;
    }

    // Final safety clamp to PT100 valid limits
    if (Max31865_temperature_x10 < -2000) Max31865_temperature_x10 = -2000;
    if (Max31865_temperature_x10 > 8500)  Max31865_temperature_x10 = 8500;
}

// -----------------------------------------------------------------------------
// Read RTD register and clear fault if present
// Returns: 1 = fault detected, 0 = OK
// -----------------------------------------------------------------------------
uint8_t Max31865_ReadRegister(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_4);   // CS low
    SPI_TransferByte(0x01);              // Read RTD MSB
    Max31865_reg1 = SPI_TransferByte(0x00);
    Max31865_reg2 = SPI_TransferByte(0x00);
    GPIO_SetBits(GPIOC, GPIO_Pin_4);     // CS high

    uint16_t fullreg = ((uint16_t)Max31865_reg1 << 8) | Max31865_reg2;
    uint8_t fault = 0;

    // Check fault bit (LSB)
    if (fullreg & 0x01)
    {
        fault = 1;

        // Clear fault by rewriting configuration
        GPIO_ResetBits(GPIOC, GPIO_Pin_4);
        SPI_TransferByte(0x80);
        SPI_TransferByte(0xD2);
        GPIO_SetBits(GPIOC, GPIO_Pin_4);
    }

    // Extract 15-bit RTD value
    Max31865_resistance_raw = fullreg >> 1;

    return fault;
}
