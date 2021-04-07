#pragma once
#include "include.h"
#include "types.h"
#include "parsing/tokens.h"
#include "compiling/compiler.h"
extern "C" {
	#include "xed/xed-interface.h"
}
#include <iostream>
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace Silica {

struct Tabs {
	int count;
	Tabs(int count): count(count) {};
};
inline std::ostream& operator<<(std::ostream& lhs, Tabs rhs) {
	while (rhs.count-- != 0) {
		lhs << "  ";
	}
	return lhs;
}
struct Ast;
struct Scope;
struct Block;
struct Node;
	struct Function;
	struct Expression;
		struct NumLitExpr;
		struct BinOpExpr;

struct Node {
	//Position position;
	virtual ~Node() {}
	virtual void print(std::ostream& stream, int tabs);
};

enum class ValueType {
	ref,     // let, const
	mutRef,  // var
	temp     // anything else; result of function call, literals and products thereof
};

struct ValExpr;
struct MutExpr;

struct ExprBase: public Node {
	const Type* type;
	bool useful;
	friend ValExpr;
	friend MutExpr;
private:
	ExprBase() {};
};

// A value-expression references:
//  - An immediate operand
//  - The value at an address (with displacement, scale etc.)
//  - A general purpose (64b) register
// ...and can't be changed
struct ValExpr : public ExprBase {
	virtual xed_operand_t eval(Ast& ast) = 0;
};

// A mutable-expression references:
//  - The value at an address (with displacement, scale etc.)
//  - A general purpose (64b) register
// ...and can be changed---- 
struct MutExpr : public ExprBase {
	virtual xed_operand_t eval(Ast& ast) = 0;
};

struct Expression: public Node {
	ValueType valueType;
	const Type* type = nullptr;
	bool useful = true;
	Expression() {};
	Expression(ValueType valueType, const Type* type, bool useful):
		valueType(valueType), type(type), useful(useful) {};
};

struct DeclareVar: public Node {
	std::string name;
	const Type* type;
	bool canBeChanged;
	bool initialized = false; // TODO
	xed_operand_t location;

	DeclareVar(std::string name, const Type* type, bool canBeChanged) :
		name(name), type(type), canBeChanged(canBeChanged) {};
	void print(std::ostream& stream, int tabs) override;
};

struct Function: public Node {
	std::vector<std::pair<std::string, const Type*>> args;
	const Type* returnType;
	std::unique_ptr<Expression> result;
	Function(std::vector<std::pair<std::string, const Type*>> args, const Type* returnType, std::unique_ptr<Expression> result) :
		args(std::move(args)), returnType(returnType), result(std::move(result)) {};
	void print(std::ostream& stream, int tabs) override;
};

struct Extern: public Node {
	std::vector<std::pair<std::string, const Type*>> args;
	const Type* returnType;
	void print(std::ostream& stream, int tabs) override;
};

struct Block: public Expression {
	Block* parent = nullptr;
	std::unordered_map<std::string, double> consts;
	std::vector<DeclareVar> variables;
	std::vector<std::unique_ptr<Expression>> expressions;
	void print(std::ostream& stream, int tabs);
};

struct Ast {
	int errorCount = 0;
	std::string errors;
	std::unordered_map<std::string, Extern> externs;
	std::unordered_map<std::string, Function> functions;
	Block* currentBlock = nullptr;
	int currentStackDepth = 0;
	xed_reg_enum_t currentReg = (xed_reg_enum_t) (int(XED_REG_RAX) - 1);
	xed_reg_enum_t getReg() {
		currentReg = (xed_reg_enum_t) (int(currentReg) + 1);
		myAssert(currentReg <= XED_REG_R15, "Out of registers :(");
		return currentReg;
	}
	xed_reg_enum_t returnReg() {
		currentReg = (xed_reg_enum_t)(int(currentReg) - 1);
		myAssert(currentReg >= (xed_reg_enum_t) (int(XED_REG_RAX) - 1));
		return currentReg;
	}

	Ast() {};
	void print(std::ostream& out);
};

struct NumLitExpr : public Expression {
	double value;
	void print(std::ostream& stream, int tabs) override;
	NumLitExpr(double value): Expression(ValueType::temp,  &Types::Float64, false), value(value) {}
};

enum class BinOpType {
	plus = (int)Token::plus,
	minus = (int)Token::minus,
	divide = (int)Token::divide,
	multiply = (int)Token::multiply
};

struct BinOpExpr : public Expression {
	std::unique_ptr<Expression> left;
	std::unique_ptr<Expression> right;
	BinOpType operation;

	BinOpExpr(std::unique_ptr<Expression> newLeft, std::unique_ptr<Expression> newRight, BinOpType operation):
		Expression(ValueType::temp, newLeft->type, false), left(std::move(newLeft)),right(std::move(newRight)), operation(operation) {
		myAssert(left->type == right->type);
	};

	void print(std::ostream& stream, int tabs) override;
};

enum class UnaryOpType {
	minus = (int) Token::minus
};

struct UnaryOpExpr : public Expression {
	std::unique_ptr<Expression> expr;
	UnaryOpType operation;

	UnaryOpExpr(std::unique_ptr<Expression> expr, UnaryOpType operation):
		Expression(ValueType::temp, expr->type, false), expr(std::move(expr)), operation(operation) {};
	void print(std::ostream& stream, int tabs) override;
};

struct ExternNode {
	bool returnsDouble;
	std::vector<std::string> args;
	std::string name;
	ExternNode(bool returnsDouble, std::vector<std::string> args, std::string name ):
		returnsDouble(returnsDouble),args(args),name(name) {};
};

struct GetVarExpr: public Expression {
	DeclareVar& decl;
	GetVarExpr(DeclareVar& decl):
		decl(decl), Expression(ValueType::mutRef, decl.type, false) {
	}
	void print(std::ostream& stream, int tabs) override;
};

struct SetVarExpr : public Expression {
	std::string& name;
	std::unique_ptr<Expression> value;
	SetVarExpr(DeclareVar& decl, std::unique_ptr<Expression> value_):
		Expression(ValueType::temp, &Types::Void, true), name(decl.name), value(std::move(value_)) {
		myAssert(value->type == decl.type);
	};
	void print(std::ostream& stream, int tabs) override;
};

struct CallFuncExpr : public Expression {
	std::string name;
	std::vector<std::unique_ptr<Expression>> args;
	
	CallFuncExpr(Function& func, std::vector<std::unique_ptr<Expression>> args):
		Expression(ValueType::temp, func.returnType, true), name(name), args(std::move(args)){};
	void print(std::ostream& stream, int tabs) override;
};

struct Return: public Expression {
	std::unique_ptr<Expression> value;
	Return(std::unique_ptr<Expression> value):
		Expression(ValueType::temp, &Types::Void, true), value(std::move(value)) {}

	void print(std::ostream& stream, int tabs) override;
};

struct Let: public Expression {
	std::string name;
	std::unique_ptr<Expression> value;
	Let(std::string name, std::unique_ptr<Expression> value):
		Expression(ValueType::temp, &Types::Void, true), name(name), value(std::move(value)) {};
	void print(std::ostream& stream, int tabs) override;
};

template<size_t size>
struct Operation: public ValExpr {
	std::array<ExprBase, size> operands;
	Token token;
	xed_operand_t eval(Ast& ast) override {

	}
};

struct IfExpr : Expression {
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Expression> ifTrue;
	std::unique_ptr<Expression> ifFalse;
	IfExpr(std::unique_ptr<Expression> condition, 
		std::unique_ptr<Expression> ifTrue_, std::unique_ptr<Expression> ifFalse_):

		ifTrue(std::move(ifTrue_)), ifFalse(std::move(ifFalse_)), condition(std::move(condition)),
		Expression(ValueType::temp, ifTrue->type, true)
	{
		myAssert(ifTrue != nullptr);
		myAssert(ifFalse == nullptr || (ifFalse->valueType == ifTrue->valueType && ifFalse->type == ifTrue->type));
	};
};

} // End namespace Silica