#include "http/http.hpp"
#include <cctype>
#include <numeric>

using namespace http;

HttpRequest::HttpRequest(HttpRequestProtocolHeader h, 
                         HeaderContainer c,
                         BodyContainer b)
    :   protocol_ { std::move(h) }
    ,   headers_ { std::move(c) }
    ,   body_ { std::move(b) }
{ }

HttpRequestHeaderBuilder::HttpRequestHeaderBuilder(HttpRequestProtocolHeader p)
    : proto_{p}
{ }

auto HttpRequestHeaderBuilder::with_header(Header h) && 
    -> HttpRequestHeaderBuilder&&
{
    headers_.emplace_back(std::move(h));
    return std::move(*this);
}

auto HttpRequestHeaderBuilder::with_headers(std::initializer_list<Header> h) &&
    -> HttpRequestHeaderBuilder&&
{
    for (auto&& hdr : h) {
        headers_.push_back(std::move(hdr));
    }
    return std::move(*this);
}

auto HttpRequestHeaderBuilder::build() && -> HttpRequest {
    return std::move(*this).build(BodyContainer { });
}

auto HttpRequestHeaderBuilder::build(BodyContainer body) && 
    -> HttpRequest 
{
    return {
        std::move(proto_),
        std::move(headers_),
        std::move(body)
    };
}

auto HttpRequestBuilder::with_protocol(HttpRequestProtocolHeader p) && 
    -> HttpRequestHeaderBuilder
{
    return { std::move(p) };        
}

HttpResponse::HttpResponse(HttpResponseProtocolHeader h, 
                           HeaderContainer c,
                           BodyContainer b)
    :   protocol_ { std::move(h) }
    ,   headers_ { std::move(c) }
    ,   body_ { std::move(b) }
{ }

HttpResponseHeaderBuilder::HttpResponseHeaderBuilder(HttpResponseProtocolHeader p)
    : proto_{p}
{ }

auto HttpResponseHeaderBuilder::with_header(Header h) && 
    -> HttpResponseHeaderBuilder&&
{
    headers_.emplace_back(std::move(h));
    return std::move(*this);
}

auto HttpResponseHeaderBuilder::with_headers(std::initializer_list<Header> h) &&
    -> HttpResponseHeaderBuilder&&
{
    for (auto&& hdr : h) {
        headers_.push_back(std::move(hdr));
    }
    return std::move(*this);
}

auto HttpResponseHeaderBuilder::build() && -> HttpResponse {
    return std::move(*this).build(BodyContainer { });
}

auto HttpResponseHeaderBuilder::build(BodyContainer body) && 
    -> HttpResponse 
{
    return {
        std::move(proto_),
        std::move(headers_),
        std::move(body)
    };
}

auto HttpResponseBuilder::with_protocol(HttpResponseProtocolHeader p) && 
    -> HttpResponseHeaderBuilder
{
    return { std::move(p) };        
}

struct Slice {
    char const* start;
    char const* end;
};

struct ParsedRequestData {
    using Header = std::pair<Slice, Slice>;
    using HeaderContainer = std::vector<Header>;

    ParsedRequestData() :
        path { nullptr, nullptr }
    ,   headers { }
    {
        headers.reserve(32);
    }

    Slice path;
    HeaderContainer headers;
    std::vector<Slice> body_chunks;
};

struct ParsedResponseData {
    using Header = std::pair<Slice, Slice>;
    using HeaderContainer = std::vector<Header>;

    ParsedResponseData() :
        status_text { nullptr, nullptr }
    ,   headers { }
    ,   body_chunks { }
    {
        headers.reserve(32);
    }

    Slice status_text;
    HeaderContainer headers;
    std::vector<Slice> body_chunks;
};

