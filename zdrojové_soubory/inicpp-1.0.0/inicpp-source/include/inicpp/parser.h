#ifndef INICPP_PARSER_H
#define INICPP_PARSER_H

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <stack>

#include "config.h"
#include "dll.h"
#include "exception.h"
#include "schema.h"
#include "string_utils.h"

namespace inicpp
{
	/**
	* This is a common ancestor to classes file_fetcher and string_strema_fetcher.
	* Hese classes allows to access to ifstream or stringstream uniformly.
	*/
	class resource
	{
	public:
		/**
		* Virtual destructor supplied because the class is expected to be derived.
		*/
		virtual ~resource() {};

		/**
		* Retrieves a line from the resource and returns it through the parameter.
		* @return true if successful, false if the end or other error was reached.
		*/
		virtual bool get_line(std::string &out_string) = 0;
	};

	/**
	* Stream in the form of a resource. Captures only a reference to the stream.
	*/
	class stream_resource : public resource
	{
	private:
		/** Reference to the encapsulated stream. */
		std::istream &stream_;
	public:
		/** Initiliazes the resource with the specified stream reference. */
		stream_resource(std::istream &stream);

		/*
		* \copydoc resource::~resource()
		*/
		virtual ~stream_resource() {};

		/**
		* \copydoc resource::get_line(std::string)
		*/
		virtual bool get_line(std::string &out_string);
	};

	/**
	* Abstract class implementing the ability to include other resources using
	* the #include directive.
	* The class is templated so that it can hold various concrete implementations
	* of various resources (not necessarily ones deriving from "resource").
	*/
	template<typename ResourceType>
	class resource_stack : public resource
	{
	private:
		/** Represents the stack of resources that are to be processed in LIFO order. */
		std::stack<ResourceType> stack_;

	protected:
		/** Retrieves a named resource. May throw exceptions. */
		virtual ResourceType get_resource(const std::string &name) = 0;

		/** Retrieves the line from the resource. Required, as the resource can be of an unknown type. */
		virtual bool get_line(ResourceType &resource, std::string &out_string) const = 0;

	public:
		/** Constructs an empty resource stack. */
		resource_stack() { }
		/** Constructs the resource stack with one element. */
		resource_stack(const ResourceType& initial_resource)
		{
			stack_.push(initial_resource);
		}
		/** Constructs the resource stack with one element. */
		resource_stack(ResourceType&& initial_resource)
		{
			stack_.push(std::move(initial_resource));
		}
		virtual ~resource_stack() {};

		/** Initializes the resource stack with one element. */
		void initialize(const ResourceType& initial_resource)
		{
			stack_.push(initial_resource);
		}
		/** Initializes the resource stack with one element. */
		void initialize(ResourceType&& initial_resource)
		{
			stack_.push(std::move(initial_resource));
		}
		/** Initializes the resource stack with one element of the given name. */
		void initialize(std::string &name)
		{
			stack_.clear();
			stack_.push(get_resource(name));
		}

		/**
		* Retrieves a line from the top of the resource. If it contains, #include
		* directive, it retrieves a new resource and pushes it on the stack.
		* Otherwise, it returns the line through the parameter.
		* @return true if successful, false if the end or other error was reached.
		*/
		virtual bool get_line(std::string &out_string)
		{
			using namespace string_utils;

			// repeat until there is a regular line, end of stack or exception
			do {
				do {
					// check whether we run out of resources
					if (stack_.empty()) {
						return false;
					}

					// try to read line from the resource
					if (get_line(stack_.top(), out_string)) {
						break;
					}

					// no more lines in the current resource, get next
					stack_.pop();
				} while (true);

				// ignore whitespaces at the beginning
				std::string line = left_trim(out_string);
				if (starts_with(line, "#include")) {
					std::string resource_name = trim(line.substr(8)); // remove "#include "

																	  // retrieve the new current resource
					stack_.push(get_resource(resource_name));
				}
				else {
					break;
				}
			} while (true);

			return true;
		}
	};

	/**
	* The following classes are enabling us to work with files and prepared string_streams alike.
	*/
	class file_resource_stack : public resource_stack<std::ifstream>
	{
	private:
		/** Returns ifstream corresponding to file with given name. */
		static std::ifstream get_file_stream(const std::string &file_name);

