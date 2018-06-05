#ifndef HTTP_HTTP_HPP_INCLUDED
#define HTTP_HTTP_HPP_INCLUDED

#include "result/result.hpp"
#include <vector>
#include <tuple>
#include <string>

namespace http {
    namespace parser {
#include "http_parser.h"
    }

    enum class Method {
        Get,
        Post,
        Put,
        Delete,
    };

    enum class Version {
        Http10,
        Http11,
    };

    struct HttpParseError { };

    template<typename T>
    using ParseResult = result::Result<T, HttpParseError>;
    using Header = std::pair<std::string, std::string>;
    using HeaderContainer = std::vector<Header>;

    struct HttpRequestProtocolHeader {
        Method method;
        std::string path;
        Version version;
    };

    struct HttpRequest {
        friend struct HttpHeaderBuilder;

        inline auto method() const -> Method 
        { return protocol_.method; }

        inline auto path() const -> std::string const& 
        { return protocol_.path; }

        inline auto version() const -> Version 
        { return protocol_.version; }

        inline auto headers() const -> HeaderContainer const& 
        { return headers_; }

    private:
        HttpRequest(HttpRequestProtocolHeader, HeaderContainer);

        HttpRequestProtocolHeader protocol_;
        HeaderContainer headers_;
    };

    struct HttpHeaderBuilder {
        HttpHeaderBuilder(HttpRequestProtocolHeader p);
        auto with_header(Header h) && 
            -> HttpHeaderBuilder&&;
        auto with_headers(std::initializer_list<Header> h) &&
            -> HttpHeaderBuilder&&;
        auto build() && -> HttpRequest;
    private:
        HttpRequestProtocolHeader proto_;
        HeaderContainer headers_;    
    };

    struct HttpRequestBuilder {
        auto with_protocol(HttpRequestProtocolHeader p) && 
            -> HttpHeaderBuilder;
    };

    template<typename Iterator>
    auto parse_request(Iterator, Iterator) -> ParseResult<HttpRequest> {
        return result::ok(HttpRequestBuilder { }
            .with_protocol( { Method::Get, "/index", Version::Http11 } )
            .with_headers({
                std::make_pair("Host", "example.com")
            })
            .build());
    }
}

#endif //HTTP_HTTP_HPP_INCLUDED
