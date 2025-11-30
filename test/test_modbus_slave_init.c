#include "unity_fixture.h"
#include "modbus_slave.h"

#include <string.h>

TEST_GROUP(modbus_slave_init);

static ModbusSlave slave;
static ModbusSlaveConfig config;

/**
 * Mock transmit function
 */
static void mock_write(const uint8_t *data, uint16_t length) {
    // Do nothing for init tests
    (void)(data);
    (void)(length);
}

TEST_SETUP(modbus_slave_init) {
    memset(&slave, 0, sizeof(slave));
    memset(&config, 0, sizeof(config));
    config.address = 0x01;
    config.write = mock_write;
}

TEST_TEAR_DOWN(modbus_slave_init) {}

/**
 * Test successful slave initialization
 */
TEST(modbus_slave_init, test_slave_init_success) {
    int result = modbus_slave_init(&slave, &config);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(config.address, slave.config.address);
    TEST_ASSERT_EQUAL(IDLE, slave.state);
    TEST_ASSERT_EQUAL(0, slave.frame_len);
    TEST_ASSERT_TRUE(slave.frame_ok);
    TEST_ASSERT_FALSE(slave.frame_available);
    TEST_ASSERT_FALSE(slave.processing_frame);
}

/**
 * Test initialization with null slave pointer
 */
TEST(modbus_slave_init, test_slave_init_null_slave) {
    int result = modbus_slave_init(NULL, &config);
    TEST_ASSERT_EQUAL(-1, result);
}

/**
 * Test initialization with null config pointer
 */
TEST(modbus_slave_init, test_slave_init_null_config) {
    int result = modbus_slave_init(&slave, NULL);
    TEST_ASSERT_EQUAL(-1, result);
}

/**
 * Test initialization with null write function
 */
TEST(modbus_slave_init, test_slave_init_null_write_function) {
    ModbusSlaveConfig bad_config = config;
    bad_config.write = NULL;
    
    int result = modbus_slave_init(&slave, &bad_config);
    TEST_ASSERT_EQUAL(-1, result);
}

/**
 * Test initialization preserves provided configuration
 */
TEST(modbus_slave_init, test_slave_init_preserves_config) {
    config.read_coils = (ModbusReadCoilsCb)0x12345678;
    config.write_single_register = (ModbusWriteSingleRegisterCb)0x87654321;
    
    int result = modbus_slave_init(&slave, &config);
    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_PTR(config.read_coils, slave.config.read_coils);
    TEST_ASSERT_EQUAL_PTR(config.write_single_register, slave.config.write_single_register);
}