#include "StringConverter.h"
#include <cstdlib>

using namespace std;

void IntegerStringConverter::IntegerToString(int value,string &value_string)const
{
	//整數值字串暫存緩衝區
	char buffer[33]{};
	//轉換整數值為字串到緩衝區
	_itoa_s(value, buffer, 32, 10);
	//建立輸出字串
	value_string = buffer;
}

int IntegerStringConverter::StringToInteger(const string &value_string)const
{
	//將字串轉換為整數int
	return atoi(value_string.c_str());
}

void FloatStringConverter::FloatToString(double value,string &value_string,int significant_digits) const
{
	//字元字串暫存緩衝區
	char buffer[33]{};
	//轉換浮點數值為字串到緩衝區
	_gcvt_s(buffer, value, significant_digits);
	//建立輸出字串
	value_string = buffer;
}

bool FloatStringConverter::StringToFloat(const string &value_string,double &value)const
{
	//字串轉換停止位置指標
	char* end_ptr(NULL);
	//字串轉換為浮點數值
	value = strtod(value_string.c_str(), &end_ptr);
	//檢查字串轉換停止位置是否在字串結束位置
	if (end_ptr != value_string.c_str() + value_string.size()) {
		return false; }
	//檢查轉換過程是否發生溢位
	if (value == HUGE_VAL || value == -HUGE_VAL) {
		return false; }
	else {
		return true; }
}
