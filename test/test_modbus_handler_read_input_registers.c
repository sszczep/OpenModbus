#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_read_input_registers);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_input_reg_addr, last_input_reg_count;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_read_input_registers(uint16_t addr, uint16_t count, uint8_t *dest) {
    last_input_reg_addr = addr;
    last_input_reg_count = count;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    if (count > 125) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    
    // Provide some test data
    for (int i = 0; i < count; i++) {
        modbus_be16_set(&dest[i * 2], 2000 + i);
    }
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_read_input_registers) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.read_input_registers = mock_read_input_registers;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_input_reg_addr = last_input_reg_count = 0;
}

TEST_TEAR_DOWN(modbus_handler_read_input_registers) {}

/**
 * Test read input registers handler with valid request
 */
TEST(modbus_handler_read_input_registers, test_handle_read_input_registers_valid) {
    // Build read input registers request: addr 0x0200, count 0x0002
    uint8_t request[] = {0x01, 0x04, 0x02, 0x00, 0x00, 0x02};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_input_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0200, last_input_reg_addr);
    TEST_ASSERT_EQUAL(0x0002, last_input_reg_count);
    TEST_ASSERT_EQUAL(0x04, response[0]); // Function code
    TEST_ASSERT_EQUAL(0x04, response[1]); // Byte count (2 registers = 4 bytes)
    TEST_ASSERT_EQUAL(2000, modbus_be16_get(&response[2])); // Register 1
    TEST_ASSERT_EQUAL(2001, modbus_be16_get(&response[4])); // Register 2
    TEST_ASSERT_EQUAL(6, response_len); // func + byte_count + 4 data bytes
}

/**
 * Test read input registers handler with unsupported function
 */
TEST(modbus_handler_read_input_registers, test_handle_read_input_registers_unsupported) {
    slave.config.read_input_registers = NULL;
    
    uint8_t request[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x02};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_input_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test read input registers handler with invalid register count (too low)
 */
TEST(modbus_handler_read_input_registers, test_handle_read_input_registers_invalid_count_low) {
    uint8_t request[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x00}; // Count = 0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_input_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read input registers handler with invalid register count (too high)
 */
TEST(modbus_handler_read_input_registers, test_handle_read_input_registers_invalid_count_high) {
    uint8_t request[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x7E}; // Count = 126
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_input_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read input registers handler with callback returning address error
 */
TEST(modbus_handler_read_input_registers, test_handle_read_input_registers_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x04, 0x03, 0xE9, 0x00, 0x01}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_input_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}