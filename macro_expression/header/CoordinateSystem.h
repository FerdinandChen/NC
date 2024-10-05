#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>

class Coordinate {
public:
	Coordinate(double x,double y,double z,double b);
	~Coordinate() {}
	double axis_X;
	double axis_Y;
	double axis_Z;
	double axis_B;
};

class CoordinateAxis {
public:
	CoordinateAxis(double, double&);
	~CoordinateAxis() {}
	//取得軸座標值
	double GetPosition()const {
		return position; }
	//設定軸座標值(直接修改)
	void SetPosition(double value) {
		position = value; }
	//設定同步軸差異值
	void SetDelta(double d) {
		delta = d; }
	//軸移動至指定位置
	bool MovePosition(double);

private:
	//目前軸座標值
	double position;
	//同步軸座標值(機械座標)
	double& machine_position;
	//同步軸差異值
	double delta;
};

class WorkingCoordinateSystem {
	friend class Controller;
public:
	WorkingCoordinateSystem();
	~WorkingCoordinateSystem() {}
	//取得目前工作座標系原點X軸機械座標值
	double SystemPositionAxisX()const {
		return working_system->axis_X; } 
	//取得目前工作座標系原點Y軸機械座標值
	double SystemPositionAxisY()const {
		return working_system->axis_Y; }
	//取得目前工作座標系原點Z軸機械座標值
	double SystemPositionAxisZ()const { 
		return working_system->axis_Z; }
	//取得目前工作座標系原點B軸機械座標值
	double SystemPositionAxisB()const { 
		return working_system->axis_B; }
	//以程式座標X軸更新所有工作座標系原點位置
	void ShiftByProgramAxisX(double, double);
	//以程式座標Y軸更新所有工作座標系原點位置
	void ShiftByProgramAxisY(double, double);
	//以程式座標Z軸更新所有工作座標系原點位置
	void ShiftByProgramAxisZ(double, double);
	//以程式座標B軸更新所有工作座標系原點位置
	void ShiftByProgramAxisB(double, double);

private:
	//工作座標系指標
	Coordinate* working_system;
	//G54座標系原點位置
	Coordinate G54_system_position;
	//G55座標系原點位置
	Coordinate G55_system_position;
	//G56座標系原點位置
	Coordinate G56_system_position;
	//G57座標系原點位置
	Coordinate G57_system_position;
	//G58座標系原點位置
	Coordinate G58_system_position;
	//G59座標系原點位置
	Coordinate G59_system_position;
	//選擇工作座標系
	bool SelectWorkingCoordinateSystem(unsigned short);
};

class ProgramCoordinateSystem {
public:
	ProgramCoordinateSystem(Coordinate&, WorkingCoordinateSystem&);
	~ProgramCoordinateSystem() {}
	//程式座標系X軸
	CoordinateAxis axis_X;
	//程式座標系Y軸
	CoordinateAxis axis_Y;
	//程式座標系Z軸
	CoordinateAxis axis_Z;
	//程式座標系B軸
	CoordinateAxis axis_B;
};

class AddressValueTable {
public:
	AddressValueTable() {}
	~AddressValueTable() {}
	bool Empty() const {
		return integer_register.empty() && float_register.empty() && string_register.empty(); }
	void InputRegister(char address, int value) {
		integer_register.insert(std::make_pair(address, value)); }
	void InputRegister(char address, double value) {
		float_register.insert(std::make_pair(address, value)); }
	void InputRegister(char address, const std::string& value) {
		string_register.insert(std::make_pair(address, value)); }
	bool OutputRegister(char, int&);
	bool OutputRegister(char, double&);
	bool OutputRegister(char, std::string&);

private:
	std::map<char, int> integer_register;
	std::map<char, double> float_register;
	std::map<char, std::string> string_register;
};
