#include "MacroParser.h"
#include <cstdlib>

using namespace std;

MacroParsingContext::MacroParsingContext()
	:flag_priority(false),
	flag_priority_logical(false),
	flag_two_arguments(false)
{
}

void MacroParsingContext::ClearContainers()
{
	ClearAdapterContainer(unary_operator_address);
	binary_operator_address.clear();
	ClearAdapterContainer(priority_binary_operator_address);
	ClearAdapterContainer(unary_operator_keyword);
	ClearAdapterContainer(unary_binary_operator_keyword);
	ClearAdapterContainer(binary_operator_keyword);
	relational_keyword.clear();
	logical_keyword.clear();
	priority_logical_keyword.clear();
	conditional_keyword.clear();
	ClearAdapterContainer(digit_string);
	ClearAdapterContainer(upper_string);
	general_operators.clear();
	priority_general_operators.clear();
}

void MacroParsingContext::ClearAll()
{
	flag_priority = false;
	flag_priority_logical = false;
	flag_two_arguments = false;
	ClearContainers();
}

MacroGenerator::MacroGenerator(MacroVariableInterface &interface)
	:current_nesting_level(0),
	//value max,value min,increment,digits max,digits min,lead zero,calculator type decimal
	macro_float_parser(converter, DBL_MAX, DBL_MIN, 0.001, 15, 1, true, true),
	macro_variable_interface(interface)
{
	InitialKeywordList();
}

void MacroGenerator::Clear()
{
	conditional_arithmetic_operator.Clear();
	conditional_branch_operator.Clear();
	conditional_loop_operator.Clear();
	loop_end_operator.Clear();

	current_nesting_level = 0;
	for (size_t i = 0; i != PRIORITY_NESTING_LEVEL_MAX + 1; ++i) {
		context[i].ClearAll(); }
}

bool MacroGenerator::LevelUp()
{
	if (current_nesting_level == PRIORITY_NESTING_LEVEL_MAX) {
		return false; }
	else {
		++current_nesting_level;
		return true;
	}
}

bool MacroGenerator::LevelDown()
{
	if (current_nesting_level == 0) return false;
	else {
		--current_nesting_level;
		return true;
	}
}

bool MacroGenerator::IsMacroMode()
{
	//目前有優先運算層
	if (current_nesting_level != 0 ||
		//有一元運算子且並非minus運算子
		!context[0].unary_operator_address.empty() && context[0].unary_operator_address.top() != '-' ||
		//有二元運算子
		!context[0].binary_operator_address.empty() ||
		//有條件運算子關鍵字
		!context[0].conditional_keyword.empty()
		) return true;
	else return false;
}

OperatorType MacroGenerator::CheckOperatorType(unsigned char ch)
{
	switch (ch) {
		//變數運算子
	case ADDRESS_VARIABLE_OPERATOR:
		return OperatorType::UNARY_OPERATOR;

		//賦值運算子
	case ADDRESS_ASSIGNMENT_OPERATOR:
		//加法運算子
	case ADDRESS_ADD_OPERATOR:
		return OperatorType::BINARY_OPERATOR;

		//乘法運算子
	case ADDRESS_MULTIPLY_OPERATOR:
		return OperatorType::PRIORITY_BINARY_OPERATOR;

		//除法運算子
	case ADDRESS_DIVIDE_OPERATOR:
		if (PriorityFlag()) {
			if (!PriorityGeneralOperators().empty()) {
				return OperatorType::PRIORITY_BINARY_OPERATOR; }
		}
		else {
			if (!GeneralOperators().empty()) {
				return OperatorType::PRIORITY_BINARY_OPERATOR; }
		}
		return OperatorType::NOT_AN_OPERATOR;

		//負值或減法運算子
	case ADDRESS_MINUS_SUBTRACT_OPERATOR:
		return OperatorType::UNARY_BINARY_OPERATOR;

		//非運算子字元
	default: return OperatorType::NOT_AN_OPERATOR;
	}
}

void MacroGenerator::CreateDigitString(string::const_iterator begin, string::const_iterator end)
{
	if (!UnaryOperatorAddress().empty() && UnaryOperatorAddress().top() == '-') {
		string digit_string;
		digit_string.push_back('-');
		digit_string += string(begin, end);
		DigitString().push(digit_string);
		UnaryOperatorAddress().pop();
	}
	else {
		DigitString().push(string(begin, end));
	}
}

bool MacroGenerator::CreateConstantOperator()
{
	//浮點數值暫存器
	double value(0.0);
	//將數字字串轉換為浮點數值
	if (macro_float_parser.StringToFloat(DigitString().top(), value) == true) {

		shared_ptr<ArithmeticOperator> handle(new ConstantOperator(value));
		//建立通用運算子Handle
		GeneralOperatorHandle constant(handle);
		//優先算術運算子模式或優先邏輯運算子模式:存入優先運算子容器
		if (PriorityFlag() || PriorityLogicalFlag()) PriorityGeneralOperators().push_back(constant);
		//一般運算子模式:存入一般運算子容器
		else GeneralOperators().push_back(constant);
		//刪除數字字串
		DigitString().pop();
		return true;
	}
	//返回錯誤:轉換發生錯誤
	else return false;
}

bool MacroGenerator::CreateUnaryAddressOperator()
{
	//運算子成功建立旗標
	bool result(false);

	//持續處理存在的一元運算子位址字元
	while (!UnaryOperatorAddress().empty()) {
		//判斷一元運算子位址字元
		switch (UnaryOperatorAddress().top()) {
			//位址字元為變數運算子
		case ADDRESS_VARIABLE_OPERATOR:
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//返回錯誤:容器內無運算元或非算術運算子
				if (PriorityGeneralOperators().empty() || !PriorityGeneralOperators().back().arithmetic) return false;
				//有運算元且確認為算術運算子
				else {
					shared_ptr<ArithmeticOperator> handle(new VariableOperator(macro_variable_interface, PriorityGeneralOperators().back().arithmetic));
					//建立通用(變數)運算子Handle
					GeneralOperatorHandle variable(handle);
					//刪除一元運算子位址字元
					UnaryOperatorAddress().pop();
					//刪除運算元(前優先運算子)
					PriorityGeneralOperators().pop_back();
					//存放新優先運算子
					PriorityGeneralOperators().push_back(variable);
					//設立運算子建立旗標
					result = true;
					break;
				}
			}
			//一般運算子模式
			else {
				//返回錯誤:容器內無運算元或非算術運算子
				if (GeneralOperators().empty() || !GeneralOperators().back().arithmetic) return false;
				//有運算元且確認為算術運算子
				else {
					shared_ptr<ArithmeticOperator> handle(new VariableOperator(macro_variable_interface, GeneralOperators().back().arithmetic));
					//建立變數運算子Handle
					GeneralOperatorHandle variable(handle);
					//刪除一元運算子位址字元
					UnaryOperatorAddress().pop();
					//刪除運算元(前運算子)
					GeneralOperators().pop_back();
					//存放新運算子
					GeneralOperators().push_back(variable);
					//設立運算子建立旗標
					result = true;
					break;
				}
			}
			//位址字元為補數或減法運算子
		case ADDRESS_MINUS_SUBTRACT_OPERATOR:
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//返回錯誤:容器內無運算元或非算術運算子
				if (PriorityGeneralOperators().empty() || !PriorityGeneralOperators().back().arithmetic) return false;
				//有運算元且確認為算術運算子
				else {
					shared_ptr<ArithmeticOperator> handle(new MinusOperator(PriorityGeneralOperators().back().arithmetic));
					//建立通用(補數)運算子Handle
					GeneralOperatorHandle minus(handle);
					//刪除一元運算子位址字元
					UnaryOperatorAddress().pop();
					//刪除運算元(前優先運算子)
					PriorityGeneralOperators().pop_back();
					//存放新優先運算子
					PriorityGeneralOperators().push_back(minus);
					//設立運算子建立旗標
					result = true;
					break;
				}
			}
			//一般運算子模式
			else {
				//返回錯誤:容器內無運算元或非算術運算子
				if (GeneralOperators().empty() || !GeneralOperators().back().arithmetic) return false;
				//有運算元且確認為算術運算子
				else {
					shared_ptr<ArithmeticOperator> handle(new MinusOperator(GeneralOperators().back().arithmetic));
					//建立通用(補數)運算子Handle
					GeneralOperatorHandle minus(handle);
					//刪除一元運算子位址字元
					UnaryOperatorAddress().pop();
					//刪除運算元(前運算子)
					GeneralOperators().pop_back();
					//存放新運算子
					GeneralOperators().push_back(minus);
					//設立運算子建立旗標
					result = true;
					break;
				}
			}
		//不合法位址字元
		default: return false;
		}
	}
	return result;
}

