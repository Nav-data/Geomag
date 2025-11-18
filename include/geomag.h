/**
 * @file geomag.h
 * @brief World Magnetic Model (WMM) C Implementation
 *
 * This is a C port of the pygeomag Python library, which itself is a port
 * of the Legacy C code provided by NOAA for the World Magnetic Model (WMM).
 *
 * The World Magnetic Model (WMM) is the standard model for navigation,
 * attitude, and heading referencing systems using the geomagnetic field.
 *
 * @author Port to optimized C
 * @date 2025
 */

#ifndef GEOMAG_H
#define GEOMAG_H

#include <stdbool.h>

/* Model sizes */
#define WMM_SIZE_STANDARD 12
#define WMM_SIZE_HIGH_RESOLUTION 133
#define WMM_MAX_SIZE 134  /* Maximum array size needed */

/* Warning zones */
#define BLACKOUT_ZONE 2000.0
#define CAUTION_ZONE 6000.0

/* Earth ellipsoid parameters (WGS84) */
#define WGS84_A 6378.137       /* Semi-major axis in km */
#define WGS84_B 6356.7523142   /* Semi-minor axis in km */
#define WGS84_RE 6371.2        /* Mean radius in km */

/* Model date ranges */
#define WMM_MODEL_2010_LOWER 2010.0
#define WMM_MODEL_2010_UPPER 2015.0
#define WMM_MODEL_2015_LOWER 2015.0
#define WMM_MODEL_2015_UPPER 2020.0
#define WMM_MODEL_2020_LOWER 2020.0
#define WMM_MODEL_2020_UPPER 2025.0
#define WMM_MODEL_2025_LOWER 2025.0
#define WMM_MODEL_2025_UPPER 2030.0

/**
 * @brief Magnetic field result structure
 */
typedef struct {
    double time;     /**< Time in decimal year */
    double alt;      /**< Altitude in km (-1 to 850) */
    double glat;     /**< Geodetic latitude in degrees (-90 to +90) */
    double glon;     /**< Geodetic longitude in degrees (-180 to +180) */

    double x;        /**< North component in nT */
    double y;        /**< East component in nT */
    double z;        /**< Vertical component (down positive) in nT */
    double h;        /**< Horizontal intensity in nT */
    double f;        /**< Total intensity in nT */
    double i;        /**< Inclination (dip angle) in degrees */
    double d;        /**< Declination (magnetic variation) in degrees */
    double gv;       /**< Grid variation in degrees (or -999.0 if not applicable) */

    bool in_blackout_zone;  /**< True if H < 2000 nT */
    bool in_caution_zone;   /**< True if 2000 <= H < 6000 nT */
    bool is_high_resolution; /**< True if using high resolution model */
} GeoMagResult;

/**
 * @brief Uncertainty result structure
 */
typedef struct {
    double x;  /**< Uncertainty of North component in nT */
    double y;  /**< Uncertainty of East component in nT */
    double z;  /**< Uncertainty of Vertical component in nT */
    double h;  /**< Uncertainty of Horizontal intensity in nT */
    double f;  /**< Uncertainty of Total intensity in nT */
    double i;  /**< Uncertainty of Inclination in degrees */
    double d;  /**< Uncertainty of Declination in degrees */
} GeoMagUncertainty;

/**
 * @brief Main GeoMag model structure
 */
typedef struct {
    int maxord;           /**< Maximum degree of spherical harmonics */
    int size;             /**< Size = maxord + 1 */

    double epoch;         /**< Model epoch year */
    char model[32];       /**< Model name (e.g., "WMM-2025") */
    char release_date[16]; /**< Release date */

    /* Coefficient arrays */
    double c[WMM_MAX_SIZE][WMM_MAX_SIZE];   /**< Gauss coefficients gnm */
    double cd[WMM_MAX_SIZE][WMM_MAX_SIZE];  /**< Secular variation dgnm */
    double k[WMM_MAX_SIZE][WMM_MAX_SIZE];   /**< Recursion factors */
    double fn[WMM_MAX_SIZE];                 /**< n+1 values */
    double fm[WMM_MAX_SIZE];                 /**< n values */
    double p[WMM_MAX_SIZE * WMM_MAX_SIZE];  /**< Schmidt semi-normalized associated Legendre functions */
} GeoMag;

/**
 * @brief Initialize a GeoMag model from a coefficient file
 *
 * @param geo_mag Pointer to GeoMag structure to initialize
 * @param coefficients_file Path to the WMM coefficient file (.COF)
 * @param high_resolution Set to true for high resolution model (133 degrees)
 * @return 0 on success, -1 on error
 */
int geomag_init(GeoMag *geo_mag, const char *coefficients_file, bool high_resolution);

/**
 * @brief Calculate magnetic field values for a given location and time
 *
 * @param geo_mag Pointer to initialized GeoMag structure
 * @param glat Geodetic latitude in degrees (-90 to +90, North positive)
 * @param glon Geodetic longitude in degrees (-180 to +180, East positive)
 * @param alt Altitude in km (-1 to 850, referenced to WGS84 ellipsoid)
 * @param time Time in decimal year (e.g., 2025.5 for mid-2025)
 * @param allow_date_outside_lifespan Set to true to allow dates outside 5-year model span
 * @param raise_in_warning_zone Set to true to return error codes for warning zones
 * @param result Pointer to GeoMagResult structure to store results
 * @return 0 on success, -1 on error, -2 if in blackout zone (if raise_in_warning_zone), -3 if in caution zone
 */
int geomag_calculate(GeoMag *geo_mag, double glat, double glon, double alt, double time,
                     bool allow_date_outside_lifespan, bool raise_in_warning_zone,
                     GeoMagResult *result);

/**
 * @brief Calculate uncertainty estimates for a result
 *
 * @param result Pointer to GeoMagResult to calculate uncertainties for
 * @param uncertainty Pointer to GeoMagUncertainty structure to store results
 * @return 0 on success, -1 on error
 */
int geomag_calculate_uncertainty(const GeoMagResult *result, GeoMagUncertainty *uncertainty);

/**
 * @brief Free resources associated with a GeoMag structure
 *
 * @param geo_mag Pointer to GeoMag structure to clean up
 */
void geomag_free(GeoMag *geo_mag);

#endif /* GEOMAG_H */