#include "MacroVariable.h"
#include <stdexcept>

using namespace std;

Variable::Variable(unsigned short begin_id, unsigned short end_id)
	:begin_ID(begin_id), end_ID(end_id)
{
	if (end_ID < begin_ID) {
		throw out_of_range("end_ID smaller than begin_ID.");
	}
	else {
		//指定容器元素量
		variable_table.reserve(static_cast<vector<double>::size_type>(end_ID - begin_ID) + 1);
		//迭代所有變數編號並建立和初始化變數值
		for (auto iter = begin_ID; iter <= end_ID; ++iter) {
			variable_table.push_back(NULL_VARIABLE); }
	}
}

bool Variable::InquiryVariableID(unsigned short variable_ID)const
{
	if (variable_ID < begin_ID || variable_ID > end_ID) {
		return false; }
	else {
		return true; }
}

bool Variable::ReadVariable(unsigned short variable_ID, double& value)
{
	if (InquiryVariableID(variable_ID)) {
		value = variable_table[static_cast<vector<double>::size_type>(variable_ID - begin_ID)];
		return true;
	}
	else {
		return false; }
}

bool Variable::WriteVariable(unsigned short variable_ID, double& value)
{
	if (variable_ID == 0) {
		throw runtime_error("runtime_error: variable #0 is read only");
	}
	else if (InquiryVariableID(variable_ID)) {
		variable_table[static_cast<vector<double>::size_type>(variable_ID - begin_ID)] = value;
		return true;
	}
	else {
		return false; }
}

bool ModalVariableLevel::CreateModalLevel(map<unsigned short, double>& arguments)
{
	//新建變數群
	Variable variable(0, 33);
	//引數迭代器
	map<unsigned short, double>::iterator iter(arguments.begin());
	//迭代所有輸入引數
	for (iter; iter != arguments.end(); ++iter) {
		//以引數設定變數群初值
		variable.WriteVariable(iter->first, iter->second); }
	//新增模式變數層
	modal_level.push(variable);
	//新變數層指標加入模式變數層清單內
	next_modal_level.push(&modal_level.top());
	
	return true;
}

bool ModalVariableLevel::DeleteModalLevel()
{
	//模式變數層不存在
	if (modal_level.empty()) {
		return false; }
	//有模式變數層
	else {
		//刪除模式變數層
		modal_level.pop();
		//刪除模式變數層指標
		next_modal_level.pop();
		return true;
	}
}

bool ModalVariableLevel::EnterModalLevel()
{
	//下一個模式變數層不存在
	if (next_modal_level.empty()) {
		return false; }
	//有下一個模式變數層
	else {
		//下一個模式變數層指標暫存到上一模式變數層指標清單(返回時使用)
		previous_modal_level.push(next_modal_level.top());
		//刪除下一模式層指標
		next_modal_level.pop();
		return true;
	}
}

bool ModalVariableLevel::ExitModalLevel()
{
	//先前模式變數層不存在
	if (previous_modal_level.empty()) {
		return false; }
	//有先前模式變數層
	else {
		//先前模式變數層指標回存至下一模式變數層指標清單
		next_modal_level.push(previous_modal_level.top());
		//刪除先前模式變數層指標
		previous_modal_level.pop();
		return true;
	}
}

LocalVariable::LocalVariable(unsigned short level_max)
	:variable_level_max(level_max)
{
	//建立變數層level 0
	local_variable.push(Variable(0, 33));
	//更新變數層清單
	variable_list.push(&local_variable.top());
}

bool LocalVariable::ExitLevel()
{
	//返回錯誤:目前變數層已在最底層
	if (local_variable.size() == 1) {
		return false; }
	//目前層數不在最底層
	else {
		//刪除最近變數層
		local_variable.pop();
		//更新變數層指標清單
		variable_list.pop();
		return true;
	}
}

bool LocalVariable::EnterModalLevel(Variable* variable)
{
	//新變數層指標加入清單內
	variable_list.push(variable);
	
	return true;
}

bool LocalVariable::ExitModalLevel()
{
	//返回錯誤:目前變數層已在最底層
	if (variable_list.size() == 1) {
		return false; }
	//目前層數不在最底層
	else {
		//更新變數層指標清單
		variable_list.pop();
		return true;
	}
}

