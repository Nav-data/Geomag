# GeoMag - World Magnetic Model (WMM) C Library

A high-performance C implementation of the World Magnetic Model (WMM). This library provides fast, accurate calculations of Earth's magnetic field components for navigation, attitude, and heading referencing systems.

## What is Magnetic Declination?

Declination is the angle between Magnetic North and True North. Many maps show it in the legend so you can adjust your compass (either physically or mentally).

**Why do you need this?**

In Washington State, the declination angle is between 14° and 16° West depending on your location. If you just pull out a compass and head North, you're really heading at about 15°. Walk 500 feet on that course, and you'll be about 130 feet to the East of where you wanted to be.

For devices with magnetometers, you need to correct raw magnetic readings to show True North to users, and these values change yearly. In the last 10 years, declination has dropped over a full degree in the Greater Seattle area.

## Features

- **Fast C implementation** - Optimized for performance
- **Standard WMM models** - Support for WMM-2025, WMM-2020, WMM-2015, and WMM-2010
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

## Performance

This C implementation is significantly faster than the Python version:

- **Standard model**: ~10-50x faster than Python
- **High resolution model**: ~50-200x faster than Python
- **Typical calculation time**: < 100 microseconds (standard), < 1 millisecond (high-res)

Optimal for real-time navigation systems, embedded systems, flight simulators, and high-frequency calculations.

