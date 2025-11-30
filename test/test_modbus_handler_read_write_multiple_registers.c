#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_slave_handlers.h"

#include <string.h>

TEST_GROUP(modbus_handler_read_write_multiple_registers);

static ModbusSlave slave;
static ModbusSlaveConfig config;

// Mock callbacks
static uint16_t last_read_addr, last_read_count, last_write_addr, last_write_count;
static const uint8_t* last_write_data;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for tests
    (void)(data);
    (void)(length);
}

static ModbusExceptionCode mock_read_write_multiple_registers(
    uint16_t read_addr, uint16_t read_count,
    uint16_t write_addr, uint16_t write_count,
    const uint8_t *write_data, uint8_t *read_data) {
    
    last_read_addr = read_addr;
    last_read_count = read_count;
    last_write_addr = write_addr;
    last_write_count = write_count;
    last_write_data = write_data;
    
    if (read_addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    if (write_addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    
    // Provide some test read data
    for (int i = 0; i < read_count; i++) {
        modbus_be16_set(&read_data[i * 2], 3000 + i);
    }
    
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_handler_read_write_multiple_registers) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.read_write_multiple_registers = mock_read_write_multiple_registers;
    config.write = mock_write;
    
    modbus_slave_init(&slave, &config);
    
    // Reset test variables
    last_read_addr = last_read_count = last_write_addr = last_write_count = 0;
    last_write_data = NULL;
}

TEST_TEAR_DOWN(modbus_handler_read_write_multiple_registers) {}

/**
 * Test read/write multiple registers handler with valid request
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_valid) {
    // Build read/write multiple registers request: 
    // read addr 0x0100 (256), read count 0x0002
    // write addr 0x0200 (512), write count 0x0002, write data 0x1234, 0x5678
    uint8_t request[] = {
        0x01, 0x17, 
        0x01, 0x00, 0x00, 0x02, // Read: addr=0x0100, count=2
        0x02, 0x00, 0x00, 0x02, // Write: addr=0x0200, count=2
        0x04, 0x12, 0x34, 0x56, 0x78 // Byte count=4, data
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_NONE, result);
    TEST_ASSERT_EQUAL(0x0100, last_read_addr);
    TEST_ASSERT_EQUAL(0x0002, last_read_count);
    TEST_ASSERT_EQUAL(0x0200, last_write_addr);
    TEST_ASSERT_EQUAL(0x0002, last_write_count);
    TEST_ASSERT_NOT_NULL(last_write_data);
    TEST_ASSERT_EQUAL(0x1234, modbus_be16_get(&last_write_data[0]));
    TEST_ASSERT_EQUAL(0x5678, modbus_be16_get(&last_write_data[2]));
    TEST_ASSERT_EQUAL(0x17, response[0]); // Function code
    TEST_ASSERT_EQUAL(0x04, response[1]); // Byte count (2 registers = 4 bytes)
    TEST_ASSERT_EQUAL(3000, modbus_be16_get(&response[2])); // Read data
    TEST_ASSERT_EQUAL(3001, modbus_be16_get(&response[4]));
    TEST_ASSERT_EQUAL(6, response_len); // func + byte_count + 4 data bytes
}

/**
 * Test read/write multiple registers handler with unsupported function
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_unsupported) {
    slave.config.read_write_multiple_registers = NULL;
    
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x02,
        0x07, 0x00, 0x00, 0x02,
        0x04, 0x12, 0x34, 0x56, 0x78
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_FUNCTION, result);
}

/**
 * Test read/write multiple registers handler with invalid read count (too low)
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_read_count_low) {
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x00, // Read count = 0 (invalid)
        0x07, 0x00, 0x00, 0x02,
        0x04, 0x12, 0x34, 0x56, 0x78
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read/write multiple registers handler with invalid read count (too high)
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_read_count_high) {
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x7E, // Read count = 126 (invalid)
        0x07, 0x00, 0x00, 0x02,
        0x04, 0x12, 0x34, 0x56, 0x78
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read/write multiple registers handler with invalid write count (too low)
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_write_count_low) {
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x02,
        0x07, 0x00, 0x00, 0x00, // Write count = 0 (invalid)
        0x00 // Byte count = 0
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read/write multiple registers handler with invalid write count (too high)
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_write_count_high) {
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x02,
        0x07, 0x00, 0x00, 0x7A, // Write count = 122 (invalid)
        0xF4, // Byte count = 244
        0x00 // Would need 244 bytes of data, but we don't have it
    };
    memcpy(slave.frame, request, 12); // Only copy header
    slave.frame_len = 12;
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read/write multiple registers handler with invalid byte count
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_byte_count) {
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x02,
        0x07, 0x00, 0x00, 0x02,
        0x03, 0x12, 0x34, 0x56 // Byte count should be 4, but we have 3
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_VALUE, result);
}

/**
 * Test read/write multiple registers handler with callback returning address error (read address)
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_read_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {
        0x01, 0x17, 
        0x03, 0xE9, 0x00, 0x02, // Read addr = 1001
        0x07, 0x00, 0x00, 0x02,
        0x04, 0x12, 0x34, 0x56, 0x78
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}

/**
 * Test read/write multiple registers handler with callback returning address error (write address)
 */
TEST(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_write_address_error) {
    // Mock will return address error for addr > 1000
    uint8_t request[] = {
        0x01, 0x17, 
        0x06, 0x00, 0x00, 0x02,
        0x03, 0xE9, 0x00, 0x02, // Write addr = 1001
        0x04, 0x12, 0x34, 0x56, 0x78
    };
    memcpy(slave.frame, request, sizeof(request));
    slave.frame_len = sizeof(request);
    
    uint8_t response[256];
    uint16_t response_len = 0;
    ModbusExceptionCode result = handle_read_write_multiple_registers(&slave, response, &response_len);
    
    TEST_ASSERT_EQUAL(MODBUS_EX_ILLEGAL_DATA_ADDRESS, result);
}