#!/usr/bin/env python3
"""
Version bumping script for semantic versioning.

This script analyzes the commit message to determine version bump type:
- MAJOR: commits with "BREAKING CHANGE:" or "feat!:" or "fix!:"
- MINOR: commits starting with "feat:"
- PATCH: commits starting with "fix:", "docs:", "style:", "refactor:", "test:", "chore:"

Usage:
    python scripts/bump_version.py [commit_message]

If no commit message is provided, it will read from the last git commit.
"""

import re
import sys
import subprocess
import os

VERSION_FILE = "src/version.h"

def get_current_version():
    """Read current version from version.h"""
    if not os.path.exists(VERSION_FILE):
        return (1, 0, 0)

    with open(VERSION_FILE, 'r') as f:
        content = f.read()

    major_match = re.search(r'#define\s+VERSION_MAJOR\s+(\d+)', content)
    minor_match = re.search(r'#define\s+VERSION_MINOR\s+(\d+)', content)
    patch_match = re.search(r'#define\s+VERSION_PATCH\s+(\d+)', content)

    major = int(major_match.group(1)) if major_match else 1
    minor = int(minor_match.group(1)) if minor_match else 0
    patch = int(patch_match.group(1)) if patch_match else 0

    return (major, minor, patch)

def determine_bump_type(commit_message):
    """Determine version bump type based on commit message"""
    commit_message = commit_message.lower().strip()

    # Breaking changes (major bump)
    if ("breaking change:" in commit_message or
        commit_message.startswith("feat!:") or
        commit_message.startswith("fix!:")):
        return "major"

    # New features (minor bump)
    if commit_message.startswith("feat:"):
        return "minor"

    # Bug fixes, docs, style, refactor, test, chore (patch bump)
    if (commit_message.startswith(("fix:", "docs:", "style:", "refactor:", "test:", "chore:"))):
        return "patch"

    # Default to patch for other commits
    return "patch"

def bump_version(current_version, bump_type):
    """Calculate new version based on bump type"""
    major, minor, patch = current_version

    if bump_type == "major":
        return (major + 1, 0, 0)
    elif bump_type == "minor":
        return (major, minor + 1, 0)
    elif bump_type == "patch":
        return (major, minor, patch + 1)
    else:
        raise ValueError(f"Unknown bump type: {bump_type}")

def write_version_file(version):
    """Write new version to version.h"""
    major, minor, patch = version
    version_string = f"{major}.{minor}.{patch}"

    content = f"""#ifndef VERSION_H
#define VERSION_H

// Semantic Versioning (MAJOR.MINOR.PATCH)
#define VERSION_MAJOR {major}
#define VERSION_MINOR {minor}
#define VERSION_PATCH {patch}

// Build version string
#define VERSION_STRING "{version_string}"

// Helper macros for version operations
#define MAKE_VERSION_STRING(major, minor, patch) #major "." #minor "." #patch
#define VERSION_STRING_FROM_NUMBERS(major, minor, patch) MAKE_VERSION_STRING(major, minor, patch)

#endif // VERSION_H"""

    with open(VERSION_FILE, 'w') as f:
        f.write(content)

    return version_string

def get_last_commit_message():
    """Get the last commit message from git"""
    try:
        result = subprocess.run(
            ['git', 'log', '-1', '--pretty=format:%s'],
            capture_output=True,
            text=True,
            check=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return ""

def main():
    # Get commit message from argument or last commit
    if len(sys.argv) > 1:
        commit_message = " ".join(sys.argv[1:])
    else:
        commit_message = get_last_commit_message()

    if not commit_message:
        print("No commit message found. Unable to determine version bump.")
        sys.exit(1)

    print(f"Commit message: {commit_message}")

    # Get current version
    current_version = get_current_version()
    print(f"Current version: {'.'.join(map(str, current_version))}")

    # Determine bump type
    bump_type = determine_bump_type(commit_message)
    print(f"Bump type: {bump_type}")

    # Calculate new version
    new_version = bump_version(current_version, bump_type)

    # Write new version
    new_version_string = write_version_file(new_version)
    print(f"New version: {new_version_string}")

    return 0

if __name__ == "__main__":
    sys.exit(main())