auto http::detail::parse_request(char const* bytes, size_t size) noexcept
    -> ParseResult<std::pair<HttpRequest, size_t>>
{
        parser::http_parser parser;
        parser::http_parser_settings parser_settings;
        parser::http_parser_init(&parser, parser::HTTP_REQUEST);
        parser::http_parser_settings_init(&parser_settings);

        parser_settings.on_header_field = 
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedRequestData*>(parser->data);
                pd.headers.emplace_back(
                    Slice { data, data + len }, 
                    Slice { nullptr, nullptr });

                return 0;
            };

        parser_settings.on_header_value = 
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedRequestData*>(parser->data);
                std::get<1>(pd.headers.back()) = 
                    Slice { data, data + len };

                return 0;
            };

        parser_settings.on_url =
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedRequestData*>(parser->data);
                pd.path = Slice { data, data + len };
                return 0;
            };

        parser_settings.on_body =
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedRequestData*>(parser->data);
                pd.body_chunks.push_back({ data, data + len });
                return 0;
            };

        auto data = ParsedRequestData { };
        parser.data = &data;

        auto parsed_len = http_parser_execute(&parser, 
                                              &parser_settings,
                                              bytes,
                                              size);

        // We need to call `http_parser_execute` twice to force the parser
        // to tell us if `data` contains a complete HTTP object or not.
        // Without this call, the parser won't return an error because it
        // thinks more data will follow.
        //
        // From [http-parser's README][1]:
        // > To tell `http_parser` about EOF, give `0` as the fourth 
        // > parameter to `http_parser_execute()`
        //
        // [1]: https://github.com/nodejs/http-parser
        http_parser_execute(&parser, 
                            &parser_settings,
                            bytes + size,
                            0);

        if (parser.http_errno) {
            return result::err(
                make_error_code(static_cast<ParseError>(parser.http_errno)));
        }

        assert(parsed_len);
        assert(parser.method >= 0);

        if (parser.method > static_cast<int>(Method::Trace)) {
            return result::err(make_error_code(ParseError::INVALID_METHOD));
        }

        auto hdrs = std::vector<Header> { };
        hdrs.reserve(data.headers.size());

        std::transform(
            data.headers.begin(), 
            data.headers.end(),
            std::back_inserter(hdrs),
            [](auto h) {
                assert(std::get<0>(h).end >= std::get<0>(h).start);
                assert(std::get<1>(h).end >= std::get<1>(h).start);

                return std::make_pair(
                    std::string { std::get<0>(h).start, std::get<0>(h).end },
                    std::string { std::get<1>(h).start, std::get<1>(h).end });
            });

        auto body = std::vector<uint8_t> { };

        body.reserve(
            std::accumulate(data.body_chunks.begin(),
                    data.body_chunks.end(),
                    static_cast<size_t>(0),
                    [](auto const& acc, auto const& item) {
                        return acc + (item.end - item.start);
                    })
        );

        std::for_each(
            data.body_chunks.begin(),
            data.body_chunks.end(),
            [&](auto ch) {
                assert(ch.start != nullptr);
                assert(ch.end != nullptr);
                std::copy(
                    ch.start,
                    ch.end,
                    std::back_inserter(body)
                );
            });

        return result::ok(std::make_pair(
            HttpRequestBuilder { }
                .with_protocol({ 
                    static_cast<Method>(parser.method), 
                    std::string { data.path.start, data.path.end },
                    Version::Http11 
                })
                .with_headers(std::move(hdrs))
                .build(std::move(body)),
            parsed_len
        ));

}

auto http::detail::parse_response(char const* bytes, size_t size) noexcept
    -> ParseResult<std::pair<HttpResponse, size_t>>
{
        parser::http_parser parser;
        parser::http_parser_settings parser_settings;
        parser::http_parser_init(&parser, parser::HTTP_RESPONSE);
        parser::http_parser_settings_init(&parser_settings);

        parser_settings.on_header_field = 
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedResponseData*>(parser->data);
                pd.headers.emplace_back(
                    Slice { data, data + len }, 
                    Slice { nullptr, nullptr });

                return 0;
            };

        parser_settings.on_header_value = 
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedResponseData*>(parser->data);
                std::get<1>(pd.headers.back()) = 
                    Slice { data, data + len };

                return 0;
            };

        parser_settings.on_status =
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedResponseData*>(parser->data);
                pd.status_text = Slice { data, data + len };
                return 0;
            };

        parser_settings.on_body =
            [](auto* parser, auto const* data, auto len) -> int {
                auto& pd = *reinterpret_cast<ParsedResponseData*>(parser->data);
                pd.body_chunks.push_back({ data, data + len });
                return 0;
            };

        auto data = ParsedResponseData { };
        parser.data = &data;

        auto parsed_len = http_parser_execute(&parser, 
                                              &parser_settings,
                                              bytes,
                                              size);

        // We need to call `http_parser_execute` twice to force the parser
        // to tell us if `data` contains a complete HTTP object or not.
        // Without this call, the parser won't return an error because it
        // thinks more data will follow.
        //
        // From [http-parser's README][1]:
        // > To tell `http_parser` about EOF, give `0` as the fourth 
        // > parameter to `http_parser_execute()`
        //
        // [1]: https://github.com/nodejs/http-parser
        http_parser_execute(&parser, 
                            &parser_settings,
                            bytes + size,
                            0);

        if (parser.http_errno) {
            return result::err(
                make_error_code(static_cast<ParseError>(parser.http_errno)));
        }

        assert(parsed_len);

        auto hdrs = std::vector<Header> { };
        hdrs.reserve(data.headers.size());

        std::transform(
            data.headers.begin(), 
            data.headers.end(),
            std::back_inserter(hdrs),
            [](auto h) {
                assert(std::get<0>(h).end >= std::get<0>(h).start);
                assert(std::get<1>(h).end >= std::get<1>(h).start);

                return std::make_pair(
                    std::string { std::get<0>(h).start, std::get<0>(h).end },
                    std::string { std::get<1>(h).start, std::get<1>(h).end });
            });

        auto body = std::vector<uint8_t> { };

        body.reserve(
            std::accumulate(data.body_chunks.begin(),
                    data.body_chunks.end(),
                    static_cast<size_t>(0),
                    [](auto const& acc, auto const& item) {
                        return acc + (item.end - item.start);
                    })
        );

        std::for_each(
            data.body_chunks.begin(),
            data.body_chunks.end(),
            [&](auto ch) {
                assert(ch.start != nullptr);
                assert(ch.end != nullptr);
                std::copy(
                    ch.start,
                    ch.end,
                    std::back_inserter(body)
                );
            });

        return result::ok(std::make_pair(
            HttpResponseBuilder { }
                .with_protocol({ 
                    Version::Http11,
                    static_cast<size_t>(parser.status_code),
                    std::string { 
                        data.status_text.start, 
                        data.status_text.end 
                    }
                })
                .with_headers(std::move(hdrs))
                .build(std::move(body)),
            parsed_len
        ));
}
