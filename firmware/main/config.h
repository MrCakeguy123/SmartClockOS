#pragma once

/**
 * SmartClock OS Configuration
 *
 * Customize these settings for your location and preferences.
 */

#ifdef __cplusplus
extern "C" {
#endif

// ===== WEATHER CONFIGURATION =====

/**
 * Your location coordinates for weather data
 *
 * To find your coordinates:
 * 1. Go to https://www.google.com/maps
 * 2. Right-click your location
 * 3. Click the coordinates to copy them
 *
 * Format: Decimal degrees
 * Example: Dry Ridge, Kentucky = 38.6820, -84.5894
 */
#ifndef WEATHER_LAT
#define WEATHER_LAT 38.6820
#endif

#ifndef WEATHER_LON
#define WEATHER_LON -84.5894
#endif

/**
 * Location name for display
 */
#ifndef LOCATION_NAME
#define LOCATION_NAME "Dry Ridge"
#endif

// ===== POWER MANAGEMENT =====

/**
 * Auto-dim timeout (seconds)
 * Time before screen dims when idle
 */
#ifndef AUTO_DIM_TIMEOUT_SEC
#define AUTO_DIM_TIMEOUT_SEC 30
#endif

/**
 * Auto-blank timeout (seconds)
 * Time before screen goes dark when idle
 */
#ifndef AUTO_BLANK_TIMEOUT_SEC
#define AUTO_BLANK_TIMEOUT_SEC 60
#endif

/**
 * Deep sleep timeout (seconds)
 * Time before entering deep sleep when idle
 */
#ifndef DEEP_SLEEP_TIMEOUT_SEC
#define DEEP_SLEEP_TIMEOUT_SEC 600
#endif

/**
 * Night mode hours
 * Screen dims faster during these hours
 */
#ifndef NIGHT_MODE_START_HOUR
#define NIGHT_MODE_START_HOUR 22  // 10 PM
#endif

#ifndef NIGHT_MODE_END_HOUR
#define NIGHT_MODE_END_HOUR 6     // 6 AM
#endif

// ===== NETWORK CONFIGURATION =====

/**
 * Wi-Fi configuration
 * Leave empty to use provisioning mode
 */
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// ===== TIME CONFIGURATION =====

/**
 * NTP server for time synchronization
 */
#ifndef NTP_SERVER
#define NTP_SERVER "pool.ntp.org"
#endif

/**
 * Timezone string (POSIX format)
 * Examples:
 * - EST5EDT,M3.2.0,M11.1.0 (US Eastern)
 * - PST8PDT,M3.2.0,M11.1.0 (US Pacific)
 * - CET-1CEST,M3.5.0,M10.5.0/3 (Central Europe)
 *
 * Find your timezone: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
 */
#ifndef TIMEZONE_STRING
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"
#endif

#ifdef __cplusplus
}
#endif
