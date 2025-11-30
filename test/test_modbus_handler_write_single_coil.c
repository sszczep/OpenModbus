#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_write_single_coil);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_coil_addr;
static uint8_t last_coil_value;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_write_single_coil(uint16_t addr, uint8_t value) {
    last_coil_addr = addr;
    last_coil_value = value;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_write_single_coil) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.write_single_coil = mock_write_single_coil;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_coil_addr = 0;
    last_coil_value = 0;
}

TEST_TEAR_DOWN(modbus_handler_write_single_coil) {}

/**
 * Test write single coil handler with valid request (ON)
 */
TEST(modbus_handler_write_single_coil, test_handle_write_single_coil_valid_on) {
    // Build write single coil request: addr 0x0300, value ON (0xFF00)
    uint8_t request[] = {0x01, 0x05, 0x03, 0x00, 0xFF, 0x00};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_coil(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0300, last_coil_addr);
    TEST_ASSERT_EQUAL(1, last_coil_value); // Should be converted to 1
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 4);
    TEST_ASSERT_EQUAL(4, response_len);
}

/**
 * Test write single coil handler with valid request (OFF)
 */
TEST(modbus_handler_write_single_coil, test_handle_write_single_coil_valid_off) {
    // Build write single coil request: addr 0x0300, value OFF (0x0000)
    uint8_t request[] = {0x01, 0x05, 0x03, 0x00, 0x00, 0x00};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_coil(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0300, last_coil_addr);
    TEST_ASSERT_EQUAL(0, last_coil_value); // Should be converted to 0
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 4);
    TEST_ASSERT_EQUAL(4, response_len);
}

/**
 * Test write single coil handler with unsupported function
 */
TEST(modbus_handler_write_single_coil, test_handle_write_single_coil_unsupported) {
    slave.config.write_single_coil = NULL;
    
    uint8_t request[] = {0x01, 0x05, 0x03, 0x00, 0xFF, 0x00};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_coil(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test write single coil handler with invalid coil value
 */
TEST(modbus_handler_write_single_coil, test_handle_write_single_coil_invalid_value) {
    // Invalid coil value (should be 0x0000 or 0xFF00)
    uint8_t request[] = {0x01, 0x05, 0x03, 0x00, 0x12, 0x34}; // Invalid value
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_coil(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write single coil handler with callback returning address error
 */
TEST(modbus_handler_write_single_coil, test_handle_write_single_coil_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x05, 0x03, 0xE9, 0xFF, 0x00}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_single_coil(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}