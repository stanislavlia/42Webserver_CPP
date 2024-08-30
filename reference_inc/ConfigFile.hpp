#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include "Webserv.hpp"

/**
 * @class ConfigFile
 * @brief Manages the configuration file used by the web server, handling file path, size, and various file operations.
 *
 * The ConfigFile class is responsible for handling operations related to the configuration file of the web server.
 * It provides methods to retrieve file information, check file existence and readability, and read the contents of the file.
 * The class encapsulates the path to the configuration file and its size.
 *
 * Attributes:
 *  - _path: The file path of the configuration file.
 *  - _size: The size of the configuration file (currently optional and might not be needed).
 */
class ConfigFile {
    private:
        std::string _path;   ///< Path to the configuration file.
        size_t      _size;   ///< Size of the configuration file (to be verified if needed).

    public:
        // Constructors and Destructor
        ConfigFile();                                ///< Default constructor.
        ConfigFile(std::string const path);          ///< Constructor that initializes the object with a file path.
        ~ConfigFile();                               ///< Destructor.

        // Static methods
        static int getTypePath(std::string const path); ///< Determine the type of the path (e.g., file or directory).
        static int checkFile(std::string const path, int mode); ///< Check if the file at the given path meets the specified mode criteria.
        static int isFileExistAndReadable(std::string const path, std::string const index); ///< Check if the file exists and is readable.

        // Member methods
        std::string readFile(std::string path);      ///< Read the contents of the file at the given path.
        std::string getPath();                       ///< Get the path of the configuration file.
        int getSize();                               ///< Get the size of the configuration file (if used).
};

#endif // CONFIGFILE_HPP
