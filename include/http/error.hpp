#ifndef HTTP_ERROR_HPP_INCLUDED
#define HTTP_ERROR_HPP_INCLUDED

#include <system_error>
#include <string>

namespace http {

    enum class ParseError {
        CB_message_begin = 1,
        CB_url,
        CB_header_field,
        CB_header_value,
        CB_headers_complete,
        CB_body,
        CB_message_complete,
        CB_status,
        CB_chunk_header,
        CB_chunk_complete,
        INVALID_EOF_STATE,
        HEADER_OVERFLOW,
        CLOSED_CONNECTION,
        INVALID_VERSION,
        INVALID_STATUS,
        INVALID_METHOD,
        INVALID_URL,
        INVALID_HOST,
        INVALID_PORT,
        INVALID_PATH,
        INVALID_QUERY_STRING,
        INVALID_FRAGMENT,
        LF_EXPECTED,
        INVALID_HEADER_TOKEN,
        INVALID_CONTENT_LENGTH,
        UNEXPECTED_CONTENT_LENGTH,
        INVALID_CHUNK_SIZE,
        INVALID_CONSTANT,
        INVALID_INTERNAL_STATE,
        STRICT,
        PAUSED,
        UNKNOWN,
    };

    namespace {
        struct ParseErrorCategory : std::error_category {
            auto name() const noexcept -> char const* override {
                return "parse";
            }

            auto message(int ec) const -> std::string override {
                switch(static_cast<ParseError>(ec)) {
                    case ParseError::CB_message_begin: 
                        return "the on_message_begin callback failed";
                    case ParseError::CB_url: 
                        return "the on_url callback failed";
                    case ParseError::CB_header_field: 
                        return "the on_header_field callback failed";
                    case ParseError::CB_header_value: 
                        return "the on_header_value callback failed";
                    case ParseError::CB_headers_complete: 
                        return "the on_headers_complete callback failed";
                    case ParseError::CB_body: 
                        return "the on_body callback failed";
                    case ParseError::CB_message_complete: 
                        return "the on_message_complete callback failed";
                    case ParseError::CB_status: 
                        return "the on_status callback failed";
                    case ParseError::CB_chunk_header: 
                        return "the on_chunk_header callback failed";
                    case ParseError::CB_chunk_complete: 
                        return "the on_chunk_complete callback failed";
                    /* Parsing-related errors */                                       
                    case ParseError::INVALID_EOF_STATE: 
                        return "stream ended at an unexpected time";
                    case ParseError::HEADER_OVERFLOW:                                                
                        return "too many header bytes seen; overflow detected";
                    case ParseError::CLOSED_CONNECTION:                                              
                        return "data received after completed connection: close message";
                    case ParseError::INVALID_VERSION: 
                        return "invalid HTTP version";
                    case ParseError::INVALID_STATUS: 
                        return "invalid HTTP status code";
                    case ParseError::INVALID_METHOD: 
                        return "invalid HTTP method";
                    case ParseError::INVALID_URL: 
                        return "invalid URL";
                    case ParseError::INVALID_HOST: 
                        return "invalid host";
                    case ParseError::INVALID_PORT: 
                        return "invalid port";
                    case ParseError::INVALID_PATH: 
                        return "invalid path";
                    case ParseError::INVALID_QUERY_STRING: 
                        return "invalid query string";
                    case ParseError::INVALID_FRAGMENT: 
                        return "invalid fragment";
                    case ParseError::LF_EXPECTED: 
                        return "LF character expected";
                    case ParseError::INVALID_HEADER_TOKEN: 
                        return "invalid character in header";
                    case ParseError::INVALID_CONTENT_LENGTH:                                         
                        return "invalid character in content-length header";
                    case ParseError::UNEXPECTED_CONTENT_LENGTH:                                      
                        return "unexpected content-length header";
                    case ParseError::INVALID_CHUNK_SIZE:                                             
                        return "invalid character in chunk size header";
                    case ParseError::INVALID_CONSTANT: 
                        return "invalid constant string";
                    case ParseError::INVALID_INTERNAL_STATE: 
                        return "encountered unexpected internal state";
                    case ParseError::STRICT: 
                        return "strict mode assertion failed";
                    case ParseError::PAUSED: 
                        return "parser is paused";
                    default: 
                        return "an unknown error occurred";
                }
            }
        };

        ParseErrorCategory const PARSE_ERROR_CATEGORY_INSTANCE { };
    }

//    auto parse_category() -> ParseErrorCategory const& {
//        return PARSE_ERROR_CATEGORY_INSTANCE;
//    }

    auto make_error_code(ParseError e) -> std::error_code {
        return { static_cast<int>(e), PARSE_ERROR_CATEGORY_INSTANCE };
    }
}
#endif //HTTP_ERROR_HPP_INCLUDED
