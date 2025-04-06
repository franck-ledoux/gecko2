/*----------------------------------------------------------------------------*/
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>
#include <gmds/quality/HexQuality.h>

/*----------------------------------------------------------------------------*/
using namespace gmds;
using namespace gmds::quality;
using Catch::Approx;

/*----------------------------------------------------------------------------*/

TEST_CASE("HexQualityTestClass - test1", "[hexquality]") {
    HexQuality he({
        0, 0, 0,
         1, 0, 0,
        1, 1, 0,
        0, 1, 0,
        0, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 1, 1});

    SECTION("Test des coordonnées de p[0]") {
        REQUIRE(he.p[0][0] == Approx(0).epsilon(0.001));
        REQUIRE(he.p[0][1] == Approx(0).epsilon(0.001));
        REQUIRE(he.p[0][2] == Approx(0).epsilon(0.001));
    }

    SECTION("Test des propriétés de HexQuality") {
        REQUIRE(he.volume() == Approx(1).epsilon(0.01));
        REQUIRE(he.diagonal() == Approx(1).epsilon(0.01));
        REQUIRE(he.jacobian() == Approx(1).epsilon(0.01));
        REQUIRE(he.scaledJacobian() == Approx(1).epsilon(0.01));
        REQUIRE(he.edgeRatio() == Approx(1).epsilon(0.01));
        REQUIRE(he.maximumEdgeRatio() == Approx(1).epsilon(0.01));
        REQUIRE(he.meanAspectFrobenius() == Approx(8).epsilon(0.01));
        REQUIRE(he.maximumAspectFrobenius() == Approx(1).epsilon(0.01));
        REQUIRE(he.stretch() == Approx(1).epsilon(0.01));
        REQUIRE(he.shear() == Approx(1).epsilon(0.01));
        REQUIRE(he.shape() == Approx(1).epsilon(0.01));
        REQUIRE(he.skew() == Approx(0).epsilon(0.01));
        REQUIRE(he.oddy() == Approx(0).epsilon(0.01));
        REQUIRE(he.taper() == Approx(0).epsilon(0.01));
    }
    HexQuality he_inv({
       0, 0, 0,
        0, 1, 0,
       1, 1, 0,
       1, 0, 0,
       0, 0, 1,
       0, 1, 1,
       1, 1, 1,
       1, 0, 1});
    REQUIRE(he_inv.scaledJacobian() == Approx(-1).epsilon(0.01));

}

TEST_CASE("HexQualityTestClass - test2", "[hexquality]") {
    HexQuality he = HexQuality::build(math::Point(0, 0, 0),
                                      math::Point(2, 0, 0),
                                      math::Point(2, 1, 0),
                                      math::Point(0, 1, 0),
                                      math::Point(0, 0, 1),
                                      math::Point(2, 0, 1),
                                      math::Point(2, 1, 1),
                                      math::Point(0, 1, 1));

        REQUIRE(he.p[0][0] == Approx(0).epsilon(0.001));
        REQUIRE(he.p[0][1] == Approx(0).epsilon(0.001));
        REQUIRE(he.p[0][2] == Approx(0).epsilon(0.001));

        REQUIRE(he.volume() == Approx(2).epsilon(0.01));
        REQUIRE(he.diagonal() == Approx(1).epsilon(0.01));
        REQUIRE(he.jacobian() == Approx(2).epsilon(0.01));
        REQUIRE(he.scaledJacobian() == Approx(1).epsilon(0.01));
        REQUIRE(he.edgeRatio() == Approx(2).epsilon(0.01));
        REQUIRE(he.maximumEdgeRatio() == Approx(2).epsilon(0.01));
        REQUIRE(he.meanAspectFrobenius() == Approx(9.797).epsilon(0.01));
        REQUIRE(he.maximumAspectFrobenius() == Approx(1.2247).epsilon(0.01));
        REQUIRE(he.stretch() == Approx(0.707).epsilon(0.01));
        REQUIRE(he.shear() == Approx(2).epsilon(0.01));
        REQUIRE(he.shape() == Approx(0.7937).epsilon(0.01));
        REQUIRE(he.skew() == Approx(0).epsilon(0.01));
        REQUIRE(he.oddy() == Approx(2.3811).epsilon(0.01));
        REQUIRE(he.taper() == Approx(0).epsilon(0.01));
}
/*----------------------------------------------------------------------------*/
TEST_CASE("HexQualityTestClass - test3", "[hexquality]") {
    HexQuality he = HexQuality::build(math::Point(188, 28, 29),
                                      math::Point(191, 28, 29),
                                      math::Point(191, 31, 26),
                                      math::Point(188, 31, 26),
                                      math::Point(188, 27, 28),
                                      math::Point(191, 27, 28),
                                      math::Point(191, 29, 25),
                                      math::Point(188, 29, 25));

    SECTION("Test du skew pour he") {
        REQUIRE(he.skew() == Approx(0.106).epsilon(0.01));
    }
}

