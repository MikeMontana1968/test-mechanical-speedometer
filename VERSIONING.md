# Semantic Versioning Guide

This project uses semantic versioning (MAJOR.MINOR.PATCH) with automatic version bumping based on commit messages.

## Version Format

- **MAJOR**: Breaking changes or incompatible API changes
- **MINOR**: New features that are backwards compatible
- **PATCH**: Bug fixes, documentation, style changes, refactoring, tests, chores

## Commit Message Format

Use conventional commit messages to trigger automatic version bumping:

### Major Version Bump (Breaking Changes)
- `feat!: add new breaking feature`
- `fix!: breaking change to fix critical issue`
- `feat: add feature with BREAKING CHANGE: description`

### Minor Version Bump (New Features)
- `feat: add new feature`
- `feat: implement user authentication`

### Patch Version Bump (Bug Fixes & Maintenance)
- `fix: resolve servo positioning bug`
- `docs: update README with setup instructions`
- `style: fix code formatting`
- `refactor: optimize speedometer calculations`
- `test: add unit tests for gear detection`
- `chore: update dependencies`

## Manual Version Bumping

### Windows
```bash
bump-version.bat "fix: resolve servo bug"
```

### Cross-platform (Python)
```bash
python scripts/bump_version.py "feat: add new speedometer feature"
```

## Automatic Version Bumping

The project includes git hooks that automatically bump versions on commit:

1. **prepare-commit-msg hook**: Analyzes commit message and updates version.h
2. **pre-commit hook**: Alternative approach for version bumping

## Version Display

The current version is displayed:
- At startup via Serial.println() in main.cpp
- Defined in src/version.h
- Available as VERSION_STRING macro

## Version File Structure

`src/version.h` contains:
```cpp
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_STRING "1.0.0"
```

## Build Integration

The version is automatically included in builds and displayed at startup:
```
=== Mechanical Speedometer Demo ===
Version: 1.0.0
Starting system initialization...
```

## Best Practices

1. Use conventional commit messages consistently
2. Test major changes before committing
3. Keep BREAKING CHANGES rare and well-documented
4. Group related changes in single commits when possible
5. Verify version bump is correct before pushing

## Example Workflow

```bash
# Make changes to code
git add .

# Commit with conventional message (version will auto-bump)
git commit -m "feat: add RPM-based speed calculation"

# Or manually bump version first
bump-version.bat "feat: add RPM-based speed calculation"
git add src/version.h
git commit -m "feat: add RPM-based speed calculation"

# Push changes
git push
```