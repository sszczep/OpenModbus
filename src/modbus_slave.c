#include "modbus_slave.h"
#include "modbus_slave_handlers.h"
#include "modbus_bytes.h"
#include "modbus_crc16.h"

#include <string.h>

// =============================================================================
// Initialization
// =============================================================================

/**
 * Initialize Modbus slave instance
 * @param slave Pointer to slave instance
 * @param cfg   Pointer to configuration
 * @return 0 on success, -1 on error
 */
int modbus_slave_init(ModbusSlave *slave, const ModbusSlaveConfig *cfg) {
    if (!slave || !cfg || !cfg->write) return -1;

    if (cfg->address == 0x00) return -1; // Address 0 is reserved for broadcast

    slave->config = *cfg;
    slave->state = IDLE;
    slave->frame_len = 0;
    slave->frame_ok = true;
    slave->frame_available = false;
    slave->processing_frame = false;

    return 0;
}

// =============================================================================
// Receive byte (ISR-safe)
// =============================================================================

/**
 * Process received byte - call from UART ISR
 * @param slave Slave instance
 * @param byte  Received byte
 */
void modbus_slave_rx_byte(ModbusSlave *slave, uint8_t byte) {
    if (slave->processing_frame) return;

    if (slave->state == IDLE) {
        slave->state = RECEPTION;
        slave->frame_len = 0;
        slave->frame_ok = true;
    }

    if (slave->state == RECEPTION) {
        if (slave->frame_len < MODBUS_MAX_FRAME_LENGTH) {
            slave->frame[slave->frame_len++] = byte;
        } else { // Drop data if frame exceeds size limit
            slave->frame_ok = false;
            slave->state = CONTROL_AND_WAITING;
        }
    }
}

// =============================================================================
// Timer ticks (call from timer ISR based on baud rate)
// =============================================================================

/**
 * 1.5 character time elapsed - end of character reception
 * @param slave Slave instance
 */
void modbus_slave_1_5t_elapsed(ModbusSlave *slave) {
    if (slave->state == RECEPTION) slave->state = CONTROL_AND_WAITING;
}

/**
 * 3.5 character time elapsed - end of frame
 * @param slave Slave instance
 */
void modbus_slave_3_5t_elapsed(ModbusSlave *slave) {
    if (slave->state != CONTROL_AND_WAITING) return;

    // Only process the frame if there were no reception errors
    if (slave->frame_ok) slave->frame_available = true;

    slave->state = IDLE;
}

// =============================================================================
// Frame validation
// =============================================================================

/**
 * Validate received Modbus frame
 * @param slave Slave instance
 * @return 0 if valid, -1 if invalid
 */
static int modbus_validate_frame(ModbusSlave *slave) {
	if (slave->frame_len < MODBUS_MIN_FRAME_LENGTH) return -1;

	uint8_t address = slave->frame[0];
	if (address != 0x00 && address != slave->config.address) return -1;

	uint16_t received_crc = modbus_le16_get(&slave->frame[slave->frame_len - 2]);
	uint16_t expected_crc = modbus_crc16(slave->frame, slave->frame_len - 2);
	if (received_crc != expected_crc) return -1;

	return 0;
}

// =============================================================================
// Frame processor
// =============================================================================

/**
 * Process valid Modbus frame and generate response
 * @param slave Slave instance
 */
static void modbus_process_frame(ModbusSlave *slave) {
    if (modbus_validate_frame(slave) != 0) return; // Drop invalid frames

    uint8_t *request = slave->frame;

    uint8_t response[MODBUS_MAX_FRAME_LENGTH];
    uint8_t *response_pdu = response + 1;
    uint16_t response_len = 0;

    ModbusExceptionCode ex_code = MODBUS_EX_NONE;

    switch(request[1]) {
        case MODBUS_FC_READ_COILS:
            ex_code = handle_read_coils(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            ex_code = handle_read_discrete_inputs(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            ex_code = handle_read_holding_registers(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_READ_INPUT_REGISTERS:
            ex_code = handle_read_input_registers(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_WRITE_SINGLE_COIL:
            ex_code = handle_write_single_coil(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            ex_code = handle_write_single_register(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
            ex_code = handle_write_multiple_coils(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            ex_code = handle_write_multiple_registers(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_MASK_WRITE_REGISTER:
            ex_code = handle_mask_write_register(slave, response_pdu, &response_len);
            break;
        case MODBUS_FC_READ_WRITE_MULTIPLE_REGS:
            ex_code = handle_read_write_multiple_registers(slave, response_pdu, &response_len);
            break;
        default:
            ex_code = MODBUS_EX_ILLEGAL_FUNCTION;
            break;
    }

    if (request[0] == 0x00) return; // Broadcast frame, no response

    response[0] = request[0]; // Reassign the address
    response_len += 1;

    if (ex_code != MODBUS_EX_NONE) { // Set an exception if there was one
        response[1] = request[1] | MODBUS_FC_EXCEPTION_MASK;
        response[2] = (uint8_t)ex_code;
        response_len = 3;
    }

    // Calculate and set CRC16
    uint16_t crc = modbus_crc16(response, response_len);
    modbus_le16_set(&response[response_len], crc);
    response_len += 2;

    // Send the response
    slave->config.write(response, response_len);
}

// =============================================================================
// Polling (call from main loop)
// =============================================================================

/**
 * Process received frames - call periodically from main loop
 * @param slave Slave instance
 */
void modbus_slave_poll(ModbusSlave *slave) {
    if (!slave->frame_available) return;

    slave->processing_frame = true;
    slave->frame_available = false;

    modbus_process_frame(slave);

    slave->frame_len = 0;
    slave->processing_frame = false;
}
