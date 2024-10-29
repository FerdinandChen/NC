#pragma once
#include <memory>
#include <string>
#include "MacroVariable.h"
#include "FanucMacroParser.h"

//巨集語言解析器抽象工廠
class MacroParserFactory {
public:
	MacroParserFactory() = default;
	~MacroParserFactory() = default;

	virtual std::unique_ptr<MacroParser> CreateParser() = 0;
};

//通用巨集語言產生器
template<typename Type>
class GeneralParserCreator :public MacroParserFactory {
public:
	GeneralParserCreator(MacroVariableInterface& mvi)
		:macro_variable_interface(mvi){}
	~GeneralParserCreator() = default;

	std::unique_ptr<MacroParser> CreateParser() override {
		return std::make_unique<Type>(macro_variable_interface); }
private:
	MacroVariableInterface& macro_variable_interface;
};
