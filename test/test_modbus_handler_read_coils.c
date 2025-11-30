#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_read_coils);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_coil_addr, last_coil_count;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_read_coils(uint16_t addr, uint16_t count, uint8_t *dest) {
    last_coil_addr = addr;
    last_coil_count = count;
    
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    if (count > 200) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    
    // Provide some test data
    for (int i = 0; i < (count + 7) / 8; i++) {
        dest[i] = 0xAA; // 10101010 pattern
    }
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_read_coils) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.read_coils = mock_read_coils;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_coil_addr = last_coil_count = 0;
}

TEST_TEAR_DOWN(modbus_handler_read_coils) {}

/**
 * Test read coils handler with valid request
 */
TEST(modbus_handler_read_coils, test_handle_read_coils_valid) {
    // Build read coils request: addr 0x0100, count 0x0010
    uint8_t request[] = {0x01, 0x01, 0x01, 0x00, 0x00, 0x10};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0100, last_coil_addr);
    TEST_ASSERT_EQUAL(0x0010, last_coil_count);
    TEST_ASSERT_EQUAL(0x01, response[0]); // Function code
    TEST_ASSERT_EQUAL(0x02, response[1]); // Byte count (16 coils = 2 bytes)
    TEST_ASSERT_EQUAL(0xAA, response[2]); // Coil data byte 1
    TEST_ASSERT_EQUAL(0xAA, response[3]); // Coil data byte 2
    TEST_ASSERT_EQUAL(4, response_len); // func + byte_count + 2 data bytes
}

/**
 * Test read coils handler with unsupported function
 */
TEST(modbus_handler_read_coils, test_handle_read_coils_unsupported) {
    slave.config.read_coils = NULL;
    
    uint8_t request[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x10};
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test read coils handler with invalid coil count (too low)
 */
TEST(modbus_handler_read_coils, test_handle_read_coils_invalid_count_low) {
    uint8_t request[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x00}; // Count = 0
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read coils handler with invalid coil count (too high)
 */
TEST(modbus_handler_read_coils, test_handle_read_coils_invalid_count_high) {
    uint8_t request[] = {0x01, 0x01, 0x00, 0x00, 0x07, 0xD1}; // Count = 2001
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read coils handler with callback returning address error
 */
TEST(modbus_handler_read_coils, test_handle_read_coils_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {0x01, 0x01, 0x03, 0xE8, 0x00, 0x01}; // Addr = 1000
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_coils(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    
    // Now test with addr > 1000
    request[2] = 0x03; request[3] = 0xE9; // Addr = 1001
    memcpy(slave.frame, request, sizeof(request));
    
    result = handle_read_coils(&slave, response, &response_len);
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}