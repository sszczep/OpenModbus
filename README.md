# OpenModbus

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![CI](https://github.com/sszczep/OpenModbus/actions/workflows/CI.yaml/badge.svg)](https://github.com/sszczep/OpenModbus/actions/workflows/CI.yaml)
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](https://github.com/sszczep/OpenModbus/actions)

A lightweight, portable, and robust Modbus slave implementation written in C for embedded systems. OpenModbus provides a complete RTU protocol stack with minimal memory footprint and comprehensive test coverage.

## Features

✅ **Vast Modbus RTU Protocol Support**
  - Read Coils (0x01)
  - Read Discrete Inputs (0x02)
  - Read Holding Registers (0x03)
  - Read Input Registers (0x04)
  - Write Single Coil (0x05)
  - Write Single Register (0x06)
  - Write Multiple Coils (0x0F)
  - Write Multiple Registers (0x10)
  - Mask Write Register (0x16)
  - Read/Write Multiple Registers (0x17)

🚀 **Optimized for Embedded Systems**
  - Minimal memory footprint
  - No dynamic memory allocation
  - ISR-safe design
  - Portable code

🧪 **Comprehensive Testing**
  - 100% test coverage
  - Unity test framework
  - Unit tests for all functions
  - Integration tests
  - Timing and state machine tests

🔧 **Easy Integration**
  - Simple callback interface
  - Flexible configuration
  - Platform-agnostic design
  - Well-documented API

## Quick Start

### Installation

```bash
git clone --recurse-submodules https://github.com/sszczep/OpenModbus.git
cd OpenModbus
```

### Basic Usage

```c
#include "modbus_slave.h"

// Define your application callbacks
ModbusExceptionCode read_holding_registers(uint16_t addr, uint16_t count, uint8_t *dest) {
    for (int i = 0; i < count; i++) {
        modbus_be16_set(&dest[i * 2], your_register_data[addr + i]);
    }
    return MODBUS_EX_NONE;
}

void transmit_data(const uint8_t *data, uint16_t length) {
    // Send data via UART, SPI, etc.
    uart_write(data, length);
}

int main() {
    ModbusSlave slave;
    ModbusSlaveConfig config = {
        .address = 0x01,
        .write = transmit_data,
        .read_holding_registers = read_holding_registers,
        // Add other callbacks as needed
    };
    
    // Initialize the Modbus slave
    if (modbus_slave_init(&slave, &config) != 0) {
        return -1;
    }
    
    while (1) {
        modbus_slave_poll(&slave);
        // Your application code here
    }
}
```

## API Documentation

### Core Functions

```c
// Initialization
int modbus_slave_init(ModbusSlave *slave, const ModbusSlaveConfig *cfg);

// Reception (call from UART ISR)
void modbus_slave_rx_byte(ModbusSlave *slave, uint8_t byte);

// Timing (call from timer ISR)
void modbus_slave_1_5t_elapsed(ModbusSlave *slave);
void modbus_slave_3_5t_elapsed(ModbusSlave *slave);

// Polling (call from main loop)
void modbus_slave_poll(ModbusSlave *slave);
```

### Configuration Structure

```c
typedef struct {
    uint8_t address;  // Slave address (0 for broadcast)
    
    // Required: Transmit callback
    void (*write)(const uint8_t *data, uint16_t length);
    
    // Optional callbacks for supported functions
    ModbusReadCoilsCb                   read_coils;
    ModbusReadDiscreteInputsCb          read_discrete_inputs;
    ModbusReadHoldingRegistersCb        read_holding_registers;
    ModbusReadInputRegistersCb          read_input_registers;
    ModbusWriteSingleCoilCb             write_single_coil;
    ModbusWriteSingleRegisterCb         write_single_register;
    ModbusWriteMultipleCoilsCb          write_multiple_coils;
    ModbusWriteMultipleRegistersCb      write_multiple_registers;
    ModbusMaskWriteRegisterCb           mask_write_register;
    ModbusReadWriteMultipleRegistersCb  read_write_multiple_registers;
} ModbusSlaveConfig;
```

## Building and Testing

### Prerequisites

- CMake (>= 3.10)

- GCC or any C99-compatible compiler

- Unity test framework (included)

### Running tests

```bash
# Run all tests
make
```

### Running specific test groups

```bash
# Run only CRC tests
./test_runner -g modbus_crc16

# Run only handler tests
./test_runner -g modbus_handler_read_coils

# Run integration tests
./test_runner -g modbus_integration
```

## Porting Guide (examples based on STM32 family)

### 1. Implement Hardware Abstraction

You need to provide two hardware-specific functions:

#### Transmit Function

```c
void your_transmit_function(const uint8_t *data, uint16_t length) {
    HAL_GPIO_WritePin(RS_DIR_GPIO_Port, RS_DIR_Pin, GPIO_PIN_SET); // Set DE pin
	HAL_UART_Transmit(&huart4, data, length, HAL_MAX_DELAY); // Transmit data
	while (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TC) == RESET); // Wait until transfer completes
	HAL_GPIO_WritePin(RS_DIR_GPIO_Port, RS_DIR_Pin, GPIO_PIN_RESET); // Reset DE pin
}
```

#### Timer Setup

1. Configure a timer to call the timing functions based on your baud rate:

    - `modbus_slave_1_5t_elapsed()` after 1.5 character times

    - `modbus_slave_3_5t_elapsed()` after 3.5 character times

2. Integrate with Your Application

```c
// In UART receive ISR
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance != UART4) return;

	__HAL_TIM_SET_COUNTER(&htim8, 0); // Reset the timer
	timer_counter = 0;

	modbus_slave_rx_byte(&modbus, uart_rx_buffer[0]); // Provide OpenModbus slave with the received byte

	HAL_UART_Receive_IT(&huart4, uart_rx_buffer, 1); // Start listening on the next byte
}

// In timer ISR
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance != TIM8) return;

    if (modbus.state != RECEPTION && modbus.state != CONTROL_AND_WAITING) return; // No need to increment the timer counter if not in RECEPTION or CONTROL_AND_WAITING state

    timer_counter++;

    if (timer_counter == 3) modbus_slave_1_5t_elapsed(&modbus);
    if (timer_counter == 7) modbus_slave_3_5t_elapsed(&modbus);
}
```

## Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

1. Fork the repository

2. Create a feature branch

3. Add tests for new functionality

4. Ensure all tests pass

5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE file](LICENSE) for details.