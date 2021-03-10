#pragma once
#include "include.h"
#include "types.h"
#include "parsing/tokens.h"
#include <variant>
#include <vector>
#include <unordered_map>

namespace Silica {

struct Position {const int line = -1; const int byte = -1;};

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
	temp     // anything else; result of function call, literals, computed variables and products thereof
};

struct Expression: public Node {
	ValueType valueType;
	const Type* type = nullptr;
	bool useful = true;
	Expression() {};
	Expression(ValueType valueType, const Type* type, bool useful):
		valueType(valueType), type(type), useful(useful) {};
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
	std::vector<std::string> lets;
	std::vector<std::string> vars;
	std::vector<std::unique_ptr<Expression>> expressions;

	void print(std::ostream& stream, int tabs);
};
constexpr auto my_size = sizeof(Expression);

struct Ast {
	int errorCount = 0;
	std::string errors;
	std::unordered_map<std::string, Extern> externs;
	std::unordered_map<std::string, Function> functions;
	Block* currentBlock = nullptr;;

	Ast() {};
	void print(std::ostream& out);
};




struct NumLitExpr : public Expression {
	double value;
	NumLitExpr(double value) : value(value) {};
	void print(std::ostream& stream, int tabs) override;
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

	BinOpExpr(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, BinOpType operation) : left(std::move(left)),right(std::move(right)), operation(operation) {};

	void print(std::ostream& stream, int tabs) override;
};

enum class UnaryOpType {
	minus = (int) Token::minus
};

struct UnaryOpExpr : public Expression {
	std::unique_ptr<Expression> expr;
	UnaryOpType operation;

	UnaryOpExpr(std::unique_ptr<Expression> expr, UnaryOpType operation):
		expr(std::move(expr)), operation(operation) {};
	void print(std::ostream& stream, int tabs) override;
};

struct ExternNode {
	bool returnsDouble;
	std::vector<std::string> args;
	std::string name;
	ExternNode(bool returnsDouble, std::vector<std::string> args, std::string name ):returnsDouble(returnsDouble),args(args),name(name) {};
};

struct GetVarExpr : public Expression {
	std::string var;
	GetVarExpr(std::string var): var(var){}
	void print(std::ostream& stream, int tabs) override;
};

struct DeclareVarExpr : public Expression {
	std::string name;
	std::unique_ptr<Expression> value;
	DeclareVarExpr(std::string name, std::unique_ptr<Expression> value) :name(name), value(std::move(value)) {};
	void print(std::ostream& stream, int tabs) override;
};

struct SetVarExpr : public Expression {
	std::string name;
	std::unique_ptr<Expression> value;
	SetVarExpr(std::string name, std::unique_ptr<Expression> value):name(name), value(std::move(value)) {};
	void print(std::ostream& stream, int tabs) override;
};

struct CallFuncExpr : public Expression {
	std::string name;
	std::vector<std::unique_ptr<Expression>> args;
	
	CallFuncExpr(std::string name, std::vector<std::unique_ptr<Expression>> args):name(name),args(std::move(args)){};
	void print(std::ostream& stream, int tabs) override;
};

struct Return: public Expression {
	std::unique_ptr<Expression> value;
	Return(std::unique_ptr<Expression> value): Expression(ValueType::temp, &Types::Void, true), value(std::move(value)) {}

	void print(std::ostream& stream, int tabs) override;
};

struct Let: Expression {
	std::string name;
	std::unique_ptr<Expression> value;
	Let(std::string name, std::unique_ptr<Expression> value): Expression(ValueType::temp, &Types::Void, true), name(name), value(std::move(value)) {
		useful = true;
	};
	void print(std::ostream& stream, int tabs) override;
};

} // End namespace Silica