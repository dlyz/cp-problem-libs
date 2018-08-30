#ifndef _PROBLIB_H_
#define _PROBLIB_H_

#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <cassert>

typedef long long int64;

template<typename T> const std::string ToString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

template<typename T> T Convert(const char *s)
{
	std::istringstream iss(s);
	T res;
	iss >> res;
	return res;
}

template<typename T> T Convert(const std::string &s)
{
	return Convert<T>(s.c_str());
}

template<typename T> T ToRange(T min, T max, T value)
{
	return std::min<T>(max, std::max<T>(min, value));
}

template<typename T> T Pow(T x, T y)
{
	if (!y) return 1;
	T p = Pow(x, y >> 1);
	p *= p;
	if (y & 1) p *= x;
	return p;
}

template<typename T> int log2up(T x)
{
	int res = 0;
	while (x)
	{
		++res, x >>= 1;
	}
	return res;
}

void SplitString(
	const std::string &s,
	const std::string &splitter,
	std::vector<std::string> &res,
	bool includeEmpty = false
	)
{
	assert(!splitter.empty());
	int from = 0;
	for (int i = 0; i < (int)s.size() - (int)splitter.size() + 1; ++i)
	{
		if (s.substr(i, splitter.size()) == splitter)
		{
			if (from < i || includeEmpty)
			{
				res.push_back(s.substr(from, i - from));
			}
			i += splitter.size() - 1;
			from = i + 1;
		}
	}
	if (from < s.size()) res.push_back(s.substr(from));
}

void SplitString(
	const std::string &s,
	int(*charSplitFunc)(int),
	std::vector<std::string> &res,
	bool includeEmpty = false
	)
{
	int from = 0;
	for (int i = 0; i < (int)s.size(); ++i)
	{
		if (charSplitFunc(s[i]))
		{
			if (from < i || includeEmpty)
			{
				res.push_back(s.substr(from, i - from));
			}
			from = i + 1;
		}
	}
	if (from < s.size()) res.push_back(s.substr(from));
}


class ArgVal
{
private:
	std::string _rawValue;
	typedef std::pair<std::string, std::string> pss;
	std::vector<pss> _vals;

public:
	ArgVal(const std::string &v);

	template<typename T>
	T get() const;

	template <typename T>
	T getInRange(T minVal, T maxVal) const;

#ifdef _TESTLIB_H_

	std::string getRndFromPattern() const;

	template <typename T>
	T getRnd() const;

	template <typename T>
	T getRndInRange(T minVal, T maxVal) const;

	template <typename T>
	T getRndUniform() const;

	template <typename T>
	T getRndUniformInRange(T minVal, T maxVal) const;

#endif

};

#pragma region ArgVal impl

ArgVal::ArgVal(const std::string &v)
{
	_rawValue = v;
	if (_rawValue.empty()) return;
	std::string tv = v.size() >= 2 && v[0] == '{' && v[v.size() - 1] == '}' ? v.substr(1, v.size() - 2) : v;

	std::vector<std::string> parts;
	SplitString(tv, ";", parts);

	for (int i = 0; i < parts.size(); ++i)
	{
		if (parts[i][0] == '[' && i + 1 < parts.size() && parts[i + 1][parts[i + 1].size() - 1] == ']')
		{
			_vals.push_back(pss(parts[i].substr(1), parts[i + 1].substr(0, parts[i + 1].size() - 1)));
			++i;
		}
		else
		{
			_vals.push_back(pss(parts[i], parts[i]));
		}
	}
}

template<typename T> T ArgVal::get() const
{
	return Convert<T>(_rawValue);
}
template<> std::string ArgVal::get() const
{
	return _rawValue;
}
template <typename T>
T ArgVal::getInRange(T minVal, T maxVal) const
{
	return ToRange(minVal, maxVal, get<T>());
}

#ifdef _TESTLIB_H_

std::string ArgVal::getRndFromPattern() const
{
	return rnd.next(_rawValue);
}