	protected:
		/** \copydoc file_resource_stack::get_file_stream(const std::string &file_name) */
		virtual std::ifstream get_resource(const std::string &file_name);

		/** \copydoc resource::get_line(std::string &out_string) */
		virtual bool get_line(std::ifstream &resource, std::string &out_string) const;

	public:
		file_resource_stack(const std::string& file_name);
	};

	namespace
	{
		/**
		* Helper template class for retrieving a stream from a map with its
		* initilization data.
		*/
		template<typename StreamType, typename StorageType>
		class stream_extractor
		{
		public:
			/** Retrieves the initiliazation data for the stream by name, creates it and returns. */
			static StreamType get_stream_from_map(std::map<std::string, StorageType> &streams, const std::string &name)
			{
				auto stream_it = streams.find(name);
				if (stream_it == streams.end()) {
					throw parser_exception("");
				}

				return StreamType(stream_it->second);
			}
		};
		/**
		* Partial specialization of stream_extractor for the case when there
		* are only already initialized stream present and as such cannot be
		* reused.
		*/
		template<typename StreamType>
		class stream_extractor<StreamType, StreamType>
		{
		public:
			/** Retrieves the stream by name, removes it from the map and returns it. */
			static StreamType get_stream_from_map(std::map<std::string, StreamType> &streams, const std::string &name)
			{
				auto stream_it = streams.find(name);
				if (stream_it == streams.end()) {
					throw parser_exception("");
				}

				StreamType result = std::move(stream_it->second);
				streams.erase(stream_it);

				return result;
			}
		};
	};

	/**
	* This class implements methods of resource_fetcher for organisation of input string streams.
	*/
	template<typename StreamType, typename StorageType = StreamType>
	class stream_resource_stack : public resource_stack<StreamType>
	{
	private:
		typedef std::map<std::string, StorageType> storage_map;
		typedef stream_extractor<StreamType, StorageType> extractor;

		/** Mapping from names to specific streams. Immutable once intialized to preserve pointers. */
		storage_map streams_;

	protected:
		/** Returns stream from the map corresponding to the given name. */
		virtual StreamType get_resource(const std::string &name)
		{
			return extractor::get_stream_from_map(streams_, name);
		}

		/** \copydoc resource::get_line(std::string &out_string) */
		virtual bool get_line(StreamType &resource, std::string &out_string) const
		{
			return static_cast<bool>(std::getline(resource, out_string));
		}

	public:
		/** Initializes the resource stack with the supplied data storage and an initial stream name. */
		stream_resource_stack(const std::string &initial_stream_name, const storage_map& streams)
			: streams_(streams)
		{
			initialize(extractor::get_stream_from_map(streams_, initial_stream_name));
		}

		/*
		* \copydoc resource::~resource()
		*/
		virtual ~stream_resource_stack() {}
	};


	/**
	 * Parser is not constructable class which contains methods
	 * which can be used to load or store ini configuration.
	 */
	class INICPP_API parser
	{
	private:
		/**
		 * Finds first nonescaped character given as parameter
		 * Escaping character is '\'
		 * @return std::string::npos if not found
		 */
		static size_t find_first_nonescaped(const std::string &str, char ch);
		/**
		 * Finds last escaped character given as parameter
		 * Escaping character is '\'
		 * @return std::string::npos if not found
		 */
		static size_t find_last_escaped(const std::string &str, char ch);
		static std::string unescape(const std::string &str);
		static std::string delete_comment(const std::string &str);
		static std::vector<std::string> parse_option_list(const std::string &str);
		static void handle_links(const config &cfg,
			const section &last_section,
			std::vector<std::string> &option_val_list,
			size_t line_number);
		static void validate_identifier(const std::string &str, size_t line_number);

		//static config internal_load(std::istream &str);
		static void internal_save(const config &cfg, const schema &schm, std::ostream &str);

		/*
		class resource_stack
		{
		private:
			std::vector< std::unique_ptr<std::istream> > istreams_;
		public:
			void push_back(std::unique_ptr< std::istream> new_istream)
			{
				istreams_.push_back(std::move(new_istream));
			}
			bool get_line(std::string & out_string);
			void release_all();


		};
		*/

