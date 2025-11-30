#ifndef MODBUS_SLAVE_HANDLERS_H
#define MODBUS_SLAVE_HANDLERS_H

#include "modbus_slave.h"

#include <stdint.h>

ModbusExceptionCode handle_read_coils(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_read_discrete_inputs(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_read_holding_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_read_input_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);

ModbusExceptionCode handle_write_single_coil(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_write_single_register(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_write_multiple_coils(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_write_multiple_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);

ModbusExceptionCode handle_mask_write_register(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);
ModbusExceptionCode handle_read_write_multiple_registers(ModbusSlave *slave, uint8_t *response, uint16_t *response_len);


#endif /* MODBUS_SLAVE_HANDLERS_H */
