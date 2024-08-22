#ifndef __GP_HPP__
#define __GP_HPP__

/*
	gp

	Graph parser library for C++ 17.
	Produced by scenent (https://github.com/scenent).
	Distributed under MIT License.

	Examples :
		x
		x * 0.5
		(x / 10.0) ^ 2 + 100
		sin(x / 10.0) * 50.0 + 50.0
		log(x / 10.0) * 50.0 + 100.0

	Supporting Functions :
		sin(x)
		cos(x)
		tan(x)
		asin(x)
		acos(x)
		atan(x)
		log(x)
		exp(x)
		sqrt(x)
		abs(x)
*/

#include <cmath>         // std::sin(), std::cos() etc...
#include <cassert>       // assert()
#include <iostream>      // std::cout
#include <string>        // std::string 
#include <vector>        // std::vector
#include <stack>         // std::stack
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <memory>        // std::move()


#define GP_EVAL_FUNCTION_SWITCH(TOKEN_TYPE, FUNC) \
	case (TOKEN_TYPE): { \
		_t1 = _stack.top(); \
		_stack.pop(); \
		assert(_t1.Type == TokenType::NumberLiteral); \
		Token _token; \
		_token.Type = TokenType::NumberLiteral; \
		_token.Data = std::to_string(FUNC(static_cast<real_t>(std::stod(_t1.Data)))); \
		_stack.push(_token); \
		break; \
	} \

#define GP_EVAL_OPERATOR_SWITCH(TOKEN_TYPE, OP) \
	case (TOKEN_TYPE): { \
		_t2 = _stack.top(); \
		_stack.pop(); \
		_t1 = _stack.top(); \
		_stack.pop(); \
		assert(_t1.Type == TokenType::NumberLiteral); \
		assert(_t2.Type == TokenType::NumberLiteral); \
		real_t _a = static_cast<real_t>(std::stod(_t1.Data)); \
		real_t _b = static_cast<real_t>(std::stod(_t2.Data)); \
		Token _token; \
		_token.Type = TokenType::NumberLiteral; \
		_token.Data = std::to_string(_a OP _b); \
		_stack.push(_token); \
		break; \
	} \


namespace gp {
	/* precision of floating type */
	using real_t = float;

	/* basic two-dimensional vector */
	template<typename T>
	struct Vector2 {
		T x, y;
		Vector2() : x(0.0), y(0.0) { }
		Vector2(const T& _x, const T& _y) : x(_x), y(_y) { }
		Vector2(const Vector2<T>& _v) {
			x = _v.x; y = _v.y;
		}
		Vector2(Vector2<T>&& _v) noexcept {
			this->x = std::move(_v.x);
			this->y = std::move(_v.y);
		}
		Vector2<T> operator+(const Vector2<T>& rhs) const { return Vector2<T>(x + rhs.x, y + rhs.y); }
		Vector2<T>& operator+=(const Vector2<T>& rhs) { x += rhs.x; y += rhs.y; return *this; }
		Vector2<T> operator-(const Vector2<T>& rhs) const { return Vector2<T>(x - rhs.x, y - rhs.y); }
		Vector2<T>& operator-=(const Vector2<T>& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
		Vector2<T> operator*(const T& rhs) const { return Vector2<T>(x * rhs, y * rhs); }
		Vector2<T>& operator*=(const T& rhs) { x *= rhs; y *= rhs; return *this; }
		Vector2<T> operator/(const T& rhs) const { return Vector2<T>(x / rhs, y / rhs); }
		Vector2<T>& operator/=(const T& rhs) { x /= rhs; y /= rhs; return *this; }
		bool operator==(const Vector2<T>& rhs) const { return x == rhs.x && y == rhs.y; }
		bool operator!=(const Vector2<T>& rhs) const { return !(*this == rhs); }
		Vector2& operator=(const Vector2<T>& other) { this->x = other.x; this->y = other.y; return *this; };
		Vector2& operator=(Vector2<T>&& other) noexcept { this->x = std::move(other.x); this->y = std::move(other.y); return *this; }
		T length() const {
			return std::sqrt((this->x * this->x) + (this->y * this->y));
		}
		T length_squared() const {
			return (this->x * this->x) + (this->y * this->y);
		}
	};
	using vec2 = Vector2<real_t>;

