#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_write_multiple_coils);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_multi_coil_addr, last_multi_coil_count;
static const uint8_t* last_multi_coil_data;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_write_multiple_coils(uint16_t addr, uint16_t count, const uint8_t *src) {
    last_multi_coil_addr = addr;
    last_multi_coil_count = count;
    last_multi_coil_data = src;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    if (count > 100) return MODBUS_EX_ILLEGAL_DATA_VALUE;
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_write_multiple_coils) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.write_multiple_coils = mock_write_multiple_coils;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_multi_coil_addr = last_multi_coil_count = 0;
    last_multi_coil_data = NULL;
}

TEST_TEAR_DOWN(modbus_handler_write_multiple_coils) {}

/**
 * Test write multiple coils handler with valid request
 */
TEST(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_valid) {
    // Build write multiple coils request: addr 0x0100 (256), count 0x0010, data 0x12, 0x34
    uint8_t request[] = {0x01, 0x0F, 0x01, 0x00, 0x00, 0x10, 0x02, 0x12, 0x34};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0100, last_multi_coil_addr);
    TEST_ASSERT_EQUAL(0x0010, last_multi_coil_count);
    TEST_ASSERT_NOT_NULL(last_multi_coil_data);
    TEST_ASSERT_EQUAL(0x12, last_multi_coil_data[0]);
    TEST_ASSERT_EQUAL(0x34, last_multi_coil_data[1]);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&request[1], response, 4);
    TEST_ASSERT_EQUAL(4, response_len);
}

/**
 * Test write multiple coils handler with unsupported function
 */
TEST(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_unsupported) {
    slave.config.write_multiple_coils = NULL;
    
    uint8_t request[] = {0x01, 0x0F, 0x04, 0x00, 0x00, 0x10, 0x02, 0x12, 0x34};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test write multiple coils handler with invalid byte count
 */
TEST(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_invalid_byte_count) {
    // Incorrect byte count (should be 2 for 16 coils, but we have 3)
    uint8_t request[] = {0x01, 0x0F, 0x04, 0x00, 0x00, 0x10, 0x03, 0x12, 0x34, 0x56};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write multiple coils handler with invalid coil count (too low)
 */
TEST(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_invalid_count_low) {
    uint8_t request[] = {0x01, 0x0F, 0x04, 0x00, 0x00, 0x00, 0x00}; // Count = 0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write multiple coils handler with invalid coil count (too high)
 */
TEST(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_invalid_count_high) {
    uint8_t request[] = {0x01, 0x0F, 0x04, 0x00, 0x07, 0xB1, 0x00}; // Count = 1969
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test write multiple coils handler with callback returning address error
 */
TEST(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x0F, 0x03, 0xE9, 0x00, 0x10, 0x02, 0x12, 0x34}; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_write_multiple_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}