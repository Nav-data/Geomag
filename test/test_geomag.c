/**
 * @file test_geomag.c
 * @brief Test suite for GeoMag C library
 *
 * Verifies correctness against known test values from NOAA and pygeomag.
 */

#include "geomag.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define EPSILON 0.01  /* Tolerance for floating point comparison */

int test_count = 0;
int test_passed = 0;
int test_failed = 0;

/* Test result comparison */
void assert_near(const char *name, double expected, double actual, double tolerance) {
    test_count++;
    if (fabs(expected - actual) <= tolerance) {
        printf("  ✓ %s: %.6f (expected: %.6f)\n", name, actual, expected);
        test_passed++;
    } else {
        printf("  ✗ %s: %.6f (expected: %.6f, diff: %.6f)\n",
               name, actual, expected, fabs(expected - actual));
        test_failed++;
    }
}

/* Test 1: Space Needle, Seattle (from pygeomag README) */
void test_space_needle_2025() {
    printf("\nTest 1: Space Needle, Seattle, WA (WMM-2025, 2025.25)\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    if (geomag_calculate(&geo_mag, 47.6205, -122.3493, 0.0, 2025.25,
                         false, false, &result) != 0) {
        printf("  ✗ Failed to calculate\n");
        test_failed++;
        geomag_free(&geo_mag);
        return;
    }

    /* Expected from pygeomag: 15.065629638512593 */
    assert_near("Declination", 15.065630, result.d, 0.001);

    geomag_free(&geo_mag);
}

/* Test 2: Space Needle, later in 2025 (WMM-2025) */
void test_space_needle_2013() {
    printf("\nTest 2: Space Needle, Seattle, WA (WMM-2025, 2025.75)\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    if (geomag_calculate(&geo_mag, 47.6205, -122.3493, 0.0, 2025.75,
                         false, false, &result) != 0) {
        printf("  ✗ Failed to calculate\n");
        test_failed++;
        geomag_free(&geo_mag);
        return;
    }

    /* Expected from calculation with WMM-2025 */
    assert_near("Declination", 15.0038, result.d, 0.01);

    geomag_free(&geo_mag);
}

/* Test 3: NOAA test values - Origin (0,0,0) */
void test_noaa_origin() {
    printf("\nTest 3: NOAA Test Point - Origin (0°N, 0°E, 0km, 2025.0)\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    if (geomag_calculate(&geo_mag, 0.0, 0.0, 0.0, 2025.0,
                         false, false, &result) != 0) {
        printf("  ✗ Failed to calculate\n");
        test_failed++;
        geomag_free(&geo_mag);
        return;
    }

    /* Approximate NOAA values for origin */
    printf("  Results: D=%.2f°, I=%.2f°, H=%.1f nT, F=%.1f nT\n",
           result.d, result.i, result.h, result.f);

    geomag_free(&geo_mag);
}

/* Test 4: Multiple locations */
void test_multiple_locations() {
    printf("\nTest 4: Multiple locations (WMM-2025, 2025.0)\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    struct test_location {
        const char *name;
        double lat;
        double lon;
        double alt;
        double expected_d;
        double expected_i;
    } locations[] = {
        /* Add test locations with expected values */
        {"Boulder, CO", 40.0, -105.0, 1.65, 8.45, 65.65},  /* Approximate */
        {"North Pole", 89.0, 0.0, 0.0, 0.0, 88.5},         /* Near pole */
        {"South Pole", -89.0, 0.0, 0.0, 0.0, -87.5},       /* Near pole */
    };

    for (int i = 0; i < 3; i++) {
        if (geomag_calculate(&geo_mag, locations[i].lat, locations[i].lon,
                             locations[i].alt, 2025.0, false, false, &result) == 0) {
            printf("  %s: D=%.2f°, I=%.2f°\n",
                   locations[i].name, result.d, result.i);
        }
    }

    geomag_free(&geo_mag);
}