bool MacroGenerator::CreateUnaryFunctionOperator(const shared_ptr<ArithmeticOperator>& operand)
{
	//檢查一元函數關鍵字容器
	if (!UnaryOperatorKeyword().empty()) {
		//取出關鍵字字串
		string keyword(UnaryOperatorKeyword().top());
		//關鍵字串為Sine函數運算子
		if (keyword == KEYWORD_SINE_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new SineOperator(operand));
			//建立Sine函數運算子Handle
			GeneralOperatorHandle sine(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(sine);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(sine);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Cosine函數運算子
		else if (keyword == KEYWORD_COSINE_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new CosineOperator(operand));
			//建立Cosine函數運算子Handle
			GeneralOperatorHandle cosine(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(cosine);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(cosine);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Tangent函數運算子
		else if (keyword == KEYWORD_TANGENT_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new TangentOperator(operand));
			//建立Tangent函數運算子Handle
			GeneralOperatorHandle tangent(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(tangent);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(tangent);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Arc Sine函數運算子
		else if (keyword == KEYWORD_ARC_SINE_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new ArcSineOperator(operand));
			//建立Arc Sine函數運算子Handle
			GeneralOperatorHandle arc_sine(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(arc_sine);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(arc_sine);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Arc Cosine函數運算子
		else if (keyword == KEYWORD_ARC_COSINE_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new ArcCosineOperator(operand));
			//建立Arc Cosine函數運算子Handle
			GeneralOperatorHandle arc_cosine(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(arc_cosine);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(arc_cosine);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Square Root函數運算子
		else if (keyword == KEYWORD_SQUARE_ROOT_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new SquareRootOperator(operand));
			//建立Square Root函數運算子Handle
			GeneralOperatorHandle square_root(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(square_root);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(square_root);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Absolute Value函數運算子
		else if (keyword == KEYWORD_ABSOLUTE_VALUE_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new AbsoluteValueOperator(operand));
			//建立Absolute Value函數運算子Handle
			GeneralOperatorHandle absolute_value(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(absolute_value);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(absolute_value);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		else if (keyword == KEYWORD_BINARY_CODE_OPERATOR) {
			return false;
		}
		else if (keyword == KEYWORD_BINARY_CODED_DECIMAL_OPERATOR) {
			return false;
		}
		//關鍵字串為四捨五入函數運算子
		else if (keyword == KEYWORD_ROUND_OFF_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new RoundOffOperator(operand));
			//建立四捨五入函數運算子Handle
			GeneralOperatorHandle round_off(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(round_off);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(round_off);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Round Down函數運算子
		else if (keyword == KEYWORD_ROUND_DOWN_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new RoundDownOperator(operand));
			//建立Round Down函數運算子Handle
			GeneralOperatorHandle round_down(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(round_down);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(round_down);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Round Up函數運算子
		else if (keyword == KEYWORD_ROUND_UP_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new RoundUpOperator(operand));
			//建立Round Up函數運算子Handle
			GeneralOperatorHandle round_up(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(round_up);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(round_up);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Natural Log函數運算子
		else if (keyword == KEYWORD_NATURAL_LOG_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new NaturalLogOperator(operand));
			//建立Natural Log函數運算子Handle
			GeneralOperatorHandle natural_log(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(natural_log);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(natural_log);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字串為Exponent函數運算子
		else if (keyword == KEYWORD_EXPONENT_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new ExponentOperator(operand));
			//建立Exponent函數運算子Handle
			GeneralOperatorHandle exponent(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(exponent);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(exponent);
			}
			//刪除容器內關鍵字字串
			UnaryOperatorKeyword().pop();
			return true;
		}
		else if (keyword == KEYWORD_ADD_DECIMAL_POINT_OPERATOR) {
			return false;
		}
		//不合法關鍵字
		else return false;
	}
	//檢查一元二元通用函數關鍵字容器
	else if (!UnaryBinaryOperatorKeyword().empty()) {
		//取出關鍵字字串
		string keyword(UnaryBinaryOperatorKeyword().top());
		//關鍵字串為Arc Tangent函數運算子
		if (keyword == KEYWORD_ARC_TANGENT_OPERATOR) {
			shared_ptr<ArithmeticOperator> handle(new ArcTangentOperator(operand));
			//建立Arc Tangent函數運算子Handle
			GeneralOperatorHandle arc_tangent(handle);
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建運算子於優先運算子容器
				PriorityGeneralOperators().push_back(arc_tangent);
			}
			//一般運算子模式
			else {
				//存放新建運算子於一般運算子容器
				GeneralOperators().push_back(arc_tangent);
			}
			//刪除容器內關鍵字字串
			UnaryBinaryOperatorKeyword().pop();
			return true;
		}
		//關鍵字不合法
		else return false;
	}
	//容器內無關鍵字
	else return false;
}

bool MacroGenerator::CreateBinaryFunctionOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
{
	//檢查二元函數關鍵字容器
	if (!BinaryOperatorKeyword().empty()) {
		//取出關鍵字字串
		string keyword(BinaryOperatorKeyword().top());
		//關鍵字串為Power函數運算子
		if (keyword == KEYWORD_POWER_OPERATOR) {
			//建立Power函數運算子Handle
			shared_ptr<ArithmeticOperator> power(new PowerOperator(left_operand, right_operand));
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建函數運算子於優先運算子容器
				PriorityGeneralOperators().push_back(GeneralOperatorHandle(power));
			}
			//一般運算子模式
			else {
				//存放新建函數運算子於一般運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(power));
			}
			//刪除容器內關鍵字串
			BinaryOperatorKeyword().pop();
			return true;
		}
		//不合法關鍵字
		else return false;
	}
	//檢查一元與二元通用函數關鍵字容器
	else if (!UnaryBinaryOperatorKeyword().empty()) {
		//取出關鍵字字串
		string keyword(UnaryBinaryOperatorKeyword().top());
		//關鍵字為Arc Tangent函數運算子
		if (keyword == KEYWORD_ARC_TANGENT_OPERATOR) {
			//建立Arc Tangent2函數運算子Handle
			shared_ptr<ArithmeticOperator> arc_tangent2(new ArcTangent2Operator(left_operand, right_operand));
			//優先運算子模式
			if (PriorityFlag() || PriorityLogicalFlag()) {
				//存放新建函數運算子於優先運算子容器
				PriorityGeneralOperators().push_back(GeneralOperatorHandle(arc_tangent2));
			}
			//一般運算子模式
			else {
				//存放新建函數運算子於一般運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(arc_tangent2));
			}
			//刪除容器內關鍵字串
			UnaryBinaryOperatorKeyword().pop();
			return true;
		}
		//不合法關鍵字
		else return false;
	}
	//關鍵字容器均為空
	else return false;
}

bool MacroGenerator::CreateBinaryOperator()
{
	//新運算子Handle
	shared_ptr<ArithmeticOperator> next_operator;
	//前運算子Handle
	shared_ptr<ArithmeticOperator> last_operator;
	//賦值運算子之左運算元暫存
	shared_ptr<ArithmeticOperator> assignment_left_operand;
	//函數雙引數之第一引數暫存
	shared_ptr<ArithmeticOperator> function_first_argument;

	//檢查函數雙引數旗標並確認容器內有算術運算元存在
	if (TwoArgumentsFlag() && !GeneralOperators().empty() && GeneralOperators().front().arithmetic) {
		//複製並暫存第一引數
		function_first_argument = GeneralOperators().front().arithmetic;
		//移除容器內第一引數
		GeneralOperators().pop_front();
	}

	//容器內仍有二元運算子位址字元則持續處理
	while (!BinaryOperatorAddress().empty()) {
		//取得下一個二元運算子位址字元
		unsigned char address(BinaryOperatorAddress().front());
		//前運算子為空Handle且容器內至少有二個運算子
		if (!last_operator && GeneralOperators().size() > 1) {
			//複製左運算元
			GeneralOperatorHandle left_operand(GeneralOperators().front());
			//清除容器內左運算元
			GeneralOperators().pop_front();
			//複製右運算元
			GeneralOperatorHandle right_operand(GeneralOperators().front());
			//清除容器內右運算元
			GeneralOperators().pop_front();
			//左右運算元均為算術運算子
			if (left_operand.arithmetic && right_operand.arithmetic) {
				//建立前運算子(加法):使用左右運算元
				if (address == ADDRESS_ADD_OPERATOR) last_operator = shared_ptr<ArithmeticOperator>(new AddOperator(left_operand.arithmetic, right_operand.arithmetic));
				//建立前運算子(減法):使用左右運算元
				else if (address == ADDRESS_MINUS_SUBTRACT_OPERATOR) last_operator = shared_ptr<ArithmeticOperator>(new SubtractOperator(left_operand.arithmetic, right_operand.arithmetic));
				//建立前運算子(賦值):使用左右運算元
				else if (address == ADDRESS_ASSIGNMENT_OPERATOR) {
					//運算元剛好只有二個,直接建立賦值運算子
					if (GeneralOperators().empty()) last_operator = shared_ptr<ArithmeticOperator>(new AssignmentOperator(left_operand.arithmetic, right_operand.arithmetic));
					//運算元超過兩個
					else {
						//暫存賦值左運算元
						assignment_left_operand = left_operand.arithmetic;
						//回存右運算元
						GeneralOperators().push_front(right_operand);
					}
				}
				//返回錯誤:不合法運算子位址字元
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值運算子位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					//檢查函數第一引數暫存
					if (function_first_argument) {
						//回存函數第一引數
						GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
					}
					return false;
				}
				//刪除一個運算子位址字元
				BinaryOperatorAddress().pop_front();
			}
			else if (left_operand.arithmetic && right_operand.logical && GeneralOperators().empty() && address == ADDRESS_ASSIGNMENT_OPERATOR) {
				last_operator = shared_ptr<ArithmeticOperator>(new AssignmentOperator(left_operand.arithmetic, right_operand.logical));
				//刪除一個運算子位址字元
				BinaryOperatorAddress().pop_front();
			}
			//左右運算元不合法
			else {
				//回存右運算元
				GeneralOperators().push_front(right_operand);
				//回存左運算元
				GeneralOperators().push_front(left_operand);
				//有暫存賦值左運算元
				if (assignment_left_operand) {
					//回存賦值運算子位址字元
					BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
					//回存賦值左運算元
					GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
				}
				//檢查函數第一引數暫存
				if (function_first_argument) {
					//回存函數第一引數
					GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
				}
				return false;
			}
		}
		//前運算子存在且容器內至少有一個運算子
		else if (last_operator && !GeneralOperators().empty()) {
			//複製右運算元
			GeneralOperatorHandle right_operand(GeneralOperators().front());
			//清除容器內右運算元
			GeneralOperators().pop_front();
			//右運算元為算術運算子
			if (right_operand.arithmetic) {
				//建立新運算子(加法):使用前(算術)運算子為左運算元,一個右運算元
				if (address == ADDRESS_ADD_OPERATOR) next_operator = shared_ptr<ArithmeticOperator>(new AddOperator(last_operator, right_operand.arithmetic));
				//建立新運算子(減法):使用前(算術)運算子為左運算元,一個右運算元
				else if (address == ADDRESS_MINUS_SUBTRACT_OPERATOR) next_operator = shared_ptr<ArithmeticOperator>(new SubtractOperator(last_operator, right_operand.arithmetic));
				//返回錯誤:不合法運算子位址字元
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值運算子位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					//檢查函數第一引數暫存
					if (function_first_argument) {
						//回存函數第一引數
						GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
					}
					return false;
				}
				//刪除一個運算子位址字元
				BinaryOperatorAddress().pop_front();
				//新運算子成為前運算子(前運算子Handle自動清空)
				last_operator = next_operator;
			}
			//右運算元不合法
			else {
				//回存右運算元
				GeneralOperators().push_front(right_operand);
				//有暫存賦值左運算元
				if (assignment_left_operand) {
					//回存賦值運算子位址字元
					BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
					//回存賦值左運算元
					GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
				}
				//檢查函數第一引數暫存
				if (function_first_argument) {
					//回存函數第一引數
					GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
				}
				return false;
			}
		}
		//返回錯誤:未滿足建立新運算元的初始條件
		else {
			//有暫存賦值左運算元
			if (assignment_left_operand) {
				//回存賦值運算子位址字元
				BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
				//回存賦值左運算元
				GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
			}
			//檢查函數第一引數暫存
			if (function_first_argument) {
				//回存函數第一引數
				GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
			}
			return false;
		}
	}

	//運算元容器已清空且前運算子存在
	if (GeneralOperators().empty() && last_operator) {
		//無暫存賦值左運算元
		if (!assignment_left_operand) {
			//將最終的前運算子放入一般運算元容器
			GeneralOperators().push_back(GeneralOperatorHandle(last_operator));
		}
		//有暫存賦值左運算元
		else {
			next_operator = shared_ptr<ArithmeticOperator>(new AssignmentOperator(assignment_left_operand, last_operator));
			//建立最終的賦值運算子並放入一般運算元容器
			GeneralOperators().push_back(GeneralOperatorHandle(next_operator));
		}
		//檢查函數第一引數暫存
		if (function_first_argument) {
			//回存函數第一引數
			GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
		}
		return true;
	}
	//未成功建立前運算子或運算子數量異常
	else {
		//有暫存賦值左運算元
		if (assignment_left_operand) {
			//回存賦值運算子位址字元
			BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
			//回存賦值左運算元
			GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
		}
		//檢查函數第一引數暫存
		if (function_first_argument) {
			//回存函數第一引數
			GeneralOperators().push_front(GeneralOperatorHandle(function_first_argument));
		}
		return false;
	}
}

bool MacroGenerator::CreatePriorityBinaryOperator()
{
	//新運算子Handle
	shared_ptr<ArithmeticOperator> next_operator;
	//前運算子Handle
	shared_ptr<ArithmeticOperator> last_operator;

	//容器內仍有優先二元運算子位址字元則持續處理
	while (!PriorityBinaryOperatorAddress().empty()) {
		//取得下一個優先二元運算子位址字元
		unsigned char address(PriorityBinaryOperatorAddress().front());
		//前運算子為空Handle且容器內至少有二個運算子
		if (!last_operator && PriorityGeneralOperators().size() > 1) {
			//複製左運算元
			GeneralOperatorHandle left_operand(PriorityGeneralOperators().front());
			//清除容器內左運算元
			PriorityGeneralOperators().pop_front();
			//複製右運算元
			GeneralOperatorHandle right_operand(PriorityGeneralOperators().front());
			//清除容器內右運算元
			PriorityGeneralOperators().pop_front();
			//左右運算元均為算術運算子
			if (left_operand.arithmetic && right_operand.arithmetic) {
				//建立前運算子(乘法):使用一個左運算元
				if (address == '*') last_operator = shared_ptr<ArithmeticOperator>(new MultiplyOperator(left_operand.arithmetic, right_operand.arithmetic));
				//建立前運算子(除法):使用一個左運算元
				else if (address == '/') last_operator = shared_ptr<ArithmeticOperator>(new DivideOperator(left_operand.arithmetic, right_operand.arithmetic));
				//返回錯誤:不合法運算子位址字元
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
				//刪除一個運算子位址字元
				PriorityBinaryOperatorAddress().pop();
			}
			//左右運算元不合法
			else {
				//回存右運算元
				PriorityGeneralOperators().push_front(right_operand);
				//回存左運算元
				PriorityGeneralOperators().push_front(left_operand);
				return false;
			}
		}
		//前運算子存在且容器內至少有一個運算子
		else if (last_operator && !PriorityGeneralOperators().empty()) {
			//複製右運算元
			GeneralOperatorHandle right_operand(PriorityGeneralOperators().front());
			//清除容器內右運算元
			PriorityGeneralOperators().pop_front();
			//右運算元為算術運算子
			if (right_operand.arithmetic) {
				//建立新運算子(乘法):使用前(算術)運算子為左運算元,一個右運算元
				if (address == '*') next_operator = shared_ptr<ArithmeticOperator>(new MultiplyOperator(last_operator, right_operand.arithmetic));
				//建立新運算子(除法):使用前運算子為左運算元,一個右運算元
				else if (address == '/') next_operator = shared_ptr<ArithmeticOperator>(new DivideOperator(last_operator, right_operand.arithmetic));
				//返回錯誤:不合法運算子位址字元
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					return false;
				}
				//刪除一個運算子位址字元
				PriorityBinaryOperatorAddress().pop();
				//新運算子成為前運算子(前運算子Handle自動清空)
				last_operator = next_operator;
			}
			//右運算元不合法
			else {
				//回存右運算元
				PriorityGeneralOperators().push_front(right_operand);
				return false;
			}
		}
		//返回錯誤:未滿足建立新運算元的初始條件
		else return false;
	}

	//運算元容器已清空且前運算子存在
	if (PriorityGeneralOperators().empty() && last_operator) {
		//將最終的前運算子放入一般運算元容器
		GeneralOperators().push_back(GeneralOperatorHandle(last_operator));
		return true;
	}
	//未成功建立前運算子或運算子數量異常
	else {
		return false;
	}
}

bool MacroGenerator::CreateRelationalOperator()
{
	//容器內有關係運算子關鍵字且有兩個運算元
	if (!RelationalKeyword().empty() && GeneralOperators().size() == 2) {
		//取得關鍵字字串
		string keyword(RelationalKeyword().back());
		//關鍵字為"等於"運算子
		if (keyword == KEYWORD_EQUAL_OPERATOR) {
			//返回錯誤:左或右運算元非算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//確認左右運算元均為算術運算子
			else {
				//建立關係運算子Handle
				shared_ptr<RelationalOperator> relation(new EqualOperator(GeneralOperators().front().arithmetic, GeneralOperators().back().arithmetic));
				//刪除關鍵字串
				RelationalKeyword().pop_back();
				//刪除左右運算元
				GeneralOperators().clear();
				//存入通用運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(relation));
				return true;
			}
		}
		//關鍵字為"不等於"運算子
		else if (keyword == KEYWORD_NOT_EQUAL_OPERATOR) {
			//返回錯誤:左或右運算元非算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//確認左右運算元均為算術運算子
			else {
				//建立關係運算子Handle
				shared_ptr<RelationalOperator> relation(new NotEqualOperator(GeneralOperators().front().arithmetic, GeneralOperators().back().arithmetic));
				//刪除關鍵字串
				RelationalKeyword().pop_back();
				//刪除左右運算元
				GeneralOperators().clear();
				//存入通用運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(relation));
				return true;
			}
		}
		//關鍵字為"大於"運算子
		else if (keyword == KEYWORD_GREATER_OPERATOR) {
			//返回錯誤:左或右運算元非算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//確認左右運算元均為算術運算子
			else {
				//建立關係運算子Handle
				shared_ptr<RelationalOperator> relation(new GreaterOperator(GeneralOperators().front().arithmetic, GeneralOperators().back().arithmetic));
				//刪除關鍵字串
				RelationalKeyword().pop_back();
				//刪除左右運算元
				GeneralOperators().clear();
				//存入通用運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(relation));
				return true;
			}
		}
		//關鍵字為"大於等於"運算子
		else if (keyword == KEYWORD_GREATER_EQUAL_OPERATOR) {
			//返回錯誤:左或右運算元非算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//確認左右運算元均為算術運算子
			else {

				//建立關係運算子Handle
				shared_ptr<RelationalOperator> relation(new GreaterEqualOperator(GeneralOperators().front().arithmetic, GeneralOperators().back().arithmetic));
				//刪除關鍵字串
				RelationalKeyword().pop_back();
				//刪除左右運算元
				GeneralOperators().clear();
				//存入通用運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(relation));
				return true;
			}
		}
		//關鍵字為"小於"運算子
		else if (keyword == KEYWORD_LESS_OPERATOR) {
			//返回錯誤:左或右運算元非算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//確認左右運算元均為算術運算子
			else {
				//建立關係運算子Handle
				shared_ptr<RelationalOperator> relation(new LessOperator(GeneralOperators().front().arithmetic, GeneralOperators().back().arithmetic));
				//刪除關鍵字串
				RelationalKeyword().pop_back();
				//刪除左右運算元
				GeneralOperators().clear();
				//存入通用運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(relation));
				return true;
			}
		}
		//關鍵字為"小於等於"運算子
		else if (keyword == KEYWORD_LESS_EQUAL_OPERATOR) {
			//返回錯誤:左或右運算元非算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//確認左右運算元均為算術運算子
			else {
				//建立關係運算子Handle
				shared_ptr<RelationalOperator> relation(new LessEqualOperator(GeneralOperators().front().arithmetic, GeneralOperators().back().arithmetic));
				//刪除關鍵字串
				RelationalKeyword().pop_back();
				//刪除左右運算元
				GeneralOperators().clear();
				//存入通用運算子容器
				GeneralOperators().push_back(GeneralOperatorHandle(relation));
				return true;
			}
		}
		//關鍵字不合法
		else return false;
	}
	//返回錯誤:未滿足建立關係運算子的初始條件
	return false;
}

bool MacroGenerator::CreateLogicalOperator()
{
	//新運算子Handle
	shared_ptr<LogicalOperator> next_operator;
	//前運算子Handle
	shared_ptr<LogicalOperator> last_operator;
	//賦值左運算元Handle
	shared_ptr<ArithmeticOperator> assignment_left_operand;

	//有賦值運算子位址字元
	if (!BinaryOperatorAddress().empty() && BinaryOperatorAddress().front() == ADDRESS_ASSIGNMENT_OPERATOR) {
		//有至少一個運算元且為算術運算子
		if (!GeneralOperators().empty() && GeneralOperators().front().arithmetic) {
			//刪除位址字元
			BinaryOperatorAddress().pop_front();
			//複製暫存賦值左運算元
			assignment_left_operand = GeneralOperators().front().arithmetic;
			//刪除左運算元
			GeneralOperators().pop_front();
		}
	}

	//容器內仍有邏輯運算子關鍵字則持續處理
	while (!LogicalKeyword().empty()) {
		//取得下一個邏輯運算子關鍵字
		string keyword(LogicalKeyword().front());
		//前運算子為空Handle且容器內至少有二個運算元
		if (!last_operator && GeneralOperators().size() > 1) {
			//複製左運算元
			GeneralOperatorHandle left_operand(GeneralOperators().front());
			//清除容器內左運算元
			GeneralOperators().pop_front();
			//複製右運算元
			GeneralOperatorHandle right_operand(GeneralOperators().front());
			//清除容器內右運算元
			GeneralOperators().pop_front();
			//左右運算元均為算術運算子
			if (left_operand.arithmetic && right_operand.arithmetic) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.arithmetic, right_operand.arithmetic));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.arithmetic, right_operand.arithmetic));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左右運算元均為邏輯運算子
			else if (left_operand.logical && right_operand.logical) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.logical, right_operand.logical));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.logical, right_operand.logical));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左運算元為算術運算子,右運算元為邏輯運算子
			else if (left_operand.arithmetic && right_operand.logical) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.arithmetic, right_operand.logical));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.arithmetic, right_operand.logical));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左運算元為邏輯運算子,右運算元為算術運算子
			else if (left_operand.logical && right_operand.arithmetic) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.logical, right_operand.arithmetic));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.logical, right_operand.arithmetic));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左右運算元均為關係運算子
			else if (left_operand.relational && right_operand.relational) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.relational, right_operand.relational));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.relational, right_operand.relational));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左運算元為關係運算子,右運算元為邏輯運算子
			else if (left_operand.relational && right_operand.logical) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.relational, right_operand.logical));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.relational, right_operand.logical));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左運算元為邏輯運算子,右運算元為關係運算子
			else if (left_operand.logical && right_operand.relational) {
				//建立前運算子(OR):使用左右運算元
				if (keyword == KEYWORD_OR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new OR_Operator(left_operand.logical, right_operand.relational));
				//建立前運算子(XOR):使用左右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) last_operator = shared_ptr<LogicalOperator>(new XOR_Operator(left_operand.logical, right_operand.relational));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//回存左運算元
					GeneralOperators().push_front(left_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
			}
			//左右運算元不合法
			else {
				//回存右運算元
				GeneralOperators().push_front(right_operand);
				//回存左運算元
				GeneralOperators().push_front(left_operand);
				//有暫存賦值左運算元
				if (assignment_left_operand) {
					//回存賦值位址字元
					BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
					//回存賦值左運算元
					GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
				}
				return false;
			}
		}
		//前運算子存在且容器內至少有一個運算元
		else if (last_operator && !GeneralOperators().empty()) {
			//複製右運算元
			GeneralOperatorHandle right_operand(GeneralOperators().front());
			//清除容器內右運算元
			GeneralOperators().pop_front();
			//右運算元為算術運算子
			if (right_operand.arithmetic) {
				//建立新運算子(OR):使用前運算子為左運算元,一個右運算元
				if (keyword == KEYWORD_OR_OPERATOR) next_operator = shared_ptr<LogicalOperator>(new OR_Operator(last_operator, right_operand.arithmetic));
				//建立新運算子(XOR):使用前運算子為左運算元,一個右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) next_operator = shared_ptr<LogicalOperator>(new XOR_Operator(last_operator, right_operand.arithmetic));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
				//新運算子成為前運算子(前運算子Handle自動清空)
				last_operator = next_operator;
			}
			//右運算元為邏輯運算子
			else if (right_operand.logical) {
				//建立新運算子(OR):使用前運算子為左運算元,一個右運算元
				if (keyword == KEYWORD_OR_OPERATOR) next_operator = shared_ptr<LogicalOperator>(new OR_Operator(last_operator, right_operand.logical));
				//建立新運算子(XOR):使用前運算子為左運算元,一個右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) next_operator = shared_ptr<LogicalOperator>(new XOR_Operator(last_operator, right_operand.logical));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
				//新運算子成為前運算子(前運算子Handle自動清空)
				last_operator = next_operator;
			}
			//右運算元為關係運算子
			else if (right_operand.relational) {
				//建立新運算子(OR):使用前運算子為左運算元,一個右運算元
				if (keyword == KEYWORD_OR_OPERATOR) next_operator = shared_ptr<LogicalOperator>(new OR_Operator(last_operator, right_operand.relational));
				//建立新運算子(XOR):使用前運算算算子為左運算元,一個右運算元
				else if (keyword == KEYWORD_XOR_OPERATOR) next_operator = shared_ptr<LogicalOperator>(new XOR_Operator(last_operator, right_operand.relational));
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					GeneralOperators().push_front(right_operand);
					//有暫存賦值左運算元
					if (assignment_left_operand) {
						//回存賦值位址字元
						BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
						//回存賦值左運算元
						GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
					}
					return false;
				}
				//刪除一個運算子關鍵字
				LogicalKeyword().pop_front();
				//新運算子成為前運算子(前運算子Handle自動清空)
				last_operator = next_operator;
			}
			//右運算元不合法
			else {
				//回存右運算元
				GeneralOperators().push_front(right_operand);
				//有暫存賦值左運算元
				if (assignment_left_operand) {
					//回存賦值位址字元
					BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
					//回存賦值左運算元
					GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
				}
				return false;
			}
		}
		//返回錯誤:未滿足建立新運算子的初始條件
		else {
			//有暫存賦值左運算元
			if (assignment_left_operand) {
				//回存賦值位址字元
				BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
				//回存賦值左運算元
				GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
			}
			return false;
		}
	}

	//運算元容器已清空且前運算子存在
	if (GeneralOperators().empty() && last_operator) {
		//無暫存賦值左運算元
		if (!assignment_left_operand) {
			//將最終的前運算子放入一般運算子容器
			GeneralOperators().push_back(GeneralOperatorHandle(last_operator));
		}
		//有暫存賦值左運算元
		else {
			//建立賦值運算子:使用暫存賦值左運算元,前運算子為右運算元
			shared_ptr<ArithmeticOperator> assignment(new AssignmentOperator(assignment_left_operand, last_operator));
			//將最終賦值運算子存入一般運算子容器
			GeneralOperators().push_back(GeneralOperatorHandle(assignment));
		}
		return true;
	}
	//未成功建立前運算子或運算子數量異常
	else {
		//有暫存賦值左運算元
		if (assignment_left_operand) {
			//回存賦值位址字元
			BinaryOperatorAddress().push_front(ADDRESS_ASSIGNMENT_OPERATOR);
			//回存賦值左運算元
			GeneralOperators().push_front(GeneralOperatorHandle(assignment_left_operand));
		}
		return false;
	}
}

