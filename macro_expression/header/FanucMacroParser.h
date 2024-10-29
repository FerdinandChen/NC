#pragma once

#include <string>
#include <queue>
#include <deque>
#include <stack>
#include <cfloat>
#include "ControllerParameter.h"
#include "MacroVariable.h"
#include "MacroOperator.h"
#include "NC_NumberDefinition.h"
#include "StringConverter.h"

//不合法整數值
constexpr int INVALID_NUMBER_VALUE = INT_MAX;
//不合法浮點數值
constexpr double INVALID_FLOAT_VALUE = DBL_MAX;
//巨集暫存容器最大層數
constexpr size_t PRIORITY_NESTING_LEVEL_MAX = 5;

//巨集運算子分類
enum OperatorType {
	NOT_AN_OPERATOR,
	UNARY_OPERATOR,
	UNARY_BINARY_OPERATOR,
	BINARY_OPERATOR,
	PRIORITY_BINARY_OPERATOR,
	UNARY_FUNCTION_OPERATOR,
	UNARY_BINARY_FUNCTION_OPERATOR,
	BINARY_FUNCTION_OPERATOR,
	RELATIONAL_OPERATOR,
	UNARY_LOGICAL_OPERATOR,
	BINARY_LOGICAL_OPERATOR,
	PRIORITY_BINARY_LOGICAL_OPERATOR,
	IF_CONDITION,
	WHILE_CONDITION,
	BRANCH_OPERATOR,
	CONDITIONAL_ARITHMETIC_OPERATOR,
	LOOP_OPERATOR,
	LOOP_END
};

//巨集運算子關鍵字元
constexpr unsigned char ADDRESS_MINUS_SUBTRACT_OPERATOR = '-';
constexpr unsigned char ADDRESS_VARIABLE_OPERATOR = '#';
constexpr unsigned char ADDRESS_ADD_OPERATOR = '+';
constexpr unsigned char ADDRESS_MULTIPLY_OPERATOR = '*';
constexpr unsigned char ADDRESS_DIVIDE_OPERATOR = '/';
constexpr unsigned char ADDRESS_ASSIGNMENT_OPERATOR = '=';
//巨集優先運算式關鍵字元
constexpr unsigned char ADDRESS_PRIORITY_RANGE_BEGIN = '[';
constexpr unsigned char ADDRESS_PRIORITY_RANGE_END = ']';
//註解關鍵字元
constexpr unsigned char ADDRESS_COMMENT_BEGIN = '(';
constexpr unsigned char ADDRESS_COMMENT_END = ')';
//巨集函數引數分隔字元
constexpr unsigned char ADDRESS_ARGUMENT_SEPARATOR = ',';

//巨集三角函數關鍵字
constexpr auto KEYWORD_SINE_OPERATOR = "SIN";
constexpr auto KEYWORD_COSINE_OPERATOR = "COS";
constexpr auto KEYWORD_TANGENT_OPERATOR = "TAN";
constexpr auto KEYWORD_ARC_SINE_OPERATOR = "ASIN";
constexpr auto KEYWORD_ARC_COSINE_OPERATOR = "ACOS";
constexpr auto KEYWORD_ARC_TANGENT_OPERATOR = "ATAN";

//巨集函數關鍵字
constexpr auto KEYWORD_SQUARE_ROOT_OPERATOR = "SQRT";
constexpr auto KEYWORD_ABSOLUTE_VALUE_OPERATOR = "ABS";
constexpr auto KEYWORD_ROUND_OFF_OPERATOR = "ROUND";
constexpr auto KEYWORD_ROUND_DOWN_OPERATOR = "FIX";
constexpr auto KEYWORD_ROUND_UP_OPERATOR = "FUP";
constexpr auto KEYWORD_NATURAL_LOG_OPERATOR = "LN";
constexpr auto KEYWORD_EXPONENT_OPERATOR = "EXP";
constexpr auto KEYWORD_POWER_OPERATOR = "POW";
constexpr auto KEYWORD_BINARY_CODE_OPERATOR = "BIN";
constexpr auto KEYWORD_BINARY_CODED_DECIMAL_OPERATOR = "BCD";
constexpr auto KEYWORD_ADD_DECIMAL_POINT_OPERATOR = "ADP";

//巨集關係運算子關鍵字
constexpr auto KEYWORD_EQUAL_OPERATOR = "EQ";
constexpr auto KEYWORD_NOT_EQUAL_OPERATOR = "NE";
constexpr auto KEYWORD_GREATER_OPERATOR = "GT";
constexpr auto KEYWORD_GREATER_EQUAL_OPERATOR = "GE";
constexpr auto KEYWORD_LESS_OPERATOR = "LT";
constexpr auto KEYWORD_LESS_EQUAL_OPERATOR = "LE";

//巨集邏輯運算子關鍵字
constexpr auto KEYWORD_AND_OPERATOR = "AND";
constexpr auto KEYWORD_OR_OPERATOR = "OR";
constexpr auto KEYWORD_XOR_OPERATOR = "XOR";

