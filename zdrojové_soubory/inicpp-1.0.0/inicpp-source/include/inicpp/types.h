#ifndef INICPP_TYPES_H
#define INICPP_TYPES_H

#include <stdexcept>
#include <string>
#include <type_traits>
#include <chrono>


namespace inicpp
{
	class internal_enum_type;
	class internal_date_type;
	class internal_locale_type;

	/**
	 * Inicpp enumeration type.
	 */
	class internal_enum_type
	{
	public:
		/** Default constructor */
		internal_enum_type() : internal_enum_type("")
		{
		}
		/** Constructor with initial value */
		internal_enum_type(const std::string &value) : data_(value)
		{
		}
		/** Constructor with initial value */
		internal_enum_type(const char *value) : data_(value)
		{
		}
		/** Copy constructor */
		internal_enum_type(const internal_enum_type &other)
		{
			this->operator=(other);
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_enum_type(bool)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_enum_type(int64_t)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_enum_type(uint64_t)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_enum_type(double)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_enum_type(const internal_date_type &other)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_enum_type(const internal_locale_type &other)
		{
			throw std::runtime_error("");
		}

		/** Assignment operator */
		internal_enum_type &operator=(const internal_enum_type &other)
		{
			data_ = other.data_;
			return *this;
		}
		/** Conversion operator to std::string type */
		operator std::string() const
		{
			return data_;
		}
		/**
		 * Conversion operator to double type - allways throws, implemented
		 * because of some template usage.
		 * @throws allways std::runtime_error exception
		 */
		operator double() const
		{
			throw std::runtime_error("Enum type cannot be converted to double");
		}
		/** Equality operator */
		bool operator==(const internal_enum_type &other) const
		{
			return data_ == other.data_;
		}
		/** Inequality operator */
		bool operator!=(const internal_enum_type &other) const
		{
			return !(*this == other);
		}
		/** Comparation less operator */
		bool operator<(const internal_enum_type &other) const
		{
			return data_ < other.data_;
		}

	private:
		/** Value of instance */
		std::string data_;
	};

	/**
	* Inicpp date type.
	*/
	class internal_date_type
	{
	public:
		/** Formatting string for std::put_date and std::get_date. */
		static constexpr char* DATE_FORMAT_STRING = "%Y-%m-%d %H:%M:%S";

		typedef std::chrono::system_clock clock;
		typedef clock::time_point time_point;

		/** Default constructor */
		internal_date_type()
		{
		}
		/** Constructor with initial value */
		internal_date_type(const time_point &value) : data_(value)
		{
		}
		/** Constructor with initial value, supplied broken-down time is passed by values because mktime modifies its argument */
		internal_date_type(std::tm value) : data_(clock::from_time_t(mktime(&value)))
		{
		}
		/** Copy constructor */
		internal_date_type(const internal_date_type &other)
		{
			this->operator=(other);
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_date_type(bool)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_date_type(int64_t)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_date_type(uint64_t)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_date_type(double)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_date_type(const internal_enum_type &other)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_date_type(const internal_locale_type &other)
		{
			throw std::runtime_error("");
		}

		/** Assignment operator */
		internal_date_type &operator=(const internal_date_type &other)
		{
			data_ = other.data_;
			return *this;
		}
		/** Retrieves the time point */
		const time_point& time() const
		{
			return data_;
		}
		/** Retrieves the broken-down time */
		const tm& as_tm() const
		{
			time_t time = clock::to_time_t(data_);
			return *localtime(&time);
		}
		/**
		* Conversion operator to double type - allways throws, implemented
		* because of some template usage.
		* @throws allways std::runtime_error exception
		*/
		operator double() const
		{
			throw std::runtime_error("Date type cannot be converted to double");
		}
		/** Equality operator */
		bool operator==(const internal_date_type &other) const
		{
			return data_ == other.data_;
		}
		/** Inequality operator */
		bool operator!=(const internal_date_type &other) const
		{
			return !(*this == other);
		}
		/** Comparation less operator */
		bool operator<(const internal_date_type &other) const
		{
			return data_ < other.data_;
		}

	private:
		/** Value of instance */
		time_point data_;
	};

	/**
	* Inicpp locale type.
	*/
	class internal_locale_type
	{
	public:
		typedef std::locale locale;

