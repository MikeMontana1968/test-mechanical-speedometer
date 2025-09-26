#ifndef VERSION_H
#define VERSION_H

// Semantic Versioning (MAJOR.MINOR.PATCH)
#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_PATCH 0

// Build version string
#define VERSION_STRING "1.2.0"

// Helper macros for version operations
#define MAKE_VERSION_STRING(major, minor, patch) #major "." #minor "." #patch
#define VERSION_STRING_FROM_NUMBERS(major, minor, patch) MAKE_VERSION_STRING(major, minor, patch)

#endif // VERSION_H