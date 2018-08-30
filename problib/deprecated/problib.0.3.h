#ifndef _PROBLIB_H_
#define _PROBLIB_H_

#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <cassert>
#include <vector>
#include <deque>
#include <stdexcept>
#undef max
#undef min
typedef long long int64;

namespace problib
{
	template<typename T> const std::string to_string(const T &value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	template<typename T> T convert(const char *s)
	{
		std::istringstream iss(s);
		T res;
		iss >> res;
		return res;
	}

	template<> const char* convert(const char *s)
	{
		return s;
	}

	template<typename T> T convert(const std::string &s)
	{
		return convert<T>(s.c_str());
	}

	// if value < min, returns min. if value > max returns max. else returns value.
	template<typename T> T to_range(T min, T max, T value)
	{
		return std::min<T>(max, std::max<T>(min, value));
	}

	template<typename T> T pow(T x, T y)
	{
		if (!y) return 1;
		T p = Pow(x, y >> 1);
		p *= p;
		if (y & 1) p *= x;
		return p;
	}

	void split(
		const std::string &s,
		const std::string &splitter,
		std::vector<std::string> &res,
		bool includeEmpty = false
	)
	{
		assert(!splitter.empty());
		if (s.size() < splitter.size()) return;
		size_t from = 0;
		for (size_t i = 0; i < s.size() - splitter.size() + 1; ++i)
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

	template<typename TPred>
	void split_cond(
		const std::string &s,
		TPred pred,
		std::vector<std::string> &res,
		bool includeEmpty = false
	)
	{
		size_t from = 0;
		for (size_t i = 0; i < s.size(); ++i)
		{
			if (pred(s[i]))
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


	// struct for integer range [from; to] and for floating range [from; to) 
	template<typename T> struct range
	{
		typedef T value_type;
		value_type from;
		value_type to;

		range() {}
		range(value_type from, value_type to) : from(from), to(to) {}

		value_type length() const;

#ifdef _TESTLIB_H_
		// returns random value in [form; to].
		// for floating range: [from; to)
		value_type get_rnd() const { return rnd.next(from, to); }

		// returns random value in intersection of [form; to] and [minVal; maxVal].
		// for floating range: [form; to) and [minVal; maxVal)
		value_type get_rnd_in_range(value_type minVal, value_type maxVal) const
		{
			return rnd.next(
				to_range(minVal, maxVal, from),
				to_range(minVal, maxVal, to)
			);
		}
#endif
	};

	template<typename T> T range<T>::length() const { return to - from + 1; }
	template<> double range<double>::length() const { return to - from; }
	template<> float range<float>::length() const { return to - from; }

	template<typename T> range<T> make_range(T from, T to)
	{
		return range<T>(from, to);
	}

	struct rnd_range_opts
	{
		enum UniformMode
		{
			UniformByValues,
			UniformByRanges,
		};

		rnd_range_opts() : Mode(UniformByValues) {}
		UniformMode Mode;
	};

	template<typename T>
	class range_array
	{
	public:
		typedef T value_type;
		typedef range<value_type> range_type;

		range_array() {}
		range_array(const rnd_range_opts& opts) : _opts(opts) {}

		void push_back(const range_type& value) { _vals.push_back(value); }
		size_t size() const { return _vals.size(); }

		value_type total_length() const
		{
			value_type result = 0;
			for (size_t i = 0; i < _vals.size(); i++)
			{
				result += _vals[i].length();
			}
			return result;
		}

		const range_type& operator[](size_t index) const { return _vals[index]; }
		range_type& operator[](size_t index) { return _vals[index]; }

		std::vector<value_type> get_all_values() const
		{
			std::vector<value_type> result;
			for (size_t ri = 0; ri < size(); ++ri)
			{
				for (value_type current = _vals[ri].from; current <= _vals[ri].to; ++current)
				{
					result.push_back(current);
				}
			}
			return result;
		}

#ifdef _TESTLIB_H_
		// returns random value in range array
		value_type get_rnd() const;

		// returns random value in array of intersections with [minVal, maxVal]
		value_type get_rnd_in_range(value_type minVal, value_type maxVal) const;
#endif

	private:
		rnd_range_opts _opts;
		std::vector<range_type> _vals;

		value_type _get_rnd_uniform_by_values() const;
		value_type _get_rnd_in_range_uniform_by_values(value_type minVal, value_type maxVal) const;
		value_type _get_rnd_uniform_by_ranges() const;
		value_type _get_rnd_in_range_uniform_by_ranges(value_type minVal, value_type maxVal) const;
	};


#pragma region Rnd in range impl

#ifdef _TESTLIB_H_

	template<typename T>
	T range_array<T>::get_rnd() const
	{
		ensuref(!_vals.empty(), "Can not get rnd for empty range array.");
		if (_vals.size() == 1) return _vals[0].get_rnd();
		switch (_opts.Mode)
		{
		case rnd_range_opts::UniformByRanges:
			return _get_rnd_uniform_by_ranges();
		case rnd_range_opts::UniformByValues:
			return _get_rnd_uniform_by_values();
		default:
			ensuref(false, "Invalid range-random mode.");
			throw;
		}
	}

	template<typename T>
	T range_array<T>::get_rnd_in_range(T minVal, T maxVal) const
	{
		ensuref(!_vals.empty(), "Can not get rnd for empty range array.");
		if (_vals.size() == 1) return _vals[0].get_rnd_in_range(minVal, maxVal);
		switch (_opts.Mode)
		{
		case rnd_range_opts::UniformByRanges:
			return _get_rnd_in_range_uniform_by_ranges(minVal, maxVal);
		case rnd_range_opts::UniformByValues:
			return _get_rnd_in_range_uniform_by_values(minVal, maxVal);
		default:
			ensuref(false, "Invalid range-random mode.");
			throw;
		}
	}

	template <typename T>
	T range_array<T>::_get_rnd_uniform_by_ranges() const
	{
		return _vals[rnd.next(_vals.size())].get_rnd();
	}

	template <typename T>
	T range_array<T>::_get_rnd_in_range_uniform_by_ranges(value_type minVal, value_type maxVal) const
	{
		return _vals[rnd.next(_vals.size())].get_rnd_in_range(minVal, maxVal);
	}

	template <typename T>
	T range_array<T>::_get_rnd_uniform_by_values() const
	{
		value_type length = total_length();
		value_type index = rnd.next(length);

		for (size_t i = _vals.size() - 1; i >= 0; --i)
		{
			length -= _vals[i].length();
			if (index >= length)
			{
				return _vals[i].from + (index - length);
			}
		}
		return _vals[0].from;
	}

	template <typename T>
	T range_array<T>::_get_rnd_in_range_uniform_by_values(value_type minVal, value_type maxVal) const
	{
		range_array<value_type> narr(_opts);
		for (size_t i = 0; i < _vals.size(); i++)
		{
			const range_type& current = _vals[i];
			value_type from = std::max(minVal, current.from);
			value_type to = std::min(maxVal, current.to);
			if (to < from) continue;
			narr.push_back(make_range(from, to));
		}
		if (narr.size() == 0)
		{
			value_type value = _vals[0].from < minVal ? minVal : maxVal;
			return rnd.next(value, value);
		}
		return narr._get_rnd_uniform_by_values();
	}

#endif

#pragma endregion

	class ArgumentValue
	{
	private:
		static const char SEPARATOR = ',';
		std::string _rawValue;
		size_t _hash;

		static size_t get_hash(const std::string &value)
		{
			static const size_t sz = sizeof(size_t);
#if sz == 8
			const size_t _FNV_offset_basis = 14695981039346656037ULL;
			const size_t _FNV_prime = 1099511628211ULL;
#else if sz == 4
			const size_t _FNV_offset_basis = 2166136261U;
			const size_t _FNV_prime = 16777619U;
#endif

			size_t _Val = _FNV_offset_basis;
			for (size_t _Next = 0; _Next < value.size(); ++_Next)
			{	// fold in another byte
				_Val ^= (size_t)value[_Next];
				_Val *= _FNV_prime;
			}

#if sz == 8
			_Val ^= _Val >> 32;
#endif
			return (_Val);
		}

	public:
		ArgumentValue(const std::string &value) : _rawValue(value), _hash(get_hash(value)) {}

		template<typename T>
		T get() const { return convert<T>(_rawValue); }


		template<typename T> const range_array<T>& get_ranges() const;

		template <typename T>
		T get_in_range(T minVal, T maxVal) const
		{
			return to_range(minVal, maxVal, get<T>());
		}

		// returns random value in range array
		template <typename T>
		T get_rnd() const
		{
			return get_ranges<T>().get_rnd();
		}

		// returns random value in array of intersections with [minVal, maxVal]
		template <typename T>
		T get_rnd_in_range(T minVal, T maxVal) const
		{
			return get_ranges<T>().get_rnd_in_range(minVal, maxVal);
		}

#ifdef _TESTLIB_H_
		std::string get_rnd_from_pattern() const
		{
			return rnd.next(_rawValue);
		}
#endif

	};

	template<> std::string ArgumentValue::get() const { return _rawValue; }

	namespace impl
	{
		template<typename T> T normalize_range_bound(const T& value, char b)
		{
			if (b == '(') return value + 1;
			if (b == ')') return value - 1;
			return value;
		}

		template<> double normalize_range_bound(const double& value, char b)
		{
			return value;
		}
		template<> float normalize_range_bound(const float& value, char b)
		{
			return value;
		}
		template<> std::string normalize_range_bound<std::string>(const std::string& value, char b)
		{
			return value;
		}

	}


	template<typename T> const range_array<T>& ArgumentValue::get_ranges() const
	{
		typedef std::deque< std::pair< std::string, range_array<T> > > cache_t;
		static const size_t HASH_SIZE = 23;
		static std::vector< cache_t > _cache(HASH_SIZE);

		if (_rawValue.empty())
		{
			throw std::runtime_error("Can not get range from empty argument value.");
		}

		cache_t & cache = _cache[_hash % HASH_SIZE];
		for (size_t i = 0; i < cache.size(); ++i)
		{
			if (cache[i].first == _rawValue) return cache[i].second;
		}


		std::string::const_iterator rbeg = _rawValue.begin();
		std::string::const_iterator rend = _rawValue.end();

		rnd_range_opts opts;
		opts.Mode = rnd_range_opts::UniformByValues;

		if ((rend - rbeg) > 0 && *rbeg == '!')
		{
			++rbeg;
			opts.Mode = rnd_range_opts::UniformByRanges;
		}

		if ((rend - rbeg) >= 2 && *rbeg == '{' && *(rend - 1) == '}')
		{
			++rbeg; --rend;
		}

		cache.push_back(std::make_pair(_rawValue, range_array<T>(opts)));
		range_array<T>& result = cache.back().second;

		std::vector<std::string> parts;
		static const std::string separatorStr(1, SEPARATOR);
		split(std::string(rbeg, rend), separatorStr, parts);

		for (size_t i = 0; i < parts.size(); ++i)
		{
			bool isRange = false;
			char open, close;
			if (i + 1 < parts.size())
			{
				open = parts[i][0];
				close = parts[i + 1][parts[i + 1].size() - 1];
				isRange = (open == '(' || open == '[') && (close == ')' || close == ']');
			}

			if (isRange)
			{
				T from = convert<T>(parts[i].substr(1));
				T to = convert<T>(parts[i + 1].substr(0, parts[i + 1].size() - 1));

				result.push_back(make_range(impl::normalize_range_bound(from, open), impl::normalize_range_bound(to, close)));
				++i;
			}
			else
			{
				T current = convert<T>(parts[i]);
				result.push_back(make_range(current, current));
			}
		}

		return result;
	}

	namespace impl
	{
		class ArgumentsDictionary
		{
			std::map<std::string, ArgumentValue> _dict;

		public:
			void init(const std::vector<std::string>& args)
			{
				_dict.clear();
				for (size_t i = 0; i < args.size(); ++i)
				{
					const std::string &str = args[i];
					size_t eqind = str.find_first_of('=');

					std::string key, value;
					if (eqind == std::string::npos)
					{
						key = str;
						value = "";
					}
					else
					{
						key = str.substr(0, eqind);
						value = str.substr(eqind + 1);
					}
					_dict.insert(std::make_pair(key, ArgumentValue(value)));
				}
			}

			void init(int argc, char* argv[])
			{
				std::vector<std::string> args;
				for (int i = 1; i < argc; i++)
				{
					args.push_back(std::string(argv[i]));
				}
				init(args);
			}

			void init(const std::string &argsStr)
			{
				std::vector<std::string> args;
				split_cond(argsStr, static_cast<int(*)(int)>(&std::isspace), args);
				init(args);
			}

#ifdef _TESTLIB_H_
			void initRegisterGen(int argc, char* argv[])
			{
				registerGen(argc, argv, 1);
				init(argc, argv);
			}

			void initRegisterGen(int argc, char* argv[], std::vector<std::string>& args)
			{
				std::vector<char*> nargv(args.size() + argc);
				for (int i = 0; i < (int)nargv.size(); ++i)
				{
					nargv[i] = i < argc ? argv[i] : &args[i - argc][0];
				}
				initRegisterGen(nargv.size(), &nargv[0]);
			}

			void initRegisterGen(int argc, char* argv[], const std::string& args)
			{
				std::vector<std::string> vargs;
				split_cond(args, static_cast<int(*)(int)>(&std::isspace), vargs);
				initRegisterGen(argc, argv, vargs);
			}
#endif

			const ArgumentValue& operator[](const std::string &key) const
			{
				if (has(key)) return _dict.find(key)->second;
				throw std::runtime_error("argument not found");
			}

			bool has(const std::string &key) const
			{
				return _dict.find(key) != _dict.end();
			}

		};
	}

	impl::ArgumentsDictionary args;
}

#endif