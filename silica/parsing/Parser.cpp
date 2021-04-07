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
std::unique_ptr<Block> Parser::handleBlock() {
	Block block;
	auto discardLine = [&]() {
		while (token != Token::newline && token != Token::eof) {
			getToken();
		};
	};
	while (true) {
		getToken();
		switch (token) {
		case Token::number:
			err("Unexpected number \'" + std::to_string(token_number) + "\' at statement beginning");
			discardLine();
			break;
		case Token::keyword_func:
			//TODO: closure :0
			err("Closures not implemented");
			discardLine();
			break;
		case Token::keyword_extern:
			err("The extern keyword is invalid here");
			note("To use extern, do it in a top level statment");
			discardLine();
			break;
		case Token::keyword_let:
			handleLet();
			break;
		case Token::keyword_return:
			getToken();
			block.expressions.push_back(std::make_unique<Return>(expectExpression()));
			break;
		case Token::eof:
			return std::make_unique<Block>(std::move(block));
		case Token::newline:
			getToken();
			continue;
		case Token::closedCurly:
			getToken();
			return std::make_unique<Block>(std::move(block));
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
	}
}
// When token is Token::openBracket, advances token to after the ')'
std::unique_ptr<Expression> Parser::handleFuncCall(std::string name) {
	// Todo: broke
	return nullptr;
	#if false
	getToken();
	std::unique_ptr<Expression> expr = expectExpression(true);
	if (expr == nullptr) {
		getToken();
		auto it = ast.functions.find(name);
		if (it == nullptr) {
			err()
		}
		
		return std::make_unique<CallFuncExpr>(0);
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
		else if (token == Token::closedBracket) {
			getToken();
			return std::make_unique<CallFuncExpr>(name, std::move(args));
		}
		else {
			err("Unexpected token " + std::string(1, current) + " in function call");
			return nullptr;
		}
		
	}
	#endif
}


//                      V-Func name
std::optional<std::pair<std::string, Extern>> Parser::parseFuncSignature() {
	auto failed = [&]() {
		// Go to the first '{'
		do {
			getToken();
			if (token == Token::eof) {
				return;
			}
		} while (token != Token::openCurly);
		getToken();

		int curlyDepth = 1;
		while (true) {
			if (token == Token::openCurly) {
				curlyDepth += 1;
			}
			else if (token == Token::closedCurly) {
				curlyDepth -= 1;
			}
			else if (token == Token::eof) {
				return;
			}
			if (curlyDepth == 0) {
				return;
			}
		}
	};
	getToken();
	if (token != Token::identifier) {
		err("Expected identifier in expected function declaration");
		return std::nullopt;
	}
	std::string name = std::move(token_string);
	getToken();
	if (token != Token::openBracket) {
		err("Expected a '(' after expected function declaration");
		return std::nullopt;
	}
	Extern signature;
	signature.returnType = &Types::Void;
	std::pair<std::string, const Type*> arg;
	getToken();
	if (token == Token::closedBracket) {
		goto end;
	}
begin:
	if (token != Token::identifier) {
		err("Expected an argument name in function");
		failed();
		return std::nullopt;
	}
	arg.first = std::move(token_string);
	getToken();
	if (token != Token::colon) {
		err("Expected a ':' after the argument name");
		failed();
		return std::nullopt;
	}
	getToken();
	arg.second = handleType(); 
	if (arg.second == nullptr) {
		err("Expected type");
		failed();
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
	} else {
		err("Expected a comma or a closed bracket after argument");
		failed();
		return std::nullopt;
	}
end:
	if (token == Token::arrow) {
		getToken();
		const Type* type = handleType();
		if (type == nullptr) {
			err("Expected type after '->'");
		}
	}
	return {{ name, std::move(signature) }};
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

			//TODO: fix
			//return std::make_unique<SetVarExpr>(name, std::move(result));
		}
	}
	else if (token == Token::openBracket) {
		return handleFuncCall(name);
	}
	else {
		// TODO:fix
		//return std::make_unique<GetVarExpr>(name);
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
	if (token != Token::openCurly) {
		err("Expected an open curly bracket after function declaration");
		return;
	}
	Function f { funcDecl->second.args, funcDecl->second.returnType, handleBlock() };
	// the function that was moved into the map
	Function& func = ast.functions.emplace(funcDecl->first, std::move(f)).first->second;
	//ast.functions.emplace(funcDecl)??;
	
	
}

