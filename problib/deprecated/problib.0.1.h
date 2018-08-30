#ifndef _PROBLIB_H_
#define _PROBLIB_H_

#include <map>
#include <string>
#include <sstream>
#include <cctype>

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


const std::string FIRST_SUF = "_1";
const std::string SECND_SUF = "_2";

class __argsDict
{
	std::map<std::string, std::string> _dict;

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

			if (!value.empty() && value[0] == '[')
			{
				value = value.substr(1, value.size() - 2);
				int scind = value.find(';');
				_dict[key + FIRST_SUF] = value.substr(0, scind);
				_dict[key + SECND_SUF] = value.substr(scind + 1);
			}
			else
			{
				_dict[key] = value;
			}
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

#ifdef _TESTLIB_H_
	void initRegisterGen(int argc, char* argv[])
	{
		registerGen(argc, argv, 1);
		init(argc, argv);
	}
#endif

	void initDebug(const std::string &argsStr)
	{
		std::vector<std::string> args;
		int from = 0;
		for (int i = 0; i < argsStr.size(); i++)
		{
			if (std::isspace(argsStr[i]))
			{
				if (from < i)
				{
					args.push_back(argsStr.substr(from, i - from));
				}
				from = i + 1;
			}
		}
		if (from < argsStr.size())
		{
			args.push_back(argsStr.substr(from));
		}
		init(args);
	}

	const std::string& operator[](const std::string &key) const
	{
		if (has(key)) return _dict.find(key)->second;
		if (key.size() >= 2 && (key.substr(key.size() - 2) == FIRST_SUF
			|| key.substr(key.size() - 2) == SECND_SUF))
			return _dict.find(key.substr(0, key.size() - 2))->second;
		throw "argument not found";
	}

	bool has(const std::string &key) const
	{
		return _dict.find(key) != _dict.end() ||
			_dict.find(key + FIRST_SUF) != _dict.end() && _dict.find(key + SECND_SUF) != _dict.end();
	}

	template <typename T>
	T get(const std::string &key) const
	{
		return Convert<T>(this->operator[](key));
	}
	template <typename T>
	T getFirst(const std::string &key) const
	{
		return get<T>(key + FIRST_SUF);
	}
	template <typename T>
	T getSecond(const std::string &key) const
	{
		return get<T>(key + SECND_SUF);
	}
	template <typename T>
	std::pair<T, T> getPair(const std::string &key) const
	{
		return std::make_pair(getFirst<T>(key), getSecond<T>(key));
	}

	template <typename T>
	T getInRange(const std::string &key, T minVal, T maxVal) const
	{
		return ToRange(minVal, maxVal, get<T>(key));
	}

#ifdef _TESTLIB_H_

	template <typename T>
	T getRnd(const std::string &key) const
	{
		return rnd.next(getFirst<T>(key), getSecond<T>(key));
	}

	template <typename T>
	T getRndInRange(const std::string &key, T minVal, T maxVal) const
	{
		return rnd.next(
			ToRange(minVal, maxVal, getFirst<T>(key)),
			ToRange(minVal, maxVal, getSecond<T>(key))
			);
	}

#endif

} args;

#endif