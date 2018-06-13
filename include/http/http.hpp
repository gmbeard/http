#ifndef HTTP_HTTP_HPP_INCLUDED
#define HTTP_HTTP_HPP_INCLUDED

#include "result/result.hpp"
#include "http/error.hpp"
#include <vector>
#include <tuple>
#include <string>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <ostream>

#include <cassert>

namespace http {
    namespace parser {
#include "http_parser.h"
    }

    enum class Method {
        Delete = 0,
        Get,
        Head,
        Post,
        Put,
        Connect,
        Options,
        Trace,
    };

    enum class Version {
        Http10,
        Http11,
    };

    template<typename T, typename Traits>
    auto operator<<(std::basic_ostream<T, Traits>& os,
                    Version const& v) -> std::basic_ostream<T, Traits>&
    {
        constexpr char HTTP[] = "HTTP/1.";
        constexpr char V1_0 = '0';
        constexpr char V1_1 = '1';

        os.write(
            reinterpret_cast<T const*>(std::addressof(*std::begin(HTTP))),
             std::distance(std::begin(HTTP), std::end(HTTP)-1));

        switch (v) {
            case Version::Http10:
                return os.write(reinterpret_cast<T const*>(&V1_0), 1);
            default:
                return os.write(reinterpret_cast<T const*>(&V1_1), 1);
        }
    }

    template<typename T, typename Traits>
    auto operator<<(std::basic_ostream<T, Traits>& os,
                    std::pair<std::string, std::string> const& p)
        -> std::basic_ostream<T, Traits>&
    {
        constexpr char SEP[] = ": ";

        auto const& first = std::get<0>(p);
        auto const& second = std::get<1>(p);

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*first.begin())),
            first.size());

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*std::begin(SEP))),
            std::distance(std::begin(SEP), std::end(SEP)-1));
