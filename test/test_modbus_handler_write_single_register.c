#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_write_single_register);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_write_addr, last_write_value;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_write_single_register(uint16_t addr, uint16_t value) {
    last_write_addr = addr;
    last_write_value = value;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_write_single_register) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.write_single_register = mock_write_single_register;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_write_addr = last_write_value = 0;
}

TEST_TEAR_DOWN(modbus_handler_write_single_register) {}

/**
 * Test write single register handler with valid request
 */
TEST(modbus_handler_write_single_register, test_handle_write_single_register_valid) {
    // Build write single register request: addr 0x0300, value 0x1234
    uint8_t request[] = {0x01, 0x06, 0x03, 0x00, 0x12, 0x34};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0300, last_write_addr);
    TEST_ASSERT_EQUAL(0x1234, last_write_value);
    // Response should echo request
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 4);
    TEST_ASSERT_EQUAL(4, response_len);
}

/**
 * Test write single register handler with unsupported function
 */
TEST(modbus_handler_write_single_register, test_handle_write_single_register_unsupported) {
    slave.config.write_single_register = NULL;
    
    uint8_t request[] = {0x01, 0x06, 0x03, 0x00, 0x12, 0x34};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test write single register handler with callback returning address error
 */
TEST(modbus_handler_write_single_register, test_handle_write_single_register_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x06, 0x03, 0xE9, 0x12, 0x34}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}

/**
 * Test write single register handler with zero address and value
 */
TEST(modbus_handler_write_single_register, test_handle_write_single_register_zero_values) {
    uint8_t request[] = {0x01, 0x06, 0x00, 0x00, 0x00, 0x00}; // Addr = 0, Value = 0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0000, last_write_addr);
    TEST_ASSERT_EQUAL(0x0000, last_write_value);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 4);
}