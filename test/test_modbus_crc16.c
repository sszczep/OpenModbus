#include "unity_fixture.h"
#include "modbus_crc16.h"

#include <string.h>

TEST_GROUP(modbus_crc16);

TEST_SETUP(modbus_crc16) {}

TEST_TEAR_DOWN(modbus_crc16) {}

/**
 * Test CRC16 calculation with empty data
 */
TEST(modbus_crc16, test_crc16_empty_data) {
    uint16_t crc = modbus_crc16(NULL, 0);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, crc);
}

/**
 * Test CRC16 calculation with known test vector
 */
TEST(modbus_crc16, test_crc16_known_vector) {
    uint8_t test_data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};
    uint16_t crc = modbus_crc16(test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL_HEX16(0x0A84, crc);
}

/**
 * Test CRC16 calculation with another known vector
 */
TEST(modbus_crc16, test_crc16_another_vector) {
    uint8_t test_data[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01};
    uint16_t crc = modbus_crc16(test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL_HEX16(0xCAD5, crc);
}

/**
 * Test CRC16 with full buffer
 */
TEST(modbus_crc16, test_crc16_full_buffer) {
    uint8_t test_data[256];
    for (int i = 0; i < 256; i++) {
        test_data[i] = i;
    }
    uint16_t crc = modbus_crc16(test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL_HEX16(0xDE6C, crc);
}

/**
 * Test CRC16 with single byte
 */
TEST(modbus_crc16, test_crc16_single_byte) {
    uint8_t test_data[] = {0x42};
    uint16_t crc = modbus_crc16(test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL_HEX16(0x713F, crc);
}

/**
 * Test CRC16 table consistency - verify first few entries
 */
TEST(modbus_crc16, test_crc16_table_consistency) {
    TEST_ASSERT_EQUAL_HEX16(0x0000, modbus_crc16_table[0]);
    TEST_ASSERT_EQUAL_HEX16(0xC0C1, modbus_crc16_table[1]);
    TEST_ASSERT_EQUAL_HEX16(0xC181, modbus_crc16_table[2]);
    TEST_ASSERT_EQUAL_HEX16(0x0140, modbus_crc16_table[3]);
}