# PT100-Temperature-Sensor-with-MAX31865-on-CH32V003-MCU
PT100 Resistance Temperature Detector  with a CH32V003 microcontroller using the MAX31865 

Overview

This project demonstrates interfacing a PT100 Resistance Temperature Detector (RTD) with a CH32V003 microcontroller using the MAX31865 RTD-to-Digital converter over SPI. The system measures temperature in the range of -200°C to +850°C and outputs the results via UART for monitoring or logging.

The implementation uses fixed-point arithmetic and the Callendar-Van Dusen (CVD) equation, ensuring precise temperature conversion without relying on floating-point calculations — ideal for resource-constrained MCUs.

# Features

Full temperature range support: -200°C to +850°C

Fixed-point Callendar-Van Dusen conversion for high accuracy

Newton-Raphson iteration for positive temperatures

Fault detection and automatic clearing

Easy-to-use SPI1 driver for MAX31865 communication

UART output for debugging and monitoring

Structured, modular driver design for reuse in other projects

# Hardware:

MCU: CH32V003 (32-bit RISC-V, 48 MHz)

Temperature sensor: PT100 RTD

RTD interface: MAX31865 via SPI1

UART interface: for monitoring and logging

Connections:
- MCU Pin	MAX31865 / PT100
- PC4	CS
- PC5	SCLK
- PC6	MOSI
- PC7	MISO
- PD5 USART TX	Monitor
  
# Software Architecture

SPI Driver (spi.c / spi.h)

Handles SPI1 initialization and byte-level transfer.

Blocking transfer with simple timeout for robustness.

MAX31865 Driver (max31865.c / max31865.h)

Implements register read/write for MAX31865.

Converts raw ADC readings to temperature using fixed-point CVD formula.

Performs Newton-Raphson refinement for positive temperatures.

Supports fault detection and automatic reset of the configuration register.

Application (main.c)

Initializes MCU peripherals (SPI, UART, delays).

Reads temperature every second.

Prints temperature and fault status over UART.

Fixed-Point Temperature Conversion

To avoid floating-point operations, the driver scales all constants:

Resistance scaled by 100

Temperature scaled by 0.1 °C

CVD coefficients scaled appropriately for integer math

This ensures precise and efficient computation on a low-power MCU without sacrificing accuracy.
