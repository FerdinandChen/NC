#pragma once
#include <memory>
#include <string>
#include "MacroVariable.h"
#include "FanucMacroParser.h"

//�����y���ѪR����H�u�t
class MacroParserFactory {
public:
	MacroParserFactory() = default;
	~MacroParserFactory() = default;

	virtual std::unique_ptr<MacroParser> CreateParser() = 0;
};

//�q�Υ����y�����;�
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
