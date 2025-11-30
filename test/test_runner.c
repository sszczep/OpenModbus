#include "unity_fixture.h"

// Test group declarations
TEST_GROUP_RUNNER(modbus_crc16) {
    RUN_TEST_CASE(modbus_crc16, test_crc16_empty_data);
    RUN_TEST_CASE(modbus_crc16, test_crc16_known_vector);
    RUN_TEST_CASE(modbus_crc16, test_crc16_another_vector);
    RUN_TEST_CASE(modbus_crc16, test_crc16_full_buffer);
    RUN_TEST_CASE(modbus_crc16, test_crc16_single_byte);
    RUN_TEST_CASE(modbus_crc16, test_crc16_table_consistency);
}

TEST_GROUP_RUNNER(modbus_bytes) {
    RUN_TEST_CASE(modbus_bytes, test_be16_get_set);
    RUN_TEST_CASE(modbus_bytes, test_be16_zero_value);
    RUN_TEST_CASE(modbus_bytes, test_be16_max_value);
    RUN_TEST_CASE(modbus_bytes, test_le16_get_set);
    RUN_TEST_CASE(modbus_bytes, test_le16_zero_value);
    RUN_TEST_CASE(modbus_bytes, test_le16_max_value);
    RUN_TEST_CASE(modbus_bytes, test_endian_consistency);
}

TEST_GROUP_RUNNER(modbus_slave_init) {
    RUN_TEST_CASE(modbus_slave_init, test_slave_init_success);
    RUN_TEST_CASE(modbus_slave_init, test_slave_init_null_slave);
    RUN_TEST_CASE(modbus_slave_init, test_slave_init_null_config);
    RUN_TEST_CASE(modbus_slave_init, test_slave_init_null_write_function);
    RUN_TEST_CASE(modbus_slave_init, test_slave_init_preserves_config);
}

TEST_GROUP_RUNNER(modbus_slave_rx) {
    RUN_TEST_CASE(modbus_slave_rx, test_rx_first_byte_transition);
    RUN_TEST_CASE(modbus_slave_rx, test_rx_multiple_bytes);
    RUN_TEST_CASE(modbus_slave_rx, test_rx_frame_overflow);
    RUN_TEST_CASE(modbus_slave_rx, test_rx_ignore_during_processing);
    RUN_TEST_CASE(modbus_slave_rx, test_1_5t_timer_transition);
    RUN_TEST_CASE(modbus_slave_rx, test_1_5t_timer_ignore_other_states);
    RUN_TEST_CASE(modbus_slave_rx, test_3_5t_timer_valid_frame);
    RUN_TEST_CASE(modbus_slave_rx, test_3_5t_timer_invalid_frame);
    RUN_TEST_CASE(modbus_slave_rx, test_3_5t_timer_ignore_other_states);
}

