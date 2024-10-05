#include "NC_NumberDefinition.h"

using namespace std;

IntegerNumberDefinition::IntegerNumberDefinition(const IntegerStringConverter& integer_string_converter, string::size_type dmax, string::size_type dmin, int vmax, int vmin, bool lead_zero)
	:converter(integer_string_converter),
	digit_max(dmax),
	digit_min(dmin),
	value_max(vmax),
	value_min(vmin),
	output_lead_zero(lead_zero)
{
}

IntegerNumberDefinition::~IntegerNumberDefinition()
{
}

bool IntegerNumberDefinition::StringToInteger(const string& s, int& value)const
{
	//檢查字串是否為空
	if (s.empty()) {
		//最小位數不為零則返回錯誤
		if (digit_min != 0) {
			return false; }
		//最小位數為零(不帶數值的NC字語)
		else {
			//直接賦予最小值並返回成功
			value = value_min;
			return true;
		}
	}

	//正負號字元計數
	string::size_type plus_minus_count(0);
	//數字零計數
	string::size_type zero_count(0);
	//非零數字計數
	string::size_type non_zero_count(0);
	//數字計數(包含零及非零)
	string::size_type digit_count(0);

	//逐一處理字串內所有字元
	for (string::size_type iter = 0; iter != s.size(); ++iter) {
		//取得字元
		char ch = s[iter];
		//字元介於數字一到九
		if (ch >= '1' && ch <= '9') {
			//累計非零數字
			++non_zero_count;
			//累計數字
			++digit_count;
		}
		//字元為數字零
		else if (ch == '0') {
			//累計數字零
			++zero_count;
			//累計數字
			++digit_count;
		}
		//累計正負號
		else if (ch == '+' || ch == '-') {
			++plus_minus_count; }
		//非合法字元則返回錯誤
		else {
			return false; }
	}

	//正負號檢查
	if (plus_minus_count) {
		//超過一個正負號則返回錯誤
		if (plus_minus_count > 1) {
			return false; }
		//正負號位置不在字串開頭則返回錯誤
		if (s[0] != '+' && s[0] != '-') {
			return false; }
	}
	//檢查數字位數是否超出最大位數與最小位數的範圍
	if (digit_count < digit_min || digit_count > digit_max) {
		return false; }

	//將字串轉換為整數值
	int number(converter.StringToInteger(s));
	//無法成功轉換非零數字則返回錯誤
	if (number == 0 && non_zero_count) {
		return false; }
	//數值為合法值
	if (ValueCheck(number)) {
		//將成功轉換的整數值寫入參照value
		value = number;
		return true;
	}
	//數值不合法
	else {
		return false; }
}

bool IntegerNumberDefinition::IntegerToString(int value, string& output_string)const
{
	//整數值合法
	if (ValueCheck(value)) {
		//轉換整數值為字串
		converter.IntegerToString(value, output_string);
		//字串長度
		string::size_type digit(output_string.size());
		//檢查字串長度是否在最大位數及最小位數範圍內
		if (digit < digit_min || digit > digit_max) {
			return false; }
		//字串輸出前導零,整數值非負數並且位數仍有增加空間
		if (output_lead_zero && value > -1 && digit < digit_max) {
			//建立前導零字串
			string lead_string(digit_max - digit, '0');
			//串接前導零字串與輸出字串
			output_string = lead_string + output_string;
		}
		return true;
	}
	//整數值不合法
	else {
		return false; }
}

inline bool IntegerNumberDefinition::ValueCheck(int value)const
{
	//檢查整數值是否超出最大值或最小值的範圍
	if (value < value_min || value > value_max) {
		return false; }
	else {
		return true; }
}

FloatNumberDefinition::FloatNumberDefinition(const FloatStringConverter& float_string_converter, double vmax, double vmin, double increment, int dmax, string::size_type dmin, bool lead, bool decimal)
	:converter(float_string_converter),
	lead_zero(lead),
	calculator_type_decimal(decimal),
	digit_max(dmax),
	digit_min(dmin),
	value_max(vmax),
	value_min(vmin),
	least_increment(increment)
{
}

bool FloatNumberDefinition::StringToFloat(const string& value_string, double& value)const
{
	//小數點計數
	string::size_type decimal_count(0);
	//檢查輸入字串格式
	if (!VerifyString(value_string, decimal_count)) {
		return false; }

	//浮點數值暫存
	double number(0.0);
	//嘗試轉換字串為浮點數值
	if (!converter.StringToFloat(value_string, number)) {
		return false; }

	//數值不帶小數點時的預設單位倍率
	if (decimal_count == 0 && calculator_type_decimal == false) {
		number *= least_increment; }
	//浮點數值為合法值
	if (ValueCheck(number)) {
		//將浮點數值寫入參照value
		value = number;
		return true;
	}
	//數值不合法
	else {
		return false; }
}