template <typename T>
T ArgVal::getRnd() const
{
	pss val = _vals[rnd.next(_vals.size())];
	return rnd.next(Convert<T>(val.first), Convert<T>(val.second));
}

template <typename T>
T ArgVal::getRndInRange(T minVal, T maxVal) const
{
	pss val = _vals[rnd.next(_vals.size())];
	return rnd.next(
		ToRange(minVal, maxVal, Convert<T>(val.first)),
		ToRange(minVal, maxVal, Convert<T>(val.second))
		);
}


template <typename T>
T ArgVal::getRndUniform() const
{
	T length = 0;
	for (int i = 0; i < _vals.size(); ++i)
	{
		length += Convert<T>(_vals[i].second) - Convert<T>(_vals[i].first) + 1;
	}
	T index = rnd.next(length);
	for (int i = _vals.size() - 1; i >= 0; --i)
	{
		length -= Convert<T>(_vals[i].second) - Convert<T>(_vals[i].first) + 1;
		if (length <= index)
		{
			return Convert<T>(_vals[i].first) + (index - length);
		}
	}
	throw "getRndUniform";
}

template <>
double ArgVal::getRndUniform() const
{
	double length = 0;
	for (int i = 0; i < _vals.size(); ++i)
	{
		if (_vals[i].first == _vals[i].second) return getRnd<double>();
		length += Convert<double>(_vals[i].second) - Convert<double>(_vals[i].first);
	}
	double index = rnd.next(length);
	for (int i = _vals.size() - 1; i >= 0; --i)
	{
		length -= Convert<double>(_vals[i].second) - Convert<double>(_vals[i].first);
		if (length <= index)
		{
			return Convert<double>(_vals[i].first) + (index - length);
		}
	}
	return Convert<double>(_vals.front().first);
}

template <>
float ArgVal::getRndUniform() const
{
	return getRndUniform<double>();
}

template <typename T>
T ArgVal::getRndUniformInRange(T minVal, T maxVal) const
{
	return ToRange(minVal, maxVal, getRndUniform<T>());
}
#endif

#pragma endregion

class __argsDict
{
	std::map<std::string, ArgVal> _dict;

	void init(std::vector<std::string> args)
	{
		_dict.clear();
		for (int i = 0; i < args.size(); ++i)
		{
			const std::string &str = args[i];
			int eqind = str.find_first_of('=');

			std::string key, value;
			if (eqind == std::string::npos || eqind + 1 == str.size())
			{
				key = str;
				value = "";
			}
			else
			{
				key = str.substr(0, eqind);
				value = str.substr(eqind + 1);
			}
			_dict.insert(std::pair<std::string, ArgVal>(key, ArgVal(value)));
		}
	}

public:
	void init(int argc, char* argv[])
	{
		std::vector<std::string> args;
		for (int i = 1; i < argc; i++)
		{
			args.push_back(std::string(argv[i]));
		}
		init(args);
	}

	void initDebug(const std::string &argsStr)
	{
		std::vector<std::string> args;
		SplitString(argsStr, std::isspace, args);
		init(args);
	}

#ifdef _TESTLIB_H_
	void initRegisterGen(int argc, char* argv[])
	{
		registerGen(argc, argv, 1);
		init(argc, argv);
	}
#endif

	const ArgVal& operator[](const std::string &key) const
	{
		if (has(key)) return _dict.find(key)->second;
		throw "argument not found";
	}

	bool has(const std::string &key) const
	{
		return _dict.find(key) != _dict.end();
	}

	template <typename T>
	T get(const std::string &key) const
	{
		return this->operator[](key).get<T>();
	}

	template <typename T>
	T getInRange(const std::string &key, T minVal, T maxVal) const
	{
		return this->operator[](key).getInRange<T>(minVal, maxVal);
	}

#ifdef _TESTLIB_H_

	template <typename T>
	T getRnd(const std::string &key) const
	{
		return this->operator[](key).getRnd<T>();
	}

	template <typename T>
	T getRndInRange(const std::string &key, T minVal, T maxVal) const
	{
		return this->operator[](key).getRndInRange(minVal, maxVal);
	}

#endif

} args;

#endif