bool MacroGenerator::CreatePriorityLogicalOperator()
{
	//新運算子Handle
	shared_ptr<LogicalOperator> next_operator;
	//前運算子Handle
	shared_ptr<LogicalOperator> last_operator;

	//容器內仍有邏輯運算子關鍵字則持續處理
	while (!PriorityLogicalKeyword().empty()) {
		//取得下一個邏輯運算子關鍵字
		string keyword(PriorityLogicalKeyword().front());
		//前運算子為空Handle且容器內至少有二個優先運算元
		if (!last_operator && PriorityGeneralOperators().size() > 1) {
			//複製左運算元
			GeneralOperatorHandle left_operand(PriorityGeneralOperators().front());
			//清除容器內左運算元
			PriorityGeneralOperators().pop_front();
			//複製右運算元
			GeneralOperatorHandle right_operand(PriorityGeneralOperators().front());
			//清除容器內右運算元
			PriorityGeneralOperators().pop_front();
			//左右運算元均為算術運算子
			if (left_operand.arithmetic && right_operand.arithmetic) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.arithmetic, right_operand.arithmetic));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左右運算元均為邏輯運算子
			else if (left_operand.logical && right_operand.logical) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.logical, right_operand.logical));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左右運算元均為關係運算子
			else if (left_operand.relational && right_operand.relational) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.relational, right_operand.relational));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左運算元為算術運算子,右運算元為邏輯運算子
			else if (left_operand.arithmetic && right_operand.logical) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.arithmetic, right_operand.logical));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左運算元為邏輯運算子,右運算元為算術運算子
			else if (left_operand.logical && right_operand.arithmetic) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.logical, right_operand.arithmetic));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左運算元為關係運算子,右運算元為邏輯運算子
			else if (left_operand.relational && right_operand.logical) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.relational, right_operand.logical));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左運算元為邏輯運算子,右運算元為關係運算子
			else if (left_operand.logical && right_operand.relational) {
				//建立前運算子(AND):使用左右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					last_operator = shared_ptr<LogicalOperator>(new AND_Operator(left_operand.logical, right_operand.relational));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					//回存左運算元
					PriorityGeneralOperators().push_front(left_operand);
					return false;
				}
			}
			//左右運算元不合法
			else {
				//回存右運算元
				PriorityGeneralOperators().push_front(right_operand);
				//回存左運算元
				PriorityGeneralOperators().push_front(left_operand);
				return false;
			}
		}
		//前運算子存在且容器內至少有一個優先運算元
		else if (last_operator && !PriorityGeneralOperators().empty()) {
			//複製右運算元
			GeneralOperatorHandle right_operand(PriorityGeneralOperators().front());
			//清除容器內右運算元
			PriorityGeneralOperators().pop_front();
			//右運算元為算術運算子
			if (right_operand.arithmetic) {
				//建立新運算子(AND):使用前運算子為左運算元,一個右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					next_operator = shared_ptr<LogicalOperator>(new AND_Operator(last_operator, right_operand.arithmetic));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
					//新運算子成為前運算子(前運算子Handle自動清空)
					last_operator = next_operator;
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					return false;
				}
			}
			//右運算元為邏輯運算子
			else if (right_operand.logical) {
				//建立新運算子(AND):使用前運算子為左運算元,一個右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					next_operator = shared_ptr<LogicalOperator>(new AND_Operator(last_operator, right_operand.logical));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
					//新運算子成為前運算子(前運算子Handle自動清空)
					last_operator = next_operator;
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					return false;
				}
			}
			//右運算元為關係運算子
			else if (right_operand.relational) {
				//建立新運算子(AND):使用前運算子為左運算元,一個右運算元
				if (keyword == KEYWORD_AND_OPERATOR) {
					next_operator = shared_ptr<LogicalOperator>(new AND_Operator(last_operator, right_operand.relational));
					//刪除一個運算子關鍵字
					PriorityLogicalKeyword().pop_front();
					//新運算子成為前運算子(前運算子Handle自動清空)
					last_operator = next_operator;
				}
				//返回錯誤:不合法運算子關鍵字
				else {
					//回存右運算元
					PriorityGeneralOperators().push_front(right_operand);
					return false;
				}
			}
			//右運算元不合法
			else {
				//回存右運算元
				PriorityGeneralOperators().push_front(right_operand);
				return false;
			}
		}
		//返回錯誤:未滿足建立新運算子的初始條件
		else return false;
	}

	//優先運算元容器已清空且前運算子存在
	if (PriorityGeneralOperators().empty() && last_operator) {
		//將最終的前運算子放入一般運算元容器
		GeneralOperators().push_back(GeneralOperatorHandle(last_operator));
		return true;
	}
	//未成功建立前運算子或運算子數量異常
	else {
		return false;
	}
}

