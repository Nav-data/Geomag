/**
 * @file geomag.c
 * @brief World Magnetic Model (WMM) C Implementation
 *
 * This is an optimized C implementation of the World Magnetic Model,
 * ported from the pygeomag Python library.
 */

#include "geomag.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Convert degrees to radians */
#define DEG2RAD(deg) ((deg) * M_PI / 180.0)

/* Convert radians to degrees */
#define RAD2DEG(rad) ((rad) * 180.0 / M_PI)

/**
 * @brief Load coefficients from a WMM coefficient file
 */
static int load_coefficients(GeoMag *geo_mag, const char *coefficients_file) {
    FILE *fp = fopen(coefficients_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open coefficient file: %s\n", coefficients_file);
        return -1;
    }

    /* Read header line: epoch, model name, release date */
    if (fscanf(fp, "%lf %s %s", &geo_mag->epoch, geo_mag->model, geo_mag->release_date) != 3) {
        fprintf(stderr, "Error: Invalid header in coefficient file\n");
        fclose(fp);
        return -1;
    }

    /* Initialize coefficient arrays */
    memset(geo_mag->c, 0, sizeof(geo_mag->c));
    memset(geo_mag->cd, 0, sizeof(geo_mag->cd));
    memset(geo_mag->k, 0, sizeof(geo_mag->k));

    geo_mag->c[0][0] = 0.0;
    geo_mag->cd[0][0] = 0.0;

    /* Read coefficient data */
    int n, m;
    double gnm, hnm, dgnm, dhnm;

    while (1) {
        /* Try to read a line */
        if (fscanf(fp, "%d %d %lf %lf %lf %lf", &n, &m, &gnm, &hnm, &dgnm, &dhnm) != 6) {
            break;  /* End of data or error */
        }

        /* Check for end marker */
        if (n == 9999) {
            break;
        }

        /* Stop if we exceed maxord */
        if (m > geo_mag->maxord) {
            break;
        }

        /* Validate indices */
        if (m > n || m < 0 || n >= geo_mag->size || m >= geo_mag->size) {
            fprintf(stderr, "Error: Corrupt record in model file (n=%d, m=%d)\n", n, m);
            fclose(fp);
            return -1;
        }

        /* Store coefficients */
        if (m <= n) {
            geo_mag->c[m][n] = gnm;
            geo_mag->cd[m][n] = dgnm;
            if (m != 0) {
                geo_mag->c[n][m - 1] = hnm;
                geo_mag->cd[n][m - 1] = dhnm;
            }
        }
    }

    fclose(fp);

    /* Convert Schmidt normalized Gauss coefficients to unnormalized */
    double snorm[WMM_MAX_SIZE * WMM_MAX_SIZE];
    snorm[0] = 1.0;
    geo_mag->fm[0] = 0.0;

    for (n = 1; n <= geo_mag->maxord; n++) {
        snorm[n] = snorm[n - 1] * (double)(2 * n - 1) / (double)n;
        int j = 2;
        m = 0;
        int D1 = 1;
        int D2 = (n - m + D1) / D1;

        while (D2 > 0) {
            double flnmj;
            geo_mag->k[m][n] = (double)((n - 1) * (n - 1) - m * m) / (double)((2 * n - 1) * (2 * n - 3));

            if (m > 0) {
                flnmj = (double)((n - m + 1) * j) / (double)(n + m);
                snorm[n + m * geo_mag->size] = snorm[n + (m - 1) * geo_mag->size] * sqrt(flnmj);
                j = 1;
                geo_mag->c[n][m - 1] = snorm[n + m * geo_mag->size] * geo_mag->c[n][m - 1];
                geo_mag->cd[n][m - 1] = snorm[n + m * geo_mag->size] * geo_mag->cd[n][m - 1];
            }

            geo_mag->c[m][n] = snorm[n + m * geo_mag->size] * geo_mag->c[m][n];
            geo_mag->cd[m][n] = snorm[n + m * geo_mag->size] * geo_mag->cd[m][n];
            D2 -= 1;
            m += D1;
        }

        geo_mag->fn[n] = (double)(n + 1);
        geo_mag->fm[n] = (double)n;
    }

    geo_mag->k[1][1] = 0.0;

    /* Store p (snorm) values */
    memcpy(geo_mag->p, snorm, sizeof(snorm));

    return 0;
}

