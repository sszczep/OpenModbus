#include "unity_fixture.h"
#include "modbus_slave.h"

#include <string.h>

TEST_GROUP(modbus_slave_rx);

static ModbusSlave slave;
static ModbusSlaveConfig config;

static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for RX tests
    (void)(data);
    (void)(length);
}

TEST_SETUP(modbus_slave_rx) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.write = mock_write;
    modbus_slave_init(&slave, &config);
}

TEST_TEAR_DOWN(modbus_slave_rx) {}

/**
 * Test receiving first byte transitions from IDLE to RECEPTION
 */
TEST(modbus_slave_rx, test_rx_first_byte_transition) {
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    
    modbus_slave_rx_byte(&slave, 0x01);
    
    TEST_ASSERT_EQUAL(RECEPTION, slave.state);
    TEST_ASSERT_EQUAL(1, slave.frame_len);
    TEST_ASSERT_EQUAL(0x01, slave.frame[0]);
    TEST_ASSERT_TRUE(slave.frame_ok);
}

/**
 * Test receiving multiple bytes in reception state
 */
TEST(modbus_slave_rx, test_rx_multiple_bytes) {
    modbus_slave_rx_byte(&slave, 0x01);
    modbus_slave_rx_byte(&slave, 0x03);
    modbus_slave_rx_byte(&slave, 0x00);
    modbus_slave_rx_byte(&slave, 0x00);
    
    TEST_ASSERT_EQUAL(RECEPTION, slave.state);
    TEST_ASSERT_EQUAL(4, slave.frame_len);
    TEST_ASSERT_EQUAL(0x01, slave.frame[0]);
    TEST_ASSERT_EQUAL(0x03, slave.frame[1]);
    TEST_ASSERT_EQUAL(0x00, slave.frame[2]);
    TEST_ASSERT_EQUAL(0x00, slave.frame[3]);
}

/**
 * Test frame overflow protection
 */
TEST(modbus_slave_rx, test_rx_frame_overflow) {
    // Fill buffer to maximum
    for (int i = 0; i < MODBUS_MAX_FRAME_LENGTH; i++) {
        modbus_slave_rx_byte(&slave, i & 0xFF);
    }
    
    TEST_ASSERT_EQUAL(MODBUS_MAX_FRAME_LENGTH, slave.frame_len);
    TEST_ASSERT_TRUE(slave.frame_ok);
    
    // One more byte should cause overflow
    modbus_slave_rx_byte(&slave, 0xFF);
    
    TEST_ASSERT_FALSE(slave.frame_ok);
    TEST_ASSERT_EQUAL(CONTROL_AND_WAITING, slave.state);
}

/**
 * Test ignoring RX when processing frame
 */
TEST(modbus_slave_rx, test_rx_ignore_during_processing) {
    slave.processing_frame = true;
    
    modbus_slave_rx_byte(&slave, 0x01);
    
    // Should be ignored - no state change or data stored
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    TEST_ASSERT_EQUAL(0, slave.frame_len);
}

/**
 * Test 1.5t timer transitions from RECEPTION to CONTROL_AND_WAITING
 */
TEST(modbus_slave_rx, test_1_5t_timer_transition) {
    modbus_slave_rx_byte(&slave, 0x01);
    TEST_ASSERT_EQUAL(RECEPTION, slave.state);
    
    modbus_slave_1_5t_elapsed(&slave);
    TEST_ASSERT_EQUAL(CONTROL_AND_WAITING, slave.state);
}

/**
 * Test 1.5t timer ignores other states
 */
TEST(modbus_slave_rx, test_1_5t_timer_ignore_other_states) {
    slave.state = IDLE;
    modbus_slave_1_5t_elapsed(&slave);
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    
    slave.state = CONTROL_AND_WAITING;
    modbus_slave_1_5t_elapsed(&slave);
    TEST_ASSERT_EQUAL(CONTROL_AND_WAITING, slave.state);
}

/**
 * Test 3.5t timer transitions to IDLE and marks frame available
 */
TEST(modbus_slave_rx, test_3_5t_timer_valid_frame) {
    slave.state = CONTROL_AND_WAITING;
    slave.frame_ok = true;
    
    modbus_slave_3_5t_elapsed(&slave);
    
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    TEST_ASSERT_TRUE(slave.frame_available);
}

/**
 * Test 3.5t timer with invalid frame
 */
TEST(modbus_slave_rx, test_3_5t_timer_invalid_frame) {
    slave.state = CONTROL_AND_WAITING;
    slave.frame_ok = false;
    
    modbus_slave_3_5t_elapsed(&slave);
    
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    TEST_ASSERT_FALSE(slave.frame_available);
}

/**
 * Test 3.5t timer ignores other states
 */
TEST(modbus_slave_rx, test_3_5t_timer_ignore_other_states) {
    slave.state = IDLE;
    slave.frame_available = false;
    modbus_slave_3_5t_elapsed(&slave);
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    TEST_ASSERT_FALSE(slave.frame_available);
    
    slave.state = RECEPTION;
    modbus_slave_3_5t_elapsed(&slave);
    TEST_ASSERT_EQUAL(RECEPTION, slave.state);
    TEST_ASSERT_FALSE(slave.frame_available);
}