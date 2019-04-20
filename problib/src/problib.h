#ifndef _PROBLIB_H_
#define _PROBLIB_H_
#define _PROBLIB_H_VER_ "0.10"

#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <sstream>
#include <cctype>
#include <cassert>
#include <vector>
#include <numeric>
#include <stdexcept>
#undef max
#undef min

namespace problib
{
	namespace str
	{
		template<typename T>
		const std::string to_string(const T& value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		template<typename T>
		bool try_parse(const std::string& s, T& val)
		{
			auto iss = std::istringstream(s);
			iss >> val;
			return !!iss;
		}

		template<typename T>
		bool try_parse(std::string_view s, T& val)
		{
			return try_parse(std::string(s), val);
		}

		template<typename T>
		T parse(const std::string& s)
		{
			T val;
			if (!try_parse(s, val))
			{
				throw std::invalid_argument("parsing failed");
			}
			return val;
		}

		template<typename T>
		T parse(std::string_view s)
		{
			return parse<T>(std::string(s));
		}



		std::vector<std::string_view> split(
			std::string_view s,
			std::string_view splitter,
			bool includeEmpty = false
		)
		{
			std::vector<std::string_view> res;
			assert(!splitter.empty());
			if (s.size() < splitter.size()) return res;

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
			return res;
		}

		namespace impl
		{
			struct char_or_sv
			{
				std::string_view sv;
				operator std::string_view() const { return sv; }
				operator char() const { return sv.front(); }
			};
		}

		template<typename TPred>
		std::vector<std::string_view> split_if(
			std::string_view s,
			TPred pred,
			bool includeEmpty = false
		)
		{
			std::vector<std::string_view> res;
			size_t from = 0;
			for (size_t i = 0; i < s.size(); ++i)
			{
				if (pred(impl::char_or_sv{ s.substr(i) }))
				{
					if (from < i || includeEmpty)
					{
						res.push_back(s.substr(from, i - from));
					}
					from = i + 1;
				}
			}
			if (from < s.size()) res.push_back(s.substr(from));
			return res;
		}

		std::string_view trim(std::string_view str)
		{
			while (!str.empty() && isspace(str.front())) str.remove_prefix(1);
			while (!str.empty() && isspace(str.back())) str.remove_suffix(1);
			return str;
		}

		bool starts_with(std::string_view str, std::string_view prefix)
		{
			return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
		}

		bool ends_with(std::string_view str, std::string_view suffix)
		{
			return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
		}

		bool enclosed_with(std::string_view str, std::string_view prefix, std::string_view suffix)
		{
			return str.size() >= prefix.size() + suffix.size() && starts_with(str, prefix) && ends_with(str, suffix);
		}


		bool try_remove_prefix(std::string_view & str, std::string_view prefix)
		{
			if (starts_with(str, prefix))
			{
				str.remove_prefix(prefix.size());
				return true;
			}
			return false;
		}

		bool try_remove_suffix(std::string_view & str, std::string_view suffix)
		{
			if (ends_with(str, suffix))
			{
				str.remove_suffix(suffix.size());
				return true;
			}
			return false;
		}

		bool try_remove_enclosed(std::string_view & str, std::string_view prefix, std::string_view suffix)
		{
			if (enclosed_with(str, prefix, suffix))
			{
				str.remove_prefix(prefix.size());
				str.remove_suffix(suffix.size());
				return true;
			}
			return false;
		}
	}


	// if value < lowerBound, returns lowerBound.
	// if value > upperBound returns upperBound.
	// else returns value.
	template<typename T>
	T bound_value(const T & lowerBound, const T & upperBound, const T & value)
	{
		return std::min<T>(upperBound, std::max<T>(lowerBound, value));
	}

	template<typename T, typename U>
	std::enable_if_t < std::is_integral<T>::value, T > pow(T x, U y)
	{
		if (!y) return 1;
		T p = Pow(x, y >> 1);
		p *= p;
		if (y & 1) p *= x;
		return p;
	}

	template<typename T, typename U>
	std::enable_if_t < std::is_integral<T>::value, T > pow_mod(T x, U y, T m)
	{
		if (!y) return 1;
		T p = Pow(x, y >> 1);
		p = p * p % m;
		if (y & 1) p = p * x % m;
		return p;
	}

	namespace impl
	{
		template<typename TIt>
		struct collection_printer
		{
			TIt first;
			TIt last;
			std::string_view separator;
		};


		template<typename TIt>
		std::ostream& operator <<(std::ostream& out, const collection_printer<TIt>& printer)
		{
			auto cur = printer.first;
			if (cur == printer.last) return out;
			out << *cur;
			for (++cur; cur != printer.last; ++cur) out << printer.separator << *cur;
			return out;
		}

	}

