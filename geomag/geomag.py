"""Python wrapper for the GeoMag C library using ctypes."""

import ctypes
import os
import platform
from pathlib import Path
from typing import Optional, Tuple


class GeoMagResult(ctypes.Structure):
    """Magnetic field result structure.

    Attributes:
        time: Time in decimal year
        alt: Altitude in km (-1 to 850)
        glat: Geodetic latitude in degrees (-90 to +90)
        glon: Geodetic longitude in degrees (-180 to +180)
        x: North component in nT
        y: East component in nT
        z: Vertical component (down positive) in nT
        h: Horizontal intensity in nT
        f: Total intensity in nT
        i: Inclination (dip angle) in degrees
        d: Declination (magnetic variation) in degrees
        gv: Grid variation in degrees (or -999.0 if not applicable)
        in_blackout_zone: True if H < 2000 nT
        in_caution_zone: True if 2000 <= H < 6000 nT
        is_high_resolution: True if using high resolution model
    """
    _fields_ = [
        ('time', ctypes.c_double),
        ('alt', ctypes.c_double),
        ('glat', ctypes.c_double),
        ('glon', ctypes.c_double),
        ('x', ctypes.c_double),
        ('y', ctypes.c_double),
        ('z', ctypes.c_double),
        ('h', ctypes.c_double),
        ('f', ctypes.c_double),
        ('i', ctypes.c_double),
        ('d', ctypes.c_double),
        ('gv', ctypes.c_double),
        ('in_blackout_zone', ctypes.c_bool),
        ('in_caution_zone', ctypes.c_bool),
        ('is_high_resolution', ctypes.c_bool),
    ]

    @property
    def declination(self) -> float:
        """Declination (magnetic variation) in degrees."""
        return self.d

    @property
    def inclination(self) -> float:
        """Inclination (dip angle) in degrees."""
        return self.i

    @property
    def total_intensity(self) -> float:
        """Total intensity in nT."""
        return self.f

    @property
    def horizontal_intensity(self) -> float:
        """Horizontal intensity in nT."""
        return self.h

    @property
    def north_component(self) -> float:
        """North component in nT."""
        return self.x

    @property
    def east_component(self) -> float:
        """East component in nT."""
        return self.y

    @property
    def vertical_component(self) -> float:
        """Vertical component (down positive) in nT."""
        return self.z

    def __repr__(self) -> str:
        return (f"GeoMagResult(lat={self.glat:.4f}, lon={self.glon:.4f}, "
                f"declination={self.d:.4f}°, inclination={self.i:.4f}°, "
                f"total={self.f:.1f}nT)")


class GeoMagUncertainty(ctypes.Structure):
    """Uncertainty result structure.

    Attributes:
        x: Uncertainty of North component in nT
        y: Uncertainty of East component in nT
        z: Uncertainty of Vertical component in nT
        h: Uncertainty of Horizontal intensity in nT
        f: Uncertainty of Total intensity in nT
        i: Uncertainty of Inclination in degrees
        d: Uncertainty of Declination in degrees
    """
    _fields_ = [
        ('x', ctypes.c_double),
        ('y', ctypes.c_double),
        ('z', ctypes.c_double),
        ('h', ctypes.c_double),
        ('f', ctypes.c_double),
        ('i', ctypes.c_double),
        ('d', ctypes.c_double),
    ]

    def __repr__(self) -> str:
        return f"GeoMagUncertainty(d=±{self.d:.4f}°, i=±{self.i:.4f}°, f=±{self.f:.1f}nT)"


