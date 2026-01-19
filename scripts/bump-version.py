#!/usr/bin/env python3
"""
Bumps the version in TelemetryKit.json (root and cppDependencies).
publish.gradle reads from this file, so only the JSON needs updating.

Usage:
    python scripts/bump-version.py 1.0.2
    python scripts/bump-version.py patch   # 1.0.1 -> 1.0.2
    python scripts/bump-version.py minor   # 1.0.1 -> 1.1.0
    python scripts/bump-version.py major   # 1.0.1 -> 2.0.0
"""

import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent
VENDORDEP_FILE = REPO_ROOT / "TelemetryKit.json"


def parse_version(version: str) -> tuple[int, int, int]:
    parts = version.split(".")
    if len(parts) != 3:
        raise ValueError(f"Invalid version format: {version}")
    return int(parts[0]), int(parts[1]), int(parts[2])


def bump_version(current: str, bump_type: str) -> str:
    major, minor, patch = parse_version(current)
    if bump_type == "major":
        return f"{major + 1}.0.0"
    elif bump_type == "minor":
        return f"{major}.{minor + 1}.0"
    elif bump_type == "patch":
        return f"{major}.{minor}.{patch + 1}"
    else:
        # Assume it's an explicit version
        parse_version(bump_type)  # Validate format
        return bump_type


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)

    arg = sys.argv[1]

    # Read current vendordep
    with open(VENDORDEP_FILE, "r") as f:
        data = json.load(f)

    current_version = data["version"]
    new_version = bump_version(current_version, arg)

    print(f"Bumping version: {current_version} -> {new_version}")

    # Update root version
    data["version"] = new_version

    # Update all cppDependencies versions
    for dep in data.get("cppDependencies", []):
        dep["version"] = new_version

    # Update all javaDependencies versions (if any)
    for dep in data.get("javaDependencies", []):
        dep["version"] = new_version

    # Write back
    with open(VENDORDEP_FILE, "w") as f:
        json.dump(data, f, indent=2)
        f.write("\n")

    print(f"Updated {VENDORDEP_FILE}")
    print("publish.gradle will read the version from the JSON at build time.")


if __name__ == "__main__":
    main()
