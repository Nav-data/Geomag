#!/usr/bin/env python3
"""Comprehensive demo of the geomag-c Python library.

This script demonstrates various features of the World Magnetic Model (WMM) library:
- Basic magnetic field calculations
- Declination for navigation and compass calibration
- Multiple coordinate systems
- Uncertainty estimation
- High-resolution model comparison
- Warning zones (blackout and caution zones)
- Time series analysis
"""

import sys
from geomag_c import GeoMag


def print_section(title):
    """Print a formatted section header."""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


def demo_basic_usage():
    """Demonstrate basic usage of the library."""
    print_section("1. Basic Usage - Calculate Magnetic Declination")

    # Initialize with WMM-2025 model
    gm = GeoMag('data/WMM.COF')
    print(f"Model Information:")
    print(f"  Name:         {gm.model}")
    print(f"  Epoch:        {gm.epoch}")
    print(f"  Release Date: {gm.release_date}")
    print(f"  Max Order:    {gm.maxord}")
    print()

    # Calculate at a specific location (Golden Gate Bridge, San Francisco)
    latitude = 37.8199   # degrees North
    longitude = -122.4783  # degrees West
    altitude = 0.067     # 67m above sea level, converted to km
    time = 2025.5        # July 2025

    result = gm.calculate(lat=latitude, lon=longitude, alt=altitude, time=time)

    print(f"Location: Golden Gate Bridge, San Francisco")
    print(f"  Coordinates: {latitude:.4f}°N, {longitude:.4f}°W")
    print(f"  Altitude:    {altitude:.3f} km (67m above sea level)")
    print(f"  Date:        {time:.2f} (July 2025)")
    print()

    print("Magnetic Field Components:")
    print(f"  Declination (D):    {result.declination:8.4f}° (angle from True North to Mag North)")
    print(f"  Inclination (I):    {result.inclination:8.4f}° (dip angle from horizontal)")
    print(f"  Total Intensity (F):{result.total_intensity:8.1f} nT (total field strength)")
    print(f"  Horizontal (H):     {result.horizontal_intensity:8.1f} nT")
    print(f"  North (X):          {result.north_component:8.1f} nT")
    print(f"  East (Y):           {result.east_component:8.1f} nT")
    print(f"  Vertical (Z):       {result.vertical_component:8.1f} nT (down positive)")
    print()

    print("Practical Use Case:")
    if result.declination > 0:
        print(f"  A compass points {abs(result.declination):.2f}° EAST of True North")
        print(f"  To find True North: subtract {abs(result.declination):.2f}° from compass heading")
    else:
        print(f"  A compass points {abs(result.declination):.2f}° WEST of True North")
        print(f"  To find True North: add {abs(result.declination):.2f}° to compass heading")


def demo_uncertainty():
    """Demonstrate uncertainty calculations."""
    print_section("2. Uncertainty Estimation")

    gm = GeoMag('data/WMM.COF')

    # New York City
    result = gm.calculate(lat=40.7128, lon=-74.0060, alt=0.0, time=2025.5)
    uncertainty = gm.calculate_uncertainty(result)

    print(f"Location: New York City")
    print(f"  Declination: {result.declination:.4f}° ± {uncertainty.d:.4f}°")
    print(f"  Inclination: {result.inclination:.4f}° ± {uncertainty.i:.4f}°")
    print(f"  Total Field: {result.total_intensity:.1f} nT ± {uncertainty.f:.1f} nT")
    print()

    print("Confidence Intervals (68.3% confidence level):")
    print(f"  Declination range: {result.declination - uncertainty.d:.4f}° to {result.declination + uncertainty.d:.4f}°")
    print(f"  Total field range: {result.total_intensity - uncertainty.f:.1f} to {result.total_intensity + uncertainty.f:.1f} nT")


def demo_world_locations():
    """Demonstrate calculations at various locations around the world."""
    print_section("3. Magnetic Field Around the World")

    gm = GeoMag('data/WMM.COF')
    time = 2025.5  # Mid-2025

    locations = [
        # Name, Latitude, Longitude
        ("New York, USA", 40.7128, -74.0060),
        ("Los Angeles, USA", 34.0522, -118.2437),
        ("Miami, USA", 25.7617, -80.1918),
        ("London, UK", 51.5074, -0.1278),
        ("Paris, France", 48.8566, 2.3522),
        ("Tokyo, Japan", 35.6762, 139.6503),
        ("Sydney, Australia", -33.8688, 151.2093),
        ("Rio de Janeiro, Brazil", -22.9068, -43.1729),
        ("Cape Town, South Africa", -33.9249, 18.4241),
        ("Moscow, Russia", 55.7558, 37.6173),
    ]

    print(f"{'Location':<25} {'Latitude':>10} {'Longitude':>11} {'Decl (°)':>10} {'Incl (°)':>10} {'Total (nT)':>12}")
    print("-" * 88)

    for name, lat, lon in locations:
        result = gm.calculate(lat=lat, lon=lon, alt=0.0, time=time)
        print(f"{name:<25} {lat:>10.4f} {lon:>11.4f} {result.declination:>10.2f} {result.inclination:>10.2f} {result.total_intensity:>12.1f}")


