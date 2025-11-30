#ifndef MODBUS_BYTES_H
#define MODBUS_BYTES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Extract 16-bit big-endian value from byte array
 * @param p Pointer to byte array (MSB first)
 * @return 16-bit value in host byte order
 */
static inline uint16_t modbus_be16_get(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

/**
 * Store 16-bit value as big-endian in byte array
 * @param p Pointer to destination byte array
 * @param value 16-bit value in host byte order
 */
static inline void modbus_be16_set(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)(value & 0xFF);
}

/**
 * Extract 16-bit little-endian value from byte array
 * @param p Pointer to byte array (LSB first)
 * @return 16-bit value in host byte order
 */
static inline uint16_t modbus_le16_get(const uint8_t *p)
{
    return ((uint16_t)p[1] << 8) | (uint16_t)p[0];
}

/**
 * Store 16-bit value as little-endian in byte array
 * @param p Pointer to destination byte array
 * @param value 16-bit value in host byte order
 */
static inline void modbus_le16_set(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)(value >> 8);
}

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_BYTES_H */