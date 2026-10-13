// LAST CALL - shared helpers for the engine-independent unit tests.
//
// Plain header so every test suite (pour solver, customer reaction, ...)
// links the same tiny harness without a test framework dependency.
// License: MIT (LICENSE).

#pragma once

#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace lc_test {

	inline int& failures()
	{
		static int value = 0;
		return value;
	}

	inline int& checks()
	{
		static int value = 0;
		return value;
	}

	inline void expect(bool condition, const char* what)
	{
		++checks();
		if (!condition)
		{
			++failures();
			std::printf("FAIL: %s\n", what);
		}
	}

	inline void expect_near(double actual, double expected, double tolerance, const char* what)
	{
		++checks();
		if (std::fabs(actual - expected) > tolerance)
		{
			++failures();
			std::printf("FAIL: %s (actual %.4f, expected %.4f)\n", what, actual, expected);
		}
	}

	// Walk up to the repository root (marked by LastCall.uproject).
	inline std::string repo_root()
	{
		std::string dir = ".";
		for (int depth = 0; depth < 8; ++depth)
		{
			std::ifstream probe(dir + "/LastCall.uproject");
			if (probe.good())
			{
				return dir;
			}
			dir += "/..";
		}
		return ".";
	}

	// Pull one numeric value out of Data/Balance/core.json without a JSON
	// library: enough for the tuning-sync checks in the tests.
	inline double balance_number(const std::string& key)
	{
		const std::string path = repo_root() + "/Data/Balance/core.json";
		std::ifstream file(path);
		if (!file)
		{
			return -1.0;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		const std::string text = buffer.str();
		const std::string needle = "\"" + key + "\"";
		const std::size_t at = text.find(needle);
		if (at == std::string::npos)
		{
			return -1.0;
		}
		const std::size_t colon = text.find(':', at);
		if (colon == std::string::npos)
		{
			return -1.0;
		}
		try
		{
			return std::stod(text.substr(colon + 1));
		}
		catch (...)
		{
			return -1.0;
		}
	}

	// Print the suite summary; returns the process exit code.
	inline int report(const char* suite)
	{
		std::printf("%s: %d checks, %d failure(s)\n", suite, checks(), failures());
		return failures() == 0 ? 0 : 1;
	}

} // namespace lc_test
