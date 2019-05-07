#include "hostname.h"
#include <iostream>

#define CATCH_CONFIG_MAIN // as a main function
#include "catch.hpp"

using namespace jouyouyun::example::unittest;

/**
  * REQUIRE VS CHECK:
  * REQUIRE: terminated when failed
  * CHECK: continue when failed
 **/

// TEST_CASE and SECTION
TEST_CASE("Get/Set Hostname", "[hostname]") {
    // define some common variables

    SECTION("Get") {
        Hostname h("testdata/hostname");
        REQUIRE(h.Get() == "Jouyouyun");
    }

    SECTION("Set") {
        Hostname h("testdata/hostname_tmp");
        REQUIRE(h.Set("Deepin") == 0);
        REQUIRE(h.Get() == "Deepin");
        REQUIRE(h.Set("Wen") == 0);
        REQUIRE(h.Get() == "Wen");
    }
}

// BDD Style
SCENARIO("Get/Set Hostname", "[hostname]") {
    GIVEN("A Hostname object") {
        Hostname h("testdata/hostname");

        WHEN("Get") {
            std::string name = h.Get();
            THEN("Result") {
                REQUIRE(name == "Jouyouyun");
            }
        }
        WHEN("Set") {
            Hostname h1("testdata/hostname_tmp");
            int ret = h1.Set("Wen");
            THEN("Set Result") {
                REQUIRE(ret == 0);
                REQUIRE(h1.Get() == "Wen");
            }
        }
    }
}
