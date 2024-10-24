#pragma once

#include "MacroVariable.h"
#include <memory>
#include <numbers>

//空浮點數值
constexpr double NULL_FLOAT_VALUE = DBL_MIN;

constexpr bool ALLOW_ADD_OPERATOR_ON_MINUS = false;
constexpr bool ALLOW_SUBTRACT_OPERATOR_ON_MINUS = false;

//運算子識別ID
enum MacroOperatorID {
	//常數運算子
	CONSTANT,
	//負值運算子
	MINUS,
	//變數運算子
	VARIABLE,
	//加法運算子
	ADD,
	//減法運算子
	SUBSTRACT,
	//乘法運算子
	MULTIPLY,
	//除法運算子
	DIVIDE,
	//賦值運算子
	ASSIGNMENT,
	//正弦運算子
	SINE,
	//餘弦運算子
	COSINE,
	//正切運算子
	TANGENT,
	//反正弦運算子
	ARC_SINE,
	//反餘弦運算子
	ARC_COSINE,
	//反正切運算子
	ARC_TANGENT,
	//反正切運算子(雙參數版本)
	ARC_TANGENT2,
	//平方根運算子
	SQUARE_ROOT,
	//絕對值運算子
	ABSOLUTE_VALUE,
	//二進碼運算子
	BINARY_CODE,
	//二進位十進制運算子
	BINARY_CODED_DECIMAL,
	//四捨五入運算子
	ROUND_OFF,
	//無條件捨去運算子
	ROUND_DOWN,
	//無條件進位運算子
	ROUND_UP,
	//自然對數運算子
	NATURAL_LOG,
	//指數運算子
	EXPONENT,
	//次方運算子
	POWER,
	//轉換浮點數運算子
	ADD_DECIMAL_POINT
};

//算術運算子基礎類別
class ArithmeticOperator {
public:
	//查詢運算子ID
	MacroOperatorID GetOperatorID() const {
		return operator_ID; }
	//核算運算子
	virtual double Evaluate() = 0;

protected:
	//建構式
	ArithmeticOperator(MacroOperatorID id);
	//解構式
	virtual ~ArithmeticOperator() {}
	//運算子依照動態型別自我複製
	virtual ArithmeticOperator* clone() const = 0;
	
	//運算子ID
	const MacroOperatorID operator_ID;
	double result;
};

//一元運算子
class UnaryOperator:public ArithmeticOperator {
protected:
	UnaryOperator(MacroOperatorID id,double opr);
	UnaryOperator(MacroOperatorID id,const std::shared_ptr<ArithmeticOperator>& opr);
	virtual ~UnaryOperator() {}
	//運算子依照動態型別自我複製
	virtual UnaryOperator* clone() const = 0;

	//浮點數運算元
	const double operand;
	//運算子運算元
	std::shared_ptr<ArithmeticOperator> operand_handle;
};

//二元運算子
class BinaryOperator :public ArithmeticOperator {
protected:
	BinaryOperator(MacroOperatorID id, const std::shared_ptr<ArithmeticOperator>& left, const std::shared_ptr<ArithmeticOperator>& right);
	virtual ~BinaryOperator() {}
	//運算子依照動態型別自我複製
	virtual BinaryOperator* clone() const = 0;

	//左(運算子)運算元
	std::shared_ptr<ArithmeticOperator> left_operand;
	//右(運算子)運算元
	std::shared_ptr<ArithmeticOperator> right_operand;
};

//關係運算子
class RelationalOperator {
protected:
	RelationalOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	virtual RelationalOperator* clone() const {
		return new RelationalOperator(*this); }
	virtual ~RelationalOperator() {}

	double left_result;
	double right_result;
	std::shared_ptr<ArithmeticOperator> left_operand;
	std::shared_ptr<ArithmeticOperator> right_operand;

public:
	virtual bool Evaluate();
};

//邏輯運算子
class LogicalOperator {
protected:
	LogicalOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	LogicalOperator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<RelationalOperator>&);
	LogicalOperator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<LogicalOperator>&);

	LogicalOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<LogicalOperator>&);
	LogicalOperator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	LogicalOperator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<LogicalOperator>&);
	LogicalOperator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<RelationalOperator>&);

	virtual LogicalOperator* clone() const {
		return new LogicalOperator(*this); }
	virtual ~LogicalOperator() {}

	unsigned left_result;
	unsigned right_result;
	std::shared_ptr<LogicalOperator> left_logical;
	std::shared_ptr<LogicalOperator> right_logical;
	std::shared_ptr<ArithmeticOperator> left_arithmetic;
	std::shared_ptr<ArithmeticOperator> right_arithmetic;
	std::shared_ptr<RelationalOperator> left_relation;
	std::shared_ptr<RelationalOperator> right_relation;

