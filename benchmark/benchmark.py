#!/usr/bin/env python3
"""Performance benchmark for the GeoMag Python library.

This benchmark measures the time to process different numbers of
geomag calculations (10000, 30000, 50000, 100000 records).
"""

import sys
import time
from pathlib import Path

# Add parent directory to path to import geomag_c
sys.path.insert(0, str(Path(__file__).parent.parent))

from geomag_c import GeoMag


def format_time(seconds):
    """Format time in appropriate units."""
    if seconds < 0.001:
        return f"{seconds * 1_000_000:.3f} µs"
    elif seconds < 1:
        return f"{seconds * 1000:.2f} ms"
    else:
        return f"{seconds:.2f} s"


def run_benchmark(geo_mag, num_records):
    """Run benchmark for a given number of records."""
    # Test locations with varied coordinates
    test_data = [
        (47.6205, -122.3493, 0.0, 2025.25),     # Seattle
        (40.7128, -74.0060, 0.1, 2025.5),       # New York
        (34.0522, -118.2437, 0.05, 2025.75),    # Los Angeles
        (25.7617, -80.1918, 0.0, 2026.0),       # Miami
        (41.8781, -87.6298, 0.2, 2025.3),       # Chicago
        (51.5074, -0.1278, 0.0, 2025.4),        # London
        (35.6762, 139.6503, 0.0, 2025.6),       # Tokyo
        (-33.8688, 151.2093, 0.0, 2025.8),      # Sydney
        (0.0, 0.0, 0.0, 2025.5),                # Null Island
        (90.0, 0.0, 0.0, 2025.5)                # North Pole
    ]
    num_test_locations = len(test_data)

    print(f"Processing {num_records:,} records... ", end='', flush=True)

    # Start timing
    start_time = time.perf_counter()

    # Run calculations
    for i in range(num_records):
        # Cycle through test locations
        idx = i % num_test_locations
        lat, lon, alt, date_time = test_data[idx]

        try:
            result = geo_mag.calculate(
                lat=lat,
                lon=lon,
                alt=alt,
                time=date_time,
                allow_date_outside_lifespan=True,
                raise_in_warning_zone=False
            )
        except Exception as e:
            print(f"\nError in calculation {i}: {e}")
            return

    # End timing
    end_time = time.perf_counter()
    elapsed_seconds = end_time - start_time
    elapsed_ms = elapsed_seconds * 1000

    # Calculate statistics
    avg_time_us = (elapsed_seconds * 1_000_000) / num_records
    records_per_sec = num_records / elapsed_seconds

    print("Done!")
    print(f"  Total time:        {elapsed_ms:10.2f} ms")
    print(f"  Average per calc:  {avg_time_us:10.3f} µs")
    print(f"  Throughput:        {records_per_sec:10,.0f} calcs/sec")
    print()


def main():
    """Run the benchmark suite."""
    print("="*80)
    print(" "*20 + "GeoMag Library Performance Benchmark (Python)")
    print("="*80)
    print()

    # Initialize with standard WMM model
    print("Initializing WMM model...")
    try:
        geo_mag = GeoMag(high_resolution=False)
    except Exception as e:
        print(f"Failed to initialize GeoMag model: {e}")
        print("Make sure you're running from the project root directory.")
        return 1

    print(f"Model: {geo_mag.model}")
    print(f"Epoch: {geo_mag.epoch}")
    print(f"Max Order: {geo_mag.maxord}")
    print()

    # Record counts to test
    record_counts = [10_000, 30_000, 50_000, 100_000]

    print("-"*80)
    print("Running benchmarks...")
    print("-"*80)
    print()

    # Run benchmarks
    for count in record_counts:
        run_benchmark(geo_mag, count)

    # Now test with high resolution model
    print("="*80)
    print("High Resolution Model Benchmark (WMMHR)")
    print("="*80)
    print()

    print("Initializing WMMHR model...")
    try:
        geo_mag_hr = GeoMag(high_resolution=True)
    except Exception as e:
        print(f"Failed to initialize WMMHR model: {e}")
        print("(High resolution model may not be available)")
        print()
    else:
        print(f"Model: {geo_mag_hr.model}")
        print(f"Epoch: {geo_mag_hr.epoch}")
        print(f"Max Order: {geo_mag_hr.maxord}")
        print()

        print("-"*80)
        print("Running high resolution benchmarks...")
        print("-"*80)
        print()

        # Run smaller benchmarks for high resolution (slower)
        for count in record_counts:
            run_benchmark(geo_mag_hr, count)

    print("="*80)
    print("Benchmark completed!")
    print("="*80)

    return 0


if __name__ == "__main__":
    sys.exit(main())