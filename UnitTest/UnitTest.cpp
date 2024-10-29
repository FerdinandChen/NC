#include "pch.h"
#include "CppUnitTest.h"
#include "MacroOperator.h"
#include "FanucMacroParser.h"
#include <numbers>
#include <cmath>
#include <string>
#include <queue>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			//測試框架需要的自訂型別(enum CommandType)模板函式
			template<>
			std::wstring ToString<CommandType>(const CommandType& command_type) {
				switch (command_type) {
					case CommandType::INVALID_COMMAND:
						return L"Invalid Command";
					case CommandType::UNKNOWN_COMMAND:
						return L"Unknown Command";
					case CommandType::NC_COMMAND:
						return L"NC Command";
					case CommandType::MACRO_COMMAND:
						return L"Macro Command";
					default:
						return L"unknown command type";
				}
			}
		}
	}
}

namespace UnitTest
{
	using std::numbers::pi;
	using std::numbers::e;
	using std::shared_ptr;
	using std::make_shared;
	using std::string;
	using std::wstring;
	using std::queue;
	using std::invalid_argument;
	using std::runtime_error;

	//轉換角度為徑度
	inline static double ConvertAngleToRadians(double angle)	{
		return angle * pi / 180.0; }

	//轉換徑度為角度
	inline static double ConvertRadiansToAngle(double radians)	{
		return radians / pi * 180.0; 	}

	//建立常數運算子
	inline static shared_ptr<ArithmeticOperator> CreateConstantOperator(double value) {
		shared_ptr<ArithmeticOperator> constant_operator(make_shared<ConstantOperator>(value));
		return constant_operator;
	}

	//建立變數運算子
	inline static shared_ptr<ArithmeticOperator> CreateVariableOperator(MacroVariableInterface& macro_variable_interface, unsigned short ID) {
		//常數運算子(變數ID)
		auto variable_ID(CreateConstantOperator(ID));
		//變數運算子
		shared_ptr<ArithmeticOperator> variable_operator(make_shared<VariableOperator>(macro_variable_interface, variable_ID));
		return variable_operator;
	}

	//剖析並驗證巨集算式
	static GeneralOperatorHandle AssertMacroExpression(MacroVariableInterface& macro_variable_interface, const string& block)
	{
		FanucMacroParser parser(macro_variable_interface);

		//剖析NC單節是否為合法巨集算式
		Assert::AreEqual(CommandType::MACRO_COMMAND, parser.ParseBlock(block));
		//確認成功產生巨集運算子
		Assert::IsFalse(parser.macro_generator.GeneralOperators().empty());
		//回傳巨集運算子Handle
		return parser.macro_generator.GeneralOperators().front();
	}

	//核算並驗證巨集算術運算式
	static double EvaluateArithmeticMacroExpression(MacroVariableInterface& macro_variable_interface, const string& block)
	{
		//剖析並驗證巨集算數運算式
		GeneralOperatorHandle handle(AssertMacroExpression(macro_variable_interface, block));
		//確認是算術運算子
		if (handle.arithmetic) {
			return handle.arithmetic->Evaluate(); }
		//非算術運算子
		else {
			throw invalid_argument("invalid_argument: arithmetic operator is empty.");
		}
	}
	
	//核算並驗證巨集邏輯運算式
	static unsigned EvaluateLogicalMacroExpression(MacroVariableInterface& macro_variable_interface, const string& block)
	{
		GeneralOperatorHandle handle(AssertMacroExpression(macro_variable_interface, block));
		//確認是邏輯運算子
		if (handle.logical) {
			return handle.logical->Evaluate();
		}
		//非邏輯運算子
		else {
			throw invalid_argument("invalid_argument: logical operator is empty.");
		}
	}

	//核算並驗證巨集關係運算式
	static bool EvaluateRelationalMacroExpression(MacroVariableInterface& macro_variable_interface, const string& block)
	{
		GeneralOperatorHandle handle(AssertMacroExpression(macro_variable_interface, block));
		//確認是關係運算子
		if (handle.relational) {
			return handle.relational->Evaluate();
		}
		//非關係運算子
		else {
			throw invalid_argument("invalid_argument: relational operator is empty.");
		}
	}

