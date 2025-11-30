#include "modbus_slave.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// READ COILS (Function Code 0x01)
// =============================================================================

/**
 * Handle Read Coils request
 * Reads multiple coil (discrete output) values
 * Request: [Address][0x01][Start Address Hi][Lo][Quantity Hi][Lo]
 * Response: [Address][0x01][Byte Count][Coil Data...]
 */
ModbusExceptionCode handle_read_coils(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.read_coils) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t count = modbus_be16_get(&slave->frame[4]);

    if (count < 0x0001 || count > 0x07D0) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.read_coils(addr, count, &response[2]);
    if (ex != MODBUS_EX_NONE) return ex;

    response[0] = slave->frame[1];
    response[1] = (count + 7) / 8;
    *response_len += 2 + response[1];

    return MODBUS_EX_NONE;
}

// =============================================================================
// READ DISCRETE INPUTS (Function Code 0x02)
// =============================================================================

/**
 * Handle Read Discrete Inputs request
 * Reads multiple discrete input values
 * Request: [Address][0x02][Start Address Hi][Lo][Quantity Hi][Lo]
 * Response: [Address][0x02][Byte Count][Input Data...]
 */
ModbusExceptionCode handle_read_discrete_inputs(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.read_discrete_inputs) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t count = modbus_be16_get(&slave->frame[4]);

    if (count < 0x0001 || count > 0x07D0) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.read_discrete_inputs(addr, count, &response[2]);
    if (ex != MODBUS_EX_NONE) return ex;

    response[0] = slave->frame[1];
    response[1] = (count + 7) / 8;
    *response_len += 2 + response[1];

    return MODBUS_EX_NONE;
}

// =============================================================================
// READ HOLDING REGISTERS (Function Code 0x03)
// =============================================================================

/**
 * Handle Read Holding Registers request
 * Reads multiple 16-bit holding register values
 * Request: [Address][0x03][Start Address Hi][Lo][Quantity Hi][Lo]
 * Response: [Address][0x03][Byte Count][Register Data Hi/Lo...]
 */
ModbusExceptionCode handle_read_holding_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.read_holding_registers) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t count = modbus_be16_get(&slave->frame[4]);

    if (count < 0x0001 || count > 0x007D) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.read_holding_registers(addr, count, &response[2]);
    if (ex != MODBUS_EX_NONE) return ex;

    response[0] = slave->frame[1];
    response[1] = count * 2;
    *response_len += 2 + response[1];

    return MODBUS_EX_NONE;
}

// =============================================================================
// READ INPUT REGISTERS (Function Code 0x04)
// =============================================================================

/**
 * Handle Read Input Registers request
 * Reads multiple 16-bit input register values
 * Request: [Address][0x04][Start Address Hi][Lo][Quantity Hi][Lo]
 * Response: [Address][0x04][Byte Count][Register Data Hi/Lo...]
 */
ModbusExceptionCode handle_read_input_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.read_input_registers) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t count = modbus_be16_get(&slave->frame[4]);

    if (count < 0x0001 || count > 0x007D) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.read_input_registers(addr, count, &response[2]);
    if (ex != MODBUS_EX_NONE) return ex;

    response[0] = slave->frame[1];
    response[1] = count * 2;
    *response_len += 2 + response[1];

    return MODBUS_EX_NONE;
}

// =============================================================================
// WRITE SINGLE COIL (Function Code 0x05)
// =============================================================================

/**
 * Handle Write Single Coil request
 * Writes one coil (discrete output) value
 * Request: [Address][0x05][Coil Address Hi][Lo][Value Hi][Lo]
 * Response: Echo of request
 */
ModbusExceptionCode handle_write_single_coil(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.write_single_coil) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t value = modbus_be16_get(&slave->frame[4]);

    // Validate coil value (should be 0x0000 or 0xFF00 per Modbus spec)
    if (value != 0x0000 && value != 0xFF00) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.write_single_coil(addr, (value == 0xFF00) ? 1 : 0);
    if (ex != MODBUS_EX_NONE) return ex;

    for (int i = 0; i < 4; ++i) response[i] = slave->frame[1 + i];
    *response_len += 4;

    return MODBUS_EX_NONE;
}

// =============================================================================
// WRITE SINGLE REGISTER (Function Code 0x06)
// =============================================================================

/**
 * Handle Write Single Register request
 * Writes one 16-bit holding register
 * Request: [Address][0x06][Register Address Hi][Lo][Value Hi][Lo]
 * Response: Echo of request
 */
ModbusExceptionCode handle_write_single_register(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.write_single_register) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t value = modbus_be16_get(&slave->frame[4]);

    ModbusExceptionCode ex = slave->config.write_single_register(addr, value);
    if (ex != MODBUS_EX_NONE) return ex;

    for (int i = 0; i < 4; ++i) response[i] = slave->frame[1 + i];
    *response_len += 4;

    return MODBUS_EX_NONE;
}

