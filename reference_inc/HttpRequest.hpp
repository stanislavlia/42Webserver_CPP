#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Webserv.hpp"

/**
 * @enum HttpMethod
 * @brief Represents the HTTP methods that the server can handle.
 *
 * This enumeration lists the possible HTTP methods (GET, POST, DELETE, PUT, HEAD, and NONE)
 * that can be encountered in an HTTP request.
 */
enum HttpMethod
{
    GET,
    POST,
    DELETE,
    PUT,
    HEAD,
    NONE
};

/**
 * @enum ParsingState
 * @brief Represents the various states of parsing an HTTP request.
 *
 * This enumeration lists the different states of parsing, from the request line
 * to the message body, handling chunked encoding and various HTTP headers.
 */
enum ParsingState
{
    Request_Line,
    Request_Line_Post_Put,
    Request_Line_Method,
    Request_Line_First_Space,
    Request_Line_URI_Path_Slash,
    Request_Line_URI_Path,
    Request_Line_URI_Query,
    Request_Line_URI_Fragment,
    Request_Line_Ver,
    Request_Line_HT,
    Request_Line_HTT,
    Request_Line_HTTP,
    Request_Line_HTTP_Slash,
    Request_Line_Major,
    Request_Line_Dot,
    Request_Line_Minor,
    Request_Line_CR,
    Request_Line_LF,
    Field_Name_Start,
    Fields_End,
    Field_Name,
    Field_Value,
    Field_Value_End,
    Chunked_Length_Begin,
    Chunked_Length,
    Chunked_Ignore,
    Chunked_Length_CR,
    Chunked_Length_LF,
    Chunked_Data,
    Chunked_Data_CR,
    Chunked_Data_LF,
    Chunked_End_CR,
    Chunked_End_LF,
    Message_Body,
    Parsing_Done
};

/**
 * @class HttpRequest
 * @brief Parses and stores an HTTP request, managing the request's various components.
 *
 * The HttpRequest class is responsible for parsing an HTTP request and storing its components,
 * such as the HTTP method, URI path, query, headers, and body. It processes the incoming request
 * data, updates its state accordingly, and triggers flags when parsing is completed. If any errors
 * are detected during parsing, the appropriate HTTP error code is set.
 *
 * Attributes:
 *  - _path: The URI path of the request.
 *  - _query: The query string of the request.
 *  - _fragment: The URI fragment of the request.
 *  - _request_headers: A map of the request's headers.
 *  - _body: The binary body of the request.
 *  - _boundary: The boundary used for multipart form data.
 *  - _method: The HTTP method of the request (e.g., GET, POST).
 *  - _method_str: A map of method identifiers to their string representations.
 *  - _state: The current state of the request parsing process.
 *  - _max_body_size: The maximum allowed size for the request body.
 *  - _body_length: The actual size of the request body.
 *  - _error_code: The HTTP error code if an error occurs during parsing.
 *  - _chunk_length: The length of the current chunk in chunked transfer encoding.
 *  - _storage: Temporary storage for parsing the request.
 *  - _key_storage: Temporary storage for the current header field being parsed.
 *  - _method_index: The index of the method being processed.
 *  - _ver_major: The major version number of the HTTP protocol.
 *  - _ver_minor: The minor version number of the HTTP protocol.
 *  - _server_name: The name of the server handling the request.
 *  - _body_str: The string representation of the request body.
 *
 * Flags:
 *  - _fields_done_flag: Indicates if the headers have been completely parsed.
 *  - _body_flag: Indicates if the body is currently being parsed.
 *  - _body_done_flag: Indicates if the body has been fully parsed.
 *  - _complete_flag: Indicates if the entire request has been successfully parsed.
 *  - _chunked_flag: Indicates if the request body is encoded using chunked transfer encoding.
 *  - _multiform_flag: Indicates if the request is a multipart form data request.
 */
class HttpRequest
{
    public:
        HttpRequest();                              ///< Default constructor.
        ~HttpRequest();                             ///< Destructor.

        // Getters
        HttpMethod                                  &getMethod();                 ///< Get the HTTP method of the request.
        std::string                                 &getPath();                   ///< Get the URI path of the request.
        std::string                                 &getQuery();                  ///< Get the query string of the request.
        std::string                                 &getFragment();               ///< Get the URI fragment of the request.
        std::string                                 getHeader(std::string const &); ///< Get a specific header value by its name.
		const std::map<std::string, std::string>    &getHeaders() const;          ///< Get all headers as a map.
		std::string                                 getMethodStr();               ///< Get the string representation of the HTTP method.
        std::string                                 &getBody();                   ///< Get the request body as a string.
        std::string                                 getServerName();              ///< Get the name of the server handling the request.
        std::string                                 &getBoundary();               ///< Get the boundary string for multipart form data.
        bool                                        getMultiformFlag();           ///< Check if the request is a multipart form data request.
        
        // Setters
        void        setMethod(HttpMethod &);                                    ///< Set the HTTP method of the request.
        void        setHeader(std::string &, std::string &);                    ///< Set a specific header value.
        void        setMaxBodySize(size_t);                                     ///< Set the maximum allowed size for the request body.
        void        setBody(std::string name);                                  ///< Set the request body.

        // Parsing and Utility Functions
        void        feed(char *data, size_t size);                               ///< Feed data into the parser for processing.
        bool        parsingCompleted();                                          ///< Check if parsing has been completed.
        void        printMessage();                                              ///< Print the parsed HTTP request (for debugging).
        void        clear();                                                     ///< Clear all the data in the HttpRequest object.
        short       errorCode();                                                 ///< Get the error code if any error occurred during parsing.
        bool        keepAlive();                                                 ///< Check if the connection should be kept alive.
        void        cutReqBody(int bytes);                                       ///< Cut the request body to the specified size.

    private:
        std::string                         _path;                               ///< URI path of the request.
        std::string                         _query;                              ///< Query string of the request.
        std::string                         _fragment;                           ///< URI fragment of the request.
        std::map<std::string, std::string>  _request_headers;                    ///< Map of HTTP request headers.
        std::vector<u_int8_t>               _body;                               ///< Binary body of the request.
        std::string                         _boundary;                           ///< Boundary for multipart form data.
        HttpMethod                          _method;                             ///< HTTP method of the request.
        std::map<u_int8_t, std::string>     _method_str;                         ///< Map of method identifiers to their string representations.
        ParsingState                        _state;                              ///< Current state of the request parsing process.
        size_t                              _max_body_size;                      ///< Maximum allowed size for the request body.
        size_t                              _body_length;                        ///< Actual size of the request body.
        short                               _error_code;                         ///< HTTP error code for any parsing errors.
        size_t                              _chunk_length;                       ///< Length of the current chunk in chunked transfer encoding.
        std::string                         _storage;                            ///< Temporary storage for request data during parsing.
        std::string                         _key_storage;                        ///< Temporary storage for the current header field being parsed.
        short                               _method_index;                       ///< Index of the method being processed.
        u_int8_t                            _ver_major;                          ///< Major version number of the HTTP protocol.
        u_int8_t                            _ver_minor;                          ///< Minor version number of the HTTP protocol.
        std::string                         _server_name;                        ///< Name of the server handling the request.
        std::string                         _body_str;                           ///< String representation of the request body.

        /* Flags */
        bool                                _fields_done_flag;                   ///< Flag indicating if headers have been fully parsed.
        bool                                _body_flag
