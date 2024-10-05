#pragma once

#include <string>
#include "StringConverter.h"

//整數字串剖析器
class IntegerNumberDefinition {
public:
	IntegerNumberDefinition(const IntegerStringConverter&, std::string::size_type, std::string::size_type, int, int, bool lead_zero = false);
	~IntegerNumberDefinition();
	//剖析字串並轉換為整數值
	bool StringToInteger(const std::string&, int&)const;
	//整數轉換為字串
	bool IntegerToString(int, std::string&)const;
	//檢查整數值是否合法
	bool ValueCheck(int)const;
	//整數字串轉換器
	const IntegerStringConverter& converter;
	//整數最大位數
	const std::string::size_type digit_max;
	//整數最小位數
	const std::string::size_type digit_min;
	//最大值
	const int value_max;
	//最小值
	const int value_min;
	//字串是否輸出前導零
	const bool output_lead_zero;
};

//浮點數字串剖析器
class FloatNumberDefinition {
public:
	FloatNumberDefinition(const FloatStringConverter&, double, double, double, int, std::string::size_type, bool, bool);
	~FloatNumberDefinition() {}
	//剖析字串並轉換為浮點數值
	bool StringToFloat(const std::string&, double&) const;
	//浮點數轉換為字串
	bool FloatToString(double, std::string&) const;
	//檢查浮點數值是否合法
	bool ValueCheck(double)const;
	//浮點數值字串轉換器
	const FloatStringConverter& converter;
	//是否允許前導零
	const bool lead_zero;
	//預設末尾小數點
	const bool calculator_type_decimal;
	//浮點數最大位數
	const int digit_max;
	//浮點數最小位數
	const std::string::size_type digit_min;
	//最大值
	const double value_max;
	//最小值
	const double value_min;
	//最小單位值
	const double least_increment;

private:
	//檢查字串是否合法
	bool VerifyString(const std::string&, std::string::size_type&)const;
};
