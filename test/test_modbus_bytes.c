#include "unity_fixture.h"
#include "modbus_bytes.h"

TEST_GROUP(modbus_bytes);

TEST_SETUP(modbus_bytes) {}

TEST_TEAR_DOWN(modbus_bytes) {}

/**
 * Test big-endian 16-bit get/set
 */
TEST(modbus_bytes, test_be16_get_set) {
    uint8_t buffer[2];
    uint16_t test_value = 0x1234;
    
    modbus_be16_set(buffer, test_value);
    TEST_ASSERT_EQUAL_HEX8(0x12, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0x34, buffer[1]);
    
    uint16_t result = modbus_be16_get(buffer);
    TEST_ASSERT_EQUAL_HEX16(test_value, result);
}

/**
 * Test big-endian with zero value
 */
TEST(modbus_bytes, test_be16_zero_value) {
    uint8_t buffer[2];
    uint16_t test_value = 0x0000;
    
    modbus_be16_set(buffer, test_value);
    TEST_ASSERT_EQUAL_HEX8(0x00, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buffer[1]);
    
    uint16_t result = modbus_be16_get(buffer);
    TEST_ASSERT_EQUAL_HEX16(test_value, result);
}

/**
 * Test big-endian with maximum value
 */
TEST(modbus_bytes, test_be16_max_value) {
    uint8_t buffer[2];
    uint16_t test_value = 0xFFFF;
    
    modbus_be16_set(buffer, test_value);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buffer[1]);
    
    uint16_t result = modbus_be16_get(buffer);
    TEST_ASSERT_EQUAL_HEX16(test_value, result);
}

/**
 * Test little-endian 16-bit get/set
 */
TEST(modbus_bytes, test_le16_get_set) {
    uint8_t buffer[2];
    uint16_t test_value = 0x1234;
    
    modbus_le16_set(buffer, test_value);
    TEST_ASSERT_EQUAL_HEX8(0x34, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0x12, buffer[1]);
    
    uint16_t result = modbus_le16_get(buffer);
    TEST_ASSERT_EQUAL_HEX16(test_value, result);
}

/**
 * Test little-endian with zero value
 */
TEST(modbus_bytes, test_le16_zero_value) {
    uint8_t buffer[2];
    uint16_t test_value = 0x0000;
    
    modbus_le16_set(buffer, test_value);
    TEST_ASSERT_EQUAL_HEX8(0x00, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, buffer[1]);
    
    uint16_t result = modbus_le16_get(buffer);
    TEST_ASSERT_EQUAL_HEX16(test_value, result);
}

/**
 * Test little-endian with maximum value
 */
TEST(modbus_bytes, test_le16_max_value) {
    uint8_t buffer[2];
    uint16_t test_value = 0xFFFF;
    
    modbus_le16_set(buffer, test_value);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buffer[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buffer[1]);
    
    uint16_t result = modbus_le16_get(buffer);
    TEST_ASSERT_EQUAL_HEX16(test_value, result);
}

/**
 * Test endian conversion consistency
 */
TEST(modbus_bytes, test_endian_consistency) {
    uint8_t buffer_be[2], buffer_le[2];
    uint16_t test_value = 0xABCD;
    
    modbus_be16_set(buffer_be, test_value);
    modbus_le16_set(buffer_le, test_value);
    
    // Verify they are different (opposite byte order)
    TEST_ASSERT_EQUAL_HEX8(buffer_be[0], buffer_le[1]);
    TEST_ASSERT_EQUAL_HEX8(buffer_be[1], buffer_le[0]);
}