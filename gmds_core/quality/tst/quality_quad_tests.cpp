/*----------------------------------------------------------------------------*/
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>

#include <gmds/quality/QuadQuality.h>
/*----------------------------------------------------------------------------*/
using namespace gmds;
using namespace gmds::quality;
using Catch::Approx;

/*----------------------------------------------------------------------------*/

TEST_CASE("QuadQualityTestClass - test1", "[quadquality]") {
    QuadQuality qe({0, 0, 0,
                    1, 0, 0,
                    1, 1, 0,
                    0, 1, 0});

    SECTION("Test des coordonnées de p[0]") {
        REQUIRE(qe.p[0][0] == Approx(0).epsilon(0.001));
        REQUIRE(qe.p[0][1] == Approx(0).epsilon(0.001));
        REQUIRE(qe.p[0][2] == Approx(0).epsilon(0.001));
    }

    SECTION("Test des propriétés de QuadQuality") {
        REQUIRE(qe.jacobian() == Approx(1).epsilon(0.01));
        REQUIRE(qe.scaledJacobian() == Approx(1).epsilon(0.01));
        REQUIRE(qe.aspectRatio() == Approx(1).epsilon(0.01));
        REQUIRE(qe.signedArea() == Approx(1).epsilon(0.01));
        REQUIRE(qe.angleDeviation() == Approx(0).epsilon(0.01));
        REQUIRE(qe.skew() == Approx(0).epsilon(0.01));
    }

    QuadQuality qe2 = QuadQuality::build(math::Point(0, 0, 0),
                                         math::Point(2, 0, 0),
                                         math::Point(2, 1, 0),
                                         math::Point(0, 1, 0));

    SECTION("Test des coordonnées de p[0] pour qe2") {
        REQUIRE(qe2.p[0][0] == Approx(0).epsilon(0.001));
        REQUIRE(qe2.p[0][1] == Approx(0).epsilon(0.001));
        REQUIRE(qe2.p[0][2] == Approx(0).epsilon(0.001));
    }

    SECTION("Test des propriétés de QuadQuality pour qe2") {
        REQUIRE(qe2.jacobian() == Approx(2).epsilon(0.01));
        REQUIRE(qe2.scaledJacobian() == Approx(1).epsilon(0.01));
        REQUIRE(qe2.aspectRatio() == Approx(6).epsilon(0.01));
        REQUIRE(qe2.signedArea() == Approx(2).epsilon(0.01));
        REQUIRE(qe2.angleDeviation() == Approx(0).epsilon(0.01));
        REQUIRE(qe2.skew() == Approx(0).epsilon(0.01));
    }

    QuadQuality qe3 = QuadQuality::build(math::Point(0, 0, 0),
                                         math::Point(2, 0, 0),
                                         math::Point(2, 1, 0),
                                         math::Point(1, 1, 0));

    SECTION("Test du skew pour qe3") {
        REQUIRE(qe3.skew() == Approx(0.447).epsilon(0.01));
    }
}