// =============================================================================
// WRITE MULTIPLE COILS (Function Code 0x0F)
// =============================================================================

/**
 * Handle Write Multiple Coils request
 * Writes multiple coil (discrete output) values
 * Request: [Address][0x0F][Start Address Hi][Lo][Quantity Hi][Lo][Byte Count][Coil Data...]
 * Response: [Address][0x0F][Start Address Hi][Lo][Quantity Hi][Lo]
 */
ModbusExceptionCode handle_write_multiple_coils(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.write_multiple_coils) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t count = modbus_be16_get(&slave->frame[4]);
    uint8_t byte_count = slave->frame[6];

    if (count < 0x0001 || count > 0x07B0) return MODBUS_EX_ILLEGAL_DATA_VALUE;
    if (byte_count != (count + 7) / 8) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.write_multiple_coils(addr, count, &slave->frame[7]);
    if (ex != MODBUS_EX_NONE) return ex;

    for (int i = 0; i < 4; ++i) response[i] = slave->frame[1 + i];
    *response_len += 4;

    return MODBUS_EX_NONE;
}

// =============================================================================
// WRITE MULTIPLE REGISTERS (Function Code 0x10)
// =============================================================================

/**
 * Handle Write Multiple Registers request
 * Writes multiple 16-bit holding registers
 * Request: [Address][0x10][Start Address Hi][Lo][Quantity Hi][Lo][Byte Count][Data...]
 * Response: [Address][0x10][Start Address Hi][Lo][Quantity Hi][Lo]
 */
ModbusExceptionCode handle_write_multiple_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.write_multiple_registers) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t count = modbus_be16_get(&slave->frame[4]);
    uint8_t byte_count = slave->frame[6];

    if (count < 0x0001 || count > 0x007B) return MODBUS_EX_ILLEGAL_DATA_VALUE;
    if (byte_count != count * 2) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.write_multiple_registers(addr, count, &slave->frame[7]);
    if (ex != MODBUS_EX_NONE) return ex;

    for (int i = 0; i < 4; ++i) response[i] = slave->frame[1 + i];
    *response_len += 4;

    return MODBUS_EX_NONE;
}

// =============================================================================
// MASK WRITE REGISTER (Function Code 0x16)
// =============================================================================

/**
 * Handle Mask Write Register request
 * Modifies specific bits in a holding register using AND/OR masks
 * Request: [Address][0x16][Register Address Hi][Lo][AND Mask Hi][Lo][OR Mask Hi][Lo]
 * Response: Echo of request
 */
ModbusExceptionCode handle_mask_write_register(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.mask_write_register) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t addr = modbus_be16_get(&slave->frame[2]);
    uint16_t and_mask = modbus_be16_get(&slave->frame[4]);
    uint16_t or_mask = modbus_be16_get(&slave->frame[6]);

    ModbusExceptionCode ex = slave->config.mask_write_register(addr, and_mask, or_mask);
    if (ex != MODBUS_EX_NONE) return ex;

    for (int i = 0; i < 6; ++i) response[i] = slave->frame[1 + i];
    *response_len += 6;

    return MODBUS_EX_NONE;
}

// =============================================================================
// READ/WRITE MULTIPLE REGISTERS (Function Code 0x17)
// =============================================================================

/**
 * Handle Read/Write Multiple Registers request
 * Performs a write operation followed by a read operation in a single request
 * Request: [Address][0x17][Read Address Hi][Lo][Read Quantity Hi][Lo]
 *          [Write Address Hi][Lo][Write Quantity Hi][Lo][Write Byte Count][Write Data...]
 * Response: [Address][0x17][Byte Count][Read Register Data Hi/Lo...]
 */
ModbusExceptionCode handle_read_write_multiple_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len) {
    if (!slave->config.read_write_multiple_registers) return MODBUS_EX_ILLEGAL_FUNCTION;

    uint16_t read_addr = modbus_be16_get(&slave->frame[2]);
    uint16_t read_count = modbus_be16_get(&slave->frame[4]);
    uint16_t write_addr = modbus_be16_get(&slave->frame[6]);
    uint16_t write_count = modbus_be16_get(&slave->frame[8]);
    uint8_t write_byte_count = slave->frame[10];

    if (read_count < 0x0001 || read_count > 0x007D) return MODBUS_EX_ILLEGAL_DATA_VALUE;
    if (write_count < 0x0001 || write_count > 0x0079) return MODBUS_EX_ILLEGAL_DATA_VALUE;
    if (write_byte_count != write_count * 2) return MODBUS_EX_ILLEGAL_DATA_VALUE;

    ModbusExceptionCode ex = slave->config.read_write_multiple_registers(
        read_addr, read_count, write_addr, write_count, 
        &slave->frame[11], &response[2]
    );
    if (ex != MODBUS_EX_NONE) return ex;

    response[0] = slave->frame[1];
    response[1] = read_count * 2;
    *response_len += 2 + response[1];

    return MODBUS_EX_NONE;
}