public:
	virtual unsigned Evaluate();
};

//通用運算子Handle
class GeneralOperatorHandle {
public:
	explicit GeneralOperatorHandle(const std::shared_ptr<ArithmeticOperator>&);
	explicit GeneralOperatorHandle(const std::shared_ptr<RelationalOperator>&);
	explicit GeneralOperatorHandle(const std::shared_ptr<LogicalOperator>&);
	~GeneralOperatorHandle() {}

	std::shared_ptr<ArithmeticOperator> arithmetic;
	std::shared_ptr<RelationalOperator> relational;
	std::shared_ptr<LogicalOperator> logical;
};

//常數運算子
class ConstantOperator:public UnaryOperator {
public:
	explicit ConstantOperator(double operand);
	~ConstantOperator() {}
	double Evaluate() override {
		return result; }

protected:
	ConstantOperator* clone() const override {
		return new ConstantOperator(*this); }
};

//負數運算子
class MinusOperator:public UnaryOperator {
public:
	MinusOperator(const std::shared_ptr<ArithmeticOperator>&);
	~MinusOperator() {}
	double Evaluate() override {
		return -(operand_handle->Evaluate()); }

protected:
	MinusOperator* clone() const override {
		return new MinusOperator(*this); }
};

//變數運算子
class VariableOperator:public UnaryOperator {
public:
	VariableOperator(MacroVariableInterface& interface, const std::shared_ptr<ArithmeticOperator>& operand);
	~VariableOperator() {}
	double Evaluate() override;
	bool WriteVariable(double);

protected:
	VariableOperator* clone() const override {
		return new VariableOperator(*this); }

private:
	unsigned short variable_ID;
	MacroVariableInterface& macro_variable_interface;
};

//正弦運算子
class SineOperator:public UnaryOperator {
public:
	SineOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~SineOperator() {}
	double Evaluate() override {
		return sin(operand_handle->Evaluate() / 180.0 * std::numbers::pi); }

protected:
	SineOperator* clone() const override {
		return new SineOperator(*this); }
};

//餘弦運算子
class CosineOperator:public UnaryOperator {
public:
	CosineOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~CosineOperator() {}
	double Evaluate() override {
		return cos(operand_handle->Evaluate() / 180.0 * std::numbers::pi); }

protected:
	CosineOperator* clone() const override {
		return new CosineOperator(*this); }
};

//正切運算子
class TangentOperator :public UnaryOperator {
public:
	TangentOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~TangentOperator() {}
	double Evaluate() override {
		return tan(operand_handle->Evaluate() / 180.0 * std::numbers::pi); }

protected:
	TangentOperator* clone() const override {
		return new TangentOperator(*this); }
};

//反正弦運算子
class ArcSineOperator :public UnaryOperator {
public:
	ArcSineOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~ArcSineOperator() {}
	double Evaluate() override {
		return asin(operand_handle->Evaluate()) / std::numbers::pi * 180.0; }

protected:
	ArcSineOperator* clone() const override {
		return new ArcSineOperator(*this); }
};

//反餘弦運算子
class ArcCosineOperator :public UnaryOperator {
public:
	ArcCosineOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~ArcCosineOperator() {}
	double Evaluate() override {
		return acos(operand_handle->Evaluate()) / std::numbers::pi * 180.0; }

protected:
	ArcCosineOperator* clone() const override {
		return new ArcCosineOperator(*this); }
};

//反正切運算子
class ArcTangentOperator :public UnaryOperator {
public:
	ArcTangentOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~ArcTangentOperator() {}
	double Evaluate() override {
		return atan(operand_handle->Evaluate()) / std::numbers::pi * 180.0; }

protected:
	ArcTangentOperator* clone() const override {
		return new ArcTangentOperator(*this); }
};

//平方根運算子
class SquareRootOperator :public UnaryOperator {
public:
	SquareRootOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~SquareRootOperator() {}
	double Evaluate() override {
		return sqrt(operand_handle->Evaluate()); }

protected:
	SquareRootOperator* clone() const override {
		return new SquareRootOperator(*this); }
};

//絕對值運算子
class AbsoluteValueOperator :public UnaryOperator {
public:
	AbsoluteValueOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~AbsoluteValueOperator() {}
	double Evaluate() override {
		return fabs(operand_handle->Evaluate()); }

protected:
	AbsoluteValueOperator* clone() const override {
		return new AbsoluteValueOperator(*this); }
};

//四捨五入運算子
class RoundOffOperator :public UnaryOperator {
public:
	RoundOffOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~RoundOffOperator() {}
	double Evaluate() override {
		return round(operand_handle->Evaluate()); }

protected:
	RoundOffOperator* clone() const override {
		return new RoundOffOperator(*this); }
};