		/** Default constructor */
		internal_locale_type()
		{
		}
		/** Constructor with initial value */
		internal_locale_type(const locale &value) : data_(value)
		{
		}
		/** Constructor with initial value */
		internal_locale_type(const std::string &value) : data_(value)
		{
		}
		/** Copy constructor */
		internal_locale_type(const internal_locale_type &other)
		{
			this->operator=(other);
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_locale_type(bool)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_locale_type(int64_t)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_locale_type(uint64_t)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_locale_type(double)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_locale_type(const internal_enum_type &other)
		{
			throw std::runtime_error("");
		}
		/** Conversion contructor - only for template compilation, allways throws std::runtime_error */
		explicit internal_locale_type(const internal_date_type &other)
		{
			throw std::runtime_error("");
		}

		/** Assignment operator */
		internal_locale_type &operator=(const internal_locale_type &other)
		{
			data_ = other.data_;
			return *this;
		}
		/** Retrieves the name of the locale. */
		std::string name() const
		{
			return data_.name();
		}
		/** Conversion operator to std::string type */
		operator std::string() const
		{
			return name();
		}
		/**
		* Conversion operator to double type - allways throws, implemented
		* because of some template usage.
		* @throws allways std::runtime_error exception
		*/
		operator double() const
		{
			throw std::runtime_error("Locale type cannot be converted to double");
		}
		/** Equality operator */
		bool operator==(const internal_locale_type &other) const
		{
			return data_ == other.data_;
		}
		/** Inequality operator */
		bool operator!=(const internal_locale_type &other) const
		{
			return !(*this == other);
		}
		/** Comparation less operator */
		bool operator<(const internal_locale_type &other) const
		{
			return std::string(data_.c_str()) < other.data_.c_str();
		}

	private:
		/** Value of instance */
		locale data_;
	};


	/**
	 * Types which can be used in option and option_schema classes.
	 * Only from and to this types casting is recommended.
	 * For all of this types appropriate typedefs are supplied.
	 */
	enum class option_type : char
	{
		boolean_e,
		signed_e,
		unsigned_e,
		float_e,
		enum_e,
		string_e,
		date_e,
		locale_e,
		invalid_e
	};

	// modern C++11 way of typedef
	using boolean_ini_t = bool;
	using signed_ini_t = int64_t;
	using unsigned_ini_t = uint64_t;
	using float_ini_t = double;
	using enum_ini_t = internal_enum_type;
	using string_ini_t = std::string;
	using date_ini_t = internal_date_type;
	using locale_ini_t = internal_locale_type;

	/**
	 * Enumeration type used in schema specification which distinguishes
	 * between single item and list of items.
	 */
	enum class option_item : bool { single, list };

	/**
	 * Type used to distinguish between mandatory and optional values
	 * in schema specification.
	 */
	enum class item_requirement : bool { mandatory, optional };

	/**
	 * Mode in which schema can be validated.
	 *  Strict - everything has to be right and in appropriate type.
	 *  Relaxed - config can contain unknown sections and options.
	 */
	enum class schema_mode : bool { strict, relaxed };

	/**
	 * Function for convert type (one of *_ini_t) to option_type
	 * enumeration type. If type cannot be converted, invalid_e
	 * is returned.
	 * @return enum representation of templated type
	 */
	template <typename ValueType> option_type get_option_enum_type()
	{
		if (std::is_same<ValueType, boolean_ini_t>::value) {
			return option_type::boolean_e;
		} else if (std::is_same<ValueType, signed_ini_t>::value) {
			return option_type::signed_e;
		} else if (std::is_same<ValueType, unsigned_ini_t>::value) {
			return option_type::unsigned_e;
		} else if (std::is_same<ValueType, float_ini_t>::value) {
			return option_type::float_e;
		} else if (std::is_same<ValueType, string_ini_t>::value) {
			return option_type::string_e;
		} else if (std::is_same<ValueType, enum_ini_t>::value) {
			return option_type::enum_e;
		} else if (std::is_same<ValueType, date_ini_t>::value) {
			return option_type::date_e;
		} else if (std::is_same<ValueType, locale_ini_t>::value) {
			return option_type::locale_e;
		} else {
			return option_type::invalid_e;
		}
	}
}

#endif // INICPP_TYPES_H
