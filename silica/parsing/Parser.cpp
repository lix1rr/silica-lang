#include "parsing/Parser.h"
#include <functional>
using namespace Silica;

void Parser::parse() {
	getToken();
	int oldByte = byte + 1;
	while (true) {
		switch (token) {
		case Token::keyword_func:
			handleFuncDecl();
			getToken();
			break;
		case Token::keyword_extern:
			handleExtern();
			getToken();
			break;
		case Token::eof:
			return;
		case Token::newline:
			getToken();
			continue;
		case Token::identifier:
			err("Invalid identifier \"" + token_string + "\" at the start of a top level statement", oldByte);
			getToken();
			continue;
		default:
			err("Invalid token (" + descibeToken(token) + ") in a top level statement", byte);
			getToken();
			continue;
		}
	}
}
//  Return is not nullptr
std::unique_ptr<Expression> Parser::handleBlock() {
	Block block;
	if (!next()) {
		return std::make_unique<Block>(std::move(block));
	}
	tabs++;
	int oldByte = byte + 1;

	#define discardLine()												   \
		while (token != Token::newline && token != Token::eof) {		   \
			getToken(); 												   \
		};

	while (true) {

		// Expects 'tabs' number of tabs
		int tabsFound = 0;
		while (current == '\t' && next()) {
			tabsFound++;
		}
		if (current == '\n') {
			if (!next()) {
				return std::make_unique<Block>(std::move(block));
			};
			continue;
		}
		if (tabsFound > tabs) {
			err("Too much indentation, expected " + std::to_string(tabs) + ", got " + std::to_string(tabs));
		}
		else if (tabsFound < tabs) {
			tabs = tabsFound;
			return std::make_unique<Block>(std::move(block));
		}
		getToken();

		switch (token) {
		case Token::number:
			err("Unexpected number \'" + std::to_string(token_number) + "\' at statement beginning", oldByte);
			discardLine();
			break;
		case Token::keyword_func:
			//TODO: closure :0
			err("Closures not implemented", oldByte);
			discardLine();
			break;
		case Token::keyword_extern:
			err("The extern keyword is invalid here", oldByte);
			note("To use extern, do it in a top level statment", oldByte);
			discardLine();
			break;
		case Token::keyword_let: {
			block.expressions.push_back(handleLet());
			break;
		}
		case Token::keyword_return:
			getToken();
			block.expressions.push_back(std::make_unique<Return>(expectExpression()));
			break;
		case Token::eof:
			return std::make_unique<Block>(std::move(block));
		case Token::newline:
			getToken();
			continue;
		default: {
			std::unique_ptr<Expression> expr = expectExpression();
			if (expr == nullptr) {
				err("Unexpected " + descibeToken(token));
				discardLine();
				continue;
			} else if (expr->useful == false) {
				err("An expression with no effects is not allowed as a statement");
				discardLine();
				continue;
			}
			block.expressions.push_back(std::move(expr));
		}
			
		}
		if (token == Token::eof) {
			return std::make_unique<Block>(std::move(block));
		}
		else if (token != Token::newline) {
			err("Expected newline after statement");
			discardLine();
		}
		if (tabs < tabsFound) {
			// A block inside this one had less indentation than this
			return std::make_unique<Block>(std::move(block));
		}
	}
}
// When token is Token::open_bracket, advances token to after the ')'
std::unique_ptr<Expression> Parser::handleFuncCall(std::string name) {
	getToken();
	std::unique_ptr<Expression> expr = expectExpression(true);
	if (expr == nullptr) {
		getToken();
		return std::unique_ptr<CallFuncExpr>(new CallFuncExpr(name, {}));
	} 
	
	std::vector<std::unique_ptr<Expression>> args;
	args.push_back(std::move(expr));
	while (true) {
		if (token == Token::comma) {
			getToken();
			std::unique_ptr<Expression> expr = expectExpression(true);
			if (expr == nullptr) {
				err("Invalid expression after comma");
				return nullptr;
			}
			args.push_back(std::move(expr));
		}
		else if (token == Token::closed_bracket) {
			getToken();
			return std::make_unique<CallFuncExpr>(name, std::move(args));
		}
		else {
			err("Unexpected token " + std::string(1, current) + " in function call");
			return nullptr;
		}
		
	}
}

//                      V-Func name
std::optional<std::pair<std::string, Extern>> Parser::parseFuncSignature() {
	getToken();
	if (token != Token::identifier) {
		err("Expected identifier in expected function declaration");
		return std::nullopt;
	}
	std::string name = token_string;
	getToken();
	if (token != Token::open_bracket) {
		err("Expected \'(\' after expected function declaration");
		return std::nullopt;
	}
	Extern signature;
	
	getToken();
	if (token == Token::closed_bracket) {
		return {{name, std::move(signature)}};
	} else if (token == Token::comma) {
		signature.args.push_back({ token_string, (const Type*) &Types::Float64});
		getToken();
		if (token != Token::identifier) {
			err("Expected identifier after comma");
			return std::nullopt;
		}
	}
	else if (token != Token::identifier) {
		err("Expected argument name in function declaration");
		return std::nullopt;
	}
	signature.args.push_back({ token_string, (const Type*) &Types::Float64 });
	while (true) {
		getToken();
		if (token == Token::closed_bracket) {
			return {{std::move(name), std::move(signature) }};
		}
		else if (token == Token::comma) {
			getToken();
			if (token != Token::identifier) {
				err("Expected identifier after comma");
				return std::nullopt;
			}
			signature.args.push_back({ token_string, (const Type*) &Types::Float64 });
		}
	}
}

