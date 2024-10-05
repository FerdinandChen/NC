#include "MacroOperator.h"
#include <stdexcept>
#include <cmath>

using namespace std;

ArithmeticOperator::ArithmeticOperator(MacroOperatorID id)
	:operator_ID(id),result(NULL_FLOAT_VALUE)
{
}

UnaryOperator::UnaryOperator(MacroOperatorID id,double opr)
	:ArithmeticOperator(id),operand(opr)
{
	result = opr;
}

UnaryOperator::UnaryOperator(MacroOperatorID id, const shared_ptr<ArithmeticOperator>& opr)
	:ArithmeticOperator(id),operand(NULL_FLOAT_VALUE),operand_handle(opr)
{
}

BinaryOperator::BinaryOperator(MacroOperatorID id, const shared_ptr<ArithmeticOperator>& left, const shared_ptr<ArithmeticOperator>& right)
	:ArithmeticOperator(id),left_operand(left),right_operand(right)
{
}

RelationalOperator::RelationalOperator(const shared_ptr<ArithmeticOperator>& left, const shared_ptr<ArithmeticOperator>& right)
	:left_result(NULL_FLOAT_VALUE),
	right_result(NULL_FLOAT_VALUE),
	left_operand(left),
	right_operand(right)
{
}

bool RelationalOperator::Evaluate()
{
	//先核算左運算元
	if (left_operand) {
		left_result = left_operand->Evaluate(); }
	//再核算右運算元
	if (right_operand) {
		right_result = right_operand->Evaluate(); }
	return true;
}

LogicalOperator::LogicalOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_arithmetic(left_operand),
	right_arithmetic(right_operand)
{
}

LogicalOperator::LogicalOperator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_relation(left_operand),
	right_relation(right_operand)
{
}

LogicalOperator::LogicalOperator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_logical(left_operand),
	right_logical(right_operand)
{
}

LogicalOperator::LogicalOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_arithmetic(left_operand),
	right_logical(right_operand)
{
}

LogicalOperator::LogicalOperator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_logical(left_operand),
	right_arithmetic(right_operand)
{
}

LogicalOperator::LogicalOperator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_relation(left_operand),
	right_logical(right_operand)
{
}

LogicalOperator::LogicalOperator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:left_result(0),
	right_result(0),
	left_logical(left_operand),
	right_relation(right_operand)
{
}

unsigned LogicalOperator::Evaluate()
{
	if (left_arithmetic) {
		left_result = static_cast<unsigned>(left_arithmetic->Evaluate());
		if (right_arithmetic) {
			right_result = static_cast<unsigned>(right_arithmetic->Evaluate()); }
		else if (right_logical) {
			right_result = right_logical->Evaluate(); }
	}
	else if (left_logical) {
		left_result = left_logical->Evaluate();
		if (right_relation) {
			right_result = static_cast<unsigned>(right_relation->Evaluate()); }
		else if (right_logical) {
			right_result = right_logical->Evaluate(); }
		else if (right_arithmetic) {
			right_result = static_cast<unsigned>(right_arithmetic->Evaluate()); }
	}
	else if (left_relation) {
		left_result = static_cast<unsigned>(left_relation->Evaluate());
		if (right_logical) {
			right_result = right_logical->Evaluate(); }
		else if (right_relation) {
			right_result = static_cast<unsigned>(right_relation->Evaluate()); }
	}
	
	return 1;
}

GeneralOperatorHandle::GeneralOperatorHandle(const shared_ptr<ArithmeticOperator>& handle)
	:arithmetic(handle)
{
}

GeneralOperatorHandle::GeneralOperatorHandle(const shared_ptr<RelationalOperator>& handle)
	:relational(handle)
{
}

GeneralOperatorHandle::GeneralOperatorHandle(const shared_ptr<LogicalOperator>& handle)
	:logical(handle)
{
}

