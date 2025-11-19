# GeoMag - World Magnetic Model (WMM) C Library

A high-performance C implementation of the World Magnetic Model (WMM). This library provides fast, accurate calculations of Earth's magnetic field components for navigation, attitude, and heading referencing systems.

## What is Magnetic Declination?

Declination is the angle between Magnetic North and True North. Many maps show it in the legend so you can adjust your compass (either physically or mentally).

**Why do you need this?**

In Washington State, the declination angle is between 14° and 16° West depending on your location. If you just pull out a compass and head North, you're really heading at about 15°. Walk 500 feet on that course, and you'll be about 130 feet to the East of where you wanted to be.

For devices with magnetometers, you need to correct raw magnetic readings to show True North to users, and these values change yearly. In the last 10 years, declination has dropped over a full degree in the Greater Seattle area.

## Features

- **Fast C implementation** - Optimized for performance
- **Standard WMM model** - WMM-2025 (valid 2025-2030)
- **High resolution model** - WMMHR-2025 with 133 degrees of spherical harmonics
- **Full magnetic field components** - Declination, inclination, intensity, and all vector components
- **Uncertainty estimates** - Calculate confidence intervals for results
- **Warning zones** - Automatic detection of blackout and caution zones near magnetic poles
- **Validated** - Test suite verifies against NOAA reference values
- **Simple API** - Easy to integrate into your projects

## Building

### Requirements

- C compiler (gcc, clang, etc.)
- Make
- Math library (libm)

### Quick Start

```bash
# Build the library and example
make

# Run the example program
make run-example

# Run tests
make test

# Run benchmark
make benchmark
```

## Installation

### From PyPI

```bash
pip install geomag-c
```

### From Source

```bash
git clone https://github.com/yourusername/geomag-c.git
cd geomag-c
pip install .
```

## Usage

### Python API

```python
from geomag_c import GeoMag

# Initialize with bundled WMM-2025 model (no file path needed!)
gm = GeoMag()

# Calculate magnetic field for a location
# Parameters: lat, lon, alt (km), time (decimal year)
result = gm.calculate(
    lat=47.6062,      # Seattle latitude
    lon=-122.3321,    # Seattle longitude
    alt=0.0,          # Sea level
    time=2025.5       # Mid-2025
)

# Access magnetic field components
print(f"Declination: {result.declination:.2f}°")  # Angle from true north
print(f"Inclination: {result.inclination:.2f}°")  # Dip angle
print(f"Total Intensity: {result.total_intensity:.2f} nT")  # Total field strength

# Vector components (in nanoTeslas)
print(f"North component: {result.north_component:.2f} nT")
print(f"East component: {result.east_component:.2f} nT")
print(f"Vertical component: {result.vertical_component:.2f} nT")
print(f"Horizontal intensity: {result.horizontal_intensity:.2f} nT")
```

### High Resolution Model

```python
# Use the bundled high-resolution model for more accurate results
from geomag_c import GeoMag

gm_hr = GeoMag(high_resolution=True)
result = gm_hr.calculate(lat=47.6062, lon=-122.3321, alt=0.0, time=2025.5)
```

### Uncertainty Estimates

```python
# Get uncertainty values for the calculations
from geomag_c import GeoMag

gm = GeoMag()
result = gm.calculate(lat=47.6062, lon=-122.3321, alt=0.0, time=2025.5)
uncertainty = gm.calculate_uncertainty(result)

print(f"Declination uncertainty: ±{uncertainty.d:.2f}°")
print(f"Inclination uncertainty: ±{uncertainty.i:.2f}°")
print(f"Total intensity uncertainty: ±{uncertainty.f:.0f} nT")
```

### Warning Zones

The library automatically detects problematic regions near magnetic poles:

```python
from geomag_c import GeoMag

gm = GeoMag()
result = gm.calculate(lat=85.0, lon=0.0, alt=0.0, time=2025.5)  # Near North Pole

if result.in_blackout_zone:
    print("Warning: Blackout zone - compass unreliable (H < 2000 nT)")
elif result.in_caution_zone:
    print("Warning: Caution zone - compass less reliable (2000 <= H < 6000 nT)")
```

### Available Models

Both models are bundled with the package - no need to download or specify file paths!

- **WMM-2025**: Current standard model with 12 degrees (default)
  - Usage: `GeoMag()` or `GeoMag(high_resolution=False)`
