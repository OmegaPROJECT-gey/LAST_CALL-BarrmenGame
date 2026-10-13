// LAST CALL - minimal JSON reader for the engine-independent cores.
//
// The cores must run against the real files in Data/ without Unreal (which
// ships its own JSON modules on the UE side). This is a small strict
// recursive-descent parser: the full JSON grammar, no extensions, no
// comments, order-preserving objects, and a clear refusal with a byte
// offset on anything malformed.
//
// Strings are handled as byte sequences: data files are UTF-8, so Cyrillic
// text passes through untouched, and \uXXXX escapes are encoded to UTF-8.
//
// License: MIT (LICENSE).

#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace lc {

	struct JsonValue
	{
		enum class Kind { Null, Bool, Number, String, Array, Object };

		Kind kind = Kind::Null;
		bool boolean = false;
		double number = 0.0;
		std::string string;
		std::vector<JsonValue> array;
		std::vector<std::pair<std::string, JsonValue>> object; // in file order

		/** Object member by key, or nullptr. */
		const JsonValue* find(const std::string& key) const;

		/** Array/object element count (0 for scalars). */
		std::size_t size() const;

		/** Convenience typed member readers; false when missing or mistyped. */
		bool get_number(const std::string& key, double& out) const;
		bool get_int(const std::string& key, int& out) const;
		bool get_bool(const std::string& key, bool& out) const;
		bool get_string(const std::string& key, std::string& out) const;

		/** True when the array (or object) contains `value` as a string. */
		bool array_contains(const std::string& value) const;
	};

	/**
	 * Parse a whole document. Returns false and, if `error` is given, fills
	 * it with "<byte offset>: <reason>". Trailing whitespace is allowed,
	 * trailing content is not.
	 */
	bool json_parse(const std::string& text, JsonValue& out, std::string* error = nullptr);

} // namespace lc