	/* type of token */
	enum class TokenType {
		Unknown = 0,
		
		// Identifiers
		InputX,     // x
		Variable,   // a, b, c ...
		Sine,       // sin()
		Cosine,     // cos()
		Tangent,    // tan()
		ArcSine,    // asin()
		ArcCosine,  // acos()
		ArcTangent, // atan()
		Log,        // log()
		Exp,        // exp()
		Sqrt,       // sqrt()
		Abs,        // abs()

		// Literals
		NumberLiteral,

		// Operators
		Add, // +
		Sub, // -
		Mul, // *
		Div, // /
		Mod, // %
		Pow, // ^
		Neg, // -

		// Punctuators
		LeftParent,
		RightParent,
	};

	/* token used for parsing */
	struct Token {
		std::string Data = "";
		TokenType Type = TokenType::Unknown;
	};

	namespace scanner {
		enum class CharType {
			Unknown,
			Whitespace,
			Number,
			Identifier,
			OperatorAndPunctuator,
		};

		static CharType GetCharType(char c) {
			if (' ' == c || '\t' == c || '\r' == c || '\n' == c) {
				return CharType::Whitespace;
			}
			if ('0' <= c && c <= '9') {
				return CharType::Number;
			}
			if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
				return CharType::Identifier;
			}
			if (
				c == '+' ||
				c == '-' ||
				c == '*' ||
				c == '/' ||
				c == '%' ||
				c == '^' ||
				c == '(' ||
				c == ')'
				) {
				return CharType::OperatorAndPunctuator;
			}
			return CharType::Unknown;
		}

		/* scan source */
		static std::vector<Token> Scan(const std::string& _src) {
			std::vector<Token> _result { };

			size_t _index = 0ULL;
			const size_t _src_len = _src.size();
			while (_index < _src_len) {
				switch (GetCharType(_src[_index])) {
				case (CharType::Unknown):
				case (CharType::Whitespace): {
					_index++; break;
				}
				case (CharType::Number): {
					Token _token{};
					_token.Type = TokenType::NumberLiteral;
					
					while ((_index < _src_len) && GetCharType(_src[_index]) == CharType::Number) {
						_token.Data += _src[_index];
						_index++;
					}

					if ((_index < _src_len) && _src[_index] == '.') {
						_token.Data += '.';
						_index++;

						while ((_index < _src_len) && GetCharType(_src[_index]) == CharType::Number) {
							_token.Data += _src[_index];
							_index++;
						}
					}

					_result.emplace_back(_token);
					break;
				}
				case (CharType::Identifier): {
					Token _token{};

					while ((_index < _src_len) && GetCharType(_src[_index]) == CharType::Identifier) {
						_token.Data += _src[_index];
						_index++;
					}

					_token.Type = (_token.Data == "x") ? TokenType::InputX : TokenType::Variable;
					
					static std::unordered_map<std::string, TokenType> _identifierToTokenType = {
						{"sin", TokenType::Sine},
						{"cos", TokenType::Cosine},
						{"tan", TokenType::Tangent},
						{"asin", TokenType::ArcSine},
						{"acos", TokenType::ArcCosine},
						{"atan", TokenType::ArcTangent},
						{"log", TokenType::Log},
						{"exp", TokenType::Exp},
						{"sqrt", TokenType::Sqrt},
						{"abs", TokenType::Abs},
					};

					if (_identifierToTokenType.find(_token.Data) != _identifierToTokenType.end()) {
						_token.Type = _identifierToTokenType[_token.Data];
					}

					_result.emplace_back(_token);
					break;
				}
				case (CharType::OperatorAndPunctuator): {
					Token _token{};

					_token.Data = _src[_index];
					_index++;

					static std::unordered_map<char, TokenType> _opToTokenType = {
						{'+', TokenType::Add},
						{'-', TokenType::Sub},
						{'*', TokenType::Mul},
						{'/', TokenType::Div},
						{'%', TokenType::Mod},
						{'^', TokenType::Pow},

						{'(', TokenType::LeftParent},
						{')', TokenType::RightParent},
					};

					assert(_token.Data.size() == 1);
					assert(_opToTokenType.find(_token.Data.front()) != _opToTokenType.end());

					_token.Type = _opToTokenType[_token.Data.front()];

					if (_token.Type == TokenType::Sub) {
						if (_result.empty() == false) {
							if (_result.back().Type != TokenType::NumberLiteral &&
								_result.back().Type != TokenType::RightParent
								) {
								_token.Type = TokenType::Neg;
							}
						}
						else {
							_token.Type = TokenType::Neg;
						}
					}

					_result.emplace_back(_token);
					break;
				}
				default: {
					assert(false);
				}
				}
			}

			return _result;
		}
	}