	namespace MacroOperators {
		TEST_CLASS(UnaryArithmeticOperators)
		{
		public:
			TEST_METHOD(Constant)
			{
				double expected(pi);
				
				auto constant(CreateConstantOperator(expected));
				double actual(constant->Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Minus)
			{
				double expected(-pi);
				
				auto constant_operator(CreateConstantOperator(pi));
				MinusOperator minus_operator(constant_operator);
				double actual(minus_operator.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Variable)
			{
				SystemParameter system_parameter;
				MacroVariableInterface mi(system_parameter);
				
				//測試變數讀取
				double expected(NULL_FLOAT_VALUE);
				unsigned short variable_ID(1);
				auto arithmetic(CreateVariableOperator(mi,variable_ID));
				double actual(arithmetic->Evaluate());
				Assert::AreEqual(expected, actual);

				//測試變數寫入
				expected = pi;
				//將算術運算子轉型回為原本的變數運算子
				if (VariableOperator* variable_operator = dynamic_cast<VariableOperator*>(arithmetic.get())) {
					//寫入變數值
					variable_operator->WriteVariable(expected); }
				//轉型失敗
				else {
					//強制產生測試失敗
					Assert::Fail(); }
				actual = arithmetic->Evaluate();
				Assert::AreEqual(expected, actual);

				//測試空變數(#0)讀取
				expected = NULL_FLOAT_VALUE;
				variable_ID = 0;
				auto null_variable(CreateVariableOperator(mi, variable_ID));
				actual = null_variable->Evaluate();
				Assert::AreEqual(expected, actual);

				//測試空變數(#0)禁止寫入
				//將算術運算子轉型回為原本的變數運算子
				if (VariableOperator* variable_operator = dynamic_cast<VariableOperator*>(null_variable.get())) {
					//測試寫入操作:預期會拋出執行期異常
					Assert::ExpectException<runtime_error>( [&variable_operator] () -> void {
						//嘗試寫入變數值
						variable_operator->WriteVariable(pi);
						}, L"expected runtime_error when writing to null variable #0."
					);
				}
				//轉型失敗
				else {
					//強制產生測試失敗
					Assert::Fail();
				}
			}

			TEST_METHOD(Sine)
			{
				double angle(30.0);
				double expected(sin(ConvertAngleToRadians(angle)));
				
				auto constant(CreateConstantOperator(angle));
				SineOperator sine(constant);
				double actual(sine.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Cosine)
			{
				double angle(30.0);
				double expected(cos(ConvertAngleToRadians(angle)));
				auto constant(CreateConstantOperator(angle));
				CosineOperator cosine(constant);
				double actual(cosine.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Tangent)
			{
				double angle(30.0);
				double expected(tan(ConvertAngleToRadians(angle)));
				
				auto constant(CreateConstantOperator(angle));
				TangentOperator tangent(constant);
				double actual(tangent.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcSine)
			{
				double ratio(0.5);
				double expected(ConvertRadiansToAngle(asin(ratio)));

				auto constant(CreateConstantOperator(ratio));
				ArcSineOperator arcsine(constant);
				double actual(arcsine.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcCosine)
			{
				double ratio(0.5);
				double expected(ConvertRadiansToAngle(acos(ratio)));
				
				auto constant(CreateConstantOperator(ratio));
				ArcCosineOperator arccosine(constant);
				double actual(arccosine.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcTangent)
			{
				double ratio(0.5);
				double expected(ConvertRadiansToAngle(atan(ratio)));
				
				auto constant(CreateConstantOperator(ratio));
				ArcTangentOperator arctangent(constant);
				double actual(arctangent.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(SquareRoot)
			{
				double param(3.0);
				double expected(sqrt(param));

				auto constant(CreateConstantOperator(param));
				SquareRootOperator square_root(constant);
				double actual(square_root.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(AbsoluteValue)
			{
				double param(-pi);
				double expected(fabs(param));

				auto constant(CreateConstantOperator(param));
				AbsoluteValueOperator absolute_value(constant);
				double actual(absolute_value.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(RoundOff)
			{
				double param(pi);
				double expected(round(param));

				auto constant(CreateConstantOperator(param));
				RoundOffOperator round_off(constant);
				double actual(round_off.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(RoundDown)
			{
				double param(pi);
				double expected(floor(param));

				auto constant(CreateConstantOperator(param));
				RoundDownOperator round_down(constant);
				double actual(round_down.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(RoundUp)
			{
				double param(pi);
				double expected(ceil(param));

				auto constant(CreateConstantOperator(param));
				RoundUpOperator round_up(constant);
				double actual(round_up.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(NaturalLog)
			{
				double param(e);
				double expected(log(param));

				auto constant(CreateConstantOperator(param));
				NaturalLogOperator natural_log(constant);
				double actual(natural_log.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Exponent)
			{
				double param(e);
				double expected(exp(param));

				auto constant(CreateConstantOperator(param));
				ExponentOperator exponent(constant);
				double actual(exponent.Evaluate());

				Assert::AreEqual(expected, actual);
			}
		};

		TEST_CLASS(BinaryArithmeticOperators)
		{
		public:
			TEST_METHOD(Power)
			{
				double base(pi);
				auto left_operand(CreateConstantOperator(base));
				double exponent(e);
				auto right_operand(CreateConstantOperator(exponent));

				double expected(pow(base, exponent));
				PowerOperator power(left_operand, right_operand);
				double actual(power.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcTangent2)
			{
				double left(1);
				auto left_operand(CreateConstantOperator(left));
				double right(1);
				auto right_operand(CreateConstantOperator(right));

				double expected(atan2(left, right) / pi * 180.0);
				ArcTangent2Operator arctangent2(left_operand, right_operand);
				double actual(arctangent2.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Add)
			{
				double left(pi);
				auto left_operand(CreateConstantOperator(left));
				double right(e);
				auto right_operand(CreateConstantOperator(right));

				double expected(left + right);
				AddOperator add(left_operand, right_operand);
				double actual(add.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Subtract)
			{
				double left(pi);
				auto left_operand(CreateConstantOperator(left));
				double right(e);
				auto right_operand(CreateConstantOperator(right));

				double expected(left - right);
				SubtractOperator subtract(left_operand, right_operand);
				double actual(subtract.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Multiply)
			{
				double left(pi);
				auto left_operand(CreateConstantOperator(left));
				double right(e);
				auto right_operand(CreateConstantOperator(right));

				double expected(left * right);
				MultiplyOperator multiply(left_operand, right_operand);
				double actual(multiply.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Divide)
			{
				double left(pi);
				auto left_operand(CreateConstantOperator(left));
				double right(e);
				auto right_operand(CreateConstantOperator(right));

				double expected(left / right);
				DivideOperator divide(left_operand, right_operand);
				double actual(divide.Evaluate());

				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Assignment)
			{
				unsigned short variable_ID(33);
				SystemParameter system_parameter;
				MacroVariableInterface mi(system_parameter);

				double expected(pi);
				
				auto left_operand(CreateVariableOperator(mi, variable_ID));
				auto right_operand(CreateConstantOperator(expected));
				AssignmentOperator assignment(left_operand, right_operand);
				double actual(assignment.Evaluate());

				Assert::AreEqual(expected, actual);
			}
		};

		TEST_CLASS(RelationalOperators)
		{
		public:
			TEST_METHOD(Equal)
			{
				double left(pi);
				double right(left);
				shared_ptr<ArithmeticOperator> left_operand(new ConstantOperator(left));
				shared_ptr<ArithmeticOperator> right_operand(new ConstantOperator(right));

				EqualOperator actual(left_operand, right_operand);
				Assert::IsTrue(actual.Evaluate());
			}

			TEST_METHOD(NotEqual)
			{
				double left(pi);
				double right(e);
				shared_ptr<ArithmeticOperator> left_operand(new ConstantOperator(left));
				shared_ptr<ArithmeticOperator> right_operand(new ConstantOperator(right));

				NotEqualOperator actual(left_operand, right_operand);
				Assert::IsTrue(actual.Evaluate());
			}

			TEST_METHOD(Greater)
			{
				double left(pi);
				double right(e);
				shared_ptr<ArithmeticOperator> left_operand(new ConstantOperator(left));
				shared_ptr<ArithmeticOperator> right_operand(new ConstantOperator(right));

				GreaterOperator actual(left_operand, right_operand);
				Assert::IsTrue(actual.Evaluate());
			}

			TEST_METHOD(GreaterEqual)
			{
				double left(pi);
				double right(e);
				shared_ptr<ArithmeticOperator> left_operand(new ConstantOperator(left));
				shared_ptr<ArithmeticOperator> right_operand(new ConstantOperator(right));

				GreaterEqualOperator actual(left_operand, right_operand);
				Assert::IsTrue(actual.Evaluate());
			}

			TEST_METHOD(Less)
			{
				double left(e);
				double right(pi);
				shared_ptr<ArithmeticOperator> left_operand(new ConstantOperator(left));
				shared_ptr<ArithmeticOperator> right_operand(new ConstantOperator(right));

				LessOperator actual(left_operand, right_operand);
				Assert::IsTrue(actual.Evaluate());
			}

			TEST_METHOD(LessEqual)
			{
				double left(e);
				double right(pi);
				shared_ptr<ArithmeticOperator> left_operand(new ConstantOperator(left));
				shared_ptr<ArithmeticOperator> right_operand(new ConstantOperator(right));

				LessEqualOperator actual(left_operand, right_operand);
				Assert::IsTrue(actual.Evaluate());
			}
		};

		TEST_CLASS(LogicalOperators)
		{
		public:
			TEST_METHOD(AND)
			{
				double small(e);
				double big(pi);
				shared_ptr<RelationalOperator> left_operand(new LessOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));
				shared_ptr<RelationalOperator> right_operand(new GreaterOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(big)), shared_ptr<ArithmeticOperator>(new ConstantOperator(small))));
				unsigned expected(true);
				AND_Operator actual(left_operand, right_operand);

				Assert::AreEqual(expected, actual.Evaluate());
			}

			TEST_METHOD(OR)
			{
				double small(e);
				double big(pi);
				shared_ptr<RelationalOperator> left_operand(new LessOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));
				shared_ptr<RelationalOperator> right_operand(new GreaterOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));
				unsigned expected(true);
				OR_Operator actual(left_operand, right_operand);

				Assert::AreEqual(expected, actual.Evaluate());
			}

			TEST_METHOD(XOR)
			{
				double small(e);
				double big(e);
				shared_ptr<RelationalOperator> left_operand(new LessOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));
				shared_ptr<RelationalOperator> right_operand(new GreaterEqualOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));
				unsigned expected(true);
				OR_Operator actual(left_operand, right_operand);

				Assert::AreEqual(expected, actual.Evaluate());
			}
		};

		TEST_CLASS(ConditionalOperators)
		{
		public:
			TEST_METHOD(ConditionalArithmetic)
			{
				double small(e);
				double big(pi);
				shared_ptr<RelationalOperator> condition(new LessOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));

				SystemParameter system_parameter;
				MacroVariableInterface mi(system_parameter);
				unsigned short ID(1);
				shared_ptr<ArithmeticOperator> variable_ID(new ConstantOperator(ID));
				shared_ptr<ArithmeticOperator> variable(new VariableOperator(mi, variable_ID));
				double param(pi);
				shared_ptr<ArithmeticOperator> constant(new ConstantOperator(param));
				shared_ptr<ArithmeticOperator> arithmetic(new AssignmentOperator(variable, constant));

				double expected(param);
				ConditionalArithmeticOperator actual(condition, arithmetic);

				Assert::IsTrue(actual.Evaluate());
				Assert::AreEqual(expected, actual.ArithmeticValue());
			}

			TEST_METHOD(ConditionalBranch)
			{
				double small(e);
				double big(pi);
				shared_ptr<RelationalOperator> condition(new LessOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));

				unsigned short sequence_number(99);
				shared_ptr<ArithmeticOperator> constant(new ConstantOperator(sequence_number));
				int expected(sequence_number);
				ConditionalBranchOperator actual(condition, constant);

				Assert::IsTrue(actual.Evaluate());
				Assert::AreEqual(expected, actual.BranchNumber());
			}

			TEST_METHOD(ConditionalLoop)
			{
				double small(e);
				double big(pi);
				shared_ptr<RelationalOperator> condition(new LessOperator(shared_ptr<ArithmeticOperator>(new ConstantOperator(small)), shared_ptr<ArithmeticOperator>(new ConstantOperator(big))));

				unsigned short loop_number(1);
				shared_ptr<ArithmeticOperator> constant(new ConstantOperator(loop_number));
				unsigned short expected(loop_number);
				ConditionalLoopOperator actual(condition, constant);

				Assert::IsTrue(actual.Evaluate());
				Assert::AreEqual(expected, actual.LoopNumber());
			}

			TEST_METHOD(LoopEnd)
			{

				unsigned short loop_number(3);
				shared_ptr<ArithmeticOperator> constant(new ConstantOperator(loop_number));
				unsigned short expected(loop_number);
				LoopEndOperator actual(constant);

				Assert::AreEqual(expected, actual.Evaluate());
			}
		};
	}
	
	namespace MacroExpressions {
		TEST_CLASS(UnaryArithmeticOperators)
		{
		public:

			TEST_METHOD(Constant)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("-3.14");

				double expected(-3.14);
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Minus)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("-#1");

				double expected(-NULL_FLOAT_VALUE);
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Variable)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("#33");

				double expected(NULL_VARIABLE);
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Sine)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("SIN[30]");

				double expected(sin(ConvertAngleToRadians(30.0)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Cosine)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("COS[30]");

				double expected(cos(ConvertAngleToRadians(30.0)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Tangent)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("TAN[30]");

				double expected(tan(ConvertAngleToRadians(30.0)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcSine)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("ASIN[0.5]");

				double expected(ConvertRadiansToAngle(asin(0.5)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcCosine)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("ACOS[0.5]");

				double expected(ConvertRadiansToAngle(acos(0.5)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(ArcTangent)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("ATAN[0.5]");

				double expected(ConvertRadiansToAngle(atan(0.5)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(SquareRoot)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("SQRT[3]");

				double expected(sqrt(3.0));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(AbsoluteValue)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("ABS[-3]");

				double expected(fabs(-3.0));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(RoundOff)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("ROUND[6.5]");

				double expected(round(6.5));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(RoundDown)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("FIX[7.9]");

				double expected(floor(7.9));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(RoundUp)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("FUP[7.4]");

				double expected(ceil(7.4));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(NaturalLog)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("LN[3]");

				double expected(log(3));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}

			TEST_METHOD(Exponent)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("EXP[1]");

				double expected(exp(1.0));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
		};

		TEST_CLASS(BinaryArithmeticOperators)
		{
		public:
			TEST_METHOD(Power)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("POW[2,8]");

				double expected(pow(2.0, 8.0));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
			
			TEST_METHOD(ArcTangent2)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				string block("ATAN[1.0,1.0]");

				double expected(ConvertRadiansToAngle(atan2(1.0,1.0)));
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
			
			TEST_METHOD(Add)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				double left(3.0), right(5.0);
				double expected(left + right);
				string block("3.0+5.0");
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
			
			TEST_METHOD(Subtract)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				double left(3.0), right(5.0);
				double expected(left - right);
				string block("3.0-5.0");
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
			
			TEST_METHOD(Multiply)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				double left(3.0), right(5.0);
				double expected(left * right);
				string block("3.0*5.0");
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
			
			TEST_METHOD(Divide)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				double left(3.0), right(5.0);
				double expected(left / right);
				string block("3.0/5.0");
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
			
			TEST_METHOD(Assignment)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				double left(3.0);
				double right(5.0);
				double expected(right);
				string block("#1=5.0");
				double actual(EvaluateArithmeticMacroExpression(macro_variable_interface, block));
				Assert::AreEqual(expected, actual);
			}
		};
		
		TEST_CLASS(RelationalOperators)
		{
		public:
			TEST_METHOD(Equal)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[#1 EQ #2]");
				Assert::IsTrue(EvaluateRelationalMacroExpression(macro_variable_interface, block));
			}
			
			TEST_METHOD(NotEqual)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[#1 NE 0]");
				Assert::IsTrue(EvaluateRelationalMacroExpression(macro_variable_interface, block));
			}
			
			TEST_METHOD(Greater)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[#1 GT 0]");
				Assert::IsTrue(EvaluateRelationalMacroExpression(macro_variable_interface, block));
			}
			
			TEST_METHOD(GreaterEqual)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[#1 GE #2]");
				Assert::IsTrue(EvaluateRelationalMacroExpression(macro_variable_interface, block));
			}
			
			TEST_METHOD(Less)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				double value(1.0);
				macro_variable_interface.WriteVariable(1, value);
				value = 2.0;
				macro_variable_interface.WriteVariable(2, value);
				string block("[#1 LT #2]");
				Assert::IsTrue(EvaluateRelationalMacroExpression(macro_variable_interface, block));
			}
			
			TEST_METHOD(LessEqual)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[#1 LE #2]");
				Assert::IsTrue(EvaluateRelationalMacroExpression(macro_variable_interface, block));
			}
		};

		TEST_CLASS(LogicalOperators)
		{
		public:
			TEST_METHOD(AND)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[1 AND 1]");
				unsigned expected(1);
				Assert::AreEqual(expected,EvaluateLogicalMacroExpression(macro_variable_interface, block));
			}

			TEST_METHOD(OR)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[1 OR 0]");
				unsigned expected(1);
				Assert::AreEqual(expected, EvaluateLogicalMacroExpression(macro_variable_interface, block));
			}

			TEST_METHOD(XOR)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);

				string block("[1 XOR 0]");
				unsigned expected(1);
				Assert::AreEqual(expected, EvaluateLogicalMacroExpression(macro_variable_interface, block));
			}
		};
		
		TEST_CLASS(ConditionalOperators)
		{
		public:
			TEST_METHOD(ConditionalArithmetic)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				FanucMacroParser parser(macro_variable_interface);

				string block("IF[#1 EQ #2] THEN #1=1");
				double expected(1.0);

				Assert::AreEqual(CommandType::MACRO_COMMAND, parser.ParseBlock(block));
				Assert::IsFalse(parser.macro_generator.conditional_arithmetic_operator.Empty());
				ConditionalArithmeticOperator macro_operator(parser.macro_generator.conditional_arithmetic_operator);
				
				Assert::IsTrue(macro_operator.Evaluate());
				Assert::AreEqual(expected, macro_operator.ArithmeticValue());
			}
			
			TEST_METHOD(ConditionalBranch)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				FanucMacroParser parser(macro_variable_interface);
				
				string block("IF[#1 EQ #2] GOTO 13");
				int expected(13);

				Assert::AreEqual(CommandType::MACRO_COMMAND, parser.ParseBlock(block));
				Assert::IsFalse(parser.macro_generator.conditional_branch_operator.Empty());
				ConditionalBranchOperator macro_operator(parser.macro_generator.conditional_branch_operator);

				Assert::IsTrue(macro_operator.Evaluate());
				Assert::AreEqual(expected, macro_operator.BranchNumber());
			}
			
			TEST_METHOD(ConditionalLoop)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				FanucMacroParser parser(macro_variable_interface);
				
				string block("WHILE[#1 EQ #2] DO 3");
				unsigned short expected(3);

				Assert::AreEqual(CommandType::MACRO_COMMAND, parser.ParseBlock(block));
				Assert::IsFalse(parser.macro_generator.conditional_loop_operator.Empty());
				ConditionalLoopOperator macro_operator(parser.macro_generator.conditional_loop_operator);

				Assert::IsTrue(macro_operator.Evaluate());
				Assert::AreEqual(expected, macro_operator.LoopNumber());
			}

			TEST_METHOD(LoopEnd)
			{
				SystemParameter system_parameter;
				MacroVariableInterface macro_variable_interface(system_parameter);
				FanucMacroParser parser(macro_variable_interface);
				
				string block("END 3");
				unsigned short expected(3);

				Assert::AreEqual(CommandType::MACRO_COMMAND, parser.ParseBlock(block));
				Assert::IsFalse(parser.macro_generator.loop_end_operator.Empty());
				LoopEndOperator macro_operator(parser.macro_generator.loop_end_operator);

				Assert::AreEqual(expected, macro_operator.Evaluate());
			}
		};
	}
}
