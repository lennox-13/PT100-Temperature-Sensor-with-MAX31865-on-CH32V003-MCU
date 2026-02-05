/******************************************************************************
 * MCU       : CH32V003F4P6
 *
 * Author    : Lennox
 * Date      : 2026-02-05
 * Version   : V1.0
 *
 * Description:
 *   SPI1 driver for communication with MAX31865.
 *   Provides initialization and single-byte transfer functions.
 *
 * Notes:
 *   - SPI Mode 1 (CPOL=0, CPHA=1)
 *   - full-duplex 8-bit transfers
 *   - simple timeout mechanism
 *
 ******************************************************************************
 */

#include "spi.h"

// -----------------------------------------------------------------------------
// Initialize SPI1 peripheral and GPIO pins for MAX31865 communication
// -----------------------------------------------------------------------------
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure  = {0};

    // Enable clocks for SPI1 and GPIOC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOC, ENABLE);

    // -------------------------------------------------------------------------
    // Chip Select (CS) pin configuration: PC4 as push-pull output
    // -------------------------------------------------------------------------
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // -------------------------------------------------------------------------
    // SPI clock and MOSI pins: PC5 (SCLK), PC6 (MOSI) as alternate function
    // -------------------------------------------------------------------------
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // -------------------------------------------------------------------------
    // SPI MISO pin: PC7 as floating input
    // -------------------------------------------------------------------------
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Disable SPI before configuration
    SPI_Cmd(SPI1, DISABLE);

    // -------------------------------------------------------------------------
    // SPI configuration (compatible with MAX31865 timing requirements)
    // Mode: CPOL = 0, CPHA = 1 ? SPI Mode 1
    // -------------------------------------------------------------------------
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial     = 7;

    SPI_Init(SPI1, &SPI_InitStructure);

    // Enable SPI peripheral
    SPI_Cmd(SPI1, ENABLE);
}

// -----------------------------------------------------------------------------
// Transfer one byte over SPI (simple timeout protection)
// Returns received byte; returns 0 on timeout
// -----------------------------------------------------------------------------
uint8_t SPI_TransferByte(uint8_t data)
{
    uint8_t retry = 0;

    // Wait until transmit buffer is empty
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
        if (++retry > 200)
            return 0;  // TX timeout
    }

    // Send byte
    SPI_I2S_SendData(SPI1, data);

    retry = 0;

    // Wait until a byte is received
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if (++retry > 200)
            return 0;  // RX timeout
    }

    // Return received byte
    return SPI_I2S_ReceiveData(SPI1);
}