- **WMMHR-2025**: High-resolution model with 133 degrees
  - Usage: `GeoMag(high_resolution=True)`
- **Custom models**: You can still load custom coefficient files
  - Usage: `GeoMag('path/to/custom.COF')`

## Performance

This C implementation is significantly faster than pure Python implementations:

- **Standard model**: ~10-50x faster than Python
- **High resolution model**: ~50-200x faster than Python
- **Typical calculation time**: < 100 microseconds (standard), < 1 millisecond (high-res)

Optimal for real-time navigation systems, embedded systems, flight simulators, and high-frequency calculations.

## C Library Usage

The underlying C library can also be used directly:

```c
#include "geomag.h"

int main() {
    GeoMagModel *model = geomag_load_model("WMM2025");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }

    GeoMagResult result;
    int status = geomag_calculate(model, 47.6062, -122.3321, 0.0, 2025.5, &result);

    if (status == 0) {
        printf("Declination: %.2f degrees\n", result.declination);
        printf("Inclination: %.2f degrees\n", result.inclination);
    }

    geomag_free_model(model);
    return 0;
}
```

See the C library documentation for more details.

## Use Cases

- **Navigation Systems**: Convert magnetic compass readings to true bearings
- **Aerospace**: Attitude and heading reference systems (AHRS)
- **Geology**: Magnetic field modeling and anomaly detection
- **GIS Applications**: Coordinate transformations and mapping
- **Robotics**: Magnetometer calibration and orientation estimation
- **Scientific Research**: Geomagnetic field analysis

## Output Components

The `calculate()` method returns a `GeoMagResult` object with the following attributes:

### Main Result Fields

| Field | Description | Units |
|-------|-------------|-------|
| `declination` or `d` | Angle between magnetic north and true north | degrees |
| `inclination` or `i` | Dip angle (positive downward) | degrees |
| `total_intensity` or `f` | Total magnetic field strength | nanoTeslas (nT) |
| `horizontal_intensity` or `h` | Horizontal component magnitude | nT |
| `north_component` or `x` | North component (X) | nT |
| `east_component` or `y` | East component (Y) | nT |
| `vertical_component` or `z` | Vertical component (Z, positive downward) | nT |
| `gv` | Grid variation | degrees |
| `in_blackout_zone` | True if H < 2000 nT | boolean |
| `in_caution_zone` | True if 2000 <= H < 6000 nT | boolean |
| `is_high_resolution` | True if using high-res model | boolean |

### Uncertainty Fields

The `calculate_uncertainty()` method returns a `GeoMagUncertainty` object:

| Field | Description | Units |
|-------|-------------|-------|
| `d` | Uncertainty in declination | degrees |
| `i` | Uncertainty in inclination | degrees |
| `f` | Uncertainty in total intensity | nT |
| `h` | Uncertainty in horizontal intensity | nT |
| `x` | Uncertainty in north component | nT |
| `y` | Uncertainty in east component | nT |
| `z` | Uncertainty in vertical component | nT |

## Coordinate System

- **Latitude**: -90° (South) to +90° (North)
- **Longitude**: -180° (West) to +180° (East)
- **Altitude**: Height above WGS84 ellipsoid in kilometers
- **Decimal Year**: Year as a decimal (e.g., 2025.5 = July 1, 2025)

## Model Validity Period

The WMM2025 model is valid for 5 years:

- **WMM2025**: 2025.0 - 2030.0
- **WMMHR2025**: 2025.0 - 2030.0

Using dates outside the validity period will produce results but with reduced accuracy.

## Testing

The library includes comprehensive tests validated against NOAA reference values:

```bash
# Run Python tests
python -m pytest tests/

# Run C library tests
make test

# Run benchmarks
make benchmark
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## References

- [NOAA World Magnetic Model](https://www.ngdc.noaa.gov/geomag/WMM/)
- [WMM Technical Report](https://www.ncei.noaa.gov/products/world-magnetic-model)
- [Geomagnetic Field Modeling](https://geomag.bgs.ac.uk/research/modelling/)

## Acknowledgments

Based on the World Magnetic Model developed by NOAA's National Centers for Environmental Information (NCEI) and the British Geological Survey (BGS).

## Support

For issues, questions, or contributions, please visit the [GitHub repository](https://github.com/yourusername/geomag-c).

