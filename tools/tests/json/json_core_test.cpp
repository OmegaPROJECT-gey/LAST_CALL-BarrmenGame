// LAST CALL - minimal JSON reader unit tests.
//
// Runs without Unreal: scripts/test-json.sh compiles this against
// Source/LastCall/Core/JsonCore.cpp with g++.

#include "Core/JsonCore.h"

#include "test_support.h"

#include <fstream>
#include <sstream>

namespace {

	using lc_test::expect;
	using lc_test::expect_near;

	using lc::JsonValue;
	using lc::json_parse;

	bool refuses(const std::string& text)
	{
		JsonValue scratch;
		std::string error;
		return !json_parse(text, scratch, &error) && !error.empty();
	}

} // namespace

int main()
{
	// --- scalars ------------------------------------------------------------------
	{
		JsonValue value;
		expect(json_parse("42", value) && value.kind == JsonValue::Kind::Number
			&& value.number == 42.0, "integer");
		expect(json_parse("-3.5", value) && value.number == -3.5, "negative fraction");
		expect(json_parse("1e3", value) && value.number == 1000.0, "exponent");
		expect(json_parse("0.5E-2", value) && value.number == 0.005, "negative exponent");
		expect(json_parse("true", value) && value.kind == JsonValue::Kind::Bool
			&& value.boolean, "true");
		expect(json_parse("false", value) && !value.boolean, "false");
		expect(json_parse("null", value) && value.kind == JsonValue::Kind::Null, "null");
		expect(json_parse(" \t\r\n 7 ", value) && value.number == 7.0, "whitespace around values");
	}

	// --- strings --------------------------------------------------------------------
	{
		JsonValue value;
		expect(json_parse("\"a\\\"b\\\\c\\/d\\n\\t\"", value)
			&& value.string == "a\"b\\c/d\n\t", "escapes decode");
	}
	{
		JsonValue value;
		expect(json_parse("\"\\u0414\\u0430\"", value) && value.string == "\xD0\x94\xD0\xB0",
			"\\u escapes encode to UTF-8");
	}
	{
		JsonValue value;
		expect(json_parse("\"\\uD83D\\uDE00\"", value)
			&& value.string == "\xF0\x9F\x98\x80", "surrogate pairs combine");
	}
	{
		// Data files are UTF-8 on disk: Cyrillic passes through as bytes.
		JsonValue value;
		expect(json_parse("\"\xD0\xA4\xD0\xB8\xD1\x80\xD0\xBC\xD0\xB5\xD0\xBD\xD0\xBD\xD1\x8B\xD0\xB9\"", value)
			&& value.string == "\xD0\xA4\xD0\xB8\xD1\x80\xD0\xBC\xD0\xB5\xD0\xBD\xD0\xBD\xD1\x8B\xD0\xB9",
			"raw UTF-8 passes through untouched");
	}

	// --- structures -------------------------------------------------------------------
	{
		JsonValue doc;
		const std::string text =
			"{\n"
			"  \"id\": \"signature_noir\",\n"
			"  \"steps\": [ { \"ing\": \"whiskey_rye\", \"w\": [0.35, 0.75] }, { \"ing\": \"oil\" } ],\n"
			"  \"continuity\": 4,\n"
			"  \"story\": null,\n"
			"  \"act1_hook\": true\n"
			"}";
		expect(json_parse(text, doc), "nested document parses");
		expect(doc.kind == JsonValue::Kind::Object && doc.size() == 5, "five members");
		std::string id;
		expect(doc.get_string("id", id) && id == "signature_noir", "get_string");
		double continuity = 0.0;
		expect(doc.get_number("continuity", continuity) && continuity == 4.0, "get_number");
		bool hook = false;
		expect(doc.get_bool("act1_hook", hook) && hook, "get_bool");
		expect(doc.find("story") && doc.find("story")->kind == JsonValue::Kind::Null,
			"null member is findable");

		const JsonValue* steps = doc.find("steps");
		expect(steps && steps->kind == JsonValue::Kind::Array && steps->size() == 2,
			"array member with two elements");
		const JsonValue* first = steps ? &steps->array[0] : nullptr;
		expect(first && first->find("ing")->string == "whiskey_rye", "nested object access");
		const JsonValue* window = first ? first->find("w") : nullptr;
		expect(window && window->array.size() == 2 && window->array[1].number == 0.75,
			"nested array access");

		// Order preservation: members come back in file order.
		expect(doc.object[0].first == "id" && doc.object[4].first == "act1_hook",
			"object members keep file order");

		double missing = -1.0;
		expect(!doc.get_number("nope", missing) && missing == -1.0, "missing key reads false");
		int as_int = 0;
		expect(doc.get_int("continuity", as_int) && as_int == 4, "get_int on an integral number");
		expect(!doc.get_int("id", as_int), "get_int refuses a string");
	}

	{
		JsonValue empty_array;
		expect(json_parse("[]", empty_array) && empty_array.size() == 0, "empty array");
		JsonValue empty_object;
		expect(json_parse("{}", empty_object) && empty_object.size() == 0, "empty object");
		JsonValue list;
		expect(json_parse("[\"Shift\", \"Night\"]", list)
			&& list.array_contains("Night") && !list.array_contains("Morning"),
			"array_contains on a string list");
	}

	// --- strict refusals ---------------------------------------------------------------
	expect(refuses(""), "empty input");
	expect(refuses("   "), "whitespace only");
	expect(refuses("{\"a\":}"), "missing value");
	expect(refuses("[1,]"), "trailing comma in array");
	expect(refuses("{\"a\":1,}"), "trailing comma in object");
	expect(refuses("{'a':1}"), "single quotes are not JSON");
	expect(refuses("01"), "leading zero");
	expect(refuses("1 2"), "trailing content");
	expect(refuses("tru"), "truncated literal");
	expect(refuses("\"unterminated"), "unterminated string");
	expect(refuses("nan"), "nan is not JSON");
	expect(refuses("\"bad\\qescape\""), "unknown escape");
	expect(refuses("\"\\uD800\""), "lone high surrogate");
	expect(refuses("\"line\nbreak\""), "raw control character in string");
	expect(refuses("{\"a\" 1}"), "missing colon");
	expect(refuses("[1 2]"), "missing comma in array");
	expect(refuses("}"), "stray closing brace");

	// --- against the real balance file -------------------------------------------------
	{
		const std::string path = lc_test::repo_root() + "/Data/Balance/core.json";
		std::ifstream file(path);
		std::stringstream buffer;
		buffer << file.rdbuf();
		JsonValue doc;
		std::string error;
		expect(json_parse(buffer.str(), doc, &error), "Data/Balance/core.json parses");
		double flow = 0.0;
		const JsonValue* pour = doc.find("pour");
		expect(pour && pour->get_number("flow_ml_per_s", flow) && flow == 60.0,
			"pour.flow_ml_per_s reads through the parser");
	}

	return lc_test::report("json");
}
