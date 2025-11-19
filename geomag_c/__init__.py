"""GeoMag-C - World Magnetic Model (WMM) Python Library

A high-performance Python wrapper around the C implementation of the World Magnetic Model (WMM).
Provides fast, accurate calculations of Earth's magnetic field components for navigation,
attitude, and heading referencing systems.

Example:
    >>> from geomag_c import GeoMag
    >>>
    >>> # Initialize with bundled WMM-2025 model
    >>> gm = GeoMag()
    >>>
    >>> # Calculate magnetic field at Space Needle, Seattle
    >>> result = gm.calculate(lat=47.6205, lon=-122.3493, alt=0, time=2025.5)
    >>> print(f"Declination: {result.declination:.2f}°")
    >>> print(f"Inclination: {result.inclination:.2f}°")
    >>>
    >>> # Use high-resolution model for more accuracy
    >>> gm_hr = GeoMag(high_resolution=True)
"""

from .geomag import GeoMag, GeoMagResult, GeoMagUncertainty

__version__ = "1.0.0"
__author__ = "Justin"
__all__ = ["GeoMag", "GeoMagResult", "GeoMagUncertainty"]