//巨集條件式運算子關鍵字
constexpr auto KEYWORD_IF_CONDITION = "IF";
constexpr auto KEYWORD_CONDITIONAL_ARITHMETIC_OPERATOR = "THEN";

//巨集分支運算子關鍵字
constexpr auto KEYWORD_BRANCH_OPERATOR = "GOTO";

//巨集迴圈運算子關鍵字
constexpr auto KEYWORD_WHILE_CONDITION = "WHILE";
constexpr auto KEYWORD_LOOP_OPERATOR = "DO";
constexpr auto KEYWORD_LOOP_END = "END";

//巨集運算式剖析資料
class MacroParsingContext {
public:
	MacroParsingContext();
	~MacroParsingContext() {}

	//優先運算子旗標
	bool flag_priority;
	//優先邏輯運算子旗標
	bool flag_priority_logical;
	//雙參數旗標
	bool flag_two_arguments;
	//一元運算子位址字元暫存器
	std::stack<unsigned char> unary_operator_address;
	//二元運算子位址字元暫存器
	std::deque<unsigned char> binary_operator_address;
	//優先二元運算子位址字元暫存器
	std::queue<unsigned char> priority_binary_operator_address;
	//一元運算子關鍵字暫存器
	std::stack<std::string> unary_operator_keyword;
	//一元及二元運算子通用關鍵字暫存器
	std::stack<std::string> unary_binary_operator_keyword;
	//二元運算子關鍵字暫存器
	std::stack<std::string> binary_operator_keyword;
	//關係運算子關鍵字暫存器
	std::deque<std::string> relational_keyword;
	//邏輯運算子關鍵字暫存器
	std::deque<std::string> logical_keyword;
	//優先邏輯運算子關鍵字暫存器
	std::deque<std::string> priority_logical_keyword;
	//條件式運算子關鍵字暫存器
	std::deque<std::string> conditional_keyword;
	//數字字串暫存器
	std::stack<std::string> digit_string;
	//大寫字串暫存器
	std::stack<std::string> upper_string;
	//通用運算子暫存器
	std::deque<GeneralOperatorHandle> general_operators;
	//優先通用運算子暫存器
	std::deque<GeneralOperatorHandle> priority_general_operators;
	//清除所有巨集暫存容器
	void ClearContainers();
	//清除全部
	void ClearAll();

private:
	template<typename T>
	//清除配接式容器
	void ClearAdapterContainer(T& c) {
		while (!c.empty()) c.pop(); }
};

//巨集運算式產生器
class MacroGenerator {
public:
	MacroGenerator(MacroVariableInterface&);
	~MacroGenerator() {}
	void Clear();
	//目前優先運算運算式的嵌套層數
	size_t CurrentLevel() const {
		return current_nesting_level; }
	//進入新的優先運算式嵌套層
	bool LevelUp();
	//退出目前的優先運算式嵌套層
	bool LevelDown();
	//是否處於雙引數模式
	bool& TwoArgumentsFlag() {
		return context[current_nesting_level].flag_two_arguments;
	}
	//是否處於巨集運算子模式
	bool IsMacroMode();
	//查詢運算子類型
	OperatorType CheckOperatorType(unsigned char);

	//建立數字字串
	void CreateDigitString(std::string::const_iterator, std::string::const_iterator);
	//建立常數運算子
	bool CreateConstantOperator();
	//建立一元算術運算子
	bool CreateUnaryAddressOperator();
	//建立一元函數運算子
	bool CreateUnaryFunctionOperator(const std::shared_ptr<ArithmeticOperator>&);
	//建立二元函數運算子
	bool CreateBinaryFunctionOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	//建立一般二元算術運算子
	bool CreateBinaryOperator();
	//建立優先二元算術運算子
	bool CreatePriorityBinaryOperator();
	//建立關係運算子
	bool CreateRelationalOperator();
	//建立一般邏輯運算子
	bool CreateLogicalOperator();
	//建立優先邏輯運算子
	bool CreatePriorityLogicalOperator();
	//建立條件式運算子
	bool CreateConditionalOperator();
	//判別並處理關鍵字運算子
	bool ProcessOperatorKeyword();
	//判別並處理算術運算子位址字元
	bool ProcessOperatorAddress(unsigned short, unsigned char);
	//判別並處理一元與二元通用算術運算子位址字元
	bool ProcessUnaryBinaryAddress(unsigned char);
	//檢查並處理算式末端運算子
	bool ProcessToFinalOperator();
	//將完成的優先運算子移回前一層
	bool ReturnPriorityOperatorToPreviousLevel();
	//取得數字字串容器
	std::stack<std::string>& DigitString() {
		return context[current_nesting_level].digit_string;
	}
	//取得大寫字母字串容器
	std::stack<std::string>& UpperString() {
		return context[current_nesting_level].upper_string;
	}
	//取得通用運算子容器
	std::deque<GeneralOperatorHandle>& GeneralOperators() {
		return context[current_nesting_level].general_operators; }

