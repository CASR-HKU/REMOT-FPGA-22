import numpy as np
import cv2
import os
import time
import matlab.engine
from hash_au import set_associate_hash_table
import argparse
import glob
import csv
eng = matlab.engine.start_matlab()

result_List = {}
# for i, x in enumerate(var):
#     main(x)
    
for f in glob.glob("hash_result/*"):
    if "tkBoxes" in f:
        tkid_dir = f.replace("tkBoxes", "tkIDs")
        HOTA, DETA, ASSA = eng.evalhota(f, tkid_dir, nargout=3)
        result_List[f] = HOTA

print(result_List)

with open('result512.csv', 'w') as csv_file:  
    writer = csv.writer(csv_file)
    for key, value in result_List.items():
       writer.writerow([key, value])

# print('{}/{}: var={}, HOTA={:.4f}, DETA={:.4f}, ASSA={:.4f}, {}'.format(0, len(var) - 1, x, HOTA[0], DETA[0], ASSA[0], time.strftime('%H:%M:%S')))

    
# io.savemat('results_auNum.mat', {'auNum': var, 'HOTA': HOTA, 'DETA': DETA, 'ASSA': ASSA})