bool FloatNumberDefinition::FloatToString(double value, string& value_string)const
{
	//浮點數值為合法值
	if (ValueCheck(value)) {
		//轉換浮點數為字串
		converter.FloatToString(value, value_string, digit_max);
		//小數點計數
		string::size_type decimal_count(0);
		//轉換後的字串合法
		if (VerifyString(value_string, decimal_count)) {
			//非預設小數點,浮點數值非零且不帶小數點(整數):字串末尾加入小數點
			if (!calculator_type_decimal && value != 0.0 && decimal_count == 0) value_string.push_back('.');
			return true;
		}
		//字串不合法
		else {
			return false; }
	}
	//數值不合法
	else {
		return false; }
}

inline bool FloatNumberDefinition::ValueCheck(double value)const
{
	//浮點數值為零
	if (value == 0.0) {
		return true; }
	//負浮點數值
	if (value < 0.0) {
		//轉換為正浮點數值
		value = fabs(value); }
	//檢查數值是否超出最大值或最小值的範圍
	if (value < value_min || value > value_max) {
		return false; }
	else {
		return true; }
}

bool FloatNumberDefinition::VerifyString(const std::string& s, string::size_type& decimal_count)const
{
	//小數點前有效位數
	string::size_type lead_digit_count(0);
	//前導零計數
	string::size_type lead_zero_count(0);
	//後導零計數
	string::size_type trail_zero_count(0);
	//非零數字計數
	string::size_type non_zero_count(0);
	//所有數字計數
	string::size_type all_digit_count(0);

	//逐一處理字串內所有字元
	for (string::const_iterator iter = s.begin(); iter != s.end(); ++iter) {
		//取得字元
		char ch = *iter;
		//字元為非零數字
		if (ch >= '1' && ch <= '9') {
			//小數點後:後導零計數歸零
			if (decimal_count != 0) {
				trail_zero_count = 0; }
			//累計非零數字
			++non_zero_count;
			//累計所有數字
			++all_digit_count;
		}
		//字元為數字零
		else if (ch == '0') {
			//小數點前
			if (decimal_count == 0) {
				//檢查並累計前導零
				if (non_zero_count == 0) {
					++lead_zero_count; }
			}
			//小數點後
			else {
				//累計後導零
				++trail_zero_count; }
			//累計所有數字
			++all_digit_count;
		}
		//字元為小數點
		else if (ch == '.') {
			//累計並檢查小數點
			if (++decimal_count > 1) {
				return false; }
			//記錄小數點前有效位數
			lead_digit_count = all_digit_count - lead_zero_count;
		}
		//檢查正負號及位置
		else if (ch == '+' || ch == '-') {
			if (iter != s.begin()) {
				return false; }
		}
		//返回錯誤:非合法字元
		else {
			return false; }
	}
	//返回錯誤:不存在任何有效數字
	if (all_digit_count == 0) {
		return false; }
	//返回錯誤:若不允許前導零
	if (!lead_zero && lead_zero_count) {
		return false; }
	//有效數字位數扣除後導零數量
	all_digit_count -= trail_zero_count;

	//小數點後有效位數
	size_t trail_digit_count(0);
	//無小數點
	if (decimal_count == 0) {
		//計算小數點前有效位數
		lead_digit_count = all_digit_count - lead_zero_count;
		//有效位數不為零且默認小數點
		if (lead_digit_count != 0 && calculator_type_decimal) {
			//依據最小單位計算小數點後有效位數
			if (least_increment == 0.001) {
				trail_digit_count = 3; }
			else if (least_increment == 0.0001) {
				trail_digit_count = 4; }
			else if (least_increment == 0.01) {
				trail_digit_count = 2; }
			else {
				return false; }
		}
	}
	//有小數點
	else {
		//計算小數點後有效位數
		trail_digit_count = all_digit_count - lead_zero_count - lead_digit_count;
		//依據最小單位檢查小數點後有效位數
		if (least_increment == 0.001) {
			if (trail_digit_count > 3) {
				return false; }
		}
		else if (least_increment == 0.0001) {
			if (trail_digit_count > 4) {
				return false; }
		}
		else if (least_increment == 0.01) {
			if (trail_digit_count > 2) {
				return false; }
		}
		else {
			return false; }
	}

	//計算小數點前後總有效位數
	size_t total_digit_count(lead_digit_count + trail_digit_count);
	//檢查總有效位數是否合法
	if (total_digit_count != 0) {
		if (total_digit_count > digit_max || total_digit_count < digit_min) {
			return false; }
	}

	return true;
}
