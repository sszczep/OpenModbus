#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_write_multiple_registers);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_multi_write_addr, last_multi_write_count;
static const uint8_t* last_multi_write_data;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_write_multiple_registers(uint16_t addr, uint16_t count, const uint8_t *src) {
    last_multi_write_addr = addr;
    last_multi_write_count = count;
    last_multi_write_data = src;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    if (count > 100) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_write_multiple_registers) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.write_multiple_registers = mock_write_multiple_registers;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_multi_write_addr = last_multi_write_count = 0;
    last_multi_write_data = NULL;
}

TEST_TEAR_DOWN(modbus_handler_write_multiple_registers) {}

/**
 * Test write multiple registers handler with valid request
 */
TEST(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_valid) {
    // Build write multiple registers request: addr 0x0064 (100), count 0x0002, data 0x1234, 0x5678
    uint8_t request[] = {0x01, 0x10, 0x00, 0x64, 0x00, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78}; // Addr = 100
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0064, last_multi_write_addr);
    TEST_ASSERT_EQUAL(0x0002, last_multi_write_count);
    TEST_ASSERT_NOT_NULL(last_multi_write_data);
    // Verify the data passed to callback
    TEST_ASSERT_EQUAL(0x1234, modbus_be16_get(&last_multi_write_data[0]));
    TEST_ASSERT_EQUAL(0x5678, modbus_be16_get(&last_multi_write_data[2]));
    // Response should echo address and count
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 4);
    TEST_ASSERT_EQUAL(4, response_len);
}

/**
 * Test write multiple registers handler with unsupported function
 */
TEST(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_unsupported) {
    slave.config.write_multiple_registers = NULL;
    
    uint8_t request[] = {0x01, 0x10, 0x04, 0x00, 0x00, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test write multiple registers handler with invalid byte count
 */
TEST(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_invalid_byte_count) {
    // Build request with incorrect byte count (should be 4 for 2 registers, but we have 3)
    uint8_t request[] = {0x01, 0x10, 0x04, 0x00, 0x00, 0x02, 0x03, 0x12, 0x34, 0x56};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write multiple registers handler with invalid register count (too low)
 */
TEST(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_invalid_count_low) {
    uint8_t request[] = {0x01, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00}; // Count = 0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write multiple registers handler with invalid register count (too high)
 */
TEST(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_invalid_count_high) {
    uint8_t request[] = {0x01, 0x10, 0x04, 0x00, 0x00, 0x7C, 0x00}; // Count = 124
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write multiple registers handler with callback returning address error
 */
TEST(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x10, 0x03, 0xE9, 0x00, 0x01, 0x02, 0x12, 0x34}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}