//        os << ": ";

        return os.write(
            reinterpret_cast<T const*>(
                std::addressof(*second.begin())),
            second.size());
    }

    template<typename T>
    using ParseResult = result::Result<T, std::error_code>;
    using Header = std::pair<std::string, std::string>;
    using HeaderContainer = std::vector<Header>;
    using BodyContainer = std::vector<uint8_t>;

    struct HttpRequestProtocolHeader {
        Method method;
        std::string path;
        Version version;
    };

    struct HttpResponseProtocolHeader {
        Version version;
        size_t status_code;
        std::string status_text;
    };

    struct HttpRequest {
        friend struct HttpRequestHeaderBuilder;

        inline auto method() const -> Method 
        { return protocol_.method; }

        inline auto path() const -> std::string const& 
        { return protocol_.path; }

        inline auto version() const -> Version 
        { return protocol_.version; }

        inline auto headers() const -> HeaderContainer const& 
        { return headers_; }

        inline auto body() const -> BodyContainer const&
        { return body_; }

    private:
        HttpRequest(HttpRequestProtocolHeader, 
                    HeaderContainer,
                    BodyContainer);

        HttpRequestProtocolHeader protocol_;
        HeaderContainer headers_;
        BodyContainer body_;
    };

    struct HttpResponse {
        friend struct HttpResponseHeaderBuilder;

        inline auto version() const -> Version
        { return protocol_.version; }

        inline auto status_code() const -> size_t
        { return protocol_.status_code; }

        inline auto status_text() const -> std::string const&
        { return protocol_.status_text; }

        inline auto headers() const -> HeaderContainer const&
        { return headers_; }

        inline auto body() const -> BodyContainer const&
        { return body_; }

    private:
        HttpResponse(HttpResponseProtocolHeader, 
                     HeaderContainer,
                     BodyContainer);

        HttpResponseProtocolHeader protocol_;
        HeaderContainer headers_;
        BodyContainer body_;
    };

    template<typename T>
    auto operator<<(std::basic_ostream<T>& os, 
                    HttpResponse const& response) 
        -> std::basic_ostream<T>&
    {
        constexpr char NL[] = "\r\n";

        os << response.version() << " ";

        auto sc = std::to_string(response.status_code());

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*sc.begin())),
            sc.size());

        os << " ";

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*response.status_text().begin())),
             response.status_text().size());

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*std::begin(NL))),
            std::distance(std::begin(NL), std::end(NL)-1));

        for (auto const& h : response.headers()) {
            os << h << "\r\n";
        }

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*std::begin(NL))),
            std::distance(std::begin(NL), std::end(NL)-1));

        os.write(
            reinterpret_cast<T const*>(
                std::addressof(*response.body().begin())),
             response.body().size());
        return os;
    }

    struct HttpRequestHeaderBuilder {
        HttpRequestHeaderBuilder(HttpRequestProtocolHeader p);
        auto with_header(Header h) && 
            -> HttpRequestHeaderBuilder&&;
        auto with_headers(std::initializer_list<Header> h) &&
            -> HttpRequestHeaderBuilder&&;

        auto with_headers(HeaderContainer&& headers) && {
            headers_ = std::move(headers);
            return std::move(*this);
        }

        auto build() && -> HttpRequest;
        auto build(BodyContainer) && -> HttpRequest;

        template<typename InputIterator>
        auto build(InputIterator first, InputIterator last) &&
            -> HttpRequest
        {
            return std::move(*this).build(BodyContainer { first, last });
        }

    private:
        HttpRequestProtocolHeader proto_;
        HeaderContainer headers_;    
    };

    struct HttpResponseHeaderBuilder {
        HttpResponseHeaderBuilder(HttpResponseProtocolHeader p);
        auto with_header(Header h) && 
            -> HttpResponseHeaderBuilder&&;
        auto with_headers(std::initializer_list<Header> h) &&
            -> HttpResponseHeaderBuilder&&;

        auto with_headers(HeaderContainer&& headers) && {
            headers_ = std::move(headers);
            return std::move(*this);
        }

        auto build() && -> HttpResponse;
        auto build(BodyContainer) && -> HttpResponse;

        template<typename InputIterator>
        auto build(InputIterator first, InputIterator last) &&
            -> HttpResponse
        {
            return std::move(*this).build(BodyContainer { first, last });
        }

    private:
        HttpResponseProtocolHeader proto_;
        HeaderContainer headers_;    
    };

    struct HttpRequestBuilder {
        auto with_protocol(HttpRequestProtocolHeader p) && 
            -> HttpRequestHeaderBuilder;
    };

    struct HttpResponseBuilder {
        auto with_protocol(HttpResponseProtocolHeader) &&
            -> HttpResponseHeaderBuilder;
    };

    namespace detail {
        auto parse_request(char const* data, size_t size) noexcept
            -> ParseResult<std::pair<HttpRequest, size_t>>;

        auto parse_response(char const* data, size_t size) noexcept
            -> ParseResult<std::pair<HttpResponse, size_t>>;
    }

    template<
        typename Iterator,
        typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<Iterator>::iterator_category,
                std::random_access_iterator_tag>::value
        >::type* = nullptr>
    auto parse_request(Iterator first, Iterator last) noexcept
        -> ParseResult<std::pair<HttpRequest, size_t>> 
    {
        return detail::parse_request(
            reinterpret_cast<char const*>(std::addressof(*first)),
            std::distance(first, last));
    }

    template<
        typename Iterator,
        typename std::enable_if<
            std::is_convertible<
                typename std::iterator_traits<Iterator>::iterator_category,
                std::random_access_iterator_tag>::value
        >::type* = nullptr>
    auto parse_response(Iterator first, Iterator last) noexcept
        -> ParseResult<std::pair<HttpResponse, size_t>> 
    {
        return detail::parse_response(
            reinterpret_cast<char const*>(std::addressof(*first)),
            std::distance(first, last));
    }
}

#endif //HTTP_HTTP_HPP_INCLUDED
