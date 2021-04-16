#pragma once
#include "include.h"
#include "tokens.h"
#include "ast/ast.h"
#include <deque>

namespace Silica {
	struct View {
		enum class Type {
			note, help, error
		} type;
		const std::string& sourceLine;
		std::string msg;
		int line;
		size_t byte;

		View(const std::string& sourceLine, std::string msg, Type type, int line, size_t byte):
			sourceLine(sourceLine), msg(std::move(msg)), type(type), line(line), byte(byte) {
		};

		void print(std::ostream& os) {
			std::string typeString;
			switch (type) {
			case Type::note:
				typeString = "note";
				break;
			case Type::error:
				typeString = "error";
				break;
			case Type::help:
				typeString = "help";
				break;
			default:
				throw "invalid enum??";
			}
			os << typeString << ": " << msg << "\n\n " << std::to_string(line) << " | " << sourceLine << "\n    ";
			for (int i = 0; i < byte - 1; i++) {
				os << ' ';
			}
			os << "^\n\n";
		}
	};

	class Parser {
	public:
		int errorCount = 0;
		std::string errors;
		Ast ast;
		std::vector<std::unique_ptr<Type>> types;
		Parser(std::istream& stream, std::string_view moduleName, std::string_view sourceName):
			stream(stream) {
			next();
			parse();
		}
		void printErrors(std::ostream& os) {
			for (auto& view : views) {
				if (view.type == View::Type::error) {
					os << "\n";
				}
				view.print(os);
			}
		}
	private:
		// TODO: optimize with buffers
		std::istream& stream;
		std::deque<std::string> referredLines {""};
		bool sourceLineReferred = false;
		// notes/help/errors etc.
		std::vector<View> views;
		Token token;

		double token_number = -3.49;
		std::string token_string;
		bool allowTabs = true;
		bool skipNextGetToken = false;

		char current = 0;
		int line = 1;
		int byte = 0;

		bool next();
		void nextToken(bool inclNewline);
		void getToken(bool inclNewline = true);
		// WS is Token::newline


		void err(std::string msg, int fmtByte);
		void note(std::string msg, int fmtByte);
		void err(std::string msg) {
			err(std::move(msg), byte);
		}
		void note(std::string msg) {
			note(std::move(msg), byte);
		}

		const Type* handleType();
		template<typename ArgType, typename ArgGetter>
		std::optional<std::vector<std::pair<std::string, ArgType>>> Parser::parseArgList(
			std::string_view listDesc, std::string_view argDesc, ArgGetter argGetter);
		std::unique_ptr<Expression> parseSingleExpr();
		std::unique_ptr<Expression> handleFuncCall(std::string name);
		std::optional<std::pair<std::string, Extern>> parseFuncSignature();
		void handleExtern();
		DeclareVar* handleLet();
		std::unique_ptr<Expression> handleIdentifier();
		void handleFuncDecl();
		std::unique_ptr<Expression> parseRhs(int precedence, std::unique_ptr<Expression> lhs);
		std::unique_ptr<Expression> expectExpression(bool inBrackets = false);

		std::unique_ptr<Block> handleBlock();
		void parse();
	};
	#define expect(expectedToken, msg) getToken();if (token != expectedToken) {err(msg); return nullptr;}



	template<typename ArgType, typename ArgGetter>
	std::optional<std::vector<std::pair<std::string, ArgType>>> Parser::parseArgList(
		std::string_view listDesc, std::string_view argDesc, ArgGetter argGetter)
	{
		std::vector<std::pair<std::string, ArgType>> result;
		std::pair<std::string, ArgType> arg;
		getToken();
		if (token == Token::closedBracket) {
			goto end;
		}
	begin:
		if (token != Token::identifier) {
			err(std::string("Expected an argument name in ") + listDesc);
			return std::nullopt;
		}
		arg.first = std::move(token_string);
		getToken();
		if (token != Token::colon) {
			err("Expected a ':' after the argument name");
			return std::nullopt;
		}
		getToken();
		arg.second = argGetter();
		if (arg.second == nullptr) {
			err(std::string("Expected ") + argDesc);
			return std::nullopt;
		}
		signature.args.emplace_back(std::move(arg));
		getToken();

		if (token == Token::comma) {
			getToken();
			goto begin;
		}
		if (token == Token::closedBracket) {
			goto end;
		}
		else {
			err(std::string("Expected a comma or a closed bracket after argument"));
			failed();
			return std::nullopt;
		}
	end:
		return { result };
	}
}

