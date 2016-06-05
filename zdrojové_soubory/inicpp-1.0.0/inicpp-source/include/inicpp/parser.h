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
	 * Parser is not constructable class which contains methods
	 * which can be used to load or store ini configuration.
	 */
	class INICPP_API parser
	{
	public:


		/**
		* This is a common ancestor to classes file_fetcher and string_strema_fetcher.
		* Hese classes allows to access to ifstream or stringstream uniformly.
		*/
		class resource
		{
		public:
			virtual ~resource() {};

			virtual bool get_line(std::string &out_string) = 0;
		};

		class stream_resource : public resource
		{
		private:
			std::istream &stream_;
		public:
			stream_resource(std::istream &stream) : stream_(stream) {}
			virtual ~stream_resource() {};

			virtual bool get_line(std::string &out_string)
			{
				return static_cast<bool>(std::getline(stream_, out_string));
			}
		};

		template<typename ResourceType>
		class resource_stack : public resource
		{
		private:
			std::stack<ResourceType> stack_;
		protected:
			virtual ResourceType get_resource(const std::string &name) = 0;
			virtual bool get_line(ResourceType &resource, std::string &out_string) const = 0;
		public:
			resource_stack() { }
			resource_stack(const ResourceType& initial_resource)
			{
				stack_.push(initial_resource);
			}
			resource_stack(ResourceType&& initial_resource)
			{
				stack_.push(std::move(initial_resource));
			}
			virtual ~resource_stack() {};

			void initialize(const ResourceType& initial_resource)
			{
				stack_.push(initial_resource);
			}
			void initialize(ResourceType&& initial_resource)
			{
				stack_.push(std::move(initial_resource));
			}
			void initialize(std::string &name)
			{
				stack_.clear();
				stack_.push(get_resource(name));
			}

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
					} else {
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
			static std::ifstream get_file_stream(const std::string &file_name)
			{
				std::ifstream result(file_name);
				if (result.fail()) {
					throw parser_exception("File reading error");
				}
				return result;
			}
		protected:
			/**
			* Returns istream corresponding to file with given name.
			* @param file_name name of fetched file.
			*/
			virtual std::ifstream get_resource(const std::string &file_name)
			{
				return get_file_stream(file_name);
			}

			virtual bool get_line(std::ifstream &resource, std::string &out_string) const
			{
				return static_cast<bool>(std::getline(resource, out_string));
			}
		public:
			file_resource_stack(const std::string& file_name) : resource_stack<std::ifstream>(get_file_stream(file_name)) {}
		};

		template<typename StreamType, typename StorageType>
		class stream_extractor
		{
		public:
			static StreamType get_stream_from_map(std::map<std::string, StorageType> &streams, const std::string &name)
			{
				auto stream_it = streams.find(name);
				if (stream_it == streams.end()) {
					throw parser_exception("");
				}

				return StreamType(stream_it->second);
			}
		};
		template<typename StreamType>
		class stream_extractor<StreamType, StreamType>
		{
		public:
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
			/**
			* Returns istream corresponding to file with given name.
			* @param file_name name of fetched file.
			*/
			virtual StreamType get_resource(const std::string &name)
			{
				return extractor::get_stream_from_map(streams_, name);
			}

			virtual bool get_line(StreamType &resource, std::string &out_string) const
			{
				return static_cast<bool>(std::getline(resource, out_string));
			}
		public:
			stream_resource_stack(const std::string &initial_stream_name, const storage_map& streams)
				: streams_(streams)
			{
				initialize(extractor::get_stream_from_map(streams_, initial_stream_name));
			}
			virtual ~stream_resource_stack() {}
		};
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
		static std::string parser::extract_file_name_from_include(const std::string &str);
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
