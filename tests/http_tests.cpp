#include "result/result.hpp"
#include "http/http.hpp"
#include "catch.hpp"
#include <streambuf>
#include <iostream>
#include <string>

SCENARIO("HTTP parsing", "[http]") {
    GIVEN("A valid HTTP request in bytes") {
        constexpr char HTTP_REQUEST[] = 
            "GET /index HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "5\r\n"
            "Hello\r\n"
            "8\r\n"
            ", World!\r\n"
            "0\r\n"
            "\r\n";

        WHEN("It is parsed") {
            using std::begin;
            using std::end;

            auto result = http::parse_request(
                begin(HTTP_REQUEST),
                end(HTTP_REQUEST)-1
            );

            if (!result) {
                throw std::system_error { result::error(std::move(result)) };
            }

            THEN("It should succeed") {
                REQUIRE(result.is_ok());
            }

            AND_THEN("It should consume the correct number of bytes") {
                auto num = std::get<1>(result::value(std::move(result)));
                REQUIRE(num == (size_t)std::distance(begin(HTTP_REQUEST),
                                                     end(HTTP_REQUEST)-1));
            }

            AND_THEN("It should have the correct HTTP protocol line") {
                auto request = std::get<0>(result::value(std::move(result)));
                REQUIRE(request.method() == http::Method::Get);
                REQUIRE(request.path() == "/index");
                REQUIRE(request.version() == http::Version::Http11);
            }

            AND_THEN("It should have the correct header values") {
                auto request = std::get<0>(result::value(std::move(result)));
                REQUIRE(3 == request.headers().size());

                auto& header = *begin(request.headers());
                REQUIRE(std::get<0>(header) == "Host");
                REQUIRE(std::get<1>(header) == "example.com");

                auto& content_type_header = request.headers().back();
                REQUIRE(std::get<0>(content_type_header) == "Content-Type");
                REQUIRE(std::get<1>(content_type_header) == "text/plain");
            }
        }
    }

    GIVEN("A valid HTTP response") {
        constexpr char HTTP_RESPONSE[] = 
            R"#(HTTP/1.1 200 OK
Server: example.com
Content-Length: 0

)#";
        WHEN("It is parsed") {
            using std::begin;
            using std::end;

            auto result = http::parse_response(
                begin(HTTP_RESPONSE),
                end(HTTP_RESPONSE)-1
            );

            if (!result) {
                throw std::system_error { result::error(std::move(result)) };
            }

            THEN("It should succeed") {
                REQUIRE(result.is_ok());
            }

            AND_THEN("It should consume the correct number of bytes") {
                auto num = std::get<1>(result::value(std::move(result)));
                REQUIRE(num == (size_t)std::distance(begin(HTTP_RESPONSE),
                                                     end(HTTP_RESPONSE)-1));
            }

            AND_THEN("It should have the correct HTTP protocol line") {
                auto response = std::get<0>(result::value(std::move(result)));
                REQUIRE(response.version() == http::Version::Http11);
                REQUIRE(response.status_code() == 200);
                REQUIRE(response.status_text() == "OK");
            }

            AND_THEN("It should have the correct header values") {
                auto request = std::get<0>(result::value(std::move(result)));
                REQUIRE(2 == request.headers().size());

                auto& header = *begin(request.headers());
                REQUIRE(std::get<0>(header) == "Server");
                REQUIRE(std::get<1>(header) == "example.com");
            }
        }
    }
}

template<typename T, typename Traits = std::char_traits<T>>
struct VectorStreamBuf : std::basic_streambuf<T, Traits> {
    VectorStreamBuf(std::vector<T>& v) {
        this->setp(v.data(), v.data() + v.size());
    }

    auto written() const -> size_t {
        return this->pptr() - this->pbase();
    }
};

SCENARIO("HTTP serialization", "[serialization]") {

    GIVEN("A user-created HTTP response") {

        auto content = std::string { "Hello, World!" };
        auto response = http::HttpResponseBuilder { }
            .with_protocol({ 
                http::Version::Http11,
                static_cast<size_t>(200),
                "OK"
            })
            .with_headers({
                std::make_pair("Server", "MyTestServer"),
                std::make_pair("Content-Type", "text/plain")
            })
            .build(content.begin(), content.end());

        WHEN("It is serialized") {

            auto buffer = std::vector<char>(256, '\0');
            auto stream = VectorStreamBuf<char> { buffer };
            std::basic_ostream<char> os { &stream };

            os << response;

            THEN("It should result in a valid byte representation") {
                auto result = http::parse_response(
                    buffer.begin(),
                    buffer.begin() + stream.written()
                );

                REQUIRE_NOTHROW([&] {
                    if (!result) {
                        throw std::system_error { result::error(result) };
                    }
                }());

                auto resp = std::get<0>(result::value(result));
                REQUIRE(std::get<1>(resp.headers().at(0)) 
                    == "MyTestServer");

                REQUIRE(std::string { resp.body().begin(), resp.body().end() }
                    == "Hello, World!");

            }
        }
    }
}
