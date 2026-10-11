# BME688 Embedded Firmware Project

A firmware project for reading environmental sensor data from a Bosch BME688 connected to an STM32F446 microcontroller over I2C. This repository is currently in active development and is intended as a practical embedded systems prototype for sensor acquisition, calibration, and early driver work.

## Project status

This project is not yet a production-ready driver or full application. It is a work-in-progress firmware workspace focused on:

- STM32F446 microcontroller bring-up
- I2C communication layer setup
- BME688 register access and calibration data loading
- Temperature, pressure, and humidity compensation logic
- Early integration testing on embedded hardware

The codebase is evolving, and some interfaces and implementation details may change during development.

## Overview

The project targets a low-level embedded implementation of the BME688 environmental sensor. The current code uses:

- STM32F446 MCU
- STM32 HAL-like register access via CMSIS headers
- Custom I2C driver functions
- Direct sensor register read/write operations
- Compensation algorithms for temperature, pressure, and humidity

This is designed as a learning and prototyping platform for embedded sensing rather than a polished library release.

## Hardware and software target

### Hardware

- MCU: STM32F446
- Sensor: Bosch BME688
- Bus: I2C
- Debug/programming: ST-Link compatible toolchain

### Software

- GCC ARM Embedded toolchain (`arm-none-eabi-*`)
- Make
- OpenOCD for flashing (optional but used in the provided Makefile)
- STM32F446 startup files and linker script included in the repository

## Repository structure

```text
.
├── Makefile
├── README.md
├── inc/
│   ├── bme688.h
│   ├── i2c.h
│   ├── stm32f446xx.h
│   ├── system_stm32f4xx.h
│   ├── core_cm4.h
│   └── mpu_armv7.h
├── linker_script.ld
├── obj/
├── build/
├── src/
│   ├── bme688.c
│   ├── i2c.c
│   ├── main.c
│   ├── rcc.c
│   └── ...
├── startup/
│   └── startup_stm32f446.s
└── ...
```

## Current implementation notes

The repository currently includes:

- A custom I2C driver abstraction for STM32 registers
- Initial BME688 calibration parameter reads
- Temperature, pressure, and humidity compensation calculations
- A simple firmware entry point that initializes the sensor and enters a loop

The current implementation is best understood as a development baseline rather than a complete driver package.

## Build instructions

From the project root:

```bash
make all
```

This builds the firmware image and places the output in the `build/` directory.

### Clean build artifacts

```bash
make clean
```

### Inspect ELF information

```bash
make readelf
```

## Flashing the target

The included Makefile has a flash target using OpenOCD:

```bash
make flash
```

This assumes:

- an ST-LINK or compatible debugger is connected
- OpenOCD is installed and configured for the STM32F4 target
- the user has permission to access the debugging interface

## Typical workflow

1. Connect the BME688 to the STM32F446 board using the intended I2C pins.
2. Verify the sensor address and wiring.
3. Build the firmware:

   ```bash
   make all
   ```

4. Flash to the board:

   ```bash
   make flash
   ```

5. Monitor the application behavior through the debug console or logic analyzer as needed.

## Sensor driver status

The driver code in `src/bme688.c` is still under construction. Areas that are actively being worked on include:

- register timing and sequencing accuracy
- full BME688 gas-sensor configuration
- heater control and gas resistance handling
- final validation against measured readings
- cleanup of naming and API consistency between BME688 and legacy BME280 references

Because of this, users should expect API changes and experimental code paths while the project matures.

## Development guidance

If you are extending this project, a good starting point is:

- review `src/main.c` for the entry-point behavior
- review `src/i2c.c` for bus control details
- review `src/bme688.c` for sensor communication and compensation logic
- keep the CMSIS headers and register definitions aligned with the STM32F446 target

## Contribution and usage

This repository is intended for experimentation, learning, and embedded system development. Contributions and refinements are welcome as the project matures.



## Notes

This README reflects the current state of the repository and should be updated as the project evolves. At the moment, the code is best described as an early embedded sensor prototype in active development.
