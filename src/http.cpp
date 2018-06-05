#include "http/http.hpp"

using namespace http;

HttpRequest::HttpRequest(HttpRequestProtocolHeader h, HeaderContainer c)
    :   protocol_ { std::move(h) }
    ,   headers_ { std::move(c) }
{ }

HttpHeaderBuilder::HttpHeaderBuilder(HttpRequestProtocolHeader p)
    : proto_{p}
{ }

auto HttpHeaderBuilder::with_header(Header h) && 
    -> HttpHeaderBuilder&&
{
    headers_.emplace_back(std::move(h));
    return std::move(*this);
}

auto HttpHeaderBuilder::with_headers(std::initializer_list<Header> h) &&
    -> HttpHeaderBuilder&&
{
    for (auto&& hdr : h) {
        headers_.push_back(std::move(hdr));
    }
    return std::move(*this);
}

auto HttpHeaderBuilder::build() && -> HttpRequest {
    return {
        std::move(proto_),
        std::move(headers_)
    };
}

auto HttpRequestBuilder::with_protocol(HttpRequestProtocolHeader p) && 
    -> HttpHeaderBuilder
{
    return { std::move(p) };        
}
