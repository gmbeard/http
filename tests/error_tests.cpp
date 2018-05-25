#include "http/error.hpp"
#include "catch.hpp"

SCENARIO("Parse errors", "[error]") {

    GIVEN("A parse error code") {

        auto ec = make_error_code(http::ParseError::INVALID_EOF_STATE);

        WHEN("It is thrown") {

            auto ex = std::system_error { ec };

            THEN("It should have the correct message") {

                CAPTURE(ex.what());
                REQUIRE(
                    std::string { "stream ended at an unexpected time" }
                        == ex.what() );
            }
        }
    }
}