	//條件式算術運算子暫存
	ConditionalArithmeticOperator conditional_arithmetic_operator;
	//條件式分支運算子暫存
	ConditionalBranchOperator conditional_branch_operator;
	//條件式迴圈運算子暫存
	ConditionalLoopOperator conditional_loop_operator;
	//迴圈終點運算子暫存
	LoopEndOperator loop_end_operator;

private:
	//初始化巨集關鍵字清單
	void InitialKeywordList();
	//初始化巨集禁用位址字元清單
	void InitialDenyAddress();
	//取得優先運算子旗標
	bool& PriorityFlag() {
		return context[current_nesting_level].flag_priority;
	}
	//取得優先邏輯運算子旗標
	bool& PriorityLogicalFlag() {
		return context[current_nesting_level].flag_priority_logical;
	}
	//取得一元運算子位址字元容器
	std::stack<unsigned char>& UnaryOperatorAddress() {
		return context[current_nesting_level].unary_operator_address;
	}
	//取得二元運算子位址字元容器
	std::deque<unsigned char>& BinaryOperatorAddress() {
		return context[current_nesting_level].binary_operator_address;
	}
	//取得優先二元運算子位址字元容器
	std::queue<unsigned char>& PriorityBinaryOperatorAddress() {
		return context[current_nesting_level].priority_binary_operator_address;
	}
	//取得一元運算子關鍵字容器
	std::stack<std::string>& UnaryOperatorKeyword() {
		return context[current_nesting_level].unary_operator_keyword;
	}
	//取得通用一元及二元運算子關鍵字容器
	std::stack<std::string>& UnaryBinaryOperatorKeyword() {
		return context[current_nesting_level].unary_binary_operator_keyword;
	}
	//取得二元運算子關鍵字容器
	std::stack<std::string>& BinaryOperatorKeyword() {
		return context[current_nesting_level].binary_operator_keyword;
	}
	//取得關係運算子關鍵字容器
	std::deque<std::string>& RelationalKeyword() {
		return context[current_nesting_level].relational_keyword;
	}
	//取得邏輯運算子關鍵字容器
	std::deque<std::string>& LogicalKeyword() {
		return context[current_nesting_level].logical_keyword;
	}
	//取得優先邏輯運算子關鍵字容器
	std::deque<std::string>& PriorityLogicalKeyword() {
		return context[current_nesting_level].priority_logical_keyword;
	}
	//取得條件運算子關鍵字容器
	std::deque<std::string>& ConditionalKeyword() {
		return context[current_nesting_level].conditional_keyword;
	}
	//取得優先通用運算子容器
	std::deque<GeneralOperatorHandle>& PriorityGeneralOperators() {
		return context[current_nesting_level].priority_general_operators;
	}

	//目前優先運算運算式的嵌套層數
	size_t current_nesting_level;
	//浮點數字串轉換器
	FloatStringConverter converter;
	//浮點數值定義
	FloatNumberDefinition macro_float_parser;
	//巨集變數存取介面
	MacroVariableInterface& macro_variable_interface;
	//巨集關鍵字清單
	std::map<std::string, unsigned char> keyword_list;
	//禁用巨集位址字元清單
	std::set<char> macro_deny_address;
	//巨集算式剖析資料暫存器
	MacroParsingContext context[PRIORITY_NESTING_LEVEL_MAX + 1];
};

class Argument {
public:
	Argument();
	~Argument() {}
	
	//註解旗標
	bool flag_control_out;
	//數字字元旗標
	bool flag_digit;
	//大寫字母字元旗標
	bool flag_upper;
	//NC位址字元暫存器
	unsigned char NC_address;

	//數字字元起始位置
	std::string::const_iterator digit_begin;
	//大寫字元起始位置
	std::string::const_iterator upper_begin;
	//字串字元迭代器
	std::string::const_iterator iter;
};

//指令類型
enum CommandType {
	//不合法指令
	INVALID_COMMAND,
	//不支援指令
	UNKNOWN_COMMAND,
	//NC指令
	NC_COMMAND,
	//巨集指令
	MACRO_COMMAND,
};

class MacroParser {
public:
	MacroParser() = default;
	virtual ~MacroParser() = default;

	virtual CommandType ParseBlock(const std::string& block) = 0;
};

//巨集單節剖析器
class FanucMacroParser:public MacroParser {
public:
	FanucMacroParser(MacroVariableInterface&);
	~FanucMacroParser() = default;
	//剖析NC碼單節
	CommandType ParseBlock(const std::string& block);
	//巨集運算子產生器
	MacroGenerator macro_generator;

private:
	//查詢字元是否為數字相關
	bool IsDigitOrDot(unsigned char ch) const {
		return isdigit(ch) || ch == '.'; }
	//建立位址字元或關鍵字字串
	bool CreateAddressOrKeyword(Argument& args);
	//建立一元運算子或常數運算子
	bool CreateUnaryOrConstantOperator(Argument& args);
};
