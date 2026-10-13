// LAST CALL - minimal JSON reader implementation. See header for the contract.

#include "Core/JsonCore.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace lc {

	const JsonValue* JsonValue::find(const std::string& key) const
	{
		if (kind != Kind::Object)
		{
			return nullptr;
		}
		for (const auto& member : object)
		{
			if (member.first == key)
			{
				return &member.second;
			}
		}
		return nullptr;
	}

	std::size_t JsonValue::size() const
	{
		if (kind == Kind::Array)
		{
			return array.size();
		}
		if (kind == Kind::Object)
		{
			return object.size();
		}
		return 0;
	}

	bool JsonValue::get_number(const std::string& key, double& out) const
	{
		const JsonValue* member = find(key);
		if (!member || member->kind != Kind::Number)
		{
			return false;
		}
		out = member->number;
		return true;
	}

	bool JsonValue::get_int(const std::string& key, int& out) const
	{
		double value = 0.0;
		if (!get_number(key, value) || value != static_cast<double>(static_cast<int>(value)))
		{
			return false;
		}
		out = static_cast<int>(value);
		return true;
	}

	bool JsonValue::get_bool(const std::string& key, bool& out) const
	{
		const JsonValue* member = find(key);
		if (!member || member->kind != Kind::Bool)
		{
			return false;
		}
		out = member->boolean;
		return true;
	}

	bool JsonValue::get_string(const std::string& key, std::string& out) const
	{
		const JsonValue* member = find(key);
		if (!member || member->kind != Kind::String)
		{
			return false;
		}
		out = member->string;
		return true;
	}

	bool JsonValue::array_contains(const std::string& value) const
	{
		if (kind != Kind::Array)
		{
			return false;
		}
		for (const JsonValue& element : array)
		{
			if (element.kind == Kind::String && element.string == value)
			{
				return true;
			}
		}
		return false;
	}

	namespace {

		struct Parser
		{
			const std::string& text;
			std::size_t pos = 0;
			std::string* error = nullptr;
			bool failed = false;

			Parser(const std::string& source, std::string* error_out)
				: text(source), error(error_out)
			{
			}

			void fail(const std::string& reason)
			{
				if (!failed)
				{
					failed = true;
					if (error)
					{
						*error = std::to_string(pos) + ": " + reason;
					}
				}
			}

			void skip_ws()
			{
				while (pos < text.size())
				{
					const char c = text[pos];
					if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
					{
						++pos;
					}
					else
					{
						break;
					}
				}
			}

			bool consume(char expected)
			{
				skip_ws();
				if (pos < text.size() && text[pos] == expected)
				{
					++pos;
					return true;
				}
				return false;
			}

			bool literal(const char* word)
			{
				const std::size_t length = std::strlen(word);
				if (text.compare(pos, length, word) == 0)
				{
					pos += length;
					return true;
				}
				return false;
			}

			static void append_utf8(std::string& out, unsigned int codepoint)
			{
				if (codepoint < 0x80)
				{
					out.push_back(static_cast<char>(codepoint));
				}
				else if (codepoint < 0x800)
				{
					out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
					out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
				}
				else if (codepoint < 0x10000)
				{
					out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
					out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
					out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
				}
				else
				{
					out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
					out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
					out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
					out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
				}
			}

			bool parse_hex4(unsigned int& out)
			{
				out = 0;
				for (int i = 0; i < 4; ++i)
				{
					if (pos >= text.size())
					{
						fail("unterminated \\u escape");
						return false;
					}
					const char c = text[pos++];
					out <<= 4;
					if (c >= '0' && c <= '9') { out |= static_cast<unsigned int>(c - '0'); }
					else if (c >= 'a' && c <= 'f') { out |= static_cast<unsigned int>(c - 'a' + 10); }
					else if (c >= 'A' && c <= 'F') { out |= static_cast<unsigned int>(c - 'A' + 10); }
					else { fail("bad hex digit in \\u escape"); return false; }
				}
				return true;
			}

			bool parse_string(std::string& out)
			{
				++pos; // opening quote
				out.clear();
				while (true)
				{
					if (pos >= text.size())
					{
						fail("unterminated string");
						return false;
					}
					const unsigned char c = static_cast<unsigned char>(text[pos]);
					if (c == '"')
					{
						++pos;
						return true;
					}
					if (c < 0x20)
					{
						fail("control character in string");
						return false;
					}
					if (c != '\\')
					{
						out.push_back(text[pos]);
						++pos;
						continue;
					}

					++pos; // backslash
					if (pos >= text.size())
					{
						fail("unterminated escape");
						return false;
					}
					const char escaped = text[pos++];
					switch (escaped)
					{
						case '"':  out.push_back('"'); break;
						case '\\': out.push_back('\\'); break;
						case '/':  out.push_back('/'); break;
						case 'b':  out.push_back('\b'); break;
						case 'f':  out.push_back('\f'); break;
						case 'n':  out.push_back('\n'); break;
						case 'r':  out.push_back('\r'); break;
						case 't':  out.push_back('\t'); break;
						case 'u':
						{
							unsigned int codepoint = 0;
							if (!parse_hex4(codepoint))
							{
								return false;
							}
							if (codepoint >= 0xD800 && codepoint <= 0xDBFF)
							{
								// High surrogate: the low one must follow.
								if (pos + 1 >= text.size() || text[pos] != '\\' || text[pos + 1] != 'u')
								{
									fail("lone high surrogate");
									return false;
								}
								pos += 2;
								unsigned int low = 0;
								if (!parse_hex4(low))
								{
									return false;
								}
								if (low < 0xDC00 || low > 0xDFFF)
								{
									fail("bad low surrogate");
									return false;
								}
								codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
							}
							else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF)
							{
								fail("lone low surrogate");
								return false;
							}
							append_utf8(out, codepoint);
							break;
						}
						default:
							fail("bad escape");
							return false;
					}
				}
			}

			bool parse_number(JsonValue& out)
			{
				const std::size_t start = pos;
				if (pos < text.size() && text[pos] == '-')
				{
					++pos;
				}
				if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos])))
				{
					fail("bad number");
					return false;
				}
				if (text[pos] == '0')
				{
					++pos; // no leading zeros
				}
				else
				{
					while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])))
					{
						++pos;
					}
				}
				if (pos < text.size() && text[pos] == '.')
				{
					++pos;
					if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos])))
					{
						fail("bad fraction");
						return false;
					}
					while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])))
					{
						++pos;
					}
				}
				if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E'))
				{
					++pos;
					if (pos < text.size() && (text[pos] == '+' || text[pos] == '-'))
					{
						++pos;
					}
					if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos])))
					{
						fail("bad exponent");
						return false;
					}
					while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])))
					{
						++pos;
					}
				}

				out.kind = JsonValue::Kind::Number;
				out.number = std::strtod(text.substr(start, pos - start).c_str(), nullptr);
				return true;
			}

			bool parse_value(JsonValue& out)
			{
				skip_ws();
				if (pos >= text.size())
				{
					fail("unexpected end of input");
					return false;
				}
				switch (text[pos])
				{
					case '{': return parse_object(out);
					case '[': return parse_array(out);
					case '"':
						out.kind = JsonValue::Kind::String;
						return parse_string(out.string);
					case 't':
						if (literal("true")) { out.kind = JsonValue::Kind::Bool; out.boolean = true; return true; }
						fail("bad literal");
						return false;
					case 'f':
						if (literal("false")) { out.kind = JsonValue::Kind::Bool; out.boolean = false; return true; }
						fail("bad literal");
						return false;
					case 'n':
						if (literal("null")) { out.kind = JsonValue::Kind::Null; return true; }
						fail("bad literal");
						return false;
					default:
						return parse_number(out);
				}
			}

			bool parse_array(JsonValue& out)
			{
				++pos; // '['
				out.kind = JsonValue::Kind::Array;
				if (consume(']'))
				{
					return true;
				}
				while (true)
				{
					JsonValue element;
					if (!parse_value(element))
					{
						return false;
					}
					out.array.push_back(std::move(element));
					if (consume(']'))
					{
						return true;
					}
					if (!consume(','))
					{
						fail("expected ',' or ']' in array");
						return false;
					}
				}
			}

			bool parse_object(JsonValue& out)
			{
				++pos; // '{'
				out.kind = JsonValue::Kind::Object;
				if (consume('}'))
				{
					return true;
				}
				while (true)
				{
					skip_ws();
					if (pos >= text.size() || text[pos] != '"')
					{
						fail("expected '\"' key in object");
						return false;
					}
					std::string key;
					if (!parse_string(key))
					{
						return false;
					}
					if (!consume(':'))
					{
						fail("expected ':' after key");
						return false;
					}
					JsonValue value;
					if (!parse_value(value))
					{
						return false;
					}
					out.object.emplace_back(std::move(key), std::move(value));
					if (consume('}'))
					{
						return true;
					}
					if (!consume(','))
					{
						fail("expected ',' or '}' in object");
						return false;
					}
				}
			}
		};

	} // namespace

	bool json_parse(const std::string& text, JsonValue& out, std::string* error)
	{
		Parser parser(text, error);
		JsonValue result;
		if (!parser.parse_value(result))
		{
			return false;
		}
		parser.skip_ws();
		if (parser.pos != text.size())
		{
			parser.fail("trailing content after the document");
			return false;
		}
		out = std::move(result);
		return true;
	}

} // namespace lc
