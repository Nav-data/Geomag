/**
 * @file benchmark.c
 * @brief Performance benchmark for the GeoMag C library
 *
 * This benchmark measures the time to process different numbers of
 * geomag calculations (10000, 30000, 50000, 100000 records).
 */

#include "geomag.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/**
 * Get current time in microseconds
 */
static double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000000.0 + (double)tv.tv_usec;
}

/**
 * Run benchmark for a given number of records
 */
static void run_benchmark(GeoMag *geo_mag, int num_records) {
    GeoMagResult result;
    double start_time, end_time, elapsed_ms;
    int ret;
    
    /* Test locations with varied coordinates */
    struct {
        double lat;
        double lon;
        double alt;
        double time;
    } test_data[] = {
        {47.6205, -122.3493, 0.0, 2025.25},    /* Seattle */
        {40.7128, -74.0060, 0.1, 2025.5},      /* New York */
        {34.0522, -118.2437, 0.05, 2025.75},   /* Los Angeles */
        {25.7617, -80.1918, 0.0, 2026.0},      /* Miami */
        {41.8781, -87.6298, 0.2, 2025.3},      /* Chicago */
        {51.5074, -0.1278, 0.0, 2025.4},       /* London */
        {35.6762, 139.6503, 0.0, 2025.6},      /* Tokyo */
        {-33.8688, 151.2093, 0.0, 2025.8},     /* Sydney */
        {0.0, 0.0, 0.0, 2025.5},                /* Null Island */
        {90.0, 0.0, 0.0, 2025.5}                /* North Pole */
    };
    int num_test_locations = sizeof(test_data) / sizeof(test_data[0]);
    
    printf("Processing %d records... ", num_records);
    fflush(stdout);
    
    /* Start timing */
    start_time = get_time_us();
    
    /* Run calculations */
    for (int i = 0; i < num_records; i++) {
        /* Cycle through test locations */
        int idx = i % num_test_locations;
        
        ret = geomag_calculate(geo_mag, 
                              test_data[idx].lat,
                              test_data[idx].lon,
                              test_data[idx].alt,
                              test_data[idx].time,
                              true,    /* allow_date_outside_lifespan */
                              false,   /* raise_in_warning_zone */
                              &result);
        
        if (ret != 0 && ret != -2 && ret != -3) {
            fprintf(stderr, "\nError in calculation %d (return code: %d)\n", i, ret);
            return;
        }
    }
    
    /* End timing */
    end_time = get_time_us();
    elapsed_ms = (end_time - start_time) / 1000.0;
    
    /* Calculate statistics */
    double avg_time_us = (end_time - start_time) / num_records;
    double records_per_sec = num_records / (elapsed_ms / 1000.0);
    
    printf("Done!\n");
    printf("  Total time:        %10.2f ms\n", elapsed_ms);
    printf("  Average per calc:  %10.3f µs\n", avg_time_us);
    printf("  Throughput:        %10.0f calcs/sec\n", records_per_sec);
    printf("\n");
}

int main(void) {
    GeoMag geo_mag;
    int ret;
    
    printf("================================================================================\n");
    printf("                    GeoMag Library Performance Benchmark\n");
    printf("================================================================================\n\n");
    
    /* Initialize with standard WMM model */
    printf("Initializing WMM model...\n");
    ret = geomag_init(&geo_mag, "data/WMM.COF", false);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize GeoMag model\n");
        return 1;
    }
    
    printf("Model: %s\n", geo_mag.model);
    printf("Epoch: %.1f\n", geo_mag.epoch);
    printf("Max Order: %d\n\n", geo_mag.maxord);
    
    /* Record counts to test */
    int record_counts[] = {10000, 30000, 50000, 100000};
    int num_tests = sizeof(record_counts) / sizeof(record_counts[0]);
    
    printf("--------------------------------------------------------------------------------\n");
    printf("Running benchmarks...\n");
    printf("--------------------------------------------------------------------------------\n\n");
    
    /* Run benchmarks */
    for (int i = 0; i < num_tests; i++) {
        run_benchmark(&geo_mag, record_counts[i]);
    }
    
    geomag_free(&geo_mag);
    
    /* Now test with high resolution model */
    printf("================================================================================\n");
    printf("High Resolution Model Benchmark (WMMHR)\n");
    printf("================================================================================\n\n");
    
    printf("Initializing WMMHR model...\n");
    ret = geomag_init(&geo_mag, "data/WMMHR.COF", true);
    if (ret != 0) {
        fprintf(stderr, "Failed to initialize WMMHR model\n");
        printf("(High resolution model may not be available)\n\n");
    } else {
        printf("Model: %s\n", geo_mag.model);
        printf("Epoch: %.1f\n", geo_mag.epoch);
        printf("Max Order: %d\n\n", geo_mag.maxord);
        
        printf("--------------------------------------------------------------------------------\n");
        printf("Running high resolution benchmarks...\n");
        printf("--------------------------------------------------------------------------------\n\n");
        
        /* Run smaller benchmarks for high resolution (slower) */
        int hr_record_counts[] = {10000, 30000, 50000, 100000};
        int num_hr_tests = sizeof(hr_record_counts) / sizeof(hr_record_counts[0]);
        
        for (int i = 0; i < num_hr_tests; i++) {
            run_benchmark(&geo_mag, hr_record_counts[i]);
        }
        
        geomag_free(&geo_mag);
    }
    
    printf("================================================================================\n");
    printf("Benchmark completed!\n");
    printf("================================================================================\n");
    
    return 0;
}

