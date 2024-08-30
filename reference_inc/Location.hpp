#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Webserv.hpp"

/**
 * @class Location
 * @brief Represents a location block in the server configuration, defining routing and handling rules for specific URIs.
 *
 * The Location class encapsulates the configuration details for a specific location (URI path) on the server.
 * It manages properties such as the root directory, allowed HTTP methods, autoindex settings, and CGI handling.
 * This class is crucial for routing requests to the correct handlers and resources on the server.
 *
 * Attributes:
 *  - _path: The URI path associated with this location.
 *  - _root: The root directory on the server's filesystem for this location.
 *  - _autoindex: A flag indicating whether autoindexing is enabled (i.e., directory listing).
 *  - _index: The default file to serve if the request is for a directory.
 *  - _methods: A vector of allowed HTTP methods (GET, POST, DELETE, etc.) for this location.
 *  - _return: The return directive for redirecting requests.
 *  - _alias: An alias for the location, effectively changing the request URI path.
 *  - _cgi_path: A vector of paths to CGI scripts that can be executed for this location.
 *  - _cgi_ext: A vector of file extensions that should trigger CGI execution.
 *  - _client_max_body_size: The maximum size of the request body allowed for this location.
 *
 * Public attributes:
 *  - _ext_path: A map of file extensions to specific paths, used for CGI or other specialized handling.
 */
class Location
{
	private:
		std::string					_path;                       ///< URI path associated with this location.
		std::string					_root;                       ///< Root directory on the server for this location.
		bool						_autoindex;                  ///< Autoindex flag for enabling/disabling directory listing.
		std::string					_index;                      ///< Default file to serve for directory requests.
		std::vector<short>			_methods;                    ///< Allowed HTTP methods for this location (GET, POST, etc.).
		std::string					_return;                     ///< Return directive for redirection.
		std::string					_alias;                      ///< Alias for the URI path, modifying the request path.
		std::vector<std::string>	_cgi_path;                   ///< Paths to CGI scripts for this location.
		std::vector<std::string>	_cgi_ext;                    ///< File extensions that trigger CGI execution.
		unsigned long				_client_max_body_size;       ///< Maximum size of the request body allowed.

	public:
		std::map<std::string, std::string> _ext_path;              ///< Map of file extensions to specific paths.

		// Constructors and Destructor
		Location();                                                 ///< Default constructor.
		Location(const Location &other);                            ///< Copy constructor.
		Location &operator=(const Location &rhs);                   ///< Assignment operator.
		~Location();                                                ///< Destructor.

		// Setters
		void setPath(std::string parametr);                         ///< Set the URI path for this location.
		void setRootLocation(std::string parametr);                 ///< Set the root directory for this location.
		void setMethods(std::vector<std::string> methods);          ///< Set the allowed HTTP methods for this location.
		void setAutoindex(std::string parametr);                    ///< Enable or disable autoindexing (directory listing).
		void setIndexLocation(std::string parametr);                ///< Set the default index file for directory requests.
		void setReturn(std::string parametr);                       ///< Set the return directive for redirection.
		void setAlias(std::string parametr);                        ///< Set the alias for the URI path.
		void setCgiPath(std::vector<std::string> path);             ///< Set the paths to CGI scripts for this location.
		void setCgiExtension(std::vector<std::string> extension);   ///< Set the file extensions that trigger CGI execution.
		void setMaxBodySize(std::string parametr);                  ///< Set the maximum allowed body size (from a string).
		void setMaxBodySize(unsigned long parametr);                ///< Set the maximum allowed body size (from a numeric value).

		// Getters
		const std::string &getPath() const;                         ///< Get the URI path for this location.
		const std::string &getRootLocation() const;                 ///< Get the root directory for this location.
		const std::vector<short> &getMethods() const;               ///< Get the allowed HTTP methods for this location.
		const bool &getAutoindex() const;                           ///< Check if autoindexing is enabled.
		const std::string &getIndexLocation() const;                ///< Get the default index file for directory requests.
		const std::string &getReturn() const;                       ///< Get the return directive for redirection.
		const std::string &getAlias() const;                        ///< Get the alias for the URI path.
		const std::vector<std::string> &getCgiPath() const;         ///< Get the paths to CGI scripts for this location.
		const std::vector<std::string> &getCgiExtension() const;    ///< Get the file extensions that trigger CGI execution.
		const std::map<std::string, std::string> &getExtensionPath() const; ///< Get the map of file extensions to paths.
		const unsigned long &getMaxBodySize() const;                ///< Get the maximum allowed body size.

		std::string getPrintMethods() const;                        ///< Get a string representation of the allowed methods (for debugging).
};

#endif // LOCATION_HPP
