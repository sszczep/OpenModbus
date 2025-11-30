#include "unity_fixture.h"
#include "modbus_slave.h"
#include "modbus_crc16.h"

#include <string.h>

TEST_GROUP(modbus_integration);

static ModbusSlave slave;
static ModbusSlaveConfig config;

static uint8_t last_transmitted_data[256];
static uint16_t last_transmitted_len;
static bool transmit_called;

static void mock_write(const uint8_t *data, uint16_t length) {
    memcpy(last_transmitted_data, data, length);
    last_transmitted_len = length;
    transmit_called = true;
}

static ModbusExceptionCode mock_read_holding_registers(uint16_t addr, uint16_t count, uint8_t *dest) {
    if (addr > 1000) return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
    
    for (int i = 0; i < count; i++) {
        modbus_be16_set(&dest[i * 2], 500 + i);
    }
    return MODBUS_EX_NONE;
}

TEST_SETUP(modbus_integration) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    memset(last_transmitted_data, 0, sizeof(last_transmitted_data));
    last_transmitted_len = 0;
    transmit_called = false;
    
    config.address = 0x01;
    config.write = mock_write;
    config.read_holding_registers = mock_read_holding_registers;
    
    modbus_slave_init(&slave, &config);
}

TEST_TEAR_DOWN(modbus_integration) {
}

/**
 * Test complete frame processing with valid request
 */
TEST(modbus_integration, test_complete_frame_processing) {
    // Build valid read holding registers request with CRC
    uint8_t request[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    uint16_t crc = modbus_crc16(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    // Simulate receiving the frame byte by byte
    for (int i = 0; i < 8; i++) {
        modbus_slave_rx_byte(&slave, request[i]);
    }
    
    // Simulate timer events
    modbus_slave_1_5t_elapsed(&slave);
    modbus_slave_3_5t_elapsed(&slave);
    
    // Process the frame
    TEST_ASSERT_TRUE(slave.frame_available);
    modbus_slave_poll(&slave);
    
    // Verify response was sent
    TEST_ASSERT_TRUE(transmit_called);
    TEST_ASSERT_EQUAL(0x01, last_transmitted_data[0]); // Address
    TEST_ASSERT_EQUAL(0x03, last_transmitted_data[1]); // Function code
    TEST_ASSERT_EQUAL(0x04, last_transmitted_data[2]); // Byte count
}

/**
 * Test frame with invalid CRC is ignored
 */
TEST(modbus_integration, test_frame_invalid_crc) {
    // Build request with bad CRC
    uint8_t request[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0x12, 0x34}; // Wrong CRC
    
    for (int i = 0; i < 8; i++) {
        modbus_slave_rx_byte(&slave, request[i]);
    }
    
    modbus_slave_1_5t_elapsed(&slave);
    modbus_slave_3_5t_elapsed(&slave);
    
    TEST_ASSERT_TRUE(slave.frame_available);
    
    transmit_called = false;
    modbus_slave_poll(&slave);
    
    // No response should be sent for invalid frame
    TEST_ASSERT_FALSE(transmit_called);
}

/**
 * Test broadcast frame generates no response
 */
TEST(modbus_integration, test_broadcast_frame_no_response) {
    // Build broadcast request (address 0)
    uint8_t request[8] = {0x00, 0x03, 0x00, 0x00, 0x00, 0x02};
    uint16_t crc = modbus_crc16(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    for (int i = 0; i < 8; i++) {
        modbus_slave_rx_byte(&slave, request[i]);
    }
    
    modbus_slave_1_5t_elapsed(&slave);
    modbus_slave_3_5t_elapsed(&slave);
    
    TEST_ASSERT_TRUE(slave.frame_available);
    
    transmit_called = false;
    modbus_slave_poll(&slave);
    
    // No response for broadcast
    TEST_ASSERT_FALSE(transmit_called);
}

/**
 * Test wrong address frame is ignored
 */
TEST(modbus_integration, test_wrong_address_frame) {
    // Build request for different address
    uint8_t request[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x02}; // Address 0x02, we're 0x01
    uint16_t crc = modbus_crc16(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    for (int i = 0; i < 8; i++) {
        modbus_slave_rx_byte(&slave, request[i]);
    }
    
    modbus_slave_1_5t_elapsed(&slave);
    modbus_slave_3_5t_elapsed(&slave);
    
    TEST_ASSERT_TRUE(slave.frame_available);
    
    transmit_called = false;
    modbus_slave_poll(&slave);
    
    // No response for wrong address
    TEST_ASSERT_FALSE(transmit_called);
}