class _GeoMagInternal(ctypes.Structure):
    """Internal C structure - not exposed to users."""
    WMM_MAX_SIZE = 134
    _fields_ = [
        ('maxord', ctypes.c_int),
        ('size', ctypes.c_int),
        ('epoch', ctypes.c_double),
        ('model', ctypes.c_char * 32),
        ('release_date', ctypes.c_char * 16),
        # Full coefficient arrays - required for proper memory allocation
        ('c', ctypes.c_double * WMM_MAX_SIZE * WMM_MAX_SIZE),
        ('cd', ctypes.c_double * WMM_MAX_SIZE * WMM_MAX_SIZE),
        ('k', ctypes.c_double * WMM_MAX_SIZE * WMM_MAX_SIZE),
        ('fn', ctypes.c_double * WMM_MAX_SIZE),
        ('fm', ctypes.c_double * WMM_MAX_SIZE),
        ('p', ctypes.c_double * (WMM_MAX_SIZE * WMM_MAX_SIZE)),
    ]


class GeoMag:
    """World Magnetic Model calculator.

    This class provides a high-level interface to the WMM C library for calculating
    Earth's magnetic field components at any location and time.

    Example:
        >>> gm = GeoMag('data/WMM.COF')
        >>> result = gm.calculate(lat=47.6205, lon=-122.3493, alt=0, time=2025.5)
        >>> print(f"Declination: {result.declination:.2f}°")
    """

    _lib = None
    _lib_path = None

    @classmethod
    def _load_library(cls):
        """Load the shared library."""
        if cls._lib is not None:
            return cls._lib

        # Try to find the library
        lib_name = _get_library_name()
        search_paths = _get_library_search_paths()

        for path in search_paths:
            lib_path = path / lib_name
            if lib_path.exists():
                try:
                    cls._lib = ctypes.CDLL(str(lib_path))
                    cls._lib_path = lib_path
                    _setup_library_functions(cls._lib)
                    return cls._lib
                except OSError:
                    continue

        raise RuntimeError(
            f"Could not find geomag library. Searched: {search_paths}\n"
            "Please build the library first with 'make' or install the package."
        )

    def __init__(self, coefficients_file: str, high_resolution: bool = False):
        """Initialize a GeoMag model.

        Args:
            coefficients_file: Path to the WMM coefficient file (.COF)
            high_resolution: Use high resolution model (133 degrees) if True

        Raises:
            RuntimeError: If initialization fails
        """
        self._lib = self._load_library()
        self._geo_mag = _GeoMagInternal()  # Allocate proper structure

        # Convert path to bytes
        if not os.path.exists(coefficients_file):
            raise FileNotFoundError(f"Coefficients file not found: {coefficients_file}")

        coef_file_bytes = coefficients_file.encode('utf-8')

        ret = self._lib.geomag_init(
            ctypes.byref(self._geo_mag),
            coef_file_bytes,
            high_resolution
        )

        if ret != 0:
            raise RuntimeError(f"Failed to initialize GeoMag from {coefficients_file}")

        # Extract model info directly from the structure
        self.model = self._geo_mag.model.decode('utf-8')
        self.epoch = self._geo_mag.epoch
        self.release_date = self._geo_mag.release_date.decode('utf-8')
        self.maxord = self._geo_mag.maxord

    def calculate(
        self,
        lat: float,
        lon: float,
        alt: float,
        time: float,
        allow_date_outside_lifespan: bool = False,
        raise_in_warning_zone: bool = False
    ) -> GeoMagResult:
        """Calculate magnetic field values.

        Args:
            lat: Geodetic latitude in degrees (-90 to +90, North positive)
            lon: Geodetic longitude in degrees (-180 to +180, East positive)
            alt: Altitude in km (-1 to 850, referenced to WGS84 ellipsoid)
            time: Time in decimal year (e.g., 2025.5 for mid-2025)
            allow_date_outside_lifespan: Allow dates outside 5-year model span
            raise_in_warning_zone: Raise exception for blackout/caution zones

        Returns:
            GeoMagResult with magnetic field components

        Raises:
            RuntimeError: If calculation fails or in warning zone (if raise_in_warning_zone=True)
        """
        result = GeoMagResult()

        ret = self._lib.geomag_calculate(
            ctypes.byref(self._geo_mag),
            ctypes.c_double(lat),
            ctypes.c_double(lon),
            ctypes.c_double(alt),
            ctypes.c_double(time),
            allow_date_outside_lifespan,
            raise_in_warning_zone,
            ctypes.byref(result)
        )

        if ret == -1:
            raise RuntimeError("Failed to calculate magnetic field")
        elif ret == -2:
            raise RuntimeError("Location is in blackout zone (H < 2000 nT)")
        elif ret == -3:
            raise RuntimeError("Location is in caution zone (2000 <= H < 6000 nT)")

        return result

    def calculate_uncertainty(self, result: GeoMagResult) -> GeoMagUncertainty:
        """Calculate uncertainty estimates for a result.

        Args:
            result: GeoMagResult to calculate uncertainties for

        Returns:
            GeoMagUncertainty with uncertainty estimates

        Raises:
            RuntimeError: If calculation fails
        """
        uncertainty = GeoMagUncertainty()

        ret = self._lib.geomag_calculate_uncertainty(
            ctypes.byref(result),
            ctypes.byref(uncertainty)
        )

        if ret != 0:
            raise RuntimeError("Failed to calculate uncertainty")

        return uncertainty

    def __del__(self):
        """Clean up resources."""
        if hasattr(self, '_lib') and self._lib is not None and hasattr(self, '_geo_mag'):
            try:
                self._lib.geomag_free(ctypes.byref(self._geo_mag))
            except:
                pass

    def __repr__(self) -> str:
        return f"GeoMag(model='{self.model}', epoch={self.epoch})"


