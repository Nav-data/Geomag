#!/usr/bin/env python3
"""Example usage of the geomag_c Python package."""

from geomag_c import GeoMag


def main():
    # Example 1: Calculate declination at Space Needle (Seattle, WA) using WMM-2025
    print("=== Example 1: Space Needle, Seattle, WA (WMM-2025) ===")

    gm = GeoMag('data/WMM.COF')
    print(f"Model: {gm.model}")
    print(f"Epoch: {gm.epoch}")
    print(f"Release Date: {gm.release_date}")
    print()

    # Space Needle coordinates
    latitude = 47.6205   # North
    longitude = -122.3493  # West
    altitude = 0.0       # Sea level
    time = 2025.25       # March 2025

    result = gm.calculate(lat=latitude, lon=longitude, alt=altitude, time=time)

    print(f"Location: {latitude:.4f}°N, {longitude:.4f}°E")
    print(f"Altitude: {altitude:.1f} km")
    print(f"Date: {time:.2f}")
    print()

    print("Magnetic Field Components:")
    print(f"  Declination (D):    {result.declination:8.4f}° (magnetic variation)")
    print(f"  Inclination (I):    {result.inclination:8.4f}° (dip angle)")
    print(f"  Total Intensity (F):{result.total_intensity:8.1f} nT")
    print(f"  Horizontal (H):     {result.horizontal_intensity:8.1f} nT")
    print(f"  North (X):          {result.north_component:8.1f} nT")
    print(f"  East (Y):           {result.east_component:8.1f} nT")
    print(f"  Vertical (Z):       {result.vertical_component:8.1f} nT")

    if result.gv != -999.0:
        print(f"  Grid Variation:     {result.gv:8.4f}°")

    # Calculate uncertainties
    uncertainty = gm.calculate_uncertainty(result)
    print("\nUncertainty Estimates:")
    print(f"  Declination: ±{uncertainty.d:.4f}°")
    print(f"  Inclination: ±{uncertainty.i:.2f}°")
    print(f"  Total:       ±{uncertainty.f:.1f} nT")

    # Example 2: Multiple locations
    print("\n\n=== Example 2: Declination at various locations (2025) ===")

    locations = [
        ("New York, NY", 40.7128, -74.0060),
        ("Los Angeles, CA", 34.0522, -118.2437),
        ("Miami, FL", 25.7617, -80.1918),
        ("Chicago, IL", 41.8781, -87.6298),
        ("Denver, CO", 39.7392, -104.9903),
        ("London, UK", 51.5074, -0.1278),
        ("Tokyo, Japan", 35.6762, 139.6503),
        ("Sydney, Australia", -33.8688, 151.2093),
    ]

    time = 2025.5  # Mid-2025

    for name, lat, lon in locations:
        result = gm.calculate(lat=lat, lon=lon, alt=0.0, time=time)
        print(f"  {name:20s}: D={result.declination:7.2f}°, I={result.inclination:6.2f}°")

    # Example 3: High resolution model
    print("\n\n=== Example 3: High Resolution Model (WMMHR-2025) ===")

    gm_hr = GeoMag('data/WMMHR.COF', high_resolution=True)
    print(f"Model: {gm_hr.model} (High Resolution, {gm_hr.maxord} degrees)")

    time = 2025.0
    result = gm_hr.calculate(lat=47.6205, lon=-122.3493, alt=0.0, time=time)
    print(f"Space Needle declination: {result.declination:.6f}°")
    print("(More precise due to crustal field modeling)")

    # Example 4: Using convenience properties
    print("\n\n=== Example 4: Using convenience properties ===")

    result = gm.calculate(lat=47.6205, lon=-122.3493, alt=0.0, time=2025.5)
    print(f"Result object: {result}")
    print(f"\nWarning zones:")
    print(f"  In blackout zone: {result.in_blackout_zone}")
    print(f"  In caution zone: {result.in_caution_zone}")
    print(f"  High resolution: {result.is_high_resolution}")

    print("\n=== All examples completed successfully! ===")


if __name__ == '__main__':
    main()