		/**
		* This internal_load which allows the #include functionality.
		*/
		static config parser::internal_load(resource &res);
	public:
		/**
		 * Deleted default constructor.
		 */
		parser() = delete;
		/**
		 * Deleted copy constructor.
		 */
		parser(const parser &source) = delete;
		/**
		 * Deleted copy assignment.
		 */
		parser &operator=(const parser &source) = delete;
		/**
		 * Deleted move constructor.
		 */
		parser(parser &&source) = delete;
		/**
		 * Deleted move assignment.
		 */
		parser &operator=(parser &&source) = delete;

		/**
		 * Load ini configuration from given string and return it.
		 * @param str ini configuration description
		 * @return newly created config class
		 * @throws parser_exception if ini configuration is wrong
		 */
		static config load(const std::string &str);
		/**
		 * Load ini configuration from given string
		 * and validate it through schema.
		 * @param str ini configuration description
		 * @param schm validation schema
		 * @param mode validation mode
		 * @return constructed config class which comply given schema
		 * @throws parser_exception if ini configuration is wrong
		 * @throws validation_exception if configuration does not comply schema
		 */
		static config load(const std::string &str, const schema &schm, schema_mode mode);
		/**
		 * Load ini configuration from given stream and return it.
		 * @param str ini configuration description
		 * @return newly created config class
		 * @throws parser_exception if ini configuration is wrong
		 */
		static config load(std::istream &str);
		/**
		 * Load ini configuration from given stream
		 * and validate it through schema.
		 * @param str ini configuration description
		 * @param schm validation schema
		 * @param mode validation mode
		 * @return constructed config class which comply given schema
		 * @throws parser_exception if ini configuration is wrong
		 * @throws validation_exception if configuration does not comply schema
		 */
		static config load(std::istream &str, const schema &schm, schema_mode mode);

		/**
		* Load in configuaration from sources stored in resource.
		* @param res ini configuration resource
		* @param res_fetcher resource fetcher to be used
		* @throws parser_exception if ini configuration is wrong
		* @throws validation_exception if configuration does not comply schema
		* @return new instance of config class
		*/
		static config load(resource &res);
		/**
		* Load in configuaration from sources stored in resource.
		* @param res ini configuration resource
		* @param schm validation schema
		* @param mode validation mode
		* @param res_fetcher resource fetcher to be used
		* @throws parser_exception if ini configuration is wrong
		* @throws validation_exception if configuration does not comply schema
		* @return new instance of config class
		*/
		static config load(resource &res, const schema &schm, schema_mode mode);

		/**
		 * Load ini configuration from file with specified name.
		 * @param file name of file which contains ini configuration
		 * @return new instance of config class
		 * @throws parser_exception if ini configuration is wrong
		 */
		static config load_file(const std::string &file);
		/**
		 * Load ini configuration from file with specified name
		 * and validate it against given schema.
		 * @param file name of file with ini configuration
		 * @param schm validation schema
		 * @param mode validation mode
		 * @return new instance of config class
		 * @throws parser_exception if ini configuration is wrong
		 * @throws validation_exception if configuration does not comply schema
		 */
		static config load_file(const std::string &file, const schema &schm, schema_mode mode);


		/**
		 * Save given configuration to file.
		 * @param cfg configuration which will be saved
		 * @param file name of output file
		 */
		static void save(const config &cfg, const std::string &file);
		/**
		 * Save configuration to output stream.
		 * @param cfg configuration which will be saved
		 * @param str output stream
		 */
		static void save(const config &cfg, std::ostream &str);
		/**
		 * Save given configuration (could be only partial) to a file. Options
		 * which are not specified will be substitued by default values from schema.
		 * @param cfg configuration which will be saved
		 * @param schm schema which will be saved
		 * @param file name of output file
		 */
		static void save(const config &cfg, const schema &schm, const std::string &file);
		/**
		 * Save given configuration (could be only partial) to output stream. Options
		 * which are not specified will be substitued by default values from schema.
		 * @param cfg configuration which will be saved
		 * @param schm schema which will be saved
		 * @param str output stream
		 */
		static void save(const config &cfg, const schema &schm, std::ostream &str);
		/**
		* Save validation schema to file.
		* @param schm schema which will be saved
		* @param file name of output file
		*/
		static void save(const schema &schm, const std::string &file);
		/**
		* Save given validation schema to output stream.
		* @param schm schema which will be saved
		* @param str output stream
		*/
		static void save(const schema &schm, std::ostream &str);
	};


	
}

#endif