bool MacroGenerator::CreateConditionalOperator()
{
	//檢查目前容器層數是否在最底層,確認有運算元(算術運算子)
	if (CurrentLevel() == 0 && !ConditionalKeyword().empty() && !GeneralOperators().empty() && GeneralOperators().back().arithmetic) {
		//取得條件式運算子關鍵字
		string keyword(ConditionalKeyword().back());
		//關鍵字為分支運算子
		if (keyword == KEYWORD_BRANCH_OPERATOR) {
			//檢查前導的條件式關鍵字存在,並確認關係運算子已建立
			if (ConditionalKeyword().front() == KEYWORD_IF_CONDITION && GeneralOperators().size() == 2) {
				if (GeneralOperators().front().relational) {
					//建立條件式分支運算子:左運算元使用關係運算子,右運算元使用算術運算子
					ConditionalBranchOperator conditional_branch(GeneralOperators().front().relational, GeneralOperators().back().arithmetic);
					//複製分支運算子
					conditional_branch_operator = conditional_branch;
				}
				else if (GeneralOperators().front().logical) {
					//建立條件式分支運算子:左運算元使用關係運算子,右運算元使用算術運算子
					ConditionalBranchOperator conditional_branch(GeneralOperators().front().logical, GeneralOperators().back().arithmetic);
					//複製分支運算子
					conditional_branch_operator = conditional_branch;
				}
				else return false;
				//清除(前導)條件式關鍵字及算術運算關鍵字
				ConditionalKeyword().clear();
				//清除關係運算子及算術運算元
				GeneralOperators().clear();
				return true;
			}
			//僅有分支運算子關鍵字且無關係運算子
			else if (ConditionalKeyword().size() == 1 && GeneralOperators().size() == 1) {
				//建立分支運算子:單一右運算元使用算術運算子
				ConditionalBranchOperator conditional_branch(GeneralOperators().front().arithmetic);
				//清除分支運算子關鍵字
				ConditionalKeyword().pop_back();
				//清除算術運算元
				GeneralOperators().pop_front();
				//複製分支運算子
				conditional_branch_operator = conditional_branch;
				return true;
			}
			else return false;
		}
		//關鍵字為條件式算術運算子
		else if (keyword == KEYWORD_CONDITIONAL_ARITHMETIC_OPERATOR) {
			//檢查前導的條件式關鍵字存在,並確認關係運算子已建立
			if (ConditionalKeyword().front() == KEYWORD_IF_CONDITION && GeneralOperators().size() == 2) {
				if (GeneralOperators().front().relational) {
					//建立條件式運算子:左運算元使用關係運算子,右運算元使用算術運算子
					ConditionalArithmeticOperator conditional_arithmetic(GeneralOperators().front().relational, GeneralOperators().back().arithmetic);
					//複製條件式運算子
					conditional_arithmetic_operator = conditional_arithmetic;
				}
				else if (GeneralOperators().front().logical) {
					//建立條件式運算子:左運算元使用關係運算子,右運算元使用算術運算子
					ConditionalArithmeticOperator conditional_arithmetic(GeneralOperators().front().logical, GeneralOperators().back().arithmetic);
					//複製條件件式運算子
					conditional_arithmetic_operator = conditional_arithmetic;
				}
				else return false;
				//清除(前導)條件式關鍵字及算術運算關鍵字
				ConditionalKeyword().clear();
				//清除關係運算子及算術運算元
				GeneralOperators().clear();
				return true;
			}
			else return false;
		}
		//關鍵字為迴圈運算子
		else if (keyword == KEYWORD_LOOP_OPERATOR) {
			//檢查前導的條件式關鍵字存在,並確認關係運算子已建立
			if (ConditionalKeyword().front() == KEYWORD_WHILE_CONDITION && GeneralOperators().size() == 2) {
				if (GeneralOperators().front().relational) {
					//建立條件式分支運算子:左運算元使用關係運算子,右運算元使用算術運算子
					ConditionalLoopOperator conditional_loop(GeneralOperators().front().relational, GeneralOperators().back().arithmetic);
					//複製分支運算子
					conditional_loop_operator = conditional_loop;
				}
				else if (GeneralOperators().front().logical) {
					//建立條件式分支運算子:左運算元使用關係運算子,右運算元使用算術運算子
					ConditionalLoopOperator conditional_loop(GeneralOperators().front().logical, GeneralOperators().back().arithmetic);
					//複製分支運算子
					conditional_loop_operator = conditional_loop;
				}
				else return false;
				//清除(前導)條件式關鍵字及算術運算關鍵字
				ConditionalKeyword().clear();
				//清除關係運算子及算術運算元
				GeneralOperators().clear();
				return true;
			}
			//僅有迴圈運算子關鍵字且無關係運算子
			else if (ConditionalKeyword().size() == 1 && GeneralOperators().size() == 1) {
				//建立迴圈運算子:單一右運算元元使用算術運算子
				ConditionalLoopOperator conditional_loop(GeneralOperators().front().arithmetic);
				//清除迴圈運算子關鍵字
				ConditionalKeyword().pop_back();
				//清除算術運算元
				GeneralOperators().pop_front();
				//複製迴圈運算子
				conditional_loop_operator = conditional_loop;
				return true;
			}
			else return false;
		}
		//關鍵字為迴圈結束
		else if (keyword == KEYWORD_LOOP_END) {
			if (ConditionalKeyword().size() == 1 && GeneralOperators().size() == 1) {
				//建立迴圈結束運算子:單一右運算元使用算術運算子
				LoopEndOperator loop_end(GeneralOperators().front().arithmetic);
				//清除迴圈結束運算子關鍵字
				ConditionalKeyword().pop_back();
				//清除算術運算元
				GeneralOperators().pop_front();
				//複製迴圈結束運算子
				loop_end_operator = loop_end;
				return true;
			}
			else return false;
		}
		else return false;
	}
	else return false;
}