int geomag_init(GeoMag *geo_mag, const char *coefficients_file, bool high_resolution) {
    if (!geo_mag || !coefficients_file) {
        return -1;
    }

    /* Set model size */
    if (high_resolution) {
        geo_mag->maxord = WMM_SIZE_HIGH_RESOLUTION;
    } else {
        geo_mag->maxord = WMM_SIZE_STANDARD;
    }
    geo_mag->size = geo_mag->maxord + 1;

    /* Load coefficients from file */
    return load_coefficients(geo_mag, coefficients_file);
}

int geomag_calculate(GeoMag *geo_mag, double glat, double glon, double alt, double time,
                     bool allow_date_outside_lifespan, bool raise_in_warning_zone,
                     GeoMagResult *result) {
    if (!geo_mag || !result) {
        return -1;
    }

    /* Initialize result structure */
    result->time = time;
    result->alt = alt;
    result->glat = glat;
    result->glon = glon;
    result->in_blackout_zone = false;
    result->in_caution_zone = false;
    result->is_high_resolution = (geo_mag->maxord == WMM_SIZE_HIGH_RESOLUTION);

    /* Working arrays */
    double tc[WMM_MAX_SIZE][WMM_MAX_SIZE];
    double dp[WMM_MAX_SIZE][WMM_MAX_SIZE];
    double sp[WMM_MAX_SIZE];
    double cp[WMM_MAX_SIZE];
    double pp[WMM_MAX_SIZE];

    /* Initialize constants */
    sp[0] = 0.0;
    cp[0] = 1.0;
    pp[0] = 1.0;
    dp[0][0] = 0.0;

    /* WGS84 ellipsoid parameters */
    const double a = WGS84_A;
    const double b = WGS84_B;
    const double re = WGS84_RE;
    const double a2 = a * a;
    const double b2 = b * b;
    const double c2 = a2 - b2;
    const double a4 = a2 * a2;
    const double b4 = b2 * b2;
    const double c4 = a4 - b4;

    /* Check date range */
    double dt = time - geo_mag->epoch;
    if (!allow_date_outside_lifespan && (dt < 0.0 || dt > 5.0)) {
        fprintf(stderr, "Error: Time extends beyond model 5-year life span\n");
        return -1;
    }

    /* Convert to radians */
    double rlon = DEG2RAD(glon);
    double rlat = DEG2RAD(glat);
    double srlon = sin(rlon);
    double srlat = sin(rlat);
    double crlon = cos(rlon);
    double crlat = cos(rlat);
    double srlat2 = srlat * srlat;
    double crlat2 = crlat * crlat;

    sp[1] = srlon;
    cp[1] = crlon;

    /* Convert from geodetic to spherical coordinates */
    double q = sqrt(a2 - c2 * srlat2);
    double q1 = alt * q;
    double q2 = ((q1 + a2) / (q1 + b2)) * ((q1 + a2) / (q1 + b2));
    double ct = srlat / sqrt(q2 * crlat2 + srlat2);
    double st = sqrt(1.0 - (ct * ct));
    double r2 = (alt * alt) + 2.0 * q1 + (a4 - c4 * srlat2) / (q * q);
    double r = sqrt(r2);
    double d = sqrt(a2 * crlat2 + b2 * srlat2);
    double ca = (alt + d) / r;
    double sa = c2 * crlat * srlat / (r * d);

    /* Compute sine and cosine of longitude multiples */
    for (int m = 2; m <= geo_mag->maxord; m++) {
        sp[m] = sp[1] * cp[m - 1] + cp[1] * sp[m - 1];
        cp[m] = cp[1] * cp[m - 1] - sp[1] * sp[m - 1];
    }

    double aor = re / r;
    double ar = aor * aor;
    double br = 0.0, bt = 0.0, bp = 0.0, bpp = 0.0;

    /* Main computation loop */
    for (int n = 1; n <= geo_mag->maxord; n++) {
        ar = ar * aor;
        int m = 0;
        int D3 = 1;
        int D4 = (n + m + D3) / D3;

        while (D4 > 0) {
            /* Compute unnormalized associated Legendre polynomials and derivatives */
            if (n == m) {
                geo_mag->p[n + m * geo_mag->size] = st * geo_mag->p[n - 1 + (m - 1) * geo_mag->size];
                dp[m][n] = st * dp[m - 1][n - 1] + ct * geo_mag->p[n - 1 + (m - 1) * geo_mag->size];
            } else if (n == 1 && m == 0) {
                geo_mag->p[n + m * geo_mag->size] = ct * geo_mag->p[n - 1 + m * geo_mag->size];
                dp[m][n] = ct * dp[m][n - 1] - st * geo_mag->p[n - 1 + m * geo_mag->size];
            } else if (n > 1 && n != m) {
                if (m > n - 2) {
                    geo_mag->p[n - 2 + m * geo_mag->size] = 0.0;
                }
                if (m > n - 2) {
                    dp[m][n - 2] = 0.0;
                }
                geo_mag->p[n + m * geo_mag->size] = ct * geo_mag->p[n - 1 + m * geo_mag->size] -
                                                      geo_mag->k[m][n] * geo_mag->p[n - 2 + m * geo_mag->size];
                dp[m][n] = ct * dp[m][n - 1] - st * geo_mag->p[n - 1 + m * geo_mag->size] -
                           geo_mag->k[m][n] * dp[m][n - 2];
            }

            /* Time adjust the Gauss coefficients */
            tc[m][n] = geo_mag->c[m][n] + dt * geo_mag->cd[m][n];
            if (m != 0) {
                tc[n][m - 1] = geo_mag->c[n][m - 1] + dt * geo_mag->cd[n][m - 1];
            }

            /* Accumulate terms of the spherical harmonic expansions */
            double par = ar * geo_mag->p[n + m * geo_mag->size];
            double temp1, temp2;

            if (m == 0) {
                temp1 = tc[m][n] * cp[m];
                temp2 = tc[m][n] * sp[m];
            } else {
                temp1 = tc[m][n] * cp[m] + tc[n][m - 1] * sp[m];
                temp2 = tc[m][n] * sp[m] - tc[n][m - 1] * cp[m];
            }

            bt = bt - ar * temp1 * dp[m][n];
            bp += geo_mag->fm[m] * temp2 * par;
            br += geo_mag->fn[n] * temp1 * par;

            /* Special case: North/South geographic poles */
            if (st == 0.0 && m == 1) {
                if (n == 1) {
                    pp[n] = pp[n - 1];
                } else {
                    pp[n] = ct * pp[n - 1] - geo_mag->k[m][n] * pp[n - 2];
                }
                double parp = ar * pp[n];
                bpp += geo_mag->fm[m] * temp2 * parp;
            }

            D4 -= 1;
            m += D3;
        }
    }

    if (st == 0.0) {
        bp = bpp;
    } else {
        bp /= st;
    }

    /* Rotate magnetic vector components from spherical to geodetic coordinates */
    double bx = -bt * ca - br * sa;
    double by = bp;
    double bz = bt * sa - br * ca;

    /* Compute declination (D), inclination (I), and total intensity (F) */
    double bh = sqrt(bx * bx + by * by);
    result->f = sqrt(bh * bh + bz * bz);
    result->d = RAD2DEG(atan2(by, bx));
    result->i = RAD2DEG(atan2(bz, bh));

    /* Compute magnetic grid variation if in Arctic or Antarctic */
    result->gv = -999.0;
    if (fabs(glat) >= 55.0) {
        if (glat > 0.0 && glon >= 0.0) {
            result->gv = result->d - glon;
        } else if (glat > 0.0 && glon < 0.0) {
            result->gv = result->d + fabs(glon);
        } else if (glat < 0.0 && glon >= 0.0) {
            result->gv = result->d + glon;
        } else if (glat < 0.0 && glon < 0.0) {
            result->gv = result->d - fabs(glon);
        }

        if (result->gv > 180.0) {
            result->gv -= 360.0;
        } else if (result->gv < -180.0) {
            result->gv += 360.0;
        }
    }

    /* Compute X, Y, Z, and H components */
    result->x = result->f * cos(DEG2RAD(result->d)) * cos(DEG2RAD(result->i));
    result->y = result->f * cos(DEG2RAD(result->i)) * sin(DEG2RAD(result->d));
    result->z = result->f * sin(DEG2RAD(result->i));
    result->h = result->f * cos(DEG2RAD(result->i));

    /* Check for blackout and caution zones */
    if (result->h < BLACKOUT_ZONE) {
        result->in_blackout_zone = true;
        if (raise_in_warning_zone) {
            fprintf(stderr, "Warning: In blackout zone (H = %.1f nT)\n", result->h);
            return -2;
        }
    } else if (result->h < CAUTION_ZONE) {
        result->in_caution_zone = true;
        if (raise_in_warning_zone) {
            fprintf(stderr, "Warning: In caution zone (H = %.1f nT)\n", result->h);
            return -3;
        }
    }

    return 0;
}

