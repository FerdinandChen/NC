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

//Fanuc巨集語言解析器產生器
class FanucMacroParserCreator :public MacroParserFactory {
public:
	FanucMacroParserCreator(MacroVariableInterface&);
	~FanucMacroParserCreator() = default;

	std::unique_ptr<MacroParser> CreateParser() override {
		return std::make_unique<FanucMacroParser>(macro_variable_interface);  }

private:
	MacroVariableInterface& macro_variable_interface;
};