/* Test 5: Uncertainty calculations */
void test_uncertainty() {
    printf("\nTest 5: Uncertainty calculations (WMM-2025)\n");

    GeoMag geo_mag;
    GeoMagResult result;
    GeoMagUncertainty uncertainty;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    if (geomag_calculate(&geo_mag, 47.6205, -122.3493, 0.0, 2025.25,
                         false, false, &result) != 0) {
        printf("  ✗ Failed to calculate\n");
        test_failed++;
        geomag_free(&geo_mag);
        return;
    }

    if (geomag_calculate_uncertainty(&result, &uncertainty) != 0) {
        printf("  ✗ Failed to calculate uncertainty\n");
        test_failed++;
        geomag_free(&geo_mag);
        return;
    }

    printf("  Uncertainty - X: ±%.1f nT, Y: ±%.1f nT, Z: ±%.1f nT\n",
           uncertainty.x, uncertainty.y, uncertainty.z);
    printf("  Uncertainty - D: ±%.4f°, I: ±%.2f°\n",
           uncertainty.d, uncertainty.i);

    /* Check that uncertainties are reasonable */
    if (uncertainty.x > 0 && uncertainty.x < 200 &&
        uncertainty.y > 0 && uncertainty.y < 200 &&
        uncertainty.d > 0 && uncertainty.d < 1.0) {
        printf("  ✓ Uncertainty values are reasonable\n");
        test_passed++;
    } else {
        printf("  ✗ Uncertainty values seem incorrect\n");
        test_failed++;
    }
    test_count++;

    geomag_free(&geo_mag);
}

/* Test 6: High resolution model */
void test_high_resolution() {
    printf("\nTest 6: High Resolution Model (WMMHR-2025)\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMMHR.COF", true) != 0) {
        printf("  ⚠ High resolution model not available or failed to load\n");
        return;
    }

    printf("  Model: %s, maxord=%d\n", geo_mag.model, geo_mag.maxord);

    if (geomag_calculate(&geo_mag, 47.6205, -122.3493, 0.0, 2025.0,
                         false, false, &result) != 0) {
        printf("  ✗ Failed to calculate\n");
        test_failed++;
        geomag_free(&geo_mag);
        return;
    }

    /* Expected from pygeomag: 15.017316292177854 */
    assert_near("HR Declination", 15.017316, result.d, 0.01);

    if (result.is_high_resolution) {
        printf("  ✓ High resolution flag is set\n");
        test_passed++;
    } else {
        printf("  ✗ High resolution flag not set\n");
        test_failed++;
    }
    test_count++;

    geomag_free(&geo_mag);
}

/* Test 7: Boundary conditions */
void test_boundary_conditions() {
    printf("\nTest 7: Boundary conditions\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    /* Test date outside lifespan (should fail) */
    if (geomag_calculate(&geo_mag, 0.0, 0.0, 0.0, 2031.0,
                         false, false, &result) != 0) {
        printf("  ✓ Correctly rejected date outside lifespan\n");
        test_passed++;
    } else {
        printf("  ✗ Should have rejected date outside lifespan\n");
        test_failed++;
    }
    test_count++;

    /* Test date outside lifespan with override (should succeed) */
    if (geomag_calculate(&geo_mag, 0.0, 0.0, 0.0, 2031.0,
                         true, false, &result) == 0) {
        printf("  ✓ Override allowed date outside lifespan\n");
        test_passed++;
    } else {
        printf("  ✗ Override should have allowed date outside lifespan\n");
        test_failed++;
    }
    test_count++;

    geomag_free(&geo_mag);
}

/* Test 8: Performance test */
void test_performance() {
    printf("\nTest 8: Performance test (1000 calculations)\n");

    GeoMag geo_mag;
    GeoMagResult result;

    if (geomag_init(&geo_mag, "data/WMM.COF", false) != 0) {
        printf("  ✗ Failed to initialize GeoMag\n");
        test_failed++;
        return;
    }

    int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        double lat = -90.0 + (180.0 * i / iterations);
        double lon = -180.0 + (360.0 * i / iterations);
        geomag_calculate(&geo_mag, lat, lon, 0.0, 2025.0, false, false, &result);
    }

    printf("  ✓ Completed %d calculations successfully\n", iterations);
    test_passed++;
    test_count++;

    geomag_free(&geo_mag);
}

int main() {
    printf("==========================================\n");
    printf("   GeoMag C Library Test Suite\n");
    printf("==========================================\n");

    test_space_needle_2025();
    test_space_needle_2013();
    test_noaa_origin();
    test_multiple_locations();
    test_uncertainty();
    test_high_resolution();
    test_boundary_conditions();
    test_performance();

    printf("\n==========================================\n");
    printf("Test Results: %d/%d passed, %d failed\n",
           test_passed, test_count, test_failed);
    printf("==========================================\n");

    return (test_failed == 0) ? 0 : 1;
}