bool MacroGenerator::ProcessOperatorKeyword()
{
	map<string, unsigned char>::const_iterator iter(keyword_list.find(UpperString().top()));
	if (iter == keyword_list.end())	return false;
	else {
		switch (iter->second) {
		//關鍵字為一元函數運算子
		case UNARY_FUNCTION_OPERATOR:
			//關鍵字存入對應容器
			UnaryOperatorKeyword().push(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

		//關鍵字為一元與二元通用函數運算子
		case UNARY_BINARY_FUNCTION_OPERATOR:
			//關鍵字存入對應容器
			UnaryBinaryOperatorKeyword().push(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

		//關鍵字為二元函數運算子
		case BINARY_FUNCTION_OPERATOR:
			//關鍵字存入對應容器
			BinaryOperatorKeyword().push(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

		//關鍵字為關係運算子
		case RELATIONAL_OPERATOR:
			//關鍵字存入對應容器
			RelationalKeyword().push_back(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

		//關鍵字為一般二元邏輯運算子
		case BINARY_LOGICAL_OPERATOR:
			//目前為優先運算子模式
			if (PriorityLogicalFlag()) {
				//建立優先邏輯運算子失敗:返回錯誤
				if (!CreatePriorityLogicalOperator()) return false;
				//成功:取消優先運算子模式
				else PriorityLogicalFlag() = false;
			}
			//邏輯運算子關鍵字存入對應容器
			LogicalKeyword().push_back(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

		//關鍵字為優先二元邏輯運算子
		case PRIORITY_BINARY_LOGICAL_OPERATOR:
			//目前為一般運算子模式
			if (!PriorityLogicalFlag()) {
				//設定優先運算子模式
				PriorityLogicalFlag() = true;
				if (!GeneralOperators().empty()) {
					//將左運算元由一般運算子容器搬移到優先運算子容器
					PriorityGeneralOperators().push_back(GeneralOperators().back());
					//刪除左運算元
					GeneralOperators().pop_back();
				}
				else return false;
			}
			//優先邏輯運算子關鍵字存入對應容器
			PriorityLogicalKeyword().push_back(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

		//關鍵字為條件式
		case IF_CONDITION:
		//關鍵字為迴圈條件式
		case WHILE_CONDITION:
		//關鍵字為分支運算子
		case BRANCH_OPERATOR:
		//關鍵字為條件式算術運算
		case CONDITIONAL_ARITHMETIC_OPERATOR:
		//關鍵字為迴圈運算子
		case LOOP_OPERATOR:
		//關鍵字為迴圈結束
		case LOOP_END:
			//關鍵字存入對應容器
			ConditionalKeyword().push_back(iter->first);
			//刪除大寫字串
			UpperString().pop();
			return true;

			//不合法關鍵字
		default: return false;
		}
	}
}

bool MacroGenerator::ProcessOperatorAddress(unsigned short address_type, unsigned char address)
{
	//字元屬於一元運算子
	if (address_type == UNARY_OPERATOR) {
		//將字元放入一元運算子位址字元容器
		UnaryOperatorAddress().push(address);
		return true;
	}
	//字元屬於一般二元運算子
	else if (address_type == BINARY_OPERATOR) {
		//目前處於優先運算子模式
		if (PriorityFlag()) {
			//處理並建立優先運算子
			if (!CreatePriorityBinaryOperator()) {
				return false;
			}
			//解除優先運算子模式
			PriorityFlag() = false;
		}
		//將字元放入一般二元運算子位址字元容器
		BinaryOperatorAddress().push_back(address);
		return true;
	}
	//字元屬於優先二元運算子
	else if (address_type == PRIORITY_BINARY_OPERATOR) {
		//目前處於優先運算子模式
		if (PriorityFlag()) {
			//將字元放入優先二元運算子位址字元容器
			PriorityBinaryOperatorAddress().push(address);
			return true;
		}
		//目前非優先運算子模式
		else {
			//設立優先運算子旗標
			PriorityFlag() = true;
			//將字元放入優先二元運算子位址字元容器
			PriorityBinaryOperatorAddress().push(address);
			//返回錯誤:左運算元不存在
			if (GeneralOperators().empty()) return false;
			//將左運算元由一般運算子容器複製到優先運算子容器
			PriorityGeneralOperators().push_back(GeneralOperators().back());
			//清除左運算元
			GeneralOperators().pop_back();
			return true;
		}
	}
	//字元為一元與二元通用運算子
	else if (address_type == UNARY_BINARY_OPERATOR) {
		//判別為一元或二元運算子並處理
		return ProcessUnaryBinaryAddress(address);
	}
	else return false;
}

bool MacroGenerator::ProcessUnaryBinaryAddress(unsigned char address)
{
	//有關係運算子關鍵字存在
	if (!RelationalKeyword().empty()) {
		//判斷為一元minus運算子位址字元
		UnaryOperatorAddress().push(address);
		return true;
	}

	//目前處於優先運算子模式
	if (PriorityFlag() == true) {
		//檢查二元優先運算子位址字元與優先運算元的配對數量
		if (PriorityGeneralOperators().size() == PriorityBinaryOperatorAddress().size()) {
			//判斷為一元minus運算子位址字元
			UnaryOperatorAddress().push(address);
			return true;
		}
		else {
			//判斷為二元subtract運算子位址字元
			//先處理並建立優先運算子
			if (!CreatePriorityBinaryOperator()) {
				return false;
			}
			//解除優先運算子模式
			PriorityFlag() = false;
			//存入一般二元運算子位址容器
			BinaryOperatorAddress().push_back(address);
			return true;
		}
	}
	//目前非優先運算子模式
	else {
		//運算元計數器
		deque<GeneralOperatorHandle>::size_type count_operand(GeneralOperators().size());
		//記數須扣除第一引數或根運算式可能存在的條件(關係或邏輯)運算子
		if (TwoArgumentsFlag() || CurrentLevel() == 0 && !GeneralOperators().empty() && !GeneralOperators().front().arithmetic) {
			//扣除運算元數量
			--count_operand;
		}
		//一般二元運算子位址字元與運算元的配對數量相同
		if (count_operand == BinaryOperatorAddress().size()) {
			//判斷為一元minus運算子位址字元
			UnaryOperatorAddress().push(address);
			return true;
		}
		//配對數量不相同
		else {
			//判斷為二元subtract運算子位址字元
			//存入一般二元運算子位址容器
			BinaryOperatorAddress().push_back(address);
			return true;
		}
	}
}

bool MacroGenerator::ProcessToFinalOperator()
{
	//有優先邏輯運算子關鍵字
	if (!PriorityLogicalKeyword().empty()) {
		//嘗試建立優先邏輯運算子
		if (!CreatePriorityLogicalOperator()) return false;
		//有一般邏輯運算子關鍵字
		if (!LogicalKeyword().empty()) {
			//嘗試建立一般邏輯運算子
			if (!CreateLogicalOperator()) return false;
		}
		//有二元算術運算子(賦值)位址字元
		if (!BinaryOperatorAddress().empty()) {
			//嘗試建立二元算術(賦值)運算子
			if (!CreateBinaryOperator()) return false;
		}
		return true;
	}
	//有一般邏輯運算子關鍵字
	else if (!LogicalKeyword().empty()) {
		//嘗試建立一般邏輯運算子
		if (!CreateLogicalOperator()) return false;
		return true;
	}

	//有優先二元運算子位址字元
	if (!PriorityBinaryOperatorAddress().empty()) {
		//返回錯誤:建立優先二元運算子失敗
		if (!CreatePriorityBinaryOperator()) return false;
		//取消優先運算子模式:成功建立優先二元運算子
		else PriorityFlag() = false;
	}
	//有一般二元運算子位址字元
	if (!BinaryOperatorAddress().empty()) {
		//第一個運算元為關係或邏輯運算子
		if (GeneralOperators().front().relational || GeneralOperators().front().logical) {
			//暫存運算元
			GeneralOperatorHandle condition(GeneralOperators().front());
			//容器內刪除運算元
			GeneralOperators().pop_front();
			//嘗試建立一般二元算術運算子
			if (!CreateBinaryOperator()) {
				//返回失敗:先回存運算元
				GeneralOperators().push_front(condition);
				return false;
			}
			//回存運算元
			else GeneralOperators().push_front(condition);
		}
		//第一個運算元為算術運算子
		else {
			//嘗試建立二元算術運算子
			return CreateBinaryOperator();
		}
	}
	//有條件式運算子關鍵字
	if (!ConditionalKeyword().empty()) {
		//嘗試建立條件式運算子
		return CreateConditionalOperator();
	}
	//不需建立任何運算子
	return true;
}

bool MacroGenerator::ReturnPriorityOperatorToPreviousLevel()
{
	//有優先二元算術運算子位址字元
	if (!PriorityBinaryOperatorAddress().empty()) {
		//返回錯誤:建立優先二元運算子失敗
		if (!CreatePriorityBinaryOperator()) return false;
		//取消優先運算子模式:成功建立優先二元運算子
		else PriorityFlag() = false;
		//嘗試建立一般二元算術運算子
		CreateBinaryOperator();
	}
	//有一般二元算術運算子位址字元
	else if (!BinaryOperatorAddress().empty()) {
		//嘗試建立一般二元算術運算子
		if (!CreateBinaryOperator()) return false;
	}
	//有關係運算子關鍵字
	else if (!RelationalKeyword().empty()) {
		//嘗試建立關係運算子
		if (!CreateRelationalOperator()) return false;
	}
	//有優先邏輯運算子關鍵字
	else if (!PriorityLogicalKeyword().empty()) {
		//嘗試建立優先邏輯運算子
		if (!CreatePriorityLogicalOperator()) return false;
		//有一般邏輯運算子關鍵字
		if (!LogicalKeyword().empty()) {
			//嘗試建立一般邏輯運算子
			if (!CreateLogicalOperator()) return false;
		}
	}
	//有一般邏輯運算子關鍵字
	else if (!LogicalKeyword().empty()) {
		//嘗試建立一般邏輯運算子
		if (!CreateLogicalOperator()) return false;
	}

	//有通用運算子
	if (!GeneralOperators().empty()) {
		//只有一個運算子
		if (GeneralOperators().size() == 1) {
			//複製上層最終運算子
			GeneralOperatorHandle final_operator(GeneralOperators().front());
			//清除上層最終運算子
			GeneralOperators().pop_front();
			//運算子暫存容器切換回下層
			if (!LevelDown()) return false;
			//最終運算子非算術運算子
			if (!final_operator.arithmetic) {
				//檢查目前是否為優先運算子模式
				if (PriorityFlag() || PriorityLogicalFlag()) {
					//複製上層最終運算子到優先運算子容器
					PriorityGeneralOperators().push_back(final_operator);
				}
				else {
					//複製上層最終運算子到下層一般運算子容器
					GeneralOperators().push_back(final_operator);
				}
			}
			//最終運算子為算術運算子
			else {
				//嘗試建立一元函數運算子
				if (!CreateUnaryFunctionOperator(final_operator.arithmetic)) {
					//檢查目前是否為優先運算子模式
					if (PriorityFlag() || PriorityLogicalFlag()) {
						//複製上層最終運算子到優先運算子容器
						PriorityGeneralOperators().push_back(final_operator);
					}
					else {
						//複製上層最終運算子到下層一般運算子容器
						GeneralOperators().push_back(final_operator);
					}
				}
			}
		}
		//有兩個運算子且雙引數旗標被設立
		else if (GeneralOperators().size() == 2 && TwoArgumentsFlag()) {
			//確認雙引數均為算術運算子
			if (!GeneralOperators().front().arithmetic || !GeneralOperators().back().arithmetic) return false;
			//複製上層第一引數運算子
			shared_ptr<ArithmeticOperator> first_argument(GeneralOperators().front().arithmetic);
			//複製上層第二引數運算子
			shared_ptr<ArithmeticOperator> second_argument(GeneralOperators().back().arithmetic);
			//清除上層所有運算子
			GeneralOperators().clear();
			//清除雙引數旗標
			TwoArgumentsFlag() = false;
			//運算子暫存容器切換回下層
			if (!LevelDown()) return false;
			//嘗試建立二元函數運算子
			if (!CreateBinaryFunctionOperator(first_argument, second_argument)) return false;
		}
		//運算子數量或條件異常
		else {
			return false;
		}
		//嘗試建立一元運算子(#,minus)
		CreateUnaryAddressOperator();
		return true;
	}
	//返回錯誤:無任何運算子
	else return false;
}

void MacroGenerator::InitialKeywordList()
{
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_SINE_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_COSINE_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_TANGENT_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ARC_SINE_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ARC_COSINE_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ARC_TANGENT_OPERATOR, OperatorType::UNARY_BINARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_SQUARE_ROOT_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ABSOLUTE_VALUE_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_BINARY_CODE_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_BINARY_CODED_DECIMAL_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ROUND_OFF_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ROUND_DOWN_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ROUND_UP_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_NATURAL_LOG_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_EXPONENT_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_POWER_OPERATOR, OperatorType::BINARY_FUNCTION_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_ADD_DECIMAL_POINT_OPERATOR, OperatorType::UNARY_FUNCTION_OPERATOR));

	keyword_list.insert(pair<string, unsigned char>(KEYWORD_EQUAL_OPERATOR, OperatorType::RELATIONAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_NOT_EQUAL_OPERATOR, OperatorType::RELATIONAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_GREATER_OPERATOR, OperatorType::RELATIONAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_GREATER_EQUAL_OPERATOR, OperatorType::RELATIONAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_LESS_OPERATOR, OperatorType::RELATIONAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_LESS_EQUAL_OPERATOR, OperatorType::RELATIONAL_OPERATOR));

	keyword_list.insert(pair<string, unsigned char>(KEYWORD_AND_OPERATOR, OperatorType::PRIORITY_BINARY_LOGICAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_OR_OPERATOR, OperatorType::BINARY_LOGICAL_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_XOR_OPERATOR, OperatorType::BINARY_LOGICAL_OPERATOR));

	keyword_list.insert(pair<string, unsigned char>(KEYWORD_IF_CONDITION, OperatorType::IF_CONDITION));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_WHILE_CONDITION, OperatorType::WHILE_CONDITION));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_BRANCH_OPERATOR, OperatorType::BRANCH_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_CONDITIONAL_ARITHMETIC_OPERATOR, OperatorType::CONDITIONAL_ARITHMETIC_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_LOOP_OPERATOR, OperatorType::LOOP_OPERATOR));
	keyword_list.insert(pair<string, unsigned char>(KEYWORD_LOOP_END, OperatorType::LOOP_END));
}

