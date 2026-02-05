/******************************************************************************
 * Tittle    : CH32V003F4P6 - PT100 MAX31865 Temperature Sensor
 *
 * Author    : Lennox
 * Date      : 2026-02-05
 * Version   : V1.0
 *
 * Description:
 *   Application for reading temperature from a PT100 sensor
 *   via MAX31865 over SPI1 and printing the result over USART.
 *
 * Notes:
 *   - temperature range: -200 C бн +850 C
 *   - uses fixed-point Callendar-Van Dusen calculation
 *   - print temperature every 1000 ms
 *
 ******************************************************************************
 */

#include <ch32v00x.h>
#include "spi.h"
#include "Max31865.h"

#define USART_Printf   // Enable UART Printf (PD5)

// -----------------------------------------------------------------------------
// Print measured temperature via USART (Printf)
// Displays temperature in C with 0.1 C resolution
// -----------------------------------------------------------------------------
void Max31865_PrintTemp(void)
{
    int16_t temp_int = Max31865_temperature_x10 / 10;  // Integer C
    int16_t temp_dec = Max31865_temperature_x10 % 10;  // Decimal part

    if (temp_dec < 0) temp_dec = -temp_dec;

    printf("Temperature: %d.%d C\r\n", temp_int, temp_dec);
}

// -----------------------------------------------------------------------------
// Main 
// -----------------------------------------------------------------------------
int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();
    SPI1_Init();
    Max31865_Init();

#ifdef USART_Printf
    USART_Printf_Init(115200);  // PD5

    Delay_Ms(4000);             // Initial delay

    printf("\r\n");
    printf("========================================\r\n");
    printf("  PT100 Temperature Sensor (IEC 60751)\r\n");
    printf("  MAX31865 RTD-to-Digital Converter\r\n");
    printf("  Range: -200 C to +850 C\r\n");
    printf("========================================\r\n\r\n");
#endif

    while (1)
    {
        if (Max31865_ReadRegister())     // Read RTD register and detect fault condition
        {
#ifdef USART_Printf
            printf("FAULT detected!\r\n");
#endif
        }
        Max31865_ConvertToTemperature(); // Convert measured resistance to temperature
#ifdef USART_Printf
        Max31865_PrintTemp();            // Print temperature over UART (PD5)
#endif
        Delay_Ms(1000);                  // Measurement period 1000 ms
    }
}
