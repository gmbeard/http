#include "result/result.hpp"
#include "http/http.hpp"
#include "catch.hpp"
#include <iostream>

SCENARIO("HTTP Requests", "[http]") {
    GIVEN("A valid HTTP request in bytes") {
        constexpr char HTTP_REQUEST[] = 
            R"#(GET /index HTTP/1.1
Host: example.com

    )#";
        WHEN("It is parsed") {
            using std::begin;
            using std::end;
            auto result = http::parse_request(
                begin(HTTP_REQUEST),
                end(HTTP_REQUEST)-1
            );

            THEN("It should succeed") {
                REQUIRE(result.is_ok());
            }

            AND_THEN("It should have the correct HTTP protocol line") {
                auto request = result::value(std::move(result));
                REQUIRE(request.method() == http::Method::Get);
                REQUIRE(request.path() == "/index");
                REQUIRE(request.version() == http::Version::Http11);
            }

            AND_THEN("It should have the correct header values") {
                auto request = result::value(std::move(result));
                REQUIRE(1 == size(request.headers()));

                auto& header = *begin(request.headers());
                REQUIRE(std::get<0>(header) == "Host");
                REQUIRE(std::get<1>(header) == "example.com");
            }
        }
    }
}