void MacroGenerator::InitialDenyAddress()
{
	//不允許搭配巨集算式之NC位址字元
	macro_deny_address.insert('O');
	macro_deny_address.insert(':');
	macro_deny_address.insert('N');
	macro_deny_address.insert('/');
}

Argument::Argument()
	:flag_control_out(false),
	flag_digit(false),
	flag_upper(false),
	NC_address(NULL),
	digit_begin(string::const_iterator()),
	upper_begin(string::const_iterator()),
	iter(string::const_iterator())
{
}

MacroParser::MacroParser(MacroVariableInterface& variable_interface)
	:macro_generator(variable_interface)
{
}

CommandType MacroParser::ParseBlock(const string& block)
{
	//剖析巨集用引數
	Argument args;
	//清除巨集算式產生器
	macro_generator.Clear();

	//逐一處理輸入字串內每一個字元
	for (args.iter = block.begin(); args.iter != block.end(); ++args.iter) {
		//取得字元
		unsigned char ch(*args.iter);
		//註解旗標開啟
		if (args.flag_control_out) {
			//字元為註解結束
			if (ch == ADDRESS_COMMENT_END) {
				//關閉註解旗標
				args.flag_control_out = false;
			}
			//標記註解文字起始位置
			if (args.upper_begin == block.end()) args.upper_begin = args.iter;
			continue;
		}
		//字元為數字
		if (IsDigitOrDot(ch)) {
			//字元為第一個數字
			if (args.flag_digit == false) {
				//記錄數字字元起始位置
				args.digit_begin = args.iter;
				//設立數字字元旗標:目前字元位置已進入數字範圍
				args.flag_digit = true;
			}
			//字元位置離開大寫字母範圍
			if (args.flag_upper) {
				//建立位址字元或關鍵字字串
				if (!CreateAddressOrKeyword(args)) {
					return CommandType::INVALID_COMMAND;
				}
			}
			continue;
		}
		//字元位置離開數字範圍
		if (args.flag_digit) {
			//建立一元或常數運算子
			if (!CreateUnaryOrConstantOperator(args)) {
				return CommandType::INVALID_COMMAND; }
		}
		//字元為大寫字母
		if (isupper(ch)) {
			//字元為第一個大寫字母
			if (!args.flag_upper) {
				//記錄大寫字母起始位置
				args.upper_begin = args.iter;
				//設立大寫字母旗標:目前字元位置已進入大寫字母範圍
				args.flag_upper = true;
				//存在NC位址字元
				if (args.NC_address != NULL) {
					//NC位址字元為大寫字母
					if (isupper(args.NC_address)) {
						//目前非巨集運算模式,已建立巨集運算子,運算子類型為算術運算子
						if (!macro_generator.IsMacroMode() && !macro_generator.GeneralOperators().empty() && macro_generator.GeneralOperators().front().arithmetic) {
							//清除NC位址字元
							args.NC_address = NULL;
							//刪除巨集運算子
							macro_generator.GeneralOperators().pop_front();
						}
					}
					//返回錯誤:NC位址字元不合法
					else return INVALID_COMMAND;
				}
			}
			continue;
		}
		//字元位置離開大寫字母範圍
		if (args.flag_upper) {
			//建立位址字元或關鍵字字串
			if (!CreateAddressOrKeyword(args)) {
				return CommandType::INVALID_COMMAND;
			}
		}

		//其他非數字且非大寫字母的字元
		//測試字元是否為運算子
		unsigned short operator_type(macro_generator.CheckOperatorType(ch));
		//字元屬於某種運算子
		if (operator_type != NOT_AN_OPERATOR) {
			if (!macro_generator.ProcessOperatorAddress(operator_type, ch)) return INVALID_COMMAND;
		}
		//字元為優先運算範圍開始
		else if (ch == ADDRESS_PRIORITY_RANGE_BEGIN) {
			if (!macro_generator.LevelUp())	return INVALID_COMMAND;
		}
		//字元為優先運算範圍結束
		else if (ch == ADDRESS_PRIORITY_RANGE_END) {
			if (!macro_generator.ReturnPriorityOperatorToPreviousLevel()) return INVALID_COMMAND;
		}
		//字元為雙引數分隔符號
		else if (ch == ADDRESS_ARGUMENT_SEPARATOR) {
			if (macro_generator.TwoArgumentsFlag()) {
				return INVALID_COMMAND; }
			else {
				macro_generator.TwoArgumentsFlag() = true; }
		}
		//字元為註解開始
		else if (ch == ADDRESS_COMMENT_BEGIN) {
			//設定忽略註解文字旗標
			args.flag_control_out = true;
		}
		//直接忽略非巨集相關字元
		else {
			continue;
		}
	}

	//字元位置離開數字範圍(單節最後一個字元為數字)
	if (args.flag_digit) {
		//建立一元或常數運算子
		if (!CreateUnaryOrConstantOperator(args)) {
			return CommandType::INVALID_COMMAND;
		}
	}

	//檢查並處理至最後的根部運算子
	if (!macro_generator.ProcessToFinalOperator()) return INVALID_COMMAND;
	else {
		//有NC位址字元及巨集算術運算子
		if (args.NC_address != NULL && macro_generator.GeneralOperators().front().arithmetic) {
			//清除NC位址字元
			args.NC_address = NULL;
			//刪除巨集運算子
			macro_generator.GeneralOperators().pop_front();
		}
	}

	//巨集運算層數在最底層
	if (macro_generator.CurrentLevel() == 0) {
		//成功建立算術運算子
		if (!macro_generator.GeneralOperators().empty()) {
			return MACRO_COMMAND;
		}
		//成功建立條件式算術運算子
		else if (!macro_generator.conditional_arithmetic_operator.Empty()) {
			return MACRO_COMMAND;
		}
		//成功建立條件式分支運算子
		else if (!macro_generator.conditional_branch_operator.Empty()) {
			return MACRO_COMMAND;
		}
		//成功建立條件式迴圈運算子
		else if (!macro_generator.conditional_loop_operator.Empty()) {
			return MACRO_COMMAND;
		}
		//成功建立迴圈結束運算子
		else if (!macro_generator.loop_end_operator.Empty()) {
			return MACRO_COMMAND;
		}
		//返回錯誤:未成功建立任何巨集運算子
		else return INVALID_COMMAND;
	}
	//返回錯誤:巨集運算層數未返回最底層
	else {
		return INVALID_COMMAND;
	}
}

