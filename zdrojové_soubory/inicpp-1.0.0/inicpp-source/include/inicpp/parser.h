#ifndef INICPP_PARSER_H
#define INICPP_PARSER_H

#include <fstream>
#include <iostream>
#include <regex>
#include <regex>
#include <sstream>
#include <string>

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
		class resource_fetcher
		{
		public:
			/**
			* This method returns a resource corresponding to given name.
			* @param name resource name
			*/
			virtual std::unique_ptr<std::istream> get_resource(const std::string & name);
			/**
			* This method release all non-used resources.
			*/
			virtual void release_all();
		};
		/**
		* The following classes are enabling us to work with files and prepared string_streams alike.
		*/
		class file_fetcher : public resource_fetcher
		{
		public:
			/**
			* Returns istream corresponding to file with given name.
			* @param file_name name of fetched file.
			*/
			virtual std::unique_ptr<std::istream> get_resource(const std::string &file_name)
			{
				std::unique_ptr<std::istream> iStream(new std::ifstream(file_name));
				return iStream;
			}
			/**
			* Files are opened on demand, therefore this method dont have any significance.
			*/
			virtual void release_all() {}
		};
		/**
		* This class implements methods of resource_fetcher for organisation of input string streams.
		*/
		class string_stream_fetcher : public resource_fetcher
		{
		private:
			std::vector< std::pair< std::string, std::unique_ptr< std::stringstream > > > resources_;
		public:
			virtual std::unique_ptr<std::istream> get_resource(const std::string &string_name);

			void insert_stream(std::string name, std::unique_ptr <std::stringstream> stream)
			{
				resources_.push_back(std::pair< std::string, std::unique_ptr < std::stringstream > >(name, std::move(stream)));
			}
			virtual void release_all()
			{
				for (auto i = resources_.begin(); i < resources_.end(); i++)
				{
					i->second.release();
				}
				resources_.clear();
			}

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
		/**
		* This internal_load which allows the #include functionality.
		*/
		static config parser::internal_load(std::unique_ptr<std::istream> initial_stream,
			const std::string & initial_stream_name,
			std::unique_ptr<resource_fetcher> resources = std::unique_ptr<resource_fetcher> (new file_fetcher()));
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
		* Load in configuaration from sources stored in resource fetcher.
		* @param init_name name of initial resource
		* @param schm validation schema
		* @param mode validation mode
		* @param res_fetcher resource fetcher to be used
		*/
		static config parser::load_from_fetcher(const std::string &init_name,
			const schema &schm, schema_mode mode,
			std::unique_ptr<resource_fetcher> res_fetcher);


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