const Type* Parser::handleType() {
	if (token != Token::identifier) {
		err("Expected an identifier to name a type");
		return nullptr;
	}
	
	const Type* type = nullptr;

	auto it1 = std::find_if(Types::all.begin(), Types::all.end(), [&](auto& elem) {
		return elem->name == token_string;
		});
	if (it1 != Types::all.end()) {
		type = *it1;
	}

	auto it2 = std::find_if(types.begin(), types.end(), [&](auto& elem) {
		return elem->name == token_string;
	});
	if (it2 != types.end()) {
		type = it2->get();
	}

	if (type == nullptr) {
		err("Identifier " + token_string + " does not name a type");
	}
	return type;
	
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
	case Token::openBracket: {
		getToken();
		std::unique_ptr<Expression> expr = expectExpression();
		if (token == Token::closedBracket) {
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
	case Token::keyword_if: {
		getToken();
		// condition
		std::unique_ptr<Expression> condition = expectExpression();
		if (condition == nullptr) {
			err("Expected an expression after if statement");
			return nullptr;
		}
		if (token != Token::openCurly) {
			err("Expected a block '{' after the condition of an if statement");
			return nullptr;
		}
		IfExpr ifExpr { std::move(condition), handleBlock(), nullptr };
		std::unique_ptr<Expression>* insertPoint = &ifExpr.ifFalse;
		while (token == Token::keyword_elif) {
			condition = expectExpression();
			if (condition == nullptr) {
				err("Expected an expression after elif");
				return nullptr;
			}
			if (token != Token::openCurly) {
				err("Expected a block '{' after the condition of an elif statement");
				return nullptr;
			}
			std::unique_ptr<IfExpr> newIfExpr = std::make_unique<IfExpr>(std::move(condition), handleBlock(), nullptr);
			std::unique_ptr<Expression>* newInsertPoint = &newIfExpr->ifFalse;
			*insertPoint = std::move(newIfExpr);
			insertPoint = newInsertPoint;
		}
		if (token == Token::keyword_else) {
			getToken();
			if (token != Token::openCurly) {
				err("Expected a block '{' after an else statement");
				return nullptr;
			}
			*insertPoint = handleBlock();
		}
		return std::make_unique<IfExpr>(std::move(ifExpr));
	}
	case Token::number:
		getToken();
		return std::make_unique<NumLitExpr>(token_number);
	case Token::identifier: {
		std::string identifier = std::move(token_string);
		getToken();
		if (token == Token::openBracket) {
			return handleFuncCall(std::move(identifier));
		}
		else {
			// Todo:fix
			// return std::make_unique<GetVarExpr>(std::move(identifier));
			
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

DeclareVar* Parser::handleLet() {
	getToken();
	if (token != Token::identifier) {
		err("Expected identifier after let statement");
		return nullptr;
	}
	std::string name = token_string;
	const Type* type = nullptr;
	getToken();
	if (token == Token::colon) {
		getToken();
		type = handleType();
	}
	if (token != Token::asign) {
		err("Unexpected token after let");
		return nullptr;
	}
	getToken();
	std::unique_ptr<Expression> expr = expectExpression();
	if (expr == nullptr) {
		err("Expected expression after let");
		return nullptr;
	}
	ast.currentBlock->variables.emplace_back(name, expr->type, false);
	ast.currentBlock->expressions.emplace_back(std::make_unique<SetVarExpr>(ast.currentBlock->variables.back(), std::move(expr)));
	return  &ast.currentBlock->variables.back();
}