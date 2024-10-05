#include "ControllerParameter.h"
#include <utility>

using namespace std;

#define INVALID_FLOAT_VALUE  DBL_MAX   //不合法浮點數值

ModalParameter::ModalParameter()
	:motion_command(0),
	working_plane(17),
	coordinate_value_type(90),
	feed_rate_type(94),
	system_unit(21),
	tool_radius_compensation(40),
	tool_length_compensation(49),
	canned_cycle_mode(80),
	canned_cycle_retract_plane(98),
	scale_mode(50),
	macro_mode(67),
	spindle_speed_mode(97),
	working_coordinate_system(54),
	corner_mode(64),
	coordinate_system_rotation(69),
	B_code(0),
	D_code(0),
	E_code(0),
	F_code(0),
	H_code(0),
	M_code(0),
	sequence_number(0),
	program_number(0),
	S_code(0),
	T_code(0),
	P_code(0)
{
}

OperationParameter::OperationParameter()
	:simulation_on(false),
	optional_skip(false),
	optional_stop(false),
	single_block_stop(false),
	auto_restart(false),
	cycle_mode(cycle_end),
	operation_mode(memory_mode)
{
}

SystemParameter::SystemParameter()
	:last_command_motion(false),
	sequence_cache_fixed(true),
	sequence_cache_variable(true),
	sequence_cache_history(true),
	sub_program_begin_sequence(true),
	sub_program_number_P8(true),
	last_reference_position(0),
	suppress_single_block_stop_wait_auxiliary_function(0),
	boring_shift_direction(1),
	program_radius(0.0),
	rapid_feed_rate_X(6000.0),
	rapid_feed_rate_Y(6000.0),
	rapid_feed_rate_Z(6000.0),
	peck_drilling_clearance(1.0),
	peck_drilling_retraction(3.0),
	intermediate_position(INVALID_FLOAT_VALUE, INVALID_FLOAT_VALUE, INVALID_FLOAT_VALUE, INVALID_FLOAT_VALUE),
	reference_position_1st(0.0, 0.0, 0.0, 0.0),
	reference_position_2nd(-1000.0, 1000.0, 150., 0.0),
	reference_position_3rd(-1000.0, -1000.0, 0.0, 0.0),
	reference_position_4th(1000.0, -1000.0, 0.0, 0.0),
	preview_program_position(INVALID_FLOAT_VALUE, INVALID_FLOAT_VALUE, INVALID_FLOAT_VALUE, INVALID_FLOAT_VALUE),
	machine_coordinate(0.0, 0.0, 0.0, 0.0),
	program_coordinate_system(machine_coordinate, working_coordinate_system)
{
	//初始化所有刀長補正值為200.0mm
	for (unsigned short iter = 1; iter <= TOOL_LENGTH_REGISTER_MAX; ++iter) {
		tool_length_offset_table.insert(make_pair(iter, 200.0)); }
}