bool LocalVariable::CreateVariable(map<unsigned short, double>& arguments)
{
	//變數層暫存
	Variable temp_variable(0, 33);
	//引數迭代器
	map<unsigned short, double>::iterator iter(arguments.begin());
	//迭代所有輸入引數
	for (iter; iter != arguments.end(); ++iter) {
		//以引數設定變數初值
		temp_variable.WriteVariable(iter->first, iter->second); }
	//新增變數層
	local_variable.push(temp_variable);
	//新變數層指標加入清單內
	variable_list.push(&local_variable.top());
	
	return true;
}

CommonVariable::CommonVariable(unsigned short low_begin_ID, unsigned short low_end_ID, unsigned short high_begin_ID, unsigned short high_end_ID)
	:lower_variable(low_begin_ID, low_end_ID),
	higher_variable(high_begin_ID, high_end_ID)
{
}

bool CommonVariable::InquiryVariableID(unsigned short variable_ID)
{
	if (lower_variable.InquiryVariableID(variable_ID)) {
		return true; }
	else if (higher_variable.InquiryVariableID(variable_ID)) {
		return true; }
	else {
		return false; }
}

bool CommonVariable::ReadVariable(unsigned short variable_ID, double& value)
{
	if (lower_variable.InquiryVariableID(variable_ID)) {
		return lower_variable.ReadVariable(variable_ID, value); }
	else if (higher_variable.InquiryVariableID(variable_ID)) {
		return higher_variable.ReadVariable(variable_ID, value); }
	else {
		return false; }
}

bool CommonVariable::WriteVariable(unsigned short variable_ID, double& value)
{
	if (lower_variable.InquiryVariableID(variable_ID)) {
		return lower_variable.WriteVariable(variable_ID, value);
	}
	else if (higher_variable.InquiryVariableID(variable_ID)) {
		return higher_variable.WriteVariable(variable_ID, value);
	}
	else {
		return false;
	}
}

SystemVariable::SystemVariable(SystemParameter& parameter)
	:system_parameter(parameter)
{
	SetVariableID(3003, &system_parameter.suppress_single_block_stop_wait_auxiliary_function);
	SetVariableID(4201, &system_parameter.current_modal_parameter.motion_command);
	SetVariableID(4202, &system_parameter.current_modal_parameter.working_plane);
	SetVariableID(4203, &system_parameter.current_modal_parameter.coordinate_value_type);
	SetVariableID(4205, &system_parameter.current_modal_parameter.feed_rate_type);
	SetVariableID(4206, &system_parameter.current_modal_parameter.system_unit);
	SetVariableID(4207, &system_parameter.current_modal_parameter.tool_radius_compensation);
	SetVariableID(4208, &system_parameter.current_modal_parameter.tool_length_compensation);
	SetVariableID(4209, &system_parameter.current_modal_parameter.canned_cycle_mode);
	SetVariableID(4210, &system_parameter.current_modal_parameter.canned_cycle_retract_plane);
	SetVariableID(4211, &system_parameter.current_modal_parameter.scale_mode);
	SetVariableID(4212, &system_parameter.current_modal_parameter.macro_mode);
	SetVariableID(4213, &system_parameter.current_modal_parameter.spindle_speed_mode);
	SetVariableID(4214, &system_parameter.current_modal_parameter.working_coordinate_system);
	SetVariableID(4215, &system_parameter.current_modal_parameter.corner_mode);
	SetVariableID(4216, &system_parameter.current_modal_parameter.coordinate_system_rotation);

	SetVariableID(4302, &system_parameter.current_modal_parameter.B_code);
	SetVariableID(4307, &system_parameter.current_modal_parameter.D_code);
	SetVariableID(4308, &system_parameter.current_modal_parameter.E_code);
	SetVariableID(4309, &system_parameter.current_modal_parameter.F_code);
	SetVariableID(4311, &system_parameter.current_modal_parameter.H_code);
	SetVariableID(4313, &system_parameter.current_modal_parameter.M_code);
	SetVariableID(4314, &system_parameter.current_modal_parameter.sequence_number);
	SetVariableID(4315, &system_parameter.current_modal_parameter.program_number);
	SetVariableID(4319, &system_parameter.current_modal_parameter.S_code);
	SetVariableID(4320, &system_parameter.current_modal_parameter.T_code);
	SetVariableID(4330, &system_parameter.current_modal_parameter.P_code);

	SetVariableID(5001, &system_parameter.preview_program_position.axis_X);
	SetVariableID(5002, &system_parameter.preview_program_position.axis_Y);
	SetVariableID(5003, &system_parameter.preview_program_position.axis_Z);
	SetVariableID(5114, &system_parameter.peck_drilling_retraction);
	SetVariableID(5115, &system_parameter.peck_drilling_clearance);
	SetVariableID(5148, &system_parameter.boring_shift_direction);
}

