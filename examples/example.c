/**
 * @file example.c
 * @brief Example usage of the GeoMag C library
 *
 * This example demonstrates how to calculate magnetic declination
 * for the Space Needle in Seattle, WA.
 */

#include "geomag.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    GeoMag geo_mag;
    GeoMagResult result;
    GeoMagUncertainty uncertainty;
    int ret;

    /* Example 1: Calculate declination at Space Needle (Seattle, WA) using WMM-2025 */
    printf("=== Example 1: Space Needle, Seattle, WA (WMM-2025) ===\n");

    ret = geomag_init(&geo_mag, "data/WMM_2025.COF", false);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize GeoMag\n");
        return 1;
    }

    printf("Model: %s\n", geo_mag.model);
    printf("Epoch: %.1f\n", geo_mag.epoch);
    printf("Release Date: %s\n\n", geo_mag.release_date);

    /* Space Needle coordinates */
    double latitude = 47.6205;   /* North */
    double longitude = -122.3493; /* West */
    double altitude = 0.0;        /* Sea level */
    double time = 2025.25;        /* March 2025 */

    ret = geomag_calculate(&geo_mag, latitude, longitude, altitude, time,
                           false, false, &result);
    if (ret != 0) {
        fprintf(stderr, "Failed to calculate magnetic field\n");
        return 1;
    }

    printf("Location: %.4f°N, %.4f°E\n", latitude, longitude);
    printf("Altitude: %.1f km\n", altitude);
    printf("Date: %.2f\n\n", time);

    printf("Magnetic Field Components:\n");
    printf("  Declination (D):    %8.4f° (magnetic variation)\n", result.d);
    printf("  Inclination (I):    %8.4f° (dip angle)\n", result.i);
    printf("  Total Intensity (F):%8.1f nT\n", result.f);
    printf("  Horizontal (H):     %8.1f nT\n", result.h);
    printf("  North (X):          %8.1f nT\n", result.x);
    printf("  East (Y):           %8.1f nT\n", result.y);
    printf("  Vertical (Z):       %8.1f nT\n", result.z);

    if (result.gv != -999.0) {
        printf("  Grid Variation:     %8.4f°\n", result.gv);
    }

    /* Calculate uncertainties */
    ret = geomag_calculate_uncertainty(&result, &uncertainty);
    if (ret == 0) {
        printf("\nUncertainty Estimates:\n");
        printf("  Declination: ±%.4f°\n", uncertainty.d);
        printf("  Inclination: ±%.2f°\n", uncertainty.i);
        printf("  Total:       ±%.1f nT\n", uncertainty.f);
    }

    geomag_free(&geo_mag);

    /* Example 2: Historical calculation using WMM-2010 */
    printf("\n\n=== Example 2: Same location 12 years earlier (WMM-2010) ===\n");

    ret = geomag_init(&geo_mag, "data/WMM_2010.COF", false);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize GeoMag\n");
        return 1;
    }

    time = 2013.25;  /* March 2013 */

    ret = geomag_calculate(&geo_mag, latitude, longitude, altitude, time,
                           false, false, &result);
    if (ret != 0) {
        fprintf(stderr, "Failed to calculate magnetic field\n");
        return 1;
    }

    printf("Model: %s\n", geo_mag.model);
    printf("Date: %.2f\n\n", time);
    printf("Declination (D): %8.4f°\n", result.d);
    printf("Change over 12 years: Declination decreased by about 1.35°\n");

    geomag_free(&geo_mag);

    /* Example 3: Multiple locations */
    printf("\n\n=== Example 3: Declination at various locations (2025) ===\n");

    ret = geomag_init(&geo_mag, "data/WMM_2025.COF", false);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize GeoMag\n");
        return 1;
    }

    struct {
        const char *name;
        double lat;
        double lon;
    } locations[] = {
        {"New York, NY", 40.7128, -74.0060},
        {"Los Angeles, CA", 34.0522, -118.2437},
        {"Miami, FL", 25.7617, -80.1918},
        {"Chicago, IL", 41.8781, -87.6298},
        {"Denver, CO", 39.7392, -104.9903},
        {"London, UK", 51.5074, -0.1278},
        {"Tokyo, Japan", 35.6762, 139.6503},
        {"Sydney, Australia", -33.8688, 151.2093}
    };

    time = 2025.5;  /* Mid-2025 */

    for (int i = 0; i < 8; i++) {
        ret = geomag_calculate(&geo_mag, locations[i].lat, locations[i].lon, 0.0, time,
                               false, false, &result);
        if (ret == 0) {
            printf("  %-20s: D=%7.2f°, I=%6.2f°\n",
                   locations[i].name, result.d, result.i);
        }
    }

    geomag_free(&geo_mag);

    /* Example 4: High resolution model */
    printf("\n\n=== Example 4: High Resolution Model (WMMHR-2025) ===\n");

    ret = geomag_init(&geo_mag, "data/WMMHR_2025.COF", true);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize high resolution model\n");
        return 1;
    }

    printf("Model: %s (High Resolution, %d degrees)\n", geo_mag.model, geo_mag.maxord);

    time = 2025.0;
    ret = geomag_calculate(&geo_mag, 47.6205, -122.3493, 0.0, time,
                           false, false, &result);
    if (ret == 0) {
        printf("Space Needle declination: %.6f°\n", result.d);
        printf("(More precise due to crustal field modeling)\n");
    }

    geomag_free(&geo_mag);

    printf("\n=== All examples completed successfully! ===\n");
    return 0;
}