def demo_altitude_effects():
    """Demonstrate how altitude affects magnetic field."""
    print_section("4. Altitude Effects on Magnetic Field")

    gm = GeoMag('data/WMM.COF')

    # Mount Everest location
    latitude = 27.9881
    longitude = 86.9250
    time = 2025.5

    altitudes = [
        (0.0, "Sea level"),
        (5.0, "Commercial aircraft cruise (5 km)"),
        (8.849, "Mount Everest summit"),
        (10.0, "High altitude aircraft"),
        (400.0, "International Space Station orbit"),
    ]

    print(f"Location: Mount Everest ({latitude:.4f}°N, {longitude:.4f}°E)\n")
    print(f"{'Altitude':<35} {'Decl (°)':>10} {'Total (nT)':>12} {'Change (nT)':>12}")
    print("-" * 70)

    sea_level_result = gm.calculate(lat=latitude, lon=longitude, alt=0.0, time=time)

    for alt_km, description in altitudes:
        result = gm.calculate(lat=latitude, lon=longitude, alt=alt_km, time=time)
        change = result.total_intensity - sea_level_result.total_intensity
        print(f"{description:<35} {result.declination:>10.4f} {result.total_intensity:>12.1f} {change:>12.1f}")


def demo_time_series():
    """Demonstrate temporal variation of magnetic field."""
    print_section("5. Temporal Variation (Secular Variation)")

    gm = GeoMag('data/WMM.COF')

    # Seattle, WA - Space Needle
    latitude = 47.6205
    longitude = -122.3493
    altitude = 0.0

    print(f"Location: Space Needle, Seattle, WA ({latitude:.4f}°N, {longitude:.4f}°W)")
    print(f"Tracking declination change over 5 years (WMM-2025 valid period)\n")

    years = [2025.0, 2025.5, 2026.0, 2026.5, 2027.0, 2027.5, 2028.0, 2028.5, 2029.0, 2029.5, 2030.0]

    print(f"{'Year':<10} {'Date':<12} {'Declination':>12} {'Annual Change':>14}")
    print("-" * 50)

    prev_result = None
    for year in years:
        result = gm.calculate(lat=latitude, lon=longitude, alt=altitude, time=year)

        if prev_result is None:
            change_str = "---"
        else:
            annual_change = (result.declination - prev_result.declination) / (year - prev_year)
            change_str = f"{annual_change:+.4f}°/year"

        # Convert decimal year to month
        year_int = int(year)
        month = int((year - year_int) * 12) + 1
        date_str = f"{year_int}-{month:02d}"

        print(f"{year:<10.1f} {date_str:<12} {result.declination:>12.4f}° {change_str:>14}")

        prev_result = result
        prev_year = year

    print(f"\nNote: The magnetic field changes continuously due to core dynamics.")
    print(f"      WMM models include secular variation to predict these changes.")


def demo_high_resolution():
    """Demonstrate high-resolution model comparison."""
    print_section("6. High-Resolution Model Comparison")

    # Standard model (12 degrees)
    gm_std = GeoMag('data/WMM.COF', high_resolution=False)

    # High-resolution model (133 degrees)
    gm_hr = GeoMag('data/WMMHR.COF', high_resolution=True)

    print("Model Comparison:")
    print(f"  Standard Model: {gm_std.model} ({gm_std.maxord} degrees)")
    print(f"  High-Res Model: {gm_hr.model} ({gm_hr.maxord} degrees)")
    print()

    # Test at multiple locations
    locations = [
        ("Seattle, WA", 47.6205, -122.3493),
        ("Tokyo, Japan", 35.6762, 139.6503),
        ("London, UK", 51.5074, -0.1278),
    ]

    time = 2025.5

    print(f"{'Location':<20} {'Standard Decl':>15} {'High-Res Decl':>15} {'Difference':>12}")
    print("-" * 63)

    for name, lat, lon in locations:
        result_std = gm_std.calculate(lat=lat, lon=lon, alt=0.0, time=time)
        result_hr = gm_hr.calculate(lat=lat, lon=lon, alt=0.0, time=time)
        diff = result_hr.declination - result_std.declination

        print(f"{name:<20} {result_std.declination:>15.6f}° {result_hr.declination:>15.6f}° {diff:>12.6f}°")

    print("\nNote: High-resolution model includes crustal magnetic field anomalies")
    print("      for more accurate local predictions, especially important for")
    print("      precision navigation and geophysical surveys.")


