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

    struct ParseErrorCategory : std::error_category {
        auto name() const noexcept -> char const* override;
        auto message(int ec) const -> std::string override;
    };

    auto parse_category() -> ParseErrorCategory const&;

    auto make_error_code(ParseError e) -> std::error_code;
}
#endif //HTTP_ERROR_HPP_INCLUDED