ConstantOperator::ConstantOperator(double operand)
	:UnaryOperator(MacroOperatorID::CONSTANT,operand)
{
	result = operand;
}

MinusOperator::MinusOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::MINUS, operand)
{
}

VariableOperator::VariableOperator(MacroVariableInterface& interface, const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::VARIABLE, operand),
	variable_ID(0),
	macro_variable_interface(interface)
{
}

double VariableOperator::Evaluate()
{
	//巨集變數ID
	variable_ID = static_cast<unsigned short>(operand_handle->Evaluate());
	//變數值
	double value(0.0);
	//嘗試讀取ID所指定的變數值
	if (macro_variable_interface.ReadVariable(variable_ID, value)) {
		return value; }
	else {
		throw out_of_range("out_of_range: the variable ID is not exist."); }
}

bool VariableOperator::WriteVariable(double value)
{
	//巨集變數ID
	variable_ID = static_cast<unsigned short>(operand_handle->Evaluate());
	//嘗試寫入ID所指定的變數值
	if (macro_variable_interface.WriteVariable(variable_ID, value)) {
		return true; }
	else {
		return false; }
}

SineOperator::SineOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::SINE, operand)
{
}

CosineOperator::CosineOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::COSINE, operand)
{
}

TangentOperator::TangentOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::TANGENT, operand)
{
}

ArcSineOperator::ArcSineOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ARC_SINE, operand)
{
}

ArcCosineOperator::ArcCosineOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ARC_COSINE, operand)
{
}

ArcTangentOperator::ArcTangentOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ARC_TANGENT, operand)
{
}

SquareRootOperator::SquareRootOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::SQUARE_ROOT, operand)
{
}

AbsoluteValueOperator::AbsoluteValueOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ABSOLUTE_VALUE, operand)
{
}

RoundOffOperator::RoundOffOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ROUND_OFF, operand)
{
}

RoundDownOperator::RoundDownOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ROUND_DOWN, operand)
{
}

double RoundDownOperator::Evaluate()
{
	double value(operand_handle->Evaluate());
	if (value < 0.0) {
		result = ceil(value);
		return result;
	}
	else {
		result = floor(value);
		return result;
	}
}

RoundUpOperator::RoundUpOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::ROUND_UP, operand)
{
}

double RoundUpOperator::Evaluate()
{
	double value(operand_handle->Evaluate());
	if (value < 0.0) {
		result = floor(value);
		return result;
	}
	else {
		result = ceil(value);
		return result;
	}
}

NaturalLogOperator::NaturalLogOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::NATURAL_LOG, operand)
{
}

ExponentOperator::ExponentOperator(const shared_ptr<ArithmeticOperator>& operand)
	:UnaryOperator(MacroOperatorID::EXPONENT, operand)
{
}

PowerOperator::PowerOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::POWER, left_operand, right_operand)
{
}

ArcTangent2Operator::ArcTangent2Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::ARC_TANGENT2, left_operand, right_operand)
{
}

AddOperator::AddOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::ADD, left_operand, right_operand)
{
}

SubtractOperator::SubtractOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::SUBSTRACT, left_operand, right_operand)
{
}

MultiplyOperator::MultiplyOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::MULTIPLY, left_operand, right_operand)
{
}

DivideOperator::DivideOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::DIVIDE, left_operand, right_operand)
{
}

AssignmentOperator::AssignmentOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::ASSIGNMENT, left_operand, right_operand),
	left_variable(static_cast<VariableOperator*>(left_operand.operator ->())),
	right_logical(nullptr)
{
}

AssignmentOperator::AssignmentOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:BinaryOperator(MacroOperatorID::ASSIGNMENT, left_operand, shared_ptr<ArithmeticOperator>(nullptr)),
	left_variable(static_cast<VariableOperator*>(left_operand.operator ->())),
	right_logical(right_operand)
{
}

