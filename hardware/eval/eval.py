import numpy as np
import cv2
import os
import time
import matlab.engine
import argparse
import glob
import csv

eng = matlab.engine.start_matlab()

result_List = {}
# for i, x in enumerate(var):
#     main(x)


parser = argparse.ArgumentParser()
parser.add_argument("--folder", default='result_hash') 
parser.add_argument("--gt", default='shapesGT.mat') 
args = parser.parse_args()
folder = os.path.join(args.folder, "*")
print(folder)
gt_dir = args.gt
for f in glob.glob(folder):
    if "tkBoxes" in f:
        tkid_dir = f.replace("tkBoxes", "tkIDs")
        HOTA, DETA, ASSA, hacc, dacc, aacc = eng.evalhota(f, tkid_dir, gt_dir, nargout=6)
        result_List[f] = HOTA
        print(f, HOTA)

print(result_List)

output_dir = os.path.join(args.folder, "result.csv")
print(output_dir)
with open(output_dir, 'w') as csv_file:  
    writer = csv.writer(csv_file)
    for key, value in result_List.items():
       writer.writerow([key, value])