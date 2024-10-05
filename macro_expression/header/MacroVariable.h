#pragma once

#include <vector>
#include <list>
#include <stack>
#include <map>
#include <cfloat>
#include <utility>
#include "ControllerParameter.h"

//空變數值
constexpr double NULL_VARIABLE = DBL_MIN;

//變數群
class Variable {
public:
	Variable(unsigned short, unsigned short);
	~Variable() {}
	//查詢變數是否存在
	bool InquiryVariableID(unsigned short) const;
	//讀取變數值
	bool ReadVariable(unsigned short, double&);
	//寫入變數值
	bool WriteVariable(unsigned short, double&);

private:
	//起始變數編號
	unsigned short begin_ID;
	//末尾變數編號
	unsigned short end_ID;
	//變數總表
	std::vector<double> variable_table;
};

//模式變數層
class ModalVariableLevel {
public:
	ModalVariableLevel() {}
	~ModalVariableLevel() {}
	//建立模式層
	bool CreateModalLevel(std::map<unsigned short, double>&);
	//刪除模式層
	bool DeleteModalLevel();
	//進入模式層
	bool EnterModalLevel();
	//退出模式層
	bool ExitModalLevel();
	//是否須進行模式巨集呼叫
	bool NextModalCall()const { 
		return !next_modal_level.empty(); }
	//模式層總數
	std::stack<Variable>::size_type TotalLevel()const {
		return modal_level.size(); }
	//取得下一模式層變數群指標
	Variable* NextModalVariable() { 
		return next_modal_level.top(); }

private:
	//模式層容器
	std::stack<Variable> modal_level;
	//下一模式層清單
	std::stack<Variable*> next_modal_level;
	//上一模式層清單
	std::stack<Variable*> previous_modal_level;
};

//局部變數群
class LocalVariable {
public:
	LocalVariable(unsigned short level_max);
	~LocalVariable() {}
	//查詢變數是否存在
	bool InquiryVariableID(unsigned short variable_ID) {
		return variable_list.top()->InquiryVariableID(variable_ID); }
	//讀取變數值
	bool ReadVariable(unsigned short variable_ID, double& value) {
		return variable_list.top()->ReadVariable(variable_ID, value); }
	//寫入變數值
	bool WriteVariable(unsigned short variable_ID, double& value) {
		return variable_list.top()->WriteVariable(variable_ID, value); }
	//進入局部變數層
	bool EnterLevel(std::map<unsigned short, double>& arguments) {
		return CreateVariable(arguments); }
	//退出局部變數層
	bool ExitLevel();
	//進入模式變數層
	bool EnterModalLevel(Variable*);
	//退出模式變數層
	bool ExitModalLevel();

	//查詢總變數層數
	std::stack<Variable>::size_type TotalLevel() const {
		return local_variable.size(); }
	//查詢最大總變數層數
	unsigned short TotalLevelMax()const { 
		return variable_level_max + 1; }

	std::stack<Variable*>::size_type CurrentLevel() const {
		return variable_list.size();
	}

private:
	//最大變數層數
	const unsigned short variable_level_max;
	//局部變數層容器
	std::stack<Variable> local_variable;
	//變數層指標清單
	std::stack<Variable*> variable_list;
	//建立新變數層
	bool CreateVariable(std::map<unsigned short, double>&);
};

//共用變數群
class CommonVariable {
public:
	CommonVariable(unsigned short, unsigned short, unsigned short, unsigned short);
	~CommonVariable() {}
	//查詢變數存在否
	bool InquiryVariableID(unsigned short);
	//讀取變數值
	bool ReadVariable(unsigned short, double&);
	//寫入變數值
	bool WriteVariable(unsigned short, double&);

private:
	//低變數群
	Variable lower_variable;
	//高變數群
	Variable higher_variable;
};

//系統變數群
class SystemVariable {
public:
	SystemVariable(SystemParameter&);
	~SystemVariable() {}
	//查詢變數是否存在
	bool InquiryVariableID(unsigned short);
	//讀取變數值
	bool ReadVariable(unsigned short, double&);
	//寫入變數值
	bool WriteVariable(unsigned short, double&);

private:
	//系統參數群
	SystemParameter& system_parameter;
	//短整數表格
	std::map<unsigned short, unsigned short*> table_unsigned_short;
	//整數表格
	std::map<unsigned short, int*> table_int;
	//浮點數表格
	std::map<unsigned short, double*> table_double;
	//建立系統參數對應變數編號
	void SetVariableID(unsigned short variable_ID, unsigned short* variable) {
		table_unsigned_short.insert(std::make_pair(variable_ID, variable)); }
	void SetVariableID(unsigned short variable_ID, int* variable) {
		table_int.insert(std::make_pair(variable_ID, variable)); }
	void SetVariableID(unsigned short variable_ID, double* variable) {
		table_double.insert(std::make_pair(variable_ID, variable)); }
};

//巨集變數存取介面
class MacroVariableInterface {
public:
	MacroVariableInterface(SystemParameter&);
	~MacroVariableInterface() {}
	//讀取變數值
	bool ReadVariable(unsigned short, double&);
	//寫入變數值
	bool WriteVariable(unsigned short, double&);
	//進入變數層
	bool EnterLevel(std::map<unsigned short, double>&);
	//退出變數層
	bool ExitLevel() { 
		return local_variable.ExitLevel(); }
	//建立模式變數層
	bool CreateModalLevel(std::map<unsigned short, double>&);
	//刪除模式變數層
	bool DeleteModalLevel() {
		//嘗試刪除模式變數層並回傳結果
		return modal_variable_level.DeleteModalLevel(); }
	//進入模式層
	bool EnterModalLevel();
	//退出模式層
	bool ExitModalLevel();
	//查詢總變數層數
	std::stack<Variable*>::size_type CurrentLevel() const {
		return local_variable.CurrentLevel(); }

private:
	//局部變數
	LocalVariable local_variable;
	//共同變數
	CommonVariable common_variable;
	//系統變數
	SystemVariable system_variable;
	//模式變數層
	ModalVariableLevel modal_variable_level;
};