//無條件捨去運算子
class RoundDownOperator :public UnaryOperator {
public:
	RoundDownOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~RoundDownOperator() {}
	double Evaluate() override;

protected:
	RoundDownOperator* clone() const override {
		return new RoundDownOperator(*this); }
};

//無條件進位運算子
class RoundUpOperator :public UnaryOperator {
public:
	RoundUpOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~RoundUpOperator() {}
	double Evaluate() override;

protected:
	RoundUpOperator* clone() const override {
		return new RoundUpOperator(*this); }
};

//自然對數運算子
class NaturalLogOperator :public UnaryOperator {
public:
	NaturalLogOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~NaturalLogOperator() {}
	double Evaluate() override {
		return log(operand_handle->Evaluate()); }

protected:
	NaturalLogOperator* clone() const override {
		return new NaturalLogOperator(*this); }
};

//指數運算子
class ExponentOperator :public UnaryOperator {
public:
	ExponentOperator(const std::shared_ptr<ArithmeticOperator>& operand);
	~ExponentOperator() {}
	double Evaluate() override {
		return exp(operand_handle->Evaluate()); }

protected:
	ExponentOperator* clone() const override {
		return new ExponentOperator(*this); }
};

//次方運算子
class PowerOperator :public BinaryOperator {
public:
	PowerOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	~PowerOperator() {}
	double Evaluate() override {
		return pow(left_operand->Evaluate(), right_operand->Evaluate()); }

protected:
	PowerOperator* clone() const override {
		return new PowerOperator(*this); }
};

//反正切運算子(雙參數版本)
class ArcTangent2Operator :public BinaryOperator {
public:
	ArcTangent2Operator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	~ArcTangent2Operator() {}
	double Evaluate() override {
		return atan2(left_operand->Evaluate(), right_operand->Evaluate()) / std::numbers::pi * 180.0; }

protected:
	ArcTangent2Operator* clone() const override {
		return new ArcTangent2Operator(*this); }
};

//加法運算子
class AddOperator :public BinaryOperator {
public:
	AddOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	~AddOperator() {}
	double Evaluate() override {
		return left_operand->Evaluate() + right_operand->Evaluate(); }

protected:
	AddOperator* clone() const override {
		return new AddOperator(*this); }
};

//減法運算子
class SubtractOperator :public BinaryOperator {
public:
	SubtractOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	~SubtractOperator() {}
	double Evaluate() override {
		return left_operand->Evaluate() - right_operand->Evaluate(); }

protected:
	SubtractOperator* clone() const override {
		return new SubtractOperator(*this); }
};

//乘法運算子
class MultiplyOperator :public BinaryOperator {
public:
	MultiplyOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	~MultiplyOperator() {}
	double Evaluate() override {
		return left_operand->Evaluate() * right_operand->Evaluate(); }

protected:
	MultiplyOperator* clone() const override {
		return new MultiplyOperator(*this); }
};

//除法運算子
class DivideOperator :public BinaryOperator {
public:
	DivideOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	~DivideOperator() {}
	double Evaluate() override {
		return left_operand->Evaluate() / right_operand->Evaluate(); }

protected:
	DivideOperator* clone() const override {
		return new DivideOperator(*this); }
};

//賦值運算子
class AssignmentOperator :public BinaryOperator {
public:
	AssignmentOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<ArithmeticOperator>& right_operand);
	AssignmentOperator(const std::shared_ptr<ArithmeticOperator>& left_operand, const std::shared_ptr<LogicalOperator>& right_operand);
	~AssignmentOperator() {}
	double Evaluate() override;

private:
	VariableOperator* left_variable;
	std::shared_ptr<LogicalOperator> right_logical;

protected:
	AssignmentOperator* clone() const override {
		return new AssignmentOperator(*this); }
};

//相等運算子
class EqualOperator :public RelationalOperator {
public:
	EqualOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~EqualOperator() {}
	bool Evaluate() override;

protected:
	EqualOperator* clone() const override {
		return new EqualOperator(*this); }
};

//不相等運算子
class NotEqualOperator :public RelationalOperator {
public:
	NotEqualOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~NotEqualOperator() {}
	bool Evaluate() override;

protected:
	NotEqualOperator* clone() const override {
		return new NotEqualOperator(*this); }
};

//大於運算子
class GreaterOperator :public RelationalOperator {
public:
	GreaterOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~GreaterOperator() {}
	bool Evaluate() override;

protected:
	GreaterOperator* clone() const override {
		return new GreaterOperator(*this); }
};

//大於等於運算子
class GreaterEqualOperator :public RelationalOperator {
public:
	GreaterEqualOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~GreaterEqualOperator() {}
	bool Evaluate() override;

protected:
	GreaterEqualOperator* clone() const override {
		return new GreaterEqualOperator(*this); }
};