def _get_library_name() -> str:
    """Get the platform-specific library name."""
    system = platform.system()
    if system == 'Darwin':  # macOS
        return 'libgeomag.dylib'
    elif system == 'Windows':
        return 'geomag.dll'
    else:  # Linux and others
        return 'libgeomag.so'


def _get_library_search_paths() -> list:
    """Get list of paths to search for the library."""
    paths = []

    # Current directory and build directory
    current = Path.cwd()
    paths.extend([
        current / 'build',
        current,
        current.parent / 'build',
    ])

    # Package directory
    package_dir = Path(__file__).parent
    paths.extend([
        package_dir,
        package_dir / 'lib',
        package_dir.parent / 'build',
    ])

    # System paths
    if platform.system() != 'Windows':
        paths.extend([
            Path('/usr/local/lib'),
            Path('/usr/lib'),
        ])

    return paths


def _setup_library_functions(lib):
    """Set up function signatures for the C library."""
    # geomag_init
    lib.geomag_init.argtypes = [
        ctypes.POINTER(_GeoMagInternal),  # geo_mag
        ctypes.c_char_p,  # coefficients_file
        ctypes.c_bool,    # high_resolution
    ]
    lib.geomag_init.restype = ctypes.c_int

    # geomag_calculate
    lib.geomag_calculate.argtypes = [
        ctypes.POINTER(_GeoMagInternal),  # geo_mag
        ctypes.c_double,    # glat
        ctypes.c_double,    # glon
        ctypes.c_double,    # alt
        ctypes.c_double,    # time
        ctypes.c_bool,      # allow_date_outside_lifespan
        ctypes.c_bool,      # raise_in_warning_zone
        ctypes.POINTER(GeoMagResult),  # result
    ]
    lib.geomag_calculate.restype = ctypes.c_int

    # geomag_calculate_uncertainty
    lib.geomag_calculate_uncertainty.argtypes = [
        ctypes.POINTER(GeoMagResult),
        ctypes.POINTER(GeoMagUncertainty),
    ]
    lib.geomag_calculate_uncertainty.restype = ctypes.c_int

    # geomag_free
    lib.geomag_free.argtypes = [ctypes.POINTER(_GeoMagInternal)]
    lib.geomag_free.restype = None
