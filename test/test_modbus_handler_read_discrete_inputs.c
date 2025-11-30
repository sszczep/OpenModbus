#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_read_discrete_inputs);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_input_addr, last_input_count;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_read_discrete_inputs(uint16_t addr, uint16_t count, uint8_t *dest) {
    last_input_addr = addr;
    last_input_count = count;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    if (count > 200) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    
    // Provide some test data
    for (int i = 0; i < (count + 7) / 8; i++) {
        dest[i] = 0x55; // 01010101 pattern
    }
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_read_discrete_inputs) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.read_discrete_inputs = mock_read_discrete_inputs;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_input_addr = last_input_count = 0;
}

TEST_TEAR_DOWN(modbus_handler_read_discrete_inputs) {}

/**
 * Test read discrete inputs handler with valid request
 */
TEST(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_valid) {
    // Build read discrete inputs request: addr 0x0100, count 0x0010
    uint8_t request[] = {0x01, 0x02, 0x01, 0x00, 0x00, 0x10};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_discrete_inputs(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0100, last_input_addr);
    TEST_ASSERT_EQUAL(0x0010, last_input_count);
    TEST_ASSERT_EQUAL(0x02, response[0]); // Function code
    TEST_ASSERT_EQUAL(0x02, response[1]); // Byte count (16 inputs = 2 bytes)
    TEST_ASSERT_EQUAL(0x55, response[2]); // Input data byte 1
    TEST_ASSERT_EQUAL(0x55, response[3]); // Input data byte 2
    TEST_ASSERT_EQUAL(4, response_len); // func + byte_count + 2 data bytes
}

/**
 * Test read discrete inputs handler with unsupported function
 */
TEST(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_unsupported) {
    slave.config.read_discrete_inputs = NULL;
    
    uint8_t request[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x10};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_discrete_inputs(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test read discrete inputs handler with invalid input count (too low)
 */
TEST(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_invalid_count_low) {
    uint8_t request[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00}; // Count = 0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_discrete_inputs(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read discrete inputs handler with invalid input count (too high)
 */
TEST(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_invalid_count_high) {
    uint8_t request[] = {0x01, 0x02, 0x00, 0x00, 0x07, 0xD1}; // Count = 2001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_discrete_inputs(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read discrete inputs handler with callback returning address error
 */
TEST(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x02, 0x03, 0xE9, 0x00, 0x01}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_discrete_inputs(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}