//小於運算子
class LessOperator :public RelationalOperator {
public:
	LessOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~LessOperator() {}
	bool Evaluate() override;

protected:
	LessOperator* clone() const override {
		return new LessOperator(*this); }
};

//小於等於運算子
class LessEqualOperator :public RelationalOperator {
public:
	LessEqualOperator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~LessEqualOperator() {}
	bool Evaluate() override;

protected:
	LessEqualOperator* clone() const override {
		return new LessEqualOperator(*this); }
};

//交集運算子
class AND_Operator :public LogicalOperator {
public:
	AND_Operator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	AND_Operator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<RelationalOperator>&);
	AND_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<LogicalOperator>&);

	AND_Operator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<LogicalOperator>&);
	AND_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	AND_Operator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<LogicalOperator>&);
	AND_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<RelationalOperator>&);
	
	~AND_Operator() {}
	unsigned Evaluate() override;

protected:
	AND_Operator* clone() const override {
		return new AND_Operator(*this); }
};

//聯集運算子
class OR_Operator :public LogicalOperator {
public:
	OR_Operator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	OR_Operator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<RelationalOperator>&);
	OR_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<LogicalOperator>&);

	OR_Operator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<LogicalOperator>&);
	OR_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	OR_Operator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<LogicalOperator>&);
	OR_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<RelationalOperator>&);

	~OR_Operator() {}
	unsigned Evaluate() override;

protected:
	OR_Operator* clone() const override {
		return new OR_Operator(*this); }
};

//互斥運算子
class XOR_Operator :public LogicalOperator {
public:
	XOR_Operator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	XOR_Operator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<RelationalOperator>&);
	XOR_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<LogicalOperator>&);

	XOR_Operator(const std::shared_ptr<ArithmeticOperator>&, const std::shared_ptr<LogicalOperator>&);
	XOR_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	XOR_Operator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<LogicalOperator>&);
	XOR_Operator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<RelationalOperator>&);

	~XOR_Operator() {}
	unsigned Evaluate() override;

protected:
	XOR_Operator* clone() const override {
		return new XOR_Operator(*this); }
};

//條件算術運算子
class ConditionalArithmeticOperator {
public:
	ConditionalArithmeticOperator() {}
	ConditionalArithmeticOperator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	ConditionalArithmeticOperator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~ConditionalArithmeticOperator() {}
	
	void Clear();
	bool Evaluate();
	bool Empty() const {
		return !relational_operator && !logical_operator || !arithmetic_operator; }
	double ArithmeticValue() {
		return arithmetic_operator->Evaluate(); }

protected:
	std::shared_ptr<RelationalOperator> relational_operator;
	std::shared_ptr<LogicalOperator> logical_operator;
	std::shared_ptr<ArithmeticOperator> arithmetic_operator;
};

//條件分支運算子
class ConditionalBranchOperator {
public:
	ConditionalBranchOperator() {}
	ConditionalBranchOperator(const std::shared_ptr<ArithmeticOperator>&);
	ConditionalBranchOperator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	ConditionalBranchOperator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~ConditionalBranchOperator() {}
	void Clear();
	bool Evaluate();
	bool Empty() const {
		return arithmetic_operand ? false : true; }
	int BranchNumber();

protected:
	std::shared_ptr<RelationalOperator> relational_operand;
	std::shared_ptr<LogicalOperator> logical_operand;
	std::shared_ptr<ArithmeticOperator> arithmetic_operand;
};

//條件迴圈運算子
class ConditionalLoopOperator {
public:
	ConditionalLoopOperator() {}
	ConditionalLoopOperator(const std::shared_ptr<ArithmeticOperator>&);
	ConditionalLoopOperator(const std::shared_ptr<RelationalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	ConditionalLoopOperator(const std::shared_ptr<LogicalOperator>&, const std::shared_ptr<ArithmeticOperator>&);
	~ConditionalLoopOperator() {}
	void Clear();
	bool Evaluate();
	bool Empty() const {
		return arithmetic_operator ? false : true; }
	unsigned short LoopNumber();

protected:
	std::shared_ptr<RelationalOperator> relational_operator;
	std::shared_ptr<LogicalOperator> logical_operator;
	std::shared_ptr<ArithmeticOperator> arithmetic_operator;
};

//迴圈終端運算子
class LoopEndOperator {
public:
	LoopEndOperator() {}
	LoopEndOperator(const std::shared_ptr<ArithmeticOperator>&);
	~LoopEndOperator() {}
	void Clear() {
		arithmetic_operator.reset(); }
	unsigned short Evaluate();
	bool Empty() const {
		return arithmetic_operator ? false : true; }

protected:
	std::shared_ptr<ArithmeticOperator> arithmetic_operator;
};
