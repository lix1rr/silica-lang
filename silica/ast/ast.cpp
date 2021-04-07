#include "ast/ast.h"
#include <iostream>

using namespace Silica;


namespace Silica {

void Ast::print(std::ostream& out) {
	out << "Functions:\n";
	for (auto& i : functions) {
		out << i.first << ":\n";
		i.second.print(out, 1);
	}

	out << "Externs:\n";
	for (auto& i : externs) {
		out << i.first << ":\n";
		i.second.print(out, 1);
	}
	out << '\n';
}

void Function::print(std::ostream& out, int tabs) {
	out << Tabs(tabs) << "Arguments:\n";
	for (auto& i : args) {
		out << Tabs(tabs + 1) << i.first << " : " << i.second->name << '\n';
	}
	out << Tabs(tabs) << "Result:\n";
	if (result == nullptr) {
		out << "nil";
	} else {
		result->print(out, tabs + 2);
	}
}

void Extern::print(std::ostream& out, int tabs) {
	out << Tabs(tabs) << "Arguments:\n";
	for (auto& i : args) {
		out << Tabs(tabs + 1) << i.first << " : " << i.second->name << '\n';
	}
}

void Block::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "Block:\n";
	for (auto& expression : expressions) {
		expression->print(stream, tabs + 1);
	}
}


void Node::print(std::ostream& stream, int tabs) {
	stream << Tabs(tabs) << "Node\n";
}

void NumLitExpr::print(std::ostream& stream, int tabs) {
	stream << Tabs(tabs) << "NumLitExpr: " << value << '\n';
}

void BinOpExpr::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "BinOpExpr: \n"
		<< Tabs(tabs + 1)
		<< "operation: " << descibeToken(Token(operation))
		<< '\n'
		<< Tabs(tabs + 1)
		<< "lhs: \n";
	left->print(stream, tabs + 2);
	stream
		<< Tabs(tabs + 1)
		<< "rhs: \n";
	right->print(stream, tabs + 2);
	stream << '\n';
}


void UnaryOpExpr::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "UnaryOpExpr: \n"
		<< Tabs(tabs + 1)
		<< "operation: " << descibeToken(Token(operation))
		<< '\n'
		<< Tabs(tabs + 1)
		<< "value: \n";
	expr->print(stream, tabs + 2);
	stream << '\n';
}


void DeclareVar::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "DeclareVar:\n"
		<< Tabs(tabs + 1)
		<< "name:" << name << '\n'
		<< Tabs(tabs + 1);
}

void SetVarExpr::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "SetVarExpr:\n"
		<< Tabs(tabs + 1)
		<< "name:" << name << '\n'
		<< Tabs(tabs + 1)
		<< "value:\n";
	value->print(stream, tabs + 2);
}

void CallFuncExpr::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "CallFuncExpr: \n"
		<< Tabs(tabs + 1)
		<< "name: " << name << '\n';
	for (size_t i = 0; i < args.size(); i++) {
		stream
			<< Tabs(tabs + 1)
			<< "args[" << i << "]: \n";
		args[i]->print(stream, tabs + 2);
	}
}

void Return::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "Return:\n"
		<< Tabs(tabs)
		<< "value:\n";
	value->print(stream, tabs + 1);
}

void Let::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "Let:\n"
		<< Tabs(tabs + 1)
		<< "name:" << name << '\n'
		<< Tabs(tabs + 1)
		<< "value:\n";
	value->print(stream, tabs + 2);
}

void GetVarExpr::print(std::ostream& stream, int tabs) {
	stream
		<< Tabs(tabs)
		<< "GetVarExpr: \n"
		<< Tabs(tabs + 1)
		<< "variable: ";
	decl.print(stream, tabs + 2);
}


};