	template<typename TIt>
	auto make_printer(TIt first, TIt last, std::string_view separator = " ")
	{
		return impl::collection_printer<TIt> { first, last, separator };
	}

	template<typename TContainer>
	auto make_printer(const TContainer & c, std::string_view separator = " ")
	{
		using std::begin;
		using std::end;
		return make_printer(begin(c), end(c), separator);
	}

#pragma region ranges

	// struct for integer range [from; to] and for floating range [from; to) 
	template<typename T>
	struct range
	{
		using value_type = T;
		value_type from;
		value_type to;

		range() = default;
		range(value_type fromto) : from(fromto), to(fromto) {}
		range(value_type from, value_type to) : from(from), to(to) {}

		value_type length() const;

#ifdef _TESTLIB_H_
		// returns random value in [form; to].
		// for floating range: [from; to)
		value_type get_rnd() const { return rnd.next(from, to); }

		// returns random value in intersection of [form; to] and [lowerBound; upperBound].
		// for floating range: [form; to) and [lowerBound; upperBound)
		value_type get_rnd(value_type lowerBound, value_type upperBound) const
		{
			assert(lowerBound <= upperBound);
			return rnd.next(
				bound_value(lowerBound, upperBound, from),
				bound_value(lowerBound, upperBound, to)
			);
		}
#endif
	};

	template<typename T> T range<T>::length() const { return to - from + 1; }
	template<> double range<double>::length() const { return to - from; }
	template<> float range<float>::length() const { return to - from; }

	template<typename T>
	range<T> make_range(T from, T to)
	{
		return range<T>(from, to);
	}

	enum class range_random_mode
	{
		uniform_by_length,
		uniform_by_count,
	};

	struct range_array_opts
	{
		range_random_mode random_mode = range_random_mode::uniform_by_length;
	};

	template<typename T>
	class range_array
	{
	public:
		using value_type = T;
		using range_type = range<value_type>;

		range_array() = default;
		range_array(const range_type& range) : _vals(1, range) { }
		explicit range_array(const range_array_opts& opts) : _opts(opts) {}

		range_array_opts& options() { return _opts; }
		const range_array_opts& options() const { return _opts; }

		void push_back(const range_type& value) { _vals.push_back(value); }
		size_t size() const { return _vals.size(); }

		value_type total_length() const
		{
			return std::accumulate(
				_vals.begin(),
				_vals.end(),
				value_type(0),
				[](value_type current, const range_type & val) { return current + val.length(); }
			);
		}

		const range_type& operator[](size_t index) const { return _vals[index]; }
		range_type& operator[](size_t index) { return _vals[index]; }

		template<typename = std::enable_if_t < std::is_integral<value_type>{} >>
			std::vector<value_type> get_all_values() const
		{
			auto result = std::vector<value_type>();
			for (const auto& val : _vals)
			{
				for (value_type current = val.from; current <= val.to; ++current)
				{
					result.push_back(current);
				}
			}
			return result;
		}

#ifdef _TESTLIB_H_
		// returns random value in range array
		value_type get_rnd() const;

		// returns random value in array of intersections with [lowerBound, upperBound]
		value_type get_rnd(value_type lowerBound, value_type upperBound) const;
#endif

	private:
		range_array_opts _opts;
		std::vector<range_type> _vals;

		value_type _get_rnd_uniform_by_values() const;
		value_type _get_rnd_in_range_uniform_by_values(value_type minVal, value_type maxVal) const;
		value_type _get_rnd_uniform_by_count() const;
		value_type _get_rnd_in_range_uniform_by_count(value_type minVal, value_type maxVal) const;
	};


#pragma region Rnd in range impl

#ifdef _TESTLIB_H_

	template<typename T>
	T range_array<T>::get_rnd() const
	{
		ensuref(!_vals.empty(), "Can not get rnd for empty range array.");
		if (_vals.size() == 1) return _vals[0].get_rnd();
		switch (_opts.random_mode)
		{
		case range_random_mode::uniform_by_count:
			return _get_rnd_uniform_by_count();
		case range_random_mode::uniform_by_length:
			return _get_rnd_uniform_by_values();
		default:
			ensuref(false, "Invalid range-random mode.");
			throw;
		}
	}

	template<typename T>
	T range_array<T>::get_rnd(T lowerBound, T upperBound) const
	{
		ensuref(!_vals.empty(), "Can not get rnd for empty range array.");
		if (_vals.size() == 1) return _vals[0].get_rnd(lowerBound, upperBound);
		switch (_opts.random_mode)
		{
		case range_random_mode::uniform_by_count:
			return _get_rnd_in_range_uniform_by_count(lowerBound, upperBound);
		case range_random_mode::uniform_by_length:
			return _get_rnd_in_range_uniform_by_values(lowerBound, upperBound);
		default:
			ensuref(false, "Invalid range-random mode.");
			throw;
		}
	}

