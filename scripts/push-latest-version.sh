#!/usr/bin/env bash
set -euo pipefail

# Script to push the latest version tag to remote
# Usage: ./scripts/push-latest-version.sh [remote] [--dry-run] [-y|--yes]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Parse arguments
REMOTE="origin"
DRY_RUN=false
YES=false
for arg in "$@"; do
    case "$arg" in
        --dry-run)
            DRY_RUN=true
            ;;
        -y|--yes)
            YES=true
            ;;
        *)
            if [ "$REMOTE" = "origin" ] && [[ ! "$arg" =~ ^-- ]]; then
                REMOTE="$arg"
            fi
            ;;
    esac
done

# Get latest version tag
get_latest_tag() {
    git describe --tags --abbrev=0 2>/dev/null || echo ""
}

# Check if tag exists locally
tag_exists() {
    local tag="$1"
    git rev-parse "${tag}" >/dev/null 2>&1
}

# Check if tag exists on remote
tag_exists_remote() {
    local tag="$1"
    local remote="$2"
    git ls-remote --tags "${remote}" | grep -q "refs/tags/${tag}$" 2>/dev/null
}

# Push tag to remote
push_tag() {
    local tag="$1"
    local remote="$2"
    
    if [ "$DRY_RUN" = true ]; then
        echo "[DRY RUN] Would push tag ${tag} to ${remote}"
        echo "  Command: git push ${remote} ${tag}"
        return 0
    fi
    
    echo "Pushing tag ${tag} to ${remote}..."
    if git push "${remote}" "${tag}"; then
        echo "✓ Successfully pushed ${tag} to ${remote}"
    else
        echo "✗ Failed to push ${tag} to ${remote}" >&2
        exit 1
    fi
}

# Main execution
main() {
    # Check if we're in a git repository
    if ! git rev-parse --git-dir >/dev/null 2>&1; then
        echo "Error: Not in a git repository" >&2
        exit 1
    fi
    
    # Check if remote exists
    if ! git remote | grep -q "^${REMOTE}$"; then
        echo "Error: Remote '${REMOTE}' does not exist" >&2
        echo "Available remotes: $(git remote | tr '\n' ' ')" >&2
        exit 1
    fi
    
    # Get latest tag
    LATEST_TAG=$(get_latest_tag)
    
    if [ -z "$LATEST_TAG" ]; then
        echo "Error: No version tags found" >&2
        echo "Create a tag first using: ./scripts/tag-version.sh [major|minor|patch|rc|release]" >&2
        exit 1
    fi
    
    echo "Latest version tag: ${LATEST_TAG}"
    
    if [ "$DRY_RUN" = true ]; then
        echo ""
        echo "=== DRY RUN MODE ==="
        echo "No changes will be made."
        echo ""
    fi
    
    # Check if tag exists locally
    if ! tag_exists "$LATEST_TAG"; then
        echo "Error: Tag ${LATEST_TAG} does not exist locally" >&2
        exit 1
    fi
    
    # Check if tag already exists on remote
    if tag_exists_remote "$LATEST_TAG" "$REMOTE"; then
        if [ "$DRY_RUN" = true ]; then
            echo "[DRY RUN] Tag ${LATEST_TAG} already exists on ${REMOTE}"
        else
            echo "Tag ${LATEST_TAG} already exists on ${REMOTE}"
            if [ "$YES" = true ]; then
                echo "Continuing anyway (--yes flag set)"
            else
                read -p "Push anyway? (y/N) " -n 1 -r
                echo
                if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                    exit 0
                fi
            fi
        fi
    else
        if [ "$DRY_RUN" = true ]; then
            echo "[DRY RUN] Tag ${LATEST_TAG} does not exist on ${REMOTE}"
        fi
    fi
    
    if [ "$DRY_RUN" = true ]; then
        # Push the tag (dry run)
        push_tag "$LATEST_TAG" "$REMOTE"
        echo ""
        echo "=== DRY RUN COMPLETE ==="
        echo "Run without --dry-run to push the tag."
        exit 0
    fi
    
    # Confirm before pushing (unless --yes flag is set)
    if [ "$YES" != true ]; then
        echo ""
        echo "About to push tag ${LATEST_TAG} to ${REMOTE}"
        read -p "Continue? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo "Cancelled."
            exit 0
        fi
    fi
    
    # Push the tag
    push_tag "$LATEST_TAG" "$REMOTE"
    
    echo ""
    echo "✓ Tag ${LATEST_TAG} pushed to ${REMOTE}"
    echo ""
    echo "Note: If you want to push commits as well:"
    echo "  git push ${REMOTE} $(git rev-parse --abbrev-ref HEAD)"
}

main "$@"

