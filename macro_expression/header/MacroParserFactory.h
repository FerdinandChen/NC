#pragma once
#include <memory>
#include <string>
#include "MacroVariable.h"
#include "FanucMacroParser.h"

class MacroParserFactory {
public:
	MacroParserFactory() = default;
	~MacroParserFactory() = default;

	virtual std::unique_ptr<MacroParser> CreateParser() = 0;
};

class FanucMacroParserCreator :public MacroParserFactory {
public:
	FanucMacroParserCreator(MacroVariableInterface&);
	~FanucMacroParserCreator() = default;

	std::unique_ptr<MacroParser> CreateParser() {
		return std::make_unique<FanucMacroParser>(macro_variable_interface);  }

private:
	MacroVariableInterface& macro_variable_interface;
};