double AssignmentOperator::Evaluate()
{
	//左運算元(變數運算子)為空
	if (!left_operand) {
		throw invalid_argument("invalid_argument: left operand(variable) is null");
	}
	//右運算元為邏輯運算子
	else if (right_logical) {
		result = right_logical->Evaluate();
		left_variable->WriteVariable(result);
	}
	//右運算元為算術運算子
	else if (right_operand) {
		result = right_operand->Evaluate();
		left_variable->WriteVariable(result);
	}
	//右運算元為空
	else {
		throw invalid_argument("invalid argument: right operand is null."); }

	return left_variable->Evaluate();
}

EqualOperator::EqualOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:RelationalOperator(left_operand, right_operand)
{
}

bool EqualOperator::Evaluate()
{
	RelationalOperator::Evaluate();
	return left_result == right_result ? true : false;
}

NotEqualOperator::NotEqualOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:RelationalOperator(left_operand, right_operand)
{
}

bool NotEqualOperator::Evaluate()
{
	RelationalOperator::Evaluate();
	return left_result != right_result ? true : false;
}

GreaterOperator::GreaterOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:RelationalOperator(left_operand, right_operand)
{
}

bool GreaterOperator::Evaluate()
{
	RelationalOperator::Evaluate();
	return left_result > right_result ? true : false;
}

GreaterEqualOperator::GreaterEqualOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:RelationalOperator(left_operand, right_operand)
{
}

bool GreaterEqualOperator::Evaluate()
{
	RelationalOperator::Evaluate();
	return left_result >= right_result ? true : false;
}

LessOperator::LessOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:RelationalOperator(left_operand, right_operand)
{
}

bool LessOperator::Evaluate()
{
	RelationalOperator::Evaluate();
	return left_result < right_result ? true : false;
}

LessEqualOperator::LessEqualOperator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:RelationalOperator(left_operand, right_operand)
{
}

bool LessEqualOperator::Evaluate()
{
	RelationalOperator::Evaluate();
	return left_result <= right_result ? true : false;
}

AND_Operator::AND_Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

AND_Operator::AND_Operator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

AND_Operator::AND_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

AND_Operator::AND_Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

AND_Operator::AND_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

AND_Operator::AND_Operator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

AND_Operator::AND_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

unsigned AND_Operator::Evaluate()
{
	LogicalOperator::Evaluate();
	return left_result & right_result;
}

OR_Operator::OR_Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

OR_Operator::OR_Operator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

OR_Operator::OR_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

OR_Operator::OR_Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

OR_Operator::OR_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

OR_Operator::OR_Operator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

OR_Operator::OR_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

unsigned OR_Operator::Evaluate()
{
	LogicalOperator::Evaluate();
	return left_result | right_result;
}

XOR_Operator::XOR_Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

XOR_Operator::XOR_Operator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

XOR_Operator::XOR_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