def demo_warning_zones():
    """Demonstrate warning zones near magnetic poles."""
    print_section("7. Warning Zones (Blackout and Caution Zones)")

    gm = GeoMag('data/WMM.COF')
    time = 2025.5

    locations = [
        ("Alert, Nunavut (North)", 82.5, -62.3),  # Near magnetic north pole
        ("Yellowknife, Canada", 62.4540, -114.3718),
        ("Fairbanks, Alaska", 64.8378, -147.7164),
        ("Reykjavik, Iceland", 64.1466, -21.9426),
        ("Seattle, WA", 47.6205, -122.3493),
        ("Vostok Station (Antarctica)", -78.4642, 106.8378),  # Near magnetic south pole
    ]

    print("Magnetic Field Strength Classification:")
    print("  Normal:    H >= 6000 nT (compass reliable)")
    print("  Caution:   2000 <= H < 6000 nT (compass less reliable)")
    print("  Blackout:  H < 2000 nT (compass unreliable)")
    print()

    print(f"{'Location':<30} {'H (nT)':>10} {'Status':>15}")
    print("-" * 56)

    for name, lat, lon in locations:
        result = gm.calculate(lat=lat, lon=lon, alt=0.0, time=time)

        if result.in_blackout_zone:
            status = "BLACKOUT"
        elif result.in_caution_zone:
            status = "CAUTION"
        else:
            status = "Normal"

        print(f"{name:<30} {result.horizontal_intensity:>10.1f} {status:>15}")

    print("\nNote: Near magnetic poles, horizontal field intensity drops significantly,")
    print("      making magnetic compasses unreliable. Use alternative navigation methods.")


def demo_compass_calibration():
    """Demonstrate practical compass calibration."""
    print_section("8. Practical Application - Compass Calibration")

    gm = GeoMag('data/WMM.COF')
    time = 2025.5

    print("Common US Cities - Compass Correction Reference:\n")

    cities = [
        ("Seattle, WA", 47.6205, -122.3493),
        ("Portland, OR", 45.5152, -122.6784),
        ("San Francisco, CA", 37.7749, -122.4194),
        ("Denver, CO", 39.7392, -104.9903),
        ("Chicago, IL", 41.8781, -87.6298),
        ("Miami, FL", 25.7617, -80.1918),
        ("New York, NY", 40.7128, -74.0060),
        ("Boston, MA", 42.3601, -71.0589),
    ]

    for name, lat, lon in cities:
        result = gm.calculate(lat=lat, lon=lon, alt=0.0, time=time)

        print(f"{name}:")
        print(f"  Location: {lat:.4f}°N, {abs(lon):.4f}°W")
        print(f"  Declination: {result.declination:.2f}°")

        if result.declination > 0:
            print(f"  Correction: Compass reads {abs(result.declination):.2f}° EAST of True North")
            print(f"  To navigate: Subtract {abs(result.declination):.2f}° from compass bearing")
        else:
            print(f"  Correction: Compass reads {abs(result.declination):.2f}° WEST of True North")
            print(f"  To navigate: Add {abs(result.declination):.2f}° to compass bearing")

        # Example bearing
        compass_bearing = 45.0  # Northeast
        true_bearing = compass_bearing - result.declination
        print(f"  Example: Compass shows {compass_bearing:.0f}° → True bearing is {true_bearing:.1f}°")
        print()


def main():
    """Run all demonstrations."""
    print("\n" + "*" * 70)
    print("*" + " " * 68 + "*")
    print("*" + " " * 15 + "GeoMag-C Python Library Demo" + " " * 25 + "*")
    print("*" + " " * 12 + "World Magnetic Model (WMM) Calculator" + " " * 19 + "*")
    print("*" + " " * 68 + "*")
    print("*" * 70)

    try:
        demo_basic_usage()
        demo_uncertainty()
        demo_world_locations()
        demo_altitude_effects()
        demo_time_series()
        demo_high_resolution()
        demo_warning_zones()
        demo_compass_calibration()

        print_section("Demo Complete!")
        print("All demonstrations completed successfully.")
        print("\nFor more information:")
        print("  - NOAA WMM: https://www.ngdc.noaa.gov/geomag/WMM/")
        print("  - Documentation: See README.md")
        print("  - Source code: https://github.com/yourusername/geomag-c")
        print()

    except FileNotFoundError as e:
        print(f"\nError: {e}")
        print("\nPlease ensure the data files are available:")
        print("  - data/WMM.COF (standard model)")
        print("  - data/WMMHR.COF (high-resolution model)")
        sys.exit(1)

    except RuntimeError as e:
        print(f"\nError: {e}")
        print("\nPlease build the library first:")
        print("  make")
        sys.exit(1)

    except Exception as e:
        print(f"\nUnexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
