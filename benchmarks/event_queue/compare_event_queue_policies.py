#!/usr/bin/env python3

import json
import sys
from pathlib import Path


def _parse(path: Path):
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)

    rows = {}
    for b in data.get("benchmarks", []):
        name = b.get("name", "")
        if not name.startswith("run_push_workload/"):
            continue
        parts = name.split("/")[1:]
        if len(parts) < 3:
            continue
        q, n, scenario = parts[:3]
        key = (int(q), int(n), int(scenario))
        rows[key] = {
            "real_time_ns": b.get("real_time"),
            "cpu_time_ns": b.get("cpu_time"),
            "drops_per_iter": b.get("drops_per_iter"),
            "retained_per_iter": b.get("retained_per_iter"),
            "retained_ratio": b.get("retained_ratio"),
        }
    return rows


def _scenario_name(idx: int) -> str:
    return {
        0: "all_noncompressible",
        1: "all_compressible",
        2: "mixed",
    }.get(idx, f"scenario_{idx}")


def main():
    if len(sys.argv) != 4:
        print(
            "Usage: compare_event_queue_policies.py <oldest.json> <half_by_type.json> <half_by_type_window.json>",
            file=sys.stderr,
        )
        return 2

    labels = ["oldest_only", "half_by_type", "half_by_type_window"]
    parsed = {label: _parse(Path(path)) for label, path in zip(labels, sys.argv[1:])}

    keys = sorted(set().union(*[set(rows.keys()) for rows in parsed.values()]))
    if not keys:
        print("No benchmark rows found.")
        return 1

    for key in keys:
        q, n, scenario = key
        print(f"\nScenario={_scenario_name(scenario)} Q={q} N={n}")
        print("policy                 real_ns    cpu_ns     drops      retained   ratio")
        for label in labels:
            row = parsed[label].get(key)
            if not row:
                print(f"{label:20s} <missing>")
                continue
            print(
                f"{label:20s} "
                f"{row['real_time_ns']:9.1f} "
                f"{row['cpu_time_ns']:9.1f} "
                f"{row['drops_per_iter']:10.2f} "
                f"{row['retained_per_iter']:10.2f} "
                f"{row['retained_ratio']:7.4f}"
            )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
