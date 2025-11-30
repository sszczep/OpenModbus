#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include "modbus_bytes.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_MIN_FRAME_LENGTH 4
#define MODBUS_MAX_FRAME_LENGTH 256
#define MODBUS_MAX_PDU_LENGTH   253
#define MODBUS_FC_EXCEPTION_MASK 0x80

/*==============================
    Modbus state machine
==============================*/
typedef enum {
    UNINITIALIZED = 0,
    IDLE,
    RECEPTION,
    CONTROL_AND_WAITING,
} ModbusState;

/*==============================
    Function codes
==============================*/
typedef enum {
    MODBUS_FC_READ_COILS                = 0x01,
    MODBUS_FC_READ_DISCRETE_INPUTS      = 0x02,
    MODBUS_FC_READ_HOLDING_REGISTERS    = 0x03,
    MODBUS_FC_READ_INPUT_REGISTERS      = 0x04,

    MODBUS_FC_WRITE_SINGLE_COIL         = 0x05,
    MODBUS_FC_WRITE_SINGLE_REGISTER     = 0x06,

    MODBUS_FC_WRITE_MULTIPLE_COILS      = 0x0F,
    MODBUS_FC_WRITE_MULTIPLE_REGISTERS  = 0x10,

    MODBUS_FC_READ_EXCEPTION_STATUS     = 0x07,
    MODBUS_FC_DIAGNOSTICS               = 0x08,
    MODBUS_FC_READ_WRITE_MULTIPLE_REGS  = 0x17,
    MODBUS_FC_MASK_WRITE_REGISTER       = 0x16,
} ModbusFunctionCode;

/*==============================
    Exception codes
==============================*/
typedef enum {
    MODBUS_EX_NONE                 = 0x00, /* No exception */
    MODBUS_EX_ILLEGAL_FUNCTION     = 0x01, /* Function not supported */
    MODBUS_EX_ILLEGAL_DATA_ADDRESS = 0x02, /* Invalid register/coil address */
    MODBUS_EX_ILLEGAL_DATA_VALUE   = 0x03, /* Invalid value */
    MODBUS_EX_SLAVE_DEVICE_FAILURE = 0x04, /* Device failure */
} ModbusExceptionCode;

/*==============================
    Callback typedefs
==============================*/
typedef ModbusExceptionCode (*ModbusReadCoilsCb)(uint16_t addr, uint16_t count, uint8_t *dest);
typedef ModbusExceptionCode (*ModbusReadDiscreteInputsCb)(uint16_t addr, uint16_t count, uint8_t *dest);
typedef ModbusExceptionCode (*ModbusReadHoldingRegistersCb)(uint16_t addr, uint16_t count, uint8_t *dest);
typedef ModbusExceptionCode (*ModbusReadInputRegistersCb)(uint16_t addr, uint16_t count, uint8_t *dest);

typedef ModbusExceptionCode (*ModbusWriteSingleCoilCb)(uint16_t addr, uint8_t value);
typedef ModbusExceptionCode (*ModbusWriteSingleRegisterCb)(uint16_t addr, uint16_t value);
typedef ModbusExceptionCode (*ModbusWriteMultipleCoilsCb)(uint16_t addr, uint16_t count, const uint8_t *src);
typedef ModbusExceptionCode (*ModbusWriteMultipleRegistersCb)(uint16_t addr, uint16_t count, const uint8_t *src);

typedef ModbusExceptionCode (*ModbusMaskWriteRegisterCb)(uint16_t addr, uint16_t and_mask, uint16_t or_mask);
typedef ModbusExceptionCode (*ModbusReadWriteMultipleRegistersCb)(
    uint16_t read_addr, uint16_t read_count,
    uint16_t write_addr, uint16_t write_count,
    const uint8_t *write_data, uint8_t *read_data
);

/*==============================
    Configuration
==============================*/
typedef struct {
    uint8_t address;
    
    void (*write)(const uint8_t *data, uint16_t length);
    
    ModbusReadCoilsCb                   read_coils;
    ModbusReadDiscreteInputsCb          read_discrete_inputs;
    ModbusReadHoldingRegistersCb        read_holding_registers;
    ModbusReadInputRegistersCb          read_input_registers;

    ModbusWriteSingleCoilCb             write_single_coil;
    ModbusWriteSingleRegisterCb         write_single_register;
    ModbusWriteMultipleCoilsCb          write_multiple_coils;
    ModbusWriteMultipleRegistersCb      write_multiple_registers;

    ModbusMaskWriteRegisterCb           mask_write_register;
    ModbusReadWriteMultipleRegistersCb  read_write_multiple_registers;
} ModbusSlaveConfig;

/*==============================
    Slave structure
==============================*/
typedef struct {
    ModbusSlaveConfig config;
    volatile ModbusState state;
    uint8_t frame[MODBUS_MAX_FRAME_LENGTH];
    volatile uint16_t frame_len;
    volatile bool frame_ok;
    volatile bool frame_available;
    volatile bool processing_frame;
} ModbusSlave;

/*==============================
    Public API
==============================*/
int modbus_slave_init(ModbusSlave *slave, const ModbusSlaveConfig *cfg);
void modbus_slave_rx_byte(ModbusSlave *slave, uint8_t byte);
void modbus_slave_1_5t_elapsed(ModbusSlave *slave);
void modbus_slave_3_5t_elapsed(ModbusSlave *slave);
void modbus_slave_poll(ModbusSlave *slave);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_SLAVE_H */
