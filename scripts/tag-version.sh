#!/usr/bin/env bash
set -euo pipefail

# Script to bump and tag semantic versions
# Usage: ./scripts/tag-version.sh [major|minor|patch|rc|release] [--dry-run]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CONANFILE="${PROJECT_ROOT}/conanfile.py"
CMAKELISTS="${PROJECT_ROOT}/CMakeLists.txt"

# Check for dry-run flag
DRY_RUN=false
if [[ "${@}" =~ --dry-run ]]; then
    DRY_RUN=true
fi

# Get current version from conanfile.py
get_current_version() {
    if [ -f "${CONANFILE}" ]; then
        grep -E '^\s*version\s*=' "${CONANFILE}" | sed -E 's/^[^"\047]*version[[:space:]]*=[[:space:]]*["'\'']([^"'\'']+)["'\''].*/\1/' | head -1 || echo ""
    else
        echo ""
    fi
}

# Get latest git tag
get_latest_tag() {
    git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo ""
}

# Parse version string (MAJOR.MINOR.PATCH-rcRC or MAJOR.MINOR.PATCH)
parse_version() {
    local version="$1"
    if [[ "$version" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)(-rc([0-9]+))?$ ]]; then
        MAJOR="${BASH_REMATCH[1]}"
        MINOR="${BASH_REMATCH[2]}"
        PATCH="${BASH_REMATCH[3]}"
        RC="${BASH_REMATCH[5]:-}"
    else
        echo "Error: Invalid version format: $version" >&2
        exit 1
    fi
}

# Bump version based on type
bump_version() {
    local bump_type="$1"
    
    case "$bump_type" in
        major)
            MAJOR=$((MAJOR + 1))
            MINOR=0
            PATCH=0
            RC=""
            ;;
        minor)
            MINOR=$((MINOR + 1))
            PATCH=0
            RC=""
            ;;
        patch)
            PATCH=$((PATCH + 1))
            RC=""
            ;;
        rc)
            if [ -z "$RC" ]; then
                RC=1
            else
                RC=$((RC + 1))
            fi
            ;;
        release)
            if [ -z "$RC" ]; then
                echo "Error: Current version is not an RC version" >&2
                exit 1
            fi
            RC=""
            ;;
        *)
            echo "Error: Invalid bump type: $bump_type" >&2
            echo "Usage: $0 [major|minor|patch|rc|release]" >&2
            exit 1
            ;;
    esac
    
    # Build new version string
    if [ -n "$RC" ]; then
        NEW_VERSION="${MAJOR}.${MINOR}.${PATCH}-rc${RC}"
    else
        NEW_VERSION="${MAJOR}.${MINOR}.${PATCH}"
    fi
}

# Update version in all files
update_version_files() {
    local new_version="$1"
    local sed_in_place
    
    if [[ "$(uname)" == "Darwin" ]]; then
        sed_in_place="sed -i ''"
    else
        sed_in_place="sed -i"
    fi
    
    if [ "$DRY_RUN" = true ]; then
        echo "[DRY RUN] Would update versions to: ${new_version}"
        echo ""
        
        # Show what would change in conanfile.py
        if [ -f "${CONANFILE}" ]; then
            local current_line=$(grep -E '^\s*version\s*=' "${CONANFILE}")
            local new_line=$(echo "$current_line" | sed "s/version = \"[^\"]*\"/version = \"${new_version}\"/")
            echo "  ${CONANFILE}:"
            echo "    - ${current_line}"
            echo "    + ${new_line}"
        fi
        
        # Show what would change in CMakeLists.txt
        if [ -f "${CMAKELISTS}" ]; then
            local current_line=$(grep -E '\s+VERSION\s+' "${CMAKELISTS}")
            local new_line=$(echo "$current_line" | sed -E "s/VERSION [0-9]+\\.[0-9]+\\.[0-9]+(-rc[0-9]+)?/VERSION ${new_version}/")
            echo "  ${CMAKELISTS}:"
            echo "    - ${current_line}"
            echo "    + ${new_line}"
        fi
        return
    fi
    
    # Update conanfile.py
    if [ -f "${CONANFILE}" ]; then
        $sed_in_place "s/version = \"[^\"]*\"/version = \"${new_version}\"/" "${CONANFILE}"
        echo "✓ Updated ${CONANFILE} to version ${new_version}"
    fi
    
    # Update CMakeLists.txt (project VERSION)
    if [ -f "${CMAKELISTS}" ]; then
        $sed_in_place -E "s/VERSION [0-9]+\\.[0-9]+\\.[0-9]+(-rc[0-9]+)?/VERSION ${new_version}/" "${CMAKELISTS}"
        echo "✓ Updated ${CMAKELISTS} to version ${new_version}"
    fi
}