XOR_Operator::XOR_Operator(const shared_ptr<ArithmeticOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

XOR_Operator::XOR_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<ArithmeticOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

XOR_Operator::XOR_Operator(const shared_ptr<RelationalOperator>& left_operand, const shared_ptr<LogicalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

XOR_Operator::XOR_Operator(const shared_ptr<LogicalOperator>& left_operand, const shared_ptr<RelationalOperator>& right_operand)
	:LogicalOperator(left_operand, right_operand)
{
}

unsigned XOR_Operator::Evaluate()
{
	LogicalOperator::Evaluate();
	return left_result ^ right_result;
}

ConditionalArithmeticOperator::ConditionalArithmeticOperator(const shared_ptr<RelationalOperator>& relational, const shared_ptr<ArithmeticOperator>& arithmetic)
	:relational_operator(relational),
	arithmetic_operator(arithmetic)
{
}

ConditionalArithmeticOperator::ConditionalArithmeticOperator(const shared_ptr<LogicalOperator>& logical, const shared_ptr<ArithmeticOperator>& arithmetic)
	:logical_operator(logical),
	arithmetic_operator(arithmetic)
{
}

void ConditionalArithmeticOperator::Clear()
{
	relational_operator.reset();
	logical_operator.reset();
	arithmetic_operator.reset();
}

bool ConditionalArithmeticOperator::Evaluate()
{
	if (Empty()) {
		return false; }
	//條件式為邏輯運算子
	if (logical_operator) {
		//滿足邏輯條件
		if (logical_operator->Evaluate()) {
			//對算數運算子進行核算
			arithmetic_operator->Evaluate();
			return true;
		}
		else {
			return false; }
	}
	//條件式為關係運算子
	else {
		//滿足關係條件
		if (relational_operator->Evaluate()) {
			//對算數運算子進行核算
			arithmetic_operator->Evaluate();
			return true;
		}
		else {
			return false; }
	}
}

ConditionalBranchOperator::ConditionalBranchOperator(const shared_ptr<ArithmeticOperator>& arithmetic)
	:arithmetic_operand(arithmetic)
{
}

ConditionalBranchOperator::ConditionalBranchOperator(const shared_ptr<RelationalOperator>& relational, const shared_ptr<ArithmeticOperator>& arithmetic)
	:relational_operand(relational),
	arithmetic_operand(arithmetic)
{
}

ConditionalBranchOperator::ConditionalBranchOperator(const shared_ptr<LogicalOperator>& logical, const shared_ptr<ArithmeticOperator>& arithmetic)
	:logical_operand(logical),
	arithmetic_operand(arithmetic)
{
}

void ConditionalBranchOperator::Clear()
{
	relational_operand.reset();
	logical_operand.reset();
	arithmetic_operand.reset();
}

bool ConditionalBranchOperator::Evaluate()
{
	if (Empty()) {
		return false; }

	if (relational_operand) {
		return relational_operand->Evaluate(); }
	else if (logical_operand) {
		return static_cast<bool>(logical_operand->Evaluate()); }
	else {
		return true; }
}

int ConditionalBranchOperator::BranchNumber()
{
	if (Empty()) {
		return 0; }
	else {
		return static_cast<int>(arithmetic_operand->Evaluate()); }
}

ConditionalLoopOperator::ConditionalLoopOperator(const shared_ptr<ArithmeticOperator>& arithmetic)
	:arithmetic_operator(arithmetic)
{
}

ConditionalLoopOperator::ConditionalLoopOperator(const shared_ptr<RelationalOperator>& relational, const shared_ptr<ArithmeticOperator>& arithmetic)
	:relational_operator(relational),
	arithmetic_operator(arithmetic)
{
}

ConditionalLoopOperator::ConditionalLoopOperator(const shared_ptr<LogicalOperator>& logical, const shared_ptr<ArithmeticOperator>& arithmetic)
	:logical_operator(logical),
	arithmetic_operator(arithmetic)
{
}

void ConditionalLoopOperator::Clear()
{
	relational_operator.reset();
	logical_operator.reset();
	arithmetic_operator.reset();
}

bool ConditionalLoopOperator::Evaluate()
{
	if (Empty()) {
		return false; }

	if (relational_operator) {
		return relational_operator->Evaluate(); }
	else if (logical_operator) {
		return static_cast<bool>(logical_operator->Evaluate()); }
	else {
		return true; }
}

unsigned short ConditionalLoopOperator::LoopNumber()
{
	if (Empty()) {
		return 0; }
	else {
		return static_cast<unsigned short>(arithmetic_operator->Evaluate()); }
}

LoopEndOperator::LoopEndOperator(const shared_ptr<ArithmeticOperator>& arithmetic)
	:arithmetic_operator(arithmetic)
{
}

unsigned short LoopEndOperator::Evaluate()
{
	if (Empty()) {
		return 0; }
	else {
		return static_cast<unsigned short>(arithmetic_operator->Evaluate()); }
}
