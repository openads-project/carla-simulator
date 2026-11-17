#!/usr/bin/env python3

import argparse
import filecmp
import json
import shutil
import sys
from pathlib import Path, PurePosixPath


def merge_json_value(base, overlay):
    if isinstance(base, dict) and isinstance(overlay, dict):
        merged = dict(base)
        for key, value in overlay.items():
            merged[key] = merge_json_value(base[key], value) if key in base else value
        return merged

    if isinstance(base, list) and isinstance(overlay, list):
        return base + overlay

    return overlay


def read_json(path):
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path, value):
    with path.open("w", encoding="utf-8") as output:
        json.dump(value, output, indent=4, ensure_ascii=False)
        output.write("\n")


def read_excludes(path):
    """Overlay paths, relative to the overlay root, one per line; '#' starts a comment."""
    excludes = []
    with path.open(encoding="utf-8") as handle:
        for line in handle:
            entry = line.split("#", 1)[0].strip().strip("/")
            if entry:
                excludes.append(entry)
    return excludes


def matching_exclude(relative_path, excludes):
    path = relative_path.as_posix()
    for entry in excludes:
        if path == entry or path.startswith(entry + "/"):
            return entry
    return None


def excluded_object_paths(excludes, game_path):
    """Unreal object path of each excluded overlay path, e.g. Static/Bicycle -> /Game/Carla/Static/Bicycle."""
    prefixes = []
    for entry in excludes:
        path = PurePosixPath(entry)
        if path.suffix.lower() in (".uasset", ".umap"):
            path = path.with_suffix("")
        prefixes.append(f"{game_path.rstrip('/')}/{path.as_posix()}")
    return prefixes


def references_excluded(value, prefixes):
    if isinstance(value, str):
        return any(
            value == prefix or (value.startswith(prefix) and value[len(prefix)] in "/.:")
            for prefix in prefixes
        )
    if isinstance(value, dict):
        return any(references_excluded(item, prefixes) for item in value.values())
    if isinstance(value, list):
        return any(references_excluded(item, prefixes) for item in value)
    return False


def filter_catalog(value, prefixes, dropped):
    """Remove the list entries (vehicles, props, ...) that name an excluded asset."""
    if isinstance(value, dict):
        return {key: filter_catalog(item, prefixes, dropped) for key, item in value.items()}
    if isinstance(value, list):
        kept = []
        for item in value:
            if references_excluded(item, prefixes):
                dropped.append(item)
            else:
                kept.append(filter_catalog(item, prefixes, dropped))
        return kept
    return value


def describe_entry(entry):
    if isinstance(entry, dict):
        if "Make" in entry and "Model" in entry:
            return f"{entry['Make']}.{entry['Model']}"
        for key in ("Name", "name", "Id"):
            if key in entry:
                return str(entry[key])
    return json.dumps(entry, ensure_ascii=False)[:80]


def merge_json_file(base_file, overlay_file, prefixes, dropped):
    dropped_before = len(dropped)
    overlay_json = filter_catalog(read_json(overlay_file), prefixes, dropped)
    filtered = len(dropped) > dropped_before

    if not base_file.exists():
        if filtered:
            write_json(base_file, overlay_json)
        else:
            shutil.copy2(overlay_file, base_file)
        return "copied"

    base_json = read_json(base_file)
    merged_json = merge_json_value(base_json, overlay_json)

    if merged_json == base_json:
        return "unchanged"

    write_json(base_file, merged_json)
    return "json_merged"


def merge_overlay(base_dir, overlay_dir, excludes=(), game_path="/Game/Carla"):
    counters = {
        "copied": 0,
        "overwritten": 0,
        "json_merged": 0,
        "unchanged": 0,
        "excluded": 0,
    }
    prefixes = excluded_object_paths(excludes, game_path)
    dropped = []
    used_excludes = set()

    for overlay_file in sorted(path for path in overlay_dir.rglob("*") if path.is_file()):
        relative_path = overlay_file.relative_to(overlay_dir)

        exclude = matching_exclude(relative_path, excludes)
        if exclude is not None:
            used_excludes.add(exclude)
            counters["excluded"] += 1
            continue

        base_file = base_dir / relative_path
        base_file.parent.mkdir(parents=True, exist_ok=True)

        if base_file.suffix.lower() == ".json":
            counters[merge_json_file(base_file, overlay_file, prefixes, dropped)] += 1
            continue

        if not base_file.exists():
            shutil.copy2(overlay_file, base_file)
            counters["copied"] += 1
            continue

        if filecmp.cmp(overlay_file, base_file, shallow=False):
            counters["unchanged"] += 1
            continue

        shutil.copy2(overlay_file, base_file)
        counters["overwritten"] += 1

    unused_excludes = [entry for entry in excludes if entry not in used_excludes]
    return counters, dropped, unused_excludes


def main():
    parser = argparse.ArgumentParser(description="Merge a content overlay into a base content tree.")
    parser.add_argument("--base", required=True, type=Path, help="Base content directory")
    parser.add_argument("--overlay", required=True, type=Path, help="Overlay content directory")
    parser.add_argument(
        "--exclude-file",
        type=Path,
        help="File listing overlay paths (relative to --overlay) to leave out, together with "
        "the catalog entries of the overlay's JSON files that name an asset under them",
    )
    parser.add_argument(
        "--game-path",
        default="/Game/Carla",
        help="Unreal path of the base content directory, used to match catalog entries "
        "against the excluded paths (default: /Game/Carla)",
    )
    args = parser.parse_args()

    if not args.base.is_dir():
        raise SystemExit(f"Base content directory not found: {args.base}")
    if not args.overlay.is_dir():
        raise SystemExit(f"Overlay content directory not found: {args.overlay}")
    if args.exclude_file is not None and not args.exclude_file.is_file():
        raise SystemExit(f"Exclude file not found: {args.exclude_file}")

    excludes = read_excludes(args.exclude_file) if args.exclude_file is not None else []
    counters, dropped, unused_excludes = merge_overlay(args.base, args.overlay, excludes, args.game_path)

    for entry in unused_excludes:
        print(f"Warning: excluded overlay path '{entry}' matches no overlay file.", file=sys.stderr)
    for entry in dropped:
        print(f"Left out catalog entry: {describe_entry(entry)}")
    print(
        "Content overlay merge complete: "
        f"{counters['copied']} copied, "
        f"{counters['overwritten']} overwritten, "
        f"{counters['json_merged']} JSON merged, "
        f"{counters['unchanged']} unchanged, "
        f"{counters['excluded']} excluded, "
        f"{len(dropped)} catalog entries left out."
    )


if __name__ == "__main__":
    main()
