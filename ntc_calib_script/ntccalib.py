"""
MIT License

Copyright (c) 2020 Fabian Friederichs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import argparse
import csv
import math

def steinhart_hart_temp(a0, a1, a2, R):
    return (1.0 / (a0 + a1 * math.log(R) + a2 * math.pow(math.log(R), 3))) - 273.15

parser = argparse.ArgumentParser(description="Calculate Steinhart-Hart coefficients for a given set of temperature/resistance pairs.")
parser.add_argument('-f', '--file', type=argparse.FileType(), help="CSV file containing temperature/resistance pairs.", required=True)
args = parser.parse_args()

csv_reader = csv.reader(args.file, delimiter=',')
temp_res_pairs = []
for row in csv_reader:
    temp_res_pairs.append((float(row[0]), float(row[1])))

A = np.zeros((len(temp_res_pairs), 3))
b = np.zeros((len(temp_res_pairs),))

T = np.zeros((len(temp_res_pairs),))
R = np.zeros((len(temp_res_pairs),))

for i in range(0, len(temp_res_pairs)):
    A[i, 0] = 1.0
    A[i, 1] = math.log(temp_res_pairs[i][1])
    A[i, 2] = math.log(temp_res_pairs[i][1]) * math.log(temp_res_pairs[i][1]) * math.log(temp_res_pairs[i][1])
    b[i] = 1.0 / (temp_res_pairs[i][0] + 273.15)
    T[i] = temp_res_pairs[i][0]
    R[i] = temp_res_pairs[i][1]

print("--- Matrix A:")
print(A)
print("--- Vector b:")
print(b)

result = np.linalg.lstsq(A, b)[0]
print("--- Result:")
print("a0 = " + str(result[0]))
print("a1 = " + str(result[1]))
print("a2 = " + str(result[2]))

print()

# plot resulting function
logR = np.log(np.array(np.linspace(np.min(R), np.max(R), 500)))
temp = (1.0 / (np.full((500,), result[0]) + result[1] * logR + result[2] * (logR * logR * logR))) - 273.15

plt.figure()
plt.scatter(R, T)
plt.plot(np.array(np.linspace(np.min(R), np.max(R), 500)), temp)
plt.show()

