#include "CoordinateSystem.h"
#include <cstdlib>

using namespace std;

Coordinate::Coordinate(double x, double y, double z, double b)
	:axis_X(x),
	axis_Y(y),
	axis_Z(z),
	axis_B(b)
{
}

CoordinateAxis::CoordinateAxis(double value, double& m_value)
	:position(value),
	machine_position(m_value),
	delta(m_value-value)
{
}

bool CoordinateAxis::MovePosition(double move_to)
{
	//軸移動至指定位置
	position = move_to;
	//同步更新機械軸的位置
	machine_position = move_to + delta;
	return true;
}

WorkingCoordinateSystem::WorkingCoordinateSystem()
	:working_system(NULL),
	G54_system_position(-700.0, -700.0, -700.0, 0.0),
	G55_system_position(-500.0, -500.0, -500.0, 0.0),
	G56_system_position(-700.0, -700.0, -700.0, 0.0),
	G57_system_position(-700.0, -700.0, -700.0, 0.0),
	G58_system_position(-700.0, -700.0, -700.0, 0.0),
	G59_system_position(-700.0, -700.0, -700.0, 0.0)
{
	//選擇預設工作座標系
	SelectWorkingCoordinateSystem(54);
}

bool WorkingCoordinateSystem::SelectWorkingCoordinateSystem(unsigned short working_system_ID)
{
	//判斷選擇的工作座標系
	switch (working_system_ID) {
		//選擇G54工作座標系
	case 54:
		//將工作座標系指標指向G54座標系
		working_system = &G54_system_position;
		break;

	case 55:
		working_system = &G55_system_position;
		break;

	case 56:
		working_system = &G56_system_position;
		break;

	case 57:
		working_system = &G57_system_position;
		break;

	case 58:
		working_system = &G58_system_position;
		break;

	case 59:
		working_system = &G59_system_position;
		break;

		//不合法或未定義的工作座標系
	default: return false;
	}

	return true;
}

void WorkingCoordinateSystem::ShiftByProgramAxisX(double machine_axis_x, double program_axis_x)
{
	//計算X軸偏移量
	double offset(machine_axis_x - program_axis_x - working_system->axis_X);
	//逐一偏移所有工作座標系的X軸原點位置
	G54_system_position.axis_X += offset;
	G55_system_position.axis_X += offset;
	G56_system_position.axis_X += offset;
	G57_system_position.axis_X += offset;
	G58_system_position.axis_X += offset;
	G59_system_position.axis_X += offset;
	
	return;
}

void WorkingCoordinateSystem::ShiftByProgramAxisY(double machine_axis_y, double program_axis_y)
{
	double offset(machine_axis_y - program_axis_y - working_system->axis_Y);
	G54_system_position.axis_Y += offset;
	G55_system_position.axis_Y += offset;
	G56_system_position.axis_Y += offset;
	G57_system_position.axis_Y += offset;
	G58_system_position.axis_Y += offset;
	G59_system_position.axis_Y += offset;
	
	return;
}

void WorkingCoordinateSystem::ShiftByProgramAxisZ(double machine_axis_z, double program_axis_z)
{
	double offset(machine_axis_z - program_axis_z - working_system->axis_Z);
	G54_system_position.axis_Z += offset;
	G55_system_position.axis_Z += offset;
	G56_system_position.axis_Z += offset;
	G57_system_position.axis_Z += offset;
	G58_system_position.axis_Z += offset;
	G59_system_position.axis_Z += offset;
	
	return;
}

void WorkingCoordinateSystem::ShiftByProgramAxisB(double machine_axis_b, double program_axis_b)
{
	double offset(machine_axis_b - program_axis_b - working_system->axis_B);
	G54_system_position.axis_B += offset;
	G55_system_position.axis_B += offset;
	G56_system_position.axis_B += offset;
	G57_system_position.axis_B += offset;
	G58_system_position.axis_B += offset;
	G59_system_position.axis_B += offset;
	
	return;
}

ProgramCoordinateSystem::ProgramCoordinateSystem(Coordinate& machine_coordinate, WorkingCoordinateSystem& working_coordinate_system)
	:axis_X(machine_coordinate.axis_X - working_coordinate_system.SystemPositionAxisX(), machine_coordinate.axis_X),
	axis_Y(machine_coordinate.axis_Y - working_coordinate_system.SystemPositionAxisY(), machine_coordinate.axis_Y),
	axis_Z(machine_coordinate.axis_Z - working_coordinate_system.SystemPositionAxisZ(), machine_coordinate.axis_Z),
	axis_B(machine_coordinate.axis_B - working_coordinate_system.SystemPositionAxisB(), machine_coordinate.axis_B)
{
}

bool AddressValueTable::OutputRegister(char address, int& value)
{
	map<char, int>::iterator iter(integer_register.find(address));
	if (iter == integer_register.end()) {
		return false; }
	else {
		value = iter->second;
		integer_register.erase(iter);
		return true;
	}
}

bool AddressValueTable::OutputRegister(char address, double& value)
{
	map<char, double>::iterator iter(float_register.find(address));
	if (iter == float_register.end()) {
		return false; }
	else {
		value = iter->second;
		float_register.erase(iter);
		return true;
	}
}

bool AddressValueTable::OutputRegister(char address, string& value)
{
	map<char, string>::iterator iter(string_register.find(address));
	if (iter == string_register.end()) {
		return false; }
	else {
		value = iter->second;
		string_register.erase(iter);
		return true;
	}
}
