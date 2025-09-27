#!/usr/bin/env bash
# mytests.sh — student test driver for MP3
# Usage:
#   bash mytests.sh                # run harness + default perf sweep
#   bash mytests.sh perf           # just the perf sweep
#   bash mytests.sh harness        # just run.sh + check.pl
#   bash mytests.sh perf sizes=20,100,1000 sorts=1,4,5 seeds=42 fields=gen listtypes=1
# Notes:
#   - Requires: lab3, geninput (or geninput.c), and optionally run.sh/check.pl
#   - CSV output: perf.csv (append mode; header written if file absent)
set -euo pipefail

# -------- helpers --------
exists() { command -v "$1" >/dev/null 2>&1; }
have()   { [ -f "$1" ]; }

msg() { printf "[mytests] %s\n" "$*" >&2; }

build_binaries() {
  if ! have ./lab3; then
    if have Makefile; then
      msg "building lab3 via Makefile…"
      make lab3
    else
      msg "ERROR: no lab3 and no Makefile — cannot build"; exit 2
    fi
  fi
  if ! have ./geninput; then
    if have ./geninput.c; then
      msg "building geninput from geninput.c…"
      gcc -Wall -g geninput.c -o geninput
    else
      msg "ERROR: geninput(c) missing"; exit 2
    fi
  fi
}

# CSV header
ensure_csv_header() {
  local csv="$1"
  if [ ! -s "$csv" ]; then
    echo "size,ms,sort_type,field,listtype,seed" > "$csv"
  fi
}

# parse key=val args like sizes=20,1000 sorts=1,2 fields=gen,ip
parse_kv() {
  local key="$1" val="$2"
  case "$key" in
    sizes)      IFS=',' read -r -a SIZES <<<"$val" ;;
    sorts)      IFS=',' read -r -a SORTS <<<"$val" ;;
    fields)     IFS=',' read -r -a FIELDS <<<"$val" ;;
    listtypes)  IFS=',' read -r -a LISTTYPES <<<"$val" ;;
    seeds)      IFS=',' read -r -a SEEDS <<<"$val" ;;
    *) msg "WARN: unknown option '$key', ignoring" ;;
  esac
}

# -------- defaults (sane sweep) --------
SIZES=(20 1000 10000 30000 50000)
SORTS=(1 2 3 4 5)        # 1:Insertion 2:RecSel 3:IterSel 4:Merge 5:qsort
FIELDS=(gen ip)          # compare field
LISTTYPES=(1)            # 1:random  2:ascending  3:descending
SEEDS=(111 222 333)

# ingest CLI key=val overrides
MODE="both"
for arg in "$@"; do
  case "$arg" in
    harness) MODE="harness" ;;
    perf)    MODE="perf" ;;
    *=*)     parse_kv "${arg%%=*}" "${arg#*=}" ;;
    *)       msg "WARN: ignoring arg '$arg'" ;;
  esac
done

# -------- harness run (instructor scripts) --------
run_harness() {
  if have ./run.sh; then
    chmod +x ./run.sh || true
    msg "running instructor run.sh…"
    ./run.sh
  else
    msg "run.sh not found — skipping harness generation step"
  fi

  if have ./check.pl; then
    chmod +x ./check.pl || true
    msg "running instructor check.pl…"
    ./check.pl | tee check.log
  else
    msg "check.pl not found — skipping harness comparison"
  fi
}

# -------- perf sweep (CSV) --------
run_perf() {
  build_binaries
  local csv="perf.csv"
  ensure_csv_header "$csv"
  local outdir="student_out"
  mkdir -p "$outdir"

  # geninput usage per spec: ./geninput listsize listtype sorttype field [seed]
  # listtype: 1=random 2=ascending 3=descending; field: gen|ip
  for n in "${SIZES[@]}"; do
    for lt in "${LISTTYPES[@]}"; do
      for s in "${SORTS[@]}"; do
        for f in "${FIELDS[@]}"; do
          for seed in "${SEEDS[@]}"; do
            local_out="${outdir}/n${n}_lt${lt}_s${s}_${f}_seed${seed}.out"
            msg "N=${n} lt=${lt} sort=${s} field=${f} seed=${seed}"
            # run and capture timing line(s)
            ./geninput "$n" "$lt" "$s" "$f" "$seed" | ./lab3 > "$local_out"

            # extract timing lines: format "N<TAB>ms<TAB>sortType"
            # keep the last one if multiple
            line="$(awk -F'\t' '/^[0-9]+\t[0-9.]+\t[1-5]$/ {rec=$0} END{print rec}' "$local_out")"
            if [ -z "$line" ]; then
              msg "ERROR: did not find timing line in $local_out"
              tail -n +1 "$local_out" | sed 's/^/[out] /' >&2
              exit 3
            fi
            # append to CSV with extra metadata
            IFS=$'\t' read -r sz ms st <<<"$line"
            echo "${sz},${ms},${st},${f},${lt},${seed}" >> "$csv"
          done
        done
      done
    done
  done

  msg "perf sweep complete → $(pwd)/${csv}"
}

# -------- main --------
case "$MODE" in
  harness) run_harness ;;
  perf)    run_perf ;;
  both)    run_harness; echo; run_perf ;;
esac
