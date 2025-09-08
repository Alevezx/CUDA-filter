import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv("results.csv")

# Plot runtime vs image size
for k in sorted(df["Kernel"].unique()):
    sub = df[df["Kernel"] == k]
    plt.figure()
    for impl in ["0.CPU", "1.PTHREAD", "2.GPU"]:
        plt.plot(sub[sub["Impl"] == impl]["ImageSize"],
                 sub[sub["Impl"] == impl]["TimeExec_ms"],
                 marker="o", label=impl)
    plt.title(f"Runtime vs Image Size (k={k})")
    plt.xlabel("Image Size (N x N)")
    plt.ylabel("Execution Time (us)")
    plt.legend()
    plt.grid(True)
    plt.xscale("log")
    plt.yscale("log")
    plt.savefig(f"runtime_k{k}.png")

# Plot speedup vs image size
cpu_times = df[df["Impl"] == "0.CPU"][["ImageSize","Kernel","TimeExec_ms"]]
for impl in ["1.PTHREAD","2.GPU"]:
    plt.figure()
    merged = df[df["Impl"] == impl].merge(cpu_times, on=["ImageSize","Kernel"],
                                          suffixes=("", "_cpu"))
    for k in sorted(merged["Kernel"].unique()):
        sub = merged[merged["Kernel"] == k]
        speedup = sub["TimeExec_ms_cpu"] / sub["TimeExec_ms"]
        plt.plot(sub["ImageSize"], speedup, marker="o", label=f"k={k}")
    plt.title(f"Speedup {impl} vs CPU")
    plt.xlabel("Image Size (N x N)")
    plt.ylabel("Speedup (×)")
    plt.legend()
    plt.grid(True)
    plt.xscale("log")
    plt.savefig(f"speedup_{impl}.png")