bool SystemVariable::InquiryVariableID(unsigned short variable_ID)
{
	if (table_unsigned_short.count(variable_ID)) {
		return true; }
	else if (table_int.count(variable_ID)) {
		return true; }
	else {
		return false; }
}

bool SystemVariable::ReadVariable(unsigned short variable_ID, double& value)
{
	if (table_unsigned_short.count(variable_ID)) {
		value = static_cast<double>(*table_unsigned_short[variable_ID]);
		return true;
	}
	else if (table_int.count(variable_ID)) {
		value = static_cast<double>(*table_int[variable_ID]);
		return true;
	}
	else if (table_double.count(variable_ID)) {
		value = static_cast<double>(*table_double[variable_ID]);
		return true;
	}
	else {
		return false; }
}

bool SystemVariable::WriteVariable(unsigned short variable_ID, double& value)
{
	if (table_unsigned_short.count(variable_ID)) {
		*table_unsigned_short[variable_ID] = static_cast<unsigned short>(value);
		return true;
	}
	else if (table_int.count(variable_ID)) {
		*table_int[variable_ID] = static_cast<unsigned int>(value);
		return true;
	}
	else if (table_double.count(variable_ID)) {
		*table_double[variable_ID] = value;
		return true;
	}
	else {
		return false; }
}

MacroVariableInterface::MacroVariableInterface(SystemParameter& system_parameter)
	:local_variable(5),
	common_variable(100, 199, 500, 999),
	system_variable(system_parameter)
{
}

bool MacroVariableInterface::ReadVariable(unsigned short variable_ID, double& value)
{
	if (local_variable.ReadVariable(variable_ID, value)) {
		return true; }
	else if (common_variable.ReadVariable(variable_ID, value)) {
		return true; }
	else if (system_variable.ReadVariable(variable_ID, value)) {
		return true; }
	else {
		return false; }
}

bool MacroVariableInterface::WriteVariable(unsigned short variable_ID, double& value)
{
	if (local_variable.WriteVariable(variable_ID, value)) {
		return true; }
	else if (common_variable.WriteVariable(variable_ID, value)) {
		return true; }
	else if (system_variable.WriteVariable(variable_ID, value)) {
		return true; }
	else {
		return false; }
}

bool MacroVariableInterface::EnterLevel(map<unsigned short, double>& arguments)
{
	//檢查新增變數層是否會超出最大層數限制
	if (local_variable.TotalLevel() + modal_variable_level.TotalLevel() == local_variable.TotalLevelMax()) {
		return false; }
	else {
		return local_variable.EnterLevel(arguments); }
}

bool MacroVariableInterface::CreateModalLevel(map<unsigned short, double>& arguments)
{
	//檢查新增變數層是否會超出最大層數限制
	if (local_variable.TotalLevel() + modal_variable_level.TotalLevel() == local_variable.TotalLevelMax()) {
		return false; }
	else {
		//嘗試建立模式變數層並回傳結果
		return modal_variable_level.CreateModalLevel(arguments); }
}

bool MacroVariableInterface::EnterModalLevel()
{
	//局部變數群進入模式變數層
	if (local_variable.EnterModalLevel(modal_variable_level.NextModalVariable())) {
		//進入新的模式變數層
		return modal_variable_level.EnterModalLevel(); }
	//進入模式層失敗
	else {
		return false; }
}

bool MacroVariableInterface::ExitModalLevel()
{
	//成功退出模式變數層
	if (modal_variable_level.ExitModalLevel()) {
		//嘗試退出局部變數層
		return local_variable.ExitModalLevel(); }
	//退出模式變數層失敗
	else {
		return false; }
}