# Create git tag
create_tag() {
    local version="$1"
    local tag="v${version}"
    
    if [ "$DRY_RUN" = true ]; then
        echo "[DRY RUN] Would create git tag: ${tag}"
        echo "  Message: \"Version ${version}\""
        
        # Check if tag already exists
        if git rev-parse "${tag}" >/dev/null 2>&1; then
            echo "  Warning: Tag ${tag} already exists"
        fi
        
        # Check for uncommitted changes
        if ! git diff-index --quiet HEAD --; then
            echo "  Warning: Uncommitted changes detected"
            git status --short
        fi
        return
    fi
    
    # Check if tag already exists
    if git rev-parse "${tag}" >/dev/null 2>&1; then
        echo "Warning: Tag ${tag} already exists" >&2
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    # Check for uncommitted changes
    if ! git diff-index --quiet HEAD --; then
        echo "Warning: You have uncommitted changes" >&2
        git status --short
        read -p "Continue with tag anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    # Create tag
    git tag -a "${tag}" -m "Version ${version}"
    echo "Created git tag: ${tag}"
}

# Main execution
main() {
    local bump_type=""
    local make_rc=false
    local args=("$@")
    
    # Parse arguments
    local i=0
    while [ $i -lt ${#args[@]} ]; do
        local arg="${args[$i]}"
        case "$arg" in
            --dry-run)
                DRY_RUN=true
                ;;
            major|minor|patch)
                bump_type="$arg"
                # Check if next argument is "rc"
                if [ $((i + 1)) -lt ${#args[@]} ] && [ "${args[$((i + 1))]}" = "rc" ]; then
                    make_rc=true
                    ((i++))  # Skip next arg
                fi
                ;;
            rc)
                if [ -z "$bump_type" ]; then
                    bump_type="rc"
                else
                    # rc after major/minor/patch
                    make_rc=true
                fi
                ;;
            release)
                bump_type="release"
                ;;
            *)
                if [ -z "$bump_type" ] && [[ ! "$arg" =~ ^-- ]]; then
                    bump_type="$arg"
                fi
                ;;
        esac
        ((i++))
    done
    
    if [ -z "$bump_type" ]; then
        echo "Usage: $0 [major|minor|patch|rc|release] [rc] [--dry-run]" >&2
        echo "" >&2
        echo "Examples:" >&2
        echo "  $0 major          # 1.2.3 -> 2.0.0" >&2
        echo "  $0 major rc        # 1.2.3 -> 2.0.0-rc1" >&2
        echo "  $0 minor rc        # 1.2.3 -> 1.3.0-rc1" >&2
        echo "  $0 patch rc        # 1.2.3 -> 1.2.4-rc1" >&2
        echo "  $0 rc              # 1.2.3 -> 1.2.3-rc1" >&2
        echo "  $0 release         # 1.2.3-rc2 -> 1.2.3" >&2
        echo "" >&2
        echo "Current version: $(get_current_version)" >&2
        echo "" >&2
        echo "Options:" >&2
        echo "  --dry-run    Show what would change without making changes" >&2
        exit 1
    fi
    
    # Get current version (prefer conanfile.py, fallback to git tag)
    CURRENT_VERSION=$(get_current_version)
    if [ -z "$CURRENT_VERSION" ]; then
        CURRENT_VERSION=$(get_latest_tag)
    fi
    
    if [ -z "$CURRENT_VERSION" ]; then
        echo "Error: Could not determine current version" >&2
        echo "Please set version in ${CONANFILE} or create an initial git tag" >&2
        exit 1
    fi
    
    echo "Current version: ${CURRENT_VERSION}"
    
    if [ "$DRY_RUN" = true ]; then
        echo ""
        echo "=== DRY RUN MODE ==="
        echo "No changes will be made."
        echo ""
    fi
    
    # Parse and bump
    parse_version "$CURRENT_VERSION"
    
    # If make_rc is true, we'll add rc after bumping
    if [ "$make_rc" = true ] && [ "$bump_type" != "rc" ] && [ "$bump_type" != "release" ]; then
        # Bump the version first (this clears RC)
        bump_version "$bump_type"
        # Then add rc (always start at rc1 for new bumped versions)
        RC=1
        NEW_VERSION="${MAJOR}.${MINOR}.${PATCH}-rc${RC}"
    else
        # Normal bump (or rc/release)
        bump_version "$bump_type"
    fi
    
    echo "New version: ${NEW_VERSION}"
    echo ""
    
    # Update all version files
    update_version_files "$NEW_VERSION"
    
    if [ "$DRY_RUN" = true ]; then
        echo ""
        create_tag "$NEW_VERSION"
        echo ""
        echo "=== DRY RUN COMPLETE ==="
        echo "Run without --dry-run to apply changes."
        exit 0
    fi
    
    # Create git tag
    read -p "Create git tag v${NEW_VERSION}? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        create_tag "$NEW_VERSION"
        echo ""
        echo "✓ Version bumped to ${NEW_VERSION}"
        echo "✓ Git tag v${NEW_VERSION} created"
        echo ""
        echo "To push the tag: git push origin v${NEW_VERSION}"
    else
        echo "Tag creation cancelled. Version updated in files only."
    fi
}

main "$@"

