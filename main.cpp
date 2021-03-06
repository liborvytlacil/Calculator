/*
author: Libor Vytlacil
 A simple REPL for arithmetic expressions involving floating point numbers, +, -, *, /, % and parentheses
 and variable declaration and usage.

 The prompt accepts a line of input at a time which it then parses, evaluates and prints its result.
 The evaluation is based on the following grammar. Any defined variable remains stored for subsequent
 statements (even for statements in subsequent prompts).

 Prompt 'q' quits the program.

 Grammar:

 Calculation ->
    Calculation Statement

 Statement ->
	Expression
	Declaration

 Declaration ->
	'let' <name> '=' Expression

 Expression ->
	Expression '+' Term
	Expression '-' Term
	Term

 Term ->
	Term '*' Primary
	Term '/' Primary
	Term '%' Primary
	Primary

 Primary -> 
	'+' Primary
	'-' Primary
	Number
	<name>
	'(' Expression ')'

 Number ->
	floating-point literal


 This is a left-recursive grammar and the implementation removes the recursion as following:
 Expression -> Term Expr1
 Expr1 -> '+' Term Expr1
 Expr1 -> '-' Term Expr1
 Expr1 -> epsilon         // empty right side

 etc..
 */

#include <iostream>
#include <string>
#include <sstream>
#include "tokenStream.h"
#include "variable.h"

using namespace std;

// prototypes
double expression(TokenStream& ts, VarTable& varTable);

// handles 'primary ->' production rules
double primary(TokenStream& ts, VarTable& varTable) {
	Token token = ts.get();
	switch (token.kind) {
	case TokenType::ADD:
		return primary(ts, varTable);
	case TokenType::SUB:
		return -primary(ts, varTable);
	case TokenType::LPAREN: {
		double expr = expression(ts, varTable);
		token = ts.get();
		if (token.kind != TokenType::RPAREN) {
			throw runtime_error("Missing a right parenthesis.");
		}
		return expr;
	}
	case TokenType::NUMBER:
		return token.value;
	case TokenType::NAME:
		return varTable.get(token.name);
	default:
		ts.putback(token);
		throw runtime_error("Expected a primary");
	}

	return 0.0;
}

// handles 'term ->' production rules
double term(TokenStream& ts, VarTable& varTable) {
	double left = primary(ts, varTable);
	Token token = ts.get();
	while (true) {
		switch (token.kind) {
		case TokenType::MUL:
			left *= primary(ts, varTable);
			token = ts.get();
			break;
		case TokenType::DIV: {
			double right = primary(ts, varTable);
			if (right == 0.0) {
				throw runtime_error("Division by zero");
			}
			left /= right;
			token = ts.get();
			break;
		}
		case TokenType::MOD: {
			double right = primary(ts, varTable);
			if (right == 0.0) {
				throw runtime_error("Division by zero");
			}
			left = fmod(left, right);
			token = ts.get();
			break;
		}
		default:
			ts.putback(token);
			return left;
		}
	}
}

// handles 'expression ->' production rules
double expression(TokenStream& ts, VarTable& varTable) {
	double left = term(ts, varTable);
	Token token = ts.get();

	while (true) {
		switch (token.kind) {
		case TokenType::ADD:
			left += term(ts, varTable);
			token = ts.get();
			break;
		case TokenType::SUB:
			left -= term(ts, varTable);
			token = ts.get();
			break;
		default:
			ts.putback(token);
			return left;
		}
	}
}

// handles 'declaration ->' production rule
double declaration(TokenStream& ts, VarTable& varTable) {
	Token token = ts.get();
	if (token.kind != TokenType::NAME) {
		ts.putback(token);
		throw runtime_error("Expected a variable name after 'let' keyword.");
	}
	string name = token.name;

	token = ts.get();
	if (token.kind != TokenType::EQUALS) {
		ts.putback(token);
		throw runtime_error("Missing '=' in a declaration of '" + name + "'");
	}

	double value = expression(ts, varTable);
	varTable.define(name, value);

	return value;
}

// handles 'statement ->' production rules
double statement(TokenStream& ts, VarTable& varTable) {
	Token token = ts.get();
	switch (token.kind) {
	case TokenType::KW_LET:
		return declaration(ts, varTable);
	default:
		ts.putback(token);
		return expression(ts, varTable);
	}
}

// hanldes 'Calculation ->' production rules 
double calculation(TokenStream& ts, VarTable& varTable) {
	double result = 0.0;
	while (true) {
		Token token = ts.get();

		if (token.kind == TokenType::INPUT_EOF) {
			break;
		}
		else {
			ts.putback(token);
			result = statement(ts, varTable);
		}
	}

	return result;
}

void testStatement(const string& input, double expected) {
	istringstream sstream(input);
	TokenStream tstream(sstream);
	VarTable varTable;

	cout << "Input: " << input << " Result: ";
	bool success = false;

	try {
		double actual = calculation(tstream, varTable);
		cout << actual;

		// Note: doubles should be ideally compared as |a - b| < epsilon
		if (expected == actual) {
			success = true;
		}
	}
	catch (runtime_error &e) {
		success = false;
		cout << "Exception thrown: " << e.what();
	}

	cout << " " << (success ? "[PASS]" : "[FAIL]") << endl;
}

const string prompt = "> ";
const string msgResult = "= ";

void test() {
	cout << "Tests: " << endl;
	testStatement("2", 2.0);
	testStatement("1+2", 3.0);
	testStatement("1-2", -1.0);
	testStatement("0+2", 2.0);
	testStatement("452+1000", 1452.0);
	testStatement("6*3+2", 20.0);
	testStatement("2+6*3", 20.0);
	testStatement("7/3", 7.0 / 3.0);
	testStatement("6/3+2", 4.0);
	testStatement("2+6/3", 4.0);
	testStatement("+1", 1.0);
	testStatement("-1", -1.0);
	testStatement("-1--1", 0.0);
	testStatement("8%3", 2.0);
	testStatement("-8%3", -2.0);
	testStatement("8%-3", 2.0);
	testStatement("-8%-3", -2.0);
	testStatement("let x = 3", 3.0);
	testStatement("let x = 2 (x + 2) * 3", 12.0);
	cout << "-----------------------------------------" << endl;
}

int main() {
	test();
	cout << endl << "Keep entering expressions with floating point numbers, +, -, *, / and parentheses." << endl;
	cout << "Terminate an expression with ';', exit program by typing 'q'." << endl << endl;

	VarTable varTable;

	while (cin) {
		cout << prompt;

		// construct a token stream out of one line of input
		string input;
		getline(cin, input);

		// 'q' means exit the program
		if (input == "q") {
			break;
		}
 
		istringstream inputStream(input);
		TokenStream ts(inputStream);

		try {
			cout << msgResult << calculation(ts, varTable) << endl;
		}
		catch (runtime_error& e) {
			cerr << e.what() << endl;
		}
	}

	return 0;
}