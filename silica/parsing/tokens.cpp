#include "parsing/Parser.h"
#include <optional>
#include <string>

using namespace Silica;
using namespace std::literals;

bool Parser::next() {
	if (stream.get(current)) {
		// not eof
		if (current == '\n') {
			byte = 1;
			line++;
			referredLines.emplace_back("");
			sourceLineReferred = false;
		}
		else if (current == '\t') {
			referredLines.back() += "    ";
			byte += 4;
		}
		else {
			referredLines.back() += current;
			byte++;
		}
		return true;
	}
	else {
		return false;
	}
}

void Parser::getToken(bool inclNewline) {
	nextToken(inclNewline);
	std::cout << '(' << descibeToken(token) << '-' << token_number << '-' << token_string << ")\n";
}



void Parser::nextToken(bool inclNewline) {
	while (true) {
		if (stream.eof()) {
			token = Token::eof;
			return;
		}

		// Skip white space
		if (current == ' ') {
			next();
			continue;
		}

		if (current == '\n') {
			if (inclNewline) {
				std::cout << "(token newline)\n";
				token = Token::newline;
				next();
				return;
			}
			else {
				std::cout << "ignored (token newline)\n";
				next();
			}
			continue;
		}

		if (current == '\t') {
			err("Unexpected tab");
			next();
			continue;
		}

		// Skip comments
		if (current == '#') {
			do {
				if (!next()) {
					token = Token::eof;
					return;
				}
			} while (current != '\n');
			continue;
		}

		// Parse number literals
		if (isdigit(current)) {
			token = Token::number;
			token_number = current - '0';
			while (next() && isdigit(current)) {
				token_number = (token_number * 10) + current - '0';
			}
			return;
		}

		// Parse identifiers and keywords
		if (isalpha(current)) {
			token_string = std::string(1, current);
			while (next() && (isalnum(current) || current == '_')) {
				token_string.push_back(current);
			}
			auto result = keywords.find(token_string);
			if (result != keywords.end()) {
				token = result->second;
			}
			else {
				token = Token::identifier;
			}
			return;
		}

		// Parse operators
		std::string fetched(1, current);
		bool found = false;
		for (auto& pair : tokens) {
			// Does pair.first start with fetched?
			while (!pair.first.compare(0, fetched.size(), fetched)) {
				// Direct match
				if (pair.first.size() == fetched.size()) {
					token = pair.second;
					found = true;
					if (!next()) {
						return;
					}
					fetched.push_back(current);
				}
				else {
					if (next()) {
						fetched.push_back(current);
					}
				}

			}
		}
		if (found) {
			return;
		}

		err("Invalid char '"s + current + "' (" + std::to_string(current) + ")", current);
		next();
		continue;

	}
}

void Parser::err(std::string msg, int fmtByte) {
	errorCount++;
	views.emplace_back(referredLines.back(), std::move(msg), View::Type::error, line, fmtByte);
	sourceLineReferred = true;
}

void Parser::note(std::string msg, int fmtByte) {
	views.emplace_back(referredLines.back(), std::move(msg), View::Type::note, line, fmtByte);
	sourceLineReferred = true;
}



std::string Silica::descibeToken(Token tok) {
	switch (tok) {
	case Token::number: return "token number";
	case Token::identifier: return "identifier";
	case Token::colon: return "colon";
	case Token::newline: return "newline";
	case Token::comma: return "comma";
	case Token::asign: return "asign";
	case Token::power: return "power";
	case Token::plus: return "plus";
	case Token::plusEquals: return "plus equals";
	case Token::minus: return "minus";
	case Token::minusEquals: return "minus equals";
	case Token::multiply: return "multiply";
	case Token::multiplyEquals: return "multiply equals";
	case Token::divide: return "divide";
	case Token::divideEquals: return "divide equals";
	case Token::greater: return "greater";
	case Token::greaterEquals: return "greater equals";
	case Token::smaller: return "smaller";
	case Token::smallerEquals: return "smaller equals";
	case Token::openBracket: return "open bracket";
	case Token::closedBracket: return "closed bracket";
	case Token::openCurly: return "open curly bracket";
	case Token::closedCurly: return "closed curly bracket";
	case Token::openSquare: return "open square bracket";
	case Token::closedSquare: return "closed square bracket";
	case Token::keyword_func: return "keyword func";
	case Token::keyword_extern: return "keyword extern";
	case Token::keyword_return: return "keyword return";
	case Token::keyword_let: return "keyword let";
	case Token::eof: return "EOF";
	default: unreachable();

	}
	return {};
}