// Handler test groups
TEST_GROUP_RUNNER(modbus_handler_read_coils) {
    RUN_TEST_CASE(modbus_handler_read_coils, test_handle_read_coils_valid);
    RUN_TEST_CASE(modbus_handler_read_coils, test_handle_read_coils_unsupported);
    RUN_TEST_CASE(modbus_handler_read_coils, test_handle_read_coils_invalid_count_low);
    RUN_TEST_CASE(modbus_handler_read_coils, test_handle_read_coils_invalid_count_high);
    RUN_TEST_CASE(modbus_handler_read_coils, test_handle_read_coils_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_read_discrete_inputs) {
    RUN_TEST_CASE(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_valid);
    RUN_TEST_CASE(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_unsupported);
    RUN_TEST_CASE(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_invalid_count_low);
    RUN_TEST_CASE(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_invalid_count_high);
    RUN_TEST_CASE(modbus_handler_read_discrete_inputs, test_handle_read_discrete_inputs_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_read_holding_registers) {
    RUN_TEST_CASE(modbus_handler_read_holding_registers, test_handle_read_holding_registers_valid);
    RUN_TEST_CASE(modbus_handler_read_holding_registers, test_handle_read_holding_registers_unsupported);
    RUN_TEST_CASE(modbus_handler_read_holding_registers, test_handle_read_holding_registers_invalid_count_low);
    RUN_TEST_CASE(modbus_handler_read_holding_registers, test_handle_read_holding_registers_invalid_count_high);
    RUN_TEST_CASE(modbus_handler_read_holding_registers, test_handle_read_holding_registers_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_read_input_registers) {
    RUN_TEST_CASE(modbus_handler_read_input_registers, test_handle_read_input_registers_valid);
    RUN_TEST_CASE(modbus_handler_read_input_registers, test_handle_read_input_registers_unsupported);
    RUN_TEST_CASE(modbus_handler_read_input_registers, test_handle_read_input_registers_invalid_count_low);
    RUN_TEST_CASE(modbus_handler_read_input_registers, test_handle_read_input_registers_invalid_count_high);
    RUN_TEST_CASE(modbus_handler_read_input_registers, test_handle_read_input_registers_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_write_single_coil) {
    RUN_TEST_CASE(modbus_handler_write_single_coil, test_handle_write_single_coil_valid_on);
    RUN_TEST_CASE(modbus_handler_write_single_coil, test_handle_write_single_coil_valid_off);
    RUN_TEST_CASE(modbus_handler_write_single_coil, test_handle_write_single_coil_unsupported);
    RUN_TEST_CASE(modbus_handler_write_single_coil, test_handle_write_single_coil_invalid_value);
    RUN_TEST_CASE(modbus_handler_write_single_coil, test_handle_write_single_coil_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_write_single_register) {
    RUN_TEST_CASE(modbus_handler_write_single_register, test_handle_write_single_register_valid);
    RUN_TEST_CASE(modbus_handler_write_single_register, test_handle_write_single_register_unsupported);
    RUN_TEST_CASE(modbus_handler_write_single_register, test_handle_write_single_register_address_error);
    RUN_TEST_CASE(modbus_handler_write_single_register, test_handle_write_single_register_zero_values);
}

TEST_GROUP_RUNNER(modbus_handler_write_multiple_coils) {
    RUN_TEST_CASE(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_valid);
    RUN_TEST_CASE(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_unsupported);
    RUN_TEST_CASE(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_invalid_byte_count);
    RUN_TEST_CASE(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_invalid_count_low);
    RUN_TEST_CASE(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_invalid_count_high);
    RUN_TEST_CASE(modbus_handler_write_multiple_coils, test_handle_write_multiple_coils_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_write_multiple_registers) {
    RUN_TEST_CASE(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_valid);
    RUN_TEST_CASE(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_unsupported);
    RUN_TEST_CASE(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_invalid_byte_count);
    RUN_TEST_CASE(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_invalid_count_low);
    RUN_TEST_CASE(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_invalid_count_high);
    RUN_TEST_CASE(modbus_handler_write_multiple_registers, test_handle_write_multiple_registers_address_error);
}

TEST_GROUP_RUNNER(modbus_handler_mask_write_register) {
    RUN_TEST_CASE(modbus_handler_mask_write_register, test_handle_mask_write_register_valid);
    RUN_TEST_CASE(modbus_handler_mask_write_register, test_handle_mask_write_register_unsupported);
    RUN_TEST_CASE(modbus_handler_mask_write_register, test_handle_mask_write_register_address_error);
    RUN_TEST_CASE(modbus_handler_mask_write_register, test_handle_mask_write_register_zero_masks);
}

TEST_GROUP_RUNNER(modbus_handler_read_write_multiple_registers) {
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_valid);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_unsupported);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_read_count_low);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_read_count_high);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_write_count_low);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_write_count_high);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_invalid_byte_count);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_read_address_error);
    RUN_TEST_CASE(modbus_handler_read_write_multiple_registers, test_handle_read_write_multiple_registers_write_address_error);
}

TEST_GROUP_RUNNER(modbus_integration) {
    RUN_TEST_CASE(modbus_integration, test_complete_frame_processing);
    RUN_TEST_CASE(modbus_integration, test_frame_invalid_crc);
    RUN_TEST_CASE(modbus_integration, test_broadcast_frame_no_response);
    RUN_TEST_CASE(modbus_integration, test_wrong_address_frame);
}

static void run_all_tests(void) {
    RUN_TEST_GROUP(modbus_crc16);
    RUN_TEST_GROUP(modbus_bytes);
    RUN_TEST_GROUP(modbus_slave_init);
    RUN_TEST_GROUP(modbus_slave_rx);
    
    // All handler test groups
    RUN_TEST_GROUP(modbus_handler_read_coils);
    RUN_TEST_GROUP(modbus_handler_read_discrete_inputs);
    RUN_TEST_GROUP(modbus_handler_read_holding_registers);
    RUN_TEST_GROUP(modbus_handler_read_input_registers);
    RUN_TEST_GROUP(modbus_handler_write_single_coil);
    RUN_TEST_GROUP(modbus_handler_write_single_register);
    RUN_TEST_GROUP(modbus_handler_write_multiple_coils);
    RUN_TEST_GROUP(modbus_handler_write_multiple_registers);
    RUN_TEST_GROUP(modbus_handler_mask_write_register);
    RUN_TEST_GROUP(modbus_handler_read_write_multiple_registers);
    
    RUN_TEST_GROUP(modbus_integration);
}

int main(int argc, const char * argv[]) {
    return UnityMain(argc, argv, run_all_tests);
}