bool MacroParser::CreateAddressOrKeyword(Argument& args)
{
	//僅有一個大寫字母
	if (args.iter - args.upper_begin == 1) {
		//設立NC位址字元
		args.NC_address = *args.upper_begin;
	}
	//超過一個大寫字母
	else {
		//建立關鍵字字串
		macro_generator.UpperString().push(string(args.upper_begin, args.iter));
		//處理關鍵字字串
		if (!macro_generator.ProcessOperatorKeyword()) {
			return false; }
	}
	//關閉大寫字母字元旗標
	args.flag_upper = false;
	//清空大寫字母起始位置
	args.upper_begin = string::const_iterator();
	
	return true;
}

bool MacroParser::CreateUnaryOrConstantOperator(Argument& args)
{
	//建立數字字串
	macro_generator.CreateDigitString(args.digit_begin, args.iter);
	//目前為巨集運算模式
	if (macro_generator.IsMacroMode()) {
		//建立常數運算子
		if (!macro_generator.CreateConstantOperator()) return false;
		else {
			//嘗試建立一元運算子(minus,#)
			macro_generator.CreateUnaryAddressOperator();
		}
	}
	//非巨集運算模式
	else {
		//返回錯誤:無NC位址字元可配對
		if (args.NC_address == NULL) {
			if (!macro_generator.CreateConstantOperator()) return false;
		}
		//有NC位址字元
		else {
			//清除數字字串
			macro_generator.DigitString().pop();
			//清除NC位址字元
			args.NC_address = NULL;
		}
	}
	//關閉數字字元旗標
	args.flag_digit = false;
	//清空數字字元起始位置
	args.digit_begin = string::const_iterator();

	return true;
}