int geomag_calculate_uncertainty(const GeoMagResult *result, GeoMagUncertainty *uncertainty) {
    if (!result || !uncertainty) {
        return -1;
    }

    /* Determine which error model to use based on time */
    if (result->time >= WMM_MODEL_2025_LOWER && result->time <= WMM_MODEL_2025_UPPER) {
        if (result->is_high_resolution) {
            /* WMMHR-2025 error model */
            uncertainty->x = 135.0;
            uncertainty->y = 85.0;
            uncertainty->z = 134.0;
            uncertainty->h = 130.0;
            uncertainty->f = 134.0;
            uncertainty->i = 0.19;
            uncertainty->d = sqrt(0.25 * 0.25 + (5205.0 / result->h) * (5205.0 / result->h));
        } else {
            /* WMM-2025 error model */
            uncertainty->x = 137.0;
            uncertainty->y = 89.0;
            uncertainty->z = 141.0;
            uncertainty->h = 133.0;
            uncertainty->f = 138.0;
            uncertainty->i = 0.20;
            uncertainty->d = sqrt(0.26 * 0.26 + (5417.0 / result->h) * (5417.0 / result->h));
        }
    } else if (result->time >= WMM_MODEL_2020_LOWER && result->time <= WMM_MODEL_2020_UPPER) {
        /* WMM-2020 error model */
        uncertainty->x = 131.0;
        uncertainty->y = 94.0;
        uncertainty->z = 157.0;
        uncertainty->h = 128.0;
        uncertainty->f = 148.0;
        uncertainty->i = 0.21;
        uncertainty->d = sqrt(0.26 * 0.26 + (5625.0 / result->h) * (5625.0 / result->h));
    } else if (result->time >= WMM_MODEL_2015_LOWER && result->time < WMM_MODEL_2015_UPPER) {
        /* WMM-2015 error model */
        uncertainty->x = 138.0;
        uncertainty->y = 89.0;
        uncertainty->z = 165.0;
        uncertainty->h = 133.0;
        uncertainty->f = 152.0;
        uncertainty->i = 0.22;
        uncertainty->d = sqrt(0.23 * 0.23 + (5430.0 / result->h) * (5430.0 / result->h));
    } else {
        fprintf(stderr, "Error: No uncertainty estimates available for time %.2f\n", result->time);
        return -1;
    }

    return 0;
}

void geomag_free(GeoMag *geo_mag) {
    /* Currently no dynamic allocation, so nothing to free */
    /* This function is provided for API completeness and future extensibility */
    (void)geo_mag;
}