	/* gp (Graph Parser) */
	class GraphParser final {
	private:
		std::unordered_map<std::string, real_t> m_VariableMap{};
	public:
		GraphParser() = default;
		~GraphParser() = default;
		GraphParser(const GraphParser&) = default;
		GraphParser(GraphParser&&) = default;
		GraphParser& operator=(const GraphParser&) = default;
		GraphParser& operator=(GraphParser&&) = default;
	public:
		/* execute source */
		std::vector<vec2> exec(const std::string& _expr, const real_t& _xBegin, const real_t& _xEnd, const real_t& _xStep, std::unordered_map<std::string, real_t>&& _varMap) {
			m_VariableMap = std::move(_varMap);

			std::string _src = _expr + " ";
			std::vector<vec2> _result { };

			// Scan source.
			auto _scanned = scanner::Scan(_src);
			// Parse source.
			auto _parsed = this->parse(_scanned);
			// Evaluate source for xBegin to xEnd.
			if (_xStep > 0.0) {
				for (real_t x = _xBegin; x <= _xEnd; x += _xStep) {
					_result.emplace_back(vec2(x, this->evaluate(_parsed, x)));
				}
			} else {
				for (real_t x = _xBegin; x >= _xEnd; x += _xStep) {
					_result.emplace_back(vec2(x, this->evaluate(_parsed, x)));
				}
			}

			return _result;
		}
	private:
		std::vector<Token> parse(const std::vector<Token>& _scanned) {
			
			static std::unordered_map<TokenType, int> _priority = {
				{TokenType::LeftParent, 0},
				{TokenType::Add, 1}, {TokenType::Sub, 1},
				{TokenType::Mul , 2}, {TokenType::Div, 2},
				{TokenType::Pow , 3},
				{TokenType::Neg , 4},
				{TokenType::Sine , 5},
				{TokenType::Cosine , 5},
				{TokenType::Tangent , 5},
				{TokenType::ArcSine , 5},
				{TokenType::ArcCosine , 5},
				{TokenType::Abs , 5},
				{TokenType::ArcTangent , 5},
				{TokenType::Log , 5},
				{TokenType::Exp , 5},
				{TokenType::Sqrt , 5},
			};
			
			std::stack<Token> _stack{};
			std::vector<Token> _postfix{};
			for (size_t i = 0; i < _scanned.size(); i++) {
				switch (_scanned[i].Type) {
				case (TokenType::InputX):
				case (TokenType::Variable):
				case (TokenType::NumberLiteral): {
					_postfix.emplace_back(_scanned[i]);
					break;
				}
				case (TokenType::LeftParent): {
					_stack.push(_scanned[i]);
					break;
				}
				case (TokenType::RightParent): {
					while (_stack.empty() == false && _stack.top().Type != TokenType::LeftParent) {
						_postfix.emplace_back(_stack.top());
						_stack.pop();
					}
					_stack.pop();
					break;
				}
				default: {
					while (_stack.empty() == false && _priority[_scanned[i].Type] <= _priority[_stack.top().Type]) {
						assert(_stack.top().Type != TokenType::RightParent);
						_postfix.emplace_back(_stack.top()); 
						_stack.pop();
					}
					_stack.push(_scanned[i]);
					break;
				}
				}
			}
			while (_stack.empty() == false) {
				_postfix.emplace_back(_stack.top());
				_stack.pop();
			}
			return _postfix;
		}