void Parser::handleExtern() {
	auto funcDecl = parseFuncSignature();
	if (!funcDecl.has_value()) {
		err("Unable to extern an invalid function declaration");
		return;
	}
	getToken();
	if (token != Token::newline && token != Token::eof) {
		err("Expected a newline or the end of file after extern");
		return;
	}
	ast.externs.emplace(funcDecl->first, std::move(funcDecl->second));
	return;
}

// When token == Token::identifier. Gets next token
std::unique_ptr<Expression> Parser::handleIdentifier() {
	std::string name = token_string;
	getToken();
	if (token == Token::asign) {
		std::unique_ptr<Expression> result = expectExpression();
		if (result == nullptr) {
			getToken();
			err("Cannot asign variable " + name + " to invalid expression");
			return nullptr;
		}
		else {
			getToken();
			return std::make_unique<SetVarExpr>(name, std::move(result));
		}
	}
	else if (token == Token::open_bracket) {
		return handleFuncCall(name);
	}
	else {
		return std::make_unique<GetVarExpr>(name);
	}
}

void Parser::handleFuncDecl() {
	auto funcDecl = parseFuncSignature();
	if (!funcDecl.has_value()) {
		err("Unable to make a function from an invalid function declaration");
		return;
	}
	if (ast.externs.find(funcDecl->first) != ast.externs.end()){
		err("Cannot create function " + funcDecl->first +", as it was already externed");
		return;
	}

	if (ast.functions.find(funcDecl->first) != ast.functions.end()) {
		err("Cannot create function " + funcDecl->first + ", as it was already declared");
		return;
	}
	getToken();
	Function f { funcDecl->second.args, funcDecl->second.returnType, expectExpression() };
	// the function that was moved into the map
	Function& func = ast.functions.emplace(funcDecl->first, std::move(f)).first->second;
	if (func.result == nullptr) {
		err("Expected an expression after function declaration");
	}
	//ast.functions.emplace(funcDecl)??;
	
	
}

// Gets the next primary expression (num/var/call/brackets) stored in token
// Advances token to the next one
std::unique_ptr<Expression> Parser::parseSingleExpr() {
	if (isUnaryOp(token)) {
		Token op = token;
		getToken();
		auto expr = parseSingleExpr();
		if (expr == nullptr) {
			err("Expected expression after unary operator (" + descibeToken(token) + ")");
			return nullptr;
		}
		return std::make_unique<UnaryOpExpr>(std::move(expr), UnaryOpType(op));
	}

	switch (token) {
	case Token::open_bracket: {
		getToken();
		std::unique_ptr<Expression> expr = expectExpression();
		if (token == Token::closed_bracket) {
			if (expr == nullptr) {
				err("Empty brackets in expression");
				return nullptr;
			}
			return expr;
		}
		err("Expected a closing brace after bracketed expression");
		// Lets just carry on with the switch and pretend nothing happened
	}
	case Token::colon:
		// Handle block
		return handleBlock();
		
		break;
	case Token::number:
		getToken();
		return std::make_unique<NumLitExpr>(token_number);
	case Token::identifier: {
		std::string identifier = std::move(token_string);
		getToken();
		if (token == Token::open_bracket) {
			return handleFuncCall(std::move(identifier));
		}
		else {
			return std::make_unique<GetVarExpr>(std::move(identifier));
		}
		break;
	}
		

	default:
		// When it reaches the end of an expression (this is not an error)
		return nullptr;
	}
}

std::unique_ptr<Expression> Parser::expectExpression(bool inBrackets /* = false */) {
	std::unique_ptr<Expression> primary = parseSingleExpr();
	if (primary == nullptr) {
		return nullptr;
	}
	return parseRhs(1, std::move(primary));
}

// TODO:complete
std::unique_ptr<Expression> Parser::parseRhs(int precedence, std::unique_ptr<Expression> lhs) {
	while (true) {
		Token op = token;
		int tokenPrecedence = getTokenPrecedence(token);
		if (tokenPrecedence < precedence) {
			
			return lhs;
		}

		getToken();
		std::unique_ptr<Expression> rhs = parseSingleExpr();
		if (rhs == nullptr) {
			err("Expected an expression after binary operator");
			return lhs;
		}

		Token nextOp = token;
		int nextPrecedence = getTokenPrecedence(token);
		if (nextPrecedence > tokenPrecedence) {
			rhs = parseRhs(tokenPrecedence + 1, std::move(rhs));
		}
		lhs = std::make_unique<BinOpExpr>(std::move(lhs), std::move(rhs), BinOpType(op));
	}
}

std::unique_ptr<Let> Parser::handleLet() {
	expect(Token::identifier, "Expected identifier after let statement");
	std::string name = token_string;
	getToken();
	if (token == Token::identifier) {
		myAssert(false, "Types not implemented yet, default is double");
	}
	else if (token == Token::asign) {
		getToken();
		std::unique_ptr<Expression> expr = expectExpression();
		if (expr == nullptr) {
			err("Expected expression after let");
			return nullptr;
		}
		return std::make_unique<Let>(name, std::move(expr));
	}
	err("Unexpected token after let");
	return nullptr;
}