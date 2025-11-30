#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_mask_write_register);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_mask_addr, last_and_mask, last_or_mask;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_mask_write_register(uint16_t addr, uint16_t and_mask, uint16_t or_mask) {
    last_mask_addr = addr;
    last_and_mask = and_mask;
    last_or_mask = or_mask;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_mask_write_register) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.mask_write_register = mock_mask_write_register;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_mask_addr = last_and_mask = last_or_mask = 0;
}

TEST_TEAR_DOWN(modbus_handler_mask_write_register) {}

/**
 * Test mask write register handler with valid request
 */
TEST(modbus_handler_mask_write_register, test_handle_mask_write_register_valid) {
    // Build mask write register request: addr 0x0100 (256), AND mask 0x00FF, OR mask 0x1234
    uint8_t request[] = {0x01, 0x16, 0x01, 0x00, 0x00, 0xFF, 0x12, 0x34};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_mask_write_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0100, last_mask_addr);
    TEST_ASSERT_EQUAL(0x00FF, last_and_mask);
    TEST_ASSERT_EQUAL(0x1234, last_or_mask);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 6);
    TEST_ASSERT_EQUAL(6, response_len);
}

/**
 * Test mask write register handler with unsupported function
 */
TEST(modbus_handler_mask_write_register, test_handle_mask_write_register_unsupported) {
    slave.config.mask_write_register = NULL;
    
    uint8_t request[] = {0x01, 0x16, 0x05, 0x00, 0x00, 0xFF, 0x12, 0x34};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_mask_write_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test mask write register handler with callback returning address error
 */
TEST(modbus_handler_mask_write_register, test_handle_mask_write_register_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x16, 0x03, 0xE9, 0x00, 0xFF, 0x12, 0x34}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_mask_write_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}

/**
 * Test mask write register handler with zero masks
 */
TEST(modbus_handler_mask_write_register, test_handle_mask_write_register_zero_masks) {
    uint8_t request[] = {0x01, 0x16, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00}; // AND=0, OR=0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_mask_write_register(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0100, last_mask_addr);
    TEST_ASSERT_EQUAL(0x0000, last_and_mask);
    TEST_ASSERT_EQUAL(0x0000, last_or_mask);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 6);
}