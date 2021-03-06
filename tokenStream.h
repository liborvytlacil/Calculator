// author: Libor Vytlacil
// Contains an implementation of a token scanner.
#pragma once
#include <vector>
#include <istream>
#include <algorithm>

using namespace std;

enum class TokenType {
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	LPAREN,
	RPAREN,
	NUMBER,
	INPUT_EOF,
	UNKNOWN,
	KW_LET,
	EQUALS,
	NAME
};

class Token {
public:
	TokenType kind;
	double value;
	string name;

	Token(TokenType type) :kind(type), value(0.0), name("") { }
	Token(double val) :kind(TokenType::NUMBER), value(val), name("") { }
	Token(string name) :kind(TokenType::NAME), value(0.0), name(name) { }
};

class TokenStream {
private:
	istream& inputStream;
	bool bufferFull;
	Token buffer;

	// Reads the next token from the input stream
	Token doReadNextToken() {
		char ch;
		inputStream >> ch;

		if (inputStream.eof()) {
			return Token{ TokenType::INPUT_EOF };
		}

		switch (ch) {
		case '+':
			return Token{ TokenType::ADD };
		case '-':
			return Token{ TokenType::SUB };
		case '*':
			return Token{ TokenType::MUL };
		case '/':
			return Token{ TokenType::DIV };
		case '%':
			return Token{ TokenType::MOD };
		case '(':
			return Token{ TokenType::LPAREN };
		case ')':
			return Token{ TokenType::RPAREN };
		case '=':
			return Token{ TokenType::EQUALS };
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.': {
			inputStream.putback(ch);
			double val;
			inputStream >> val;
			return Token{ val };
		}
		default:
			if (isalpha(ch)) {
				return readVariableOrKeyword(ch);
			}
			else {
				return Token{ TokenType::UNKNOWN };
			}
		}
	}

	// Reads the next keyword or variable name token, when starting character ch
	// has already been read
	Token readVariableOrKeyword(char ch) {
		string name;
		name += ch;

		while (inputStream.get(ch)) {
			if (isalpha(ch) || isdigit(ch)) {
				name += ch;
			}
			else {
				inputStream.putback(ch);
				break;
			}
		}

		if (name == "let") {
			return Token{ TokenType::KW_LET };
		}

		return Token{ name };
	}
public:
	TokenStream(istream& inputStream)
		:inputStream(inputStream), bufferFull(false), buffer(Token{ TokenType::INPUT_EOF }) {}
	
	// Gets the next token
	Token get() {
		if (bufferFull) {
			bufferFull = false;
			return buffer;
		}

		Token nextToken = doReadNextToken();
		if (nextToken.kind == TokenType::UNKNOWN) {
			throw runtime_error("Unexpected token.");
		}
		return nextToken;
	}

	// Returns the given token to the buffer, to that it is read by the
	// next call to get
	void putback(Token token) {
		if (bufferFull) {
			throw runtime_error("Called pushfront with the buffer already full.");
		}
		bufferFull = true;
		buffer = token;
	}

	// Reads and discards all tokens until a token of the given type
	// is read or end of input is reached
	void ignore(TokenType tokenType) {
		// first look into the buffer
		if (bufferFull == true && buffer.kind == tokenType) {
			bufferFull = false;
		}
		else {
			// otherwise search the input
			bufferFull = false;
			while (doReadNextToken().kind != TokenType::INPUT_EOF) { }
		}
	}
};