	template <typename T>
	T range_array<T>::_get_rnd_uniform_by_count() const
	{
		return _vals[rnd.next(_vals.size())].get_rnd();
	}

	template <typename T>
	T range_array<T>::_get_rnd_in_range_uniform_by_count(value_type minVal, value_type maxVal) const
	{
		return _vals[rnd.next(_vals.size())].get_rnd(minVal, maxVal);
	}

	template <typename T>
	T range_array<T>::_get_rnd_uniform_by_values() const
	{
		value_type length = total_length();
		value_type index = rnd.next(length);

		for (auto i = ptrdiff_t(_vals.size()) - 1; i >= 0; --i)
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

#pragma endregion

	namespace arguments
	{
		struct brackets
		{
			std::string range_open_exclusive = "(";
			std::string range_close_exclusive = ")";
			std::string range_open_inclusive = "[";
			std::string range_close_inclusive = "]";
			std::string rangeset_open = "{";
			std::string rangeset_close = "}";
		};

		struct prefixes
		{
			std::string rnd_mode_uniform_by_count = "!";
		};

		struct parsing_options
		{
			brackets brackets;
			prefixes prefixes;
			std::string items_separator = ",";
		};

		namespace impl_arg_parsing
		{
			template<typename T>
			std::enable_if_t < std::is_integral<T>::value, T > normalize_range_bound(const T& value, int sign)
			{
				if (sign < 0) return value - 1;
				if (sign > 0) return value + 1;
				return value;
			}

			template<typename T>
			std::enable_if_t < !std::is_integral<T>::value, T > normalize_range_bound(const T& value, int sign)
			{
				return value;
			}


			template<typename T>
			std::pair<range<T>, bool> try_parse_range_from_parts(std::string_view left, std::string_view right, const brackets& brackets)
			{
				bool isOpenInclusive = str::try_remove_prefix(left, brackets.range_open_inclusive);
				bool isOpenExclusive = !isOpenInclusive && str::try_remove_prefix(left, brackets.range_open_exclusive);
				bool isCloseInclusive = str::try_remove_suffix(right, brackets.range_close_inclusive);
				bool isCloseExclusive = !isCloseInclusive && str::try_remove_suffix(right, brackets.range_close_exclusive);

				if ((isOpenInclusive || isOpenExclusive) && (isCloseInclusive || isCloseExclusive))
				{
					auto from = str::parse<T>(left);
					auto to = str::parse<T>(right);

					return std::make_pair(make_range(
						normalize_range_bound(from, isOpenInclusive ? 0 : 1),
						normalize_range_bound(to, isCloseInclusive ? 0 : -1)
					), true);
				}

				return std::make_pair(range<T>(), false);
			}

			template<typename T>
			range_array<T> parse_range(std::string_view value, const parsing_options & options)
			{
				if (value.empty())
				{
					throw std::runtime_error("Can not get range from empty argument value.");
				}

				const auto& brackets = options.brackets;

				std::string_view current = value;

				range_array_opts rnd_opts;

				if (str::try_remove_prefix(current, options.prefixes.rnd_mode_uniform_by_count))
				{
					rnd_opts.random_mode = range_random_mode::uniform_by_count;
				}

				str::try_remove_enclosed(current, brackets.rangeset_open, brackets.rangeset_close);

				auto result = range_array<T>(rnd_opts);

				auto parts = str::split(current, options.items_separator);
				for (auto& part : parts)
				{
					part = str::trim(part);
				}

				for (size_t i = 0; i < parts.size(); ++i)
				{
					auto cur = parts[i];
					if (i + 1 < parts.size())
					{
						auto next = parts[i + 1];
						range<T> rng;
						bool isRange;
						std::tie(rng, isRange) = try_parse_range_from_parts<T>(cur, next, brackets);

						if (isRange)
						{
							result.push_back(rng);
							++i;
							continue;
						}
					}

					T current = str::parse<T>(cur);
					result.push_back(make_range(current, current));
				}

				return result;
			}

		}


		class argument_view
		{
		private:
			const std::string* _rawValue;
			const std::string& _rw() const
			{
				if (!_rawValue) throw std::logic_error("value not exists");
				return *_rawValue;
			}

		public:
			argument_view() : _rawValue(nullptr) {}

			explicit argument_view(const std::string& value)
				: _rawValue(&value)
			{}

			explicit operator bool() const
			{
				return _rawValue;
			}

			template<typename T>
			T value() const { return str::parse<T>(_rw()); }

			template<typename T>
			T value_or(const T& default_value) const
			{
				if (_rawValue)
				{
					return str::parse<T>(*_rawValue);
				}
				else
				{
					return default_value;
				}
			}

			template<typename T>
			range_array<T> ranges() const { return ranges<T>(parsing_options{}); }

			template<typename T, typename U>
			range_array<T> ranges_or(U&& default_value) const
			{
				return ranges_or<T>(std::forward<U>(default_value), parsing_options{});
			}

			template<typename T>
			range_array<T> ranges(const parsing_options& opts) const
			{
				return impl_arg_parsing::parse_range<T>(_rw(), opts);
			}

			template<typename T>
			range_array<T> ranges_or(std::string_view default_value, const parsing_options& opts) const
			{
				if (_rawValue)
				{
					return impl_arg_parsing::parse_range<T>(*_rawValue, opts);
				}
				else
				{
					return impl_arg_parsing::parse_range<T>(default_value, opts);
				}
			}

			template<typename T, typename U>
			std::enable_if_t<std::is_convertible_v<U&&, range_array<T>> || std::is_convertible_v<U&&, range<T>>, range_array<T>>
				ranges_or(U&& default_value, const parsing_options& opts) const
			{
				if (_rawValue)
				{
					return impl_arg_parsing::parse_range<T>(*_rawValue, opts);
				}
				else
				{
					return  range_array<T>(std::forward<U>(default_value));
				}
			}

			template <typename T>
			T bounded_value(const T& lowerBound, const T& upperBound) const
			{
				return bound_value(lowerBound, upperBound, value<T>());
			}

			template <typename T>
			T bounded_value_or(const T& lowerBound, const T& upperBound, const T& default_value) const
			{
				return bound_value(lowerBound, upperBound, value_or<T>(default_value));
			}

#ifdef _TESTLIB_H_

			std::string rnd_from_pattern() const
			{
				return rnd.next(_rw());
			}
#endif

		};

		template<> std::string argument_view::value() const { return _rw(); }

		class arguments_dictionary
		{
		public:
			using map_type = std::map<std::string, std::string, std::less<>>;

		private:
			map_type _dict;

		public:
			void reset(map_type&& values)
			{
				_dict = std::move(values);
			}

			argument_view operator[](const std::string_view& key) const
			{
				auto it = _dict.find(key);
				if (it != _dict.end())
				{
					return argument_view(it->second);
				}
				else
				{
					return argument_view();
				}
			}

			bool has(const std::string_view& key) const
			{
				return _dict.find(key) != _dict.end();
			}
		};

	}

	using arguments::argument_view;
	using arguments::arguments_dictionary;

	namespace impl
	{
		using args_t = const char* const [];

		std::vector<std::string_view> split_args(std::string_view args)
		{
			return str::split_if(args, &isspace);
		}

		std::pair<std::string_view, std::string_view> parse_arg(std::string_view key_value)
		{
			size_t eqind = key_value.find_first_of('=');

			if (eqind == std::string::npos)
			{
				return std::make_pair(key_value, std::string_view(""));
			}
			else
			{
				return std::make_pair(
					key_value.substr(0, eqind),
					key_value.substr(eqind + 1)
				);
			}
		}

		arguments_dictionary::map_type make_args_map(const std::vector<std::string_view> & args)
		{
			arguments_dictionary::map_type values;
			for (size_t i = 0; i < args.size(); ++i)
			{
				auto str = args[i];
				std::string_view key, value;
				std::tie(key, value) = parse_arg(str);
				values.emplace(key, value);
			}
			return values;
		}

		class generator_root : public arguments_dictionary
		{
		public:
			void init(const std::vector<std::string_view>& args)
			{
				reset(make_args_map(args));
			}

			void init(int argc, args_t argv)
			{
				auto args = std::vector<std::string_view>(argv + 1, argv + argc);
				init(args);
			}

			void init(std::string_view args)
			{
				init(split_args(args));
			}

#ifdef _TESTLIB_H_
			void initRegisterGen(int argc, args_t argv)
			{
				registerGen(argc, const_cast<char**>(argv), 1);
				init(argc, argv);
			}

			void initRegisterGen(int argc, args_t argv, const std::vector<std::string> & args)
			{
				std::vector<const char*> nargv(argv, argv + argc);
				std::transform(args.begin(), args.end(), std::back_inserter(nargv), [](const std::string & item) { return item.c_str(); });
				initRegisterGen(nargv.size(), nargv.data());
			}

			void initRegisterGen(int argc, args_t argv, std::string_view args)
			{
				auto splitted = split_args(args);
				initRegisterGen(argc, argv, std::vector<std::string>(splitted.begin(), splitted.end()));
			}
#endif
		};
	}

	impl::generator_root args;
}

#endif