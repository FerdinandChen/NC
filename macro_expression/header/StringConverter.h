#pragma once
#include <string>

class IntegerStringConverter {
public:
	IntegerStringConverter() {}
	~IntegerStringConverter() {}
	void IntegerToString(int, std::string&)const;
	int StringToInteger(const std::string&)const;
};

class FloatStringConverter {
public:
	FloatStringConverter() {}
	~FloatStringConverter() {}
	void FloatToString(double, std::string&, int)const;
	bool StringToFloat(const std::string&, double&)const;
};
