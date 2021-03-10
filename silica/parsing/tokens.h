#pragma once
#include "include.h"
#include <map>

namespace Silica {


enum class Token: uint_least32_t  {
	// Precedence             XX
	// Unique number       XX
	// Category         XX
	// x            = 0x**'**'**

	// Category 1 binop
	// Category 2 unaryop
	// Category 3 both
	minusEquals     = 0x01'01'05,
	plusEquals      = 0x01'02'05,
	multiplyEquals  = 0x01'03'05,
	divideEquals    = 0x01'04'05,
	asign           = 0x01'05'05,
	greaterEquals   = 0x01'0c'04,
	smaller         = 0x01'0d'04,
	greater         = 0x01'0b'04,
	smallerEquals   = 0x01'0e'04,
	power           = 0x01'0a'03,
	multiply        = 0x01'08'02,
	divide          = 0x01'09'02,
	minus           = 0x03'07'01,
	plus            = 0x03'06'01,

	// Category 4 others
	open_bracket    = 0x04'0f'00,
	closed_bracket  = 0x04'10'00,
	number          = 0x04'11'00,
	identifier      = 0x04'12'00,
	colon           = 0x04'13'00,
	comma           = 0x04'14'00,
	newline         = 0x04'15'00,

	// Category 5 keywords
	keyword_func    = 0x05'16'00,
	keyword_extern  = 0x05'17'00,
	keyword_return  = 0x05'18'00,
	keyword_let     = 0x05'19'00,

	// Category 0 eof
	eof             = 0x00'00'00
};


constexpr inline int getTokenCategory(Token tok) {
	return uint_least32_t(tok) & 0x11'00'00;
};

constexpr inline bool isOperator(Token tok) {
	return getTokenCategory(tok) == 2;
}

constexpr unsigned precedenceMultiplier = 8;


constexpr bool isUnaryOp(Token tok) {
	auto category = uint_least32_t(tok) & 0x00'FF'00;
	return category == 0x02'00 || category == 0x03'00;
}

// Returns 0 if it isnt an operator
constexpr inline unsigned getTokenPrecedence(Token tok) {
	return  (uint_least32_t(tok) & 0x00'00'FF) * precedenceMultiplier;
};

std::string descibeToken(Token tok);

const std::map<std::string_view, Token> tokens = {
	{":",  Token::colon},
	{"=",  Token::asign},
	{",",  Token::comma},
	{"**", Token::power},
	{"+",  Token::plus},
	{"+=", Token::plusEquals},
	{"-",  Token::minus},
	{"-=", Token::minusEquals},
	{"*",  Token::multiply},
	{"×",  Token::multiply},
	{"×=", Token::multiplyEquals},
	{"*=", Token::multiplyEquals},
	{"÷",  Token::divide},
	{"/",  Token::divide},
	{"÷=", Token::divideEquals},
	{"/=", Token::divideEquals},
	{"(",  Token::open_bracket},
	{")",  Token::closed_bracket},
	{">",  Token::greater},
	{"<",  Token::smaller},
	{">=", Token::greaterEquals},
	{"<=", Token::smallerEquals}
	//{"\n", Token::newline}
};

const std::string_view operator_chars = ":=,*+-/×÷<>()";

const std::map<std::string_view, Token> keywords = {
	{"func",    Token::keyword_func},
	{"use",     Token::keyword_extern},
	{"return",  Token::keyword_return},
	{"let",     Token::keyword_let},
	{"__NL",    Token::newline},
	{"__EOF",   Token::eof}
};

}; // End namespace Silica