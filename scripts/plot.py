import pandas
from matplotlib import pyplot as plt
import numpy as np
import os
import sys

if os.system(f"{sys.argv[1]} --benchmark_format=csv > output.csv") != 0:
    sys.stderr.write("Error Occured")
    sys.exit(-1)

df = pandas.read_csv("output.csv")

plt.style.use("seaborn")

plt.ylabel("nanoseconds")

num_comps = len(df["real_time"])
x = np.arange(num_comps)
plt.bar(x, df["real_time"].values.flatten())
plt.xticks(x, df["name"], rotation=15)
plt.show()