		real_t evaluate(const std::vector<Token>& _postfix, const real_t& _xValue) const {
			std::stack<Token> _stack{};
			Token _t1{}, _t2{};
			const size_t _postfix_len = _postfix.size();
			for (size_t i = 0; i < _postfix_len; i++) {
				switch (_postfix[i].Type) {
				case (TokenType::NumberLiteral): {
					_stack.push(_postfix[i]);
					break;
				}
				case (TokenType::Neg): {
					_t1 = _stack.top();
					_stack.pop();
					assert(_t1.Type == TokenType::NumberLiteral);
					Token _token;
					_token.Type = TokenType::NumberLiteral;
					_token.Data = "-" + _t1.Data;
					_stack.push(_token);
					break;
				}
				GP_EVAL_FUNCTION_SWITCH(TokenType::Sine, std::sin);
				GP_EVAL_FUNCTION_SWITCH(TokenType::Cosine, std::cos);
				GP_EVAL_FUNCTION_SWITCH(TokenType::Tangent, std::tan);
				GP_EVAL_FUNCTION_SWITCH(TokenType::ArcSine, std::asin);
				GP_EVAL_FUNCTION_SWITCH(TokenType::ArcCosine, std::acos);
				GP_EVAL_FUNCTION_SWITCH(TokenType::ArcTangent, std::atan);
				GP_EVAL_FUNCTION_SWITCH(TokenType::Log, std::log);
				GP_EVAL_FUNCTION_SWITCH(TokenType::Exp, std::exp);
				GP_EVAL_FUNCTION_SWITCH(TokenType::Sqrt, std::sqrt);
				GP_EVAL_FUNCTION_SWITCH(TokenType::Abs, std::abs);
				GP_EVAL_OPERATOR_SWITCH(TokenType::Add, +);
				GP_EVAL_OPERATOR_SWITCH(TokenType::Sub, -);
				GP_EVAL_OPERATOR_SWITCH(TokenType::Mul, *);
				GP_EVAL_OPERATOR_SWITCH(TokenType::Div, /);
				case (TokenType::Mod): {
						_t2 = _stack.top();
						_stack.pop();
						_t1 = _stack.top();
						_stack.pop();
						assert(_t1.Type == TokenType::NumberLiteral);
						assert(_t2.Type == TokenType::NumberLiteral);
						int64_t _a = static_cast<int64_t>(std::stod(_t1.Data));
						int64_t _b = static_cast<int64_t>(std::stod(_t2.Data));
						Token _token;
						_token.Type = TokenType::NumberLiteral;
						_token.Data = std::to_string(_a % _b);
						_stack.push(_token);
						break;
				}
				case (TokenType::Pow): {
					_t2 = _stack.top();
					_stack.pop();
					_t1 = _stack.top();
					_stack.pop();
					assert(_t1.Type == TokenType::NumberLiteral);
					assert(_t2.Type == TokenType::NumberLiteral);
					real_t _a = static_cast<real_t>(std::stod(_t1.Data));
					real_t _b = static_cast<real_t>(std::stod(_t2.Data));
					Token _token;
					_token.Type = TokenType::NumberLiteral;
					_token.Data = std::to_string(std::pow(_a, _b));
					_stack.push(_token);
					break;
				}
				case (TokenType::InputX): {
					Token _token;
					_token.Type = TokenType::NumberLiteral;
					_token.Data = std::to_string(_xValue);
					_stack.push(_token);
					break;
				}
				case (TokenType::Variable): {
					Token _token;
					_token.Type = TokenType::NumberLiteral;
					_token.Data = std::to_string(this->m_VariableMap.at(_postfix[i].Data));
					_stack.push(_token);
					break;
				}
				default: {
					assert(false);
					break;
				}
				}
			}
			assert(_stack.empty() == false);
			return static_cast<real_t>(std::stod(_stack.top().Data));
		}
	};
}



#endif