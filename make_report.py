#!/usr/bin/env python3
"""
make_report.py — Build MP3 timing report PDF from perf.csv
Usage:
  python3 make_report.py perf.csv [--out mp3_test_log.pdf]
Notes:
  - Expects perf.csv with columns: size,ms,sort_type,field,listtype,seed
  - Produces a multi-page PDF with summary + plots
"""
import sys, argparse, math
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

ALGONAME = {
    1: "Insertion",
    2: "Selection (Recursive)",
    3: "Selection (Iterative)",
    4: "Merge",
    5: "Quick (qsort_r)",
}

def load_perf(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    # Normalize / enforce dtypes
    df = df.rename(columns={c: c.strip() for c in df.columns})
    df["size"] = pd.to_numeric(df["size"], errors="coerce")
    df["ms"] = pd.to_numeric(df["ms"], errors="coerce")
    df["sort_type"] = pd.to_numeric(df["sort_type"], errors="coerce")
    df["field"] = df["field"].astype(str)
    df["listtype"] = pd.to_numeric(df["listtype"], errors="coerce")
    df["seed"] = pd.to_numeric(df["seed"], errors="coerce")
    df = df.dropna(subset=["size", "ms", "sort_type"])
    df["sort_name"] = df["sort_type"].map(ALGONAME).fillna(df["sort_type"].astype(str))
    return df

def page_title(pdf: PdfPages, df: pd.DataFrame, out_name: str):
    fig = plt.figure(figsize=(8.5, 11))
    fig.suptitle("MP3 Timing Report", fontsize=20, y=0.96)
    txt = []
    txt.append(f"Input file: {out_name}")
    txt.append(f"Total runs: {len(df)}")
    txt.append(f"Sizes: {', '.join(map(str, sorted(df['size'].unique())))}")
    txt.append(f"Sort types: {', '.join(sorted(df['sort_name'].unique()))}")
    txt.append(f"Fields: {', '.join(sorted(df['field'].unique()))}")
    txt.append(f"List types: {', '.join(map(str, sorted(df['listtype'].unique())))}")
    txt.append(f"Seeds: {', '.join(map(lambda x: str(int(x)), sorted(df['seed'].dropna().unique())))}")
    txt.append("")
    txt.append("Each timing line from lab3 has format: <N>\\t<milliseconds>\\t<sort_type>.")
    txt.append("This report aggregates by median over seeds for clarity.")
    ax = fig.add_axes([0.08, 0.15, 0.84, 0.75]); ax.axis('off')
    ax.text(0, 1, "\n".join(txt), va="top", fontsize=12)
    pdf.savefig(fig); plt.close(fig)

def plot_field_lines(pdf: PdfPages, df: pd.DataFrame, field: str):
    # median over seeds for each (size, sort_type)
    g = (df[df["field"] == field]
         .groupby(["size", "sort_type", "sort_name"], as_index=False)["ms"].median())
    # line plot per algorithm
    fig = plt.figure(figsize=(11, 8.5))
    ax = fig.add_subplot(111)
    for key, sub in g.groupby(["sort_type", "sort_name"]):
        sub = sub.sort_values("size")
        ax.plot(sub["size"], sub["ms"], marker='o', label=f"{key[1]} (#{int(key[0])})")
    ax.set_title(f"Runtime vs Size — field = {field} (median over seeds)")
    ax.set_xlabel("N (records)")
    ax.set_ylabel("Time (ms)")
    ax.grid(True, which="both", linestyle=":")
    ax.legend(loc="best", ncol=2, fontsize=9)
    pdf.savefig(fig); plt.close(fig)

def plot_loglog(pdf: PdfPages, df: pd.DataFrame, field: str):
    g = (df[df["field"] == field]
         .groupby(["size", "sort_type", "sort_name"], as_index=False)["ms"].median())
    fig = plt.figure(figsize=(11, 8.5))
    ax = fig.add_subplot(111)
    for key, sub in g.groupby(["sort_type", "sort_name"]):
        sub = sub.sort_values("size")
        ax.plot(sub["size"], sub["ms"], marker='o', label=f"{key[1]} (#{int(key[0])})")
    ax.set_xscale("log"); ax.set_yscale("log")
    ax.set_title(f"Log-Log Runtime — field = {field} (median over seeds)")
    ax.set_xlabel("N (log scale)")
    ax.set_ylabel("Time (ms, log scale)")
    ax.grid(True, which="both", linestyle=":")
    ax.legend(loc="best", ncol=2, fontsize=9)
    pdf.savefig(fig); plt.close(fig)

def table_largest_n(pdf: PdfPages, df: pd.DataFrame, field: str):
    # pick the largest N present for this field
    field_df = df[df["field"] == field]
    if field_df.empty:
        return
    Nmax = int(field_df["size"].max())
    # aggregate median across seeds at Nmax
    agg = (field_df[field_df["size"] == Nmax]
           .groupby(["sort_type", "sort_name"], as_index=False)["ms"].median()
           .sort_values("ms"))
    fig = plt.figure(figsize=(8.5, 11))
    ax = fig.add_axes([0.08, 0.1, 0.84, 0.8]); ax.axis('off')
    lines = [f"Field = {field} — Largest N = {Nmax}",
             "", "Median times at Nmax (smaller is better):", ""]
    for _, row in agg.iterrows():
        lines.append(f"  {row['sort_name']} (#{int(row['sort_type'])}): {row['ms']:.3f} ms")
    # compute speedup vs fastest
    if len(agg) > 0:
        fastest = agg.iloc[0]["ms"]
        lines.append("")
        lines.append("Speedup vs fastest at Nmax:")
        for _, row in agg.iterrows():
            sp = (row["ms"] / fastest) if fastest > 0 else float('inf')
            lines.append(f"  {row['sort_name']} (#{int(row['sort_type'])}): {sp:.2f}×")
    ax.text(0, 1, "\n".join(lines), va="top", fontsize=12)
    pdf.savefig(fig); plt.close(fig)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", type=str, help="perf.csv path")
    ap.add_argument("--out", type=str, default="mp3_test_log.pdf", help="output PDF")
    args = ap.parse_args()

    csv_path = Path(args.csv)
    if not csv_path.exists():
        print(f"ERROR: {csv_path} not found", file=sys.stderr)
        sys.exit(2)

    df = load_perf(csv_path)
    out_pdf = Path(args.out)

    with PdfPages(out_pdf) as pdf:
        page_title(pdf, df, csv_path.name)
        # per-field line plots
        for field in sorted(df["field"].unique()):
            plot_field_lines(pdf, df, field)
        # per-field log-log plots
        for field in sorted(df["field"].unique()):
            plot_loglog(pdf, df, field)
        # per-field table at largest N
        for field in sorted(df["field"].unique()):
            table_largest_n(pdf, df, field)

    print(f"Wrote {out_pdf.resolve()}")

if __name__ == "__main__":
    main()
