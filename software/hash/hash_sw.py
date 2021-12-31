from scipy import io
from scipy.spatial.distance import directed_hausdorff, pdist
from scipy.spatial.distance import squareform
from itertools import combinations
from sklearn.cluster import DBSCAN
import numpy as np
from threading import Thread, Event
from queue import Queue
import cv2
import os
import time
from hash_au import set_associate_hash_table
import argparse
import math
from tqdm import tqdm 
import matlab.engine
from au_functions import *
import csv


class AU_Controller():
    def __init__(self, args):
        self.input = args.input
        self.init_data(self.input)
        self.cmap = io.loadmat('cmap.mat')['cmap'] * 255

   
        self.pFrame = -1
        self.nFrame = self.events[:, 4][-1] + 1
        self.frame0 = np.zeros((self.ly, self.lx, 3), 'uint8')
        self.iFrame = self.frame0.copy()
        self.frames = np.tile(self.frame0, (self.nFrame, 1, 1, 1))

  
        self.tkBoxes = []
        self.tkIDs = []   

   
        self.tFrame = self.tFrame
        self.qSize = 100 
        self.auSize = 200 
        self.auNum = 30 
        

        self.dAdd = 4 
        self.tDel = 2 * self.tFrame
        self.areaDel = 64 
        self.tLive = 3 * self.tFrame
        self.areaLive = 256 
        self.numLive = 15 

        self.tPause = 0.5 * self.tFrame
        self.t = self.tPause

    
        self.epsDiv = 25 
        self.minptsDiv = 1


        self.iomMer = 0.2
        self.dsMer = 5 
        self.minptsMer = 1
        self.epsMer = 1.

        self.AUs = []
        self.globalID = -1

        self.total_size = int(2**12)
        self.ways = args.ways
        self.lines = args.lines
        self.folder = args.folder
        self.name = args.name
        
    def init_data(self, file):
        self.tFrame = 44065
        self.events = io.loadmat(file)['m']
        self.lx, self.ly = self.events[:, :2].max(0) + 1



    def Shrink(self):
        for au in self.AUs:
            idxFade = np.argwhere(au.auEvents[:, 2] < au.auEvents[-1, 2] - self.tFrame).flatten()

            if idxFade.size == 0:
                continue

            for ievt in idxFade:
                au.shrink(au.auEvents[ievt, 0], au.auEvents[ievt, 1])

            au.auEvents = np.delete(au.auEvents, idxFade, axis=0)

        for au in self.AUs:
            au.auBox = bbox(au.auEvents[:, 0], au.auEvents[:, 1])


    def Split(self):
        idxDel = []

        for j, au in enumerate(self.AUs):
            idxGroup = DBSCAN(eps=self.epsDiv, min_samples=self.minptsDiv).fit_predict(au.auEvents[:, :2])
            idxGroup[idxGroup < 0] = 0
            idxGroup = idxGroup[: au.auSize]

            if max(idxGroup) <= 0:
                continue
            else:
                idxDel.append(j)

            pTrack = np.argmax([sum(idxGroup == idx) for idx in np.unique(idxGroup)])


            for k in range(max(idxGroup) + 1):
                idxEvents = np.argwhere(idxGroup == k).flatten()


                newau = AU(au.auEvents[idxEvents], self.lx, self.ly, self.dAdd, self.auSize, self.lines, self.ways)
                newau.auBox = bbox(newau.auEvents[:, 0], newau.auEvents[:, 1])
                

                if k == pTrack:
                    newau.auNumber = au.auNumber
                else:
                    if au.auNumber[0] and bbArea(newau.auBox) > self.areaLive and newau.auEvents.shape[0] > self.numLive:
                        self.globalID += 1
                        id = self.globalID
                    else:
                        id = 0
                    newau.auNumber = [id, min(newau.auEvents[:, 2])]


                self.AUs.append(newau)



        if len(idxDel) > 0:
            for idx in sorted(idxDel, reverse=True):
                self.AUs[idx].delete()
                self.AUs.pop(idx)


    def Merge(self):

        idxDel = []
        
        if len(self.AUs) < 2:
            return 

        auPoints = [bbox2points(au.auBox) for au in self.AUs]
        idxnk = list(combinations(range(len(self.AUs)), 2))
        logicalDist = [bboxOverlapRatio(self.AUs[idx[0]].auBox, self.AUs[idx[1]].auBox) < self.iomMer and max(directed_hausdorff(auPoints[idx[0]], auPoints[idx[1]])[0], directed_hausdorff(auPoints[idx[1]], auPoints[idx[0]])[0]) > self.dsMer for idx in idxnk]
        idxGroup = DBSCAN(eps=self.epsMer, min_samples=self.minptsMer).fit_predict(squareform(logicalDist))


        for j in range(max(idxGroup) + 1):
            idxAU = np.argwhere(idxGroup == j).flatten()
            if idxAU.size < 2:
                continue
            idxDel.extend(idxAU)


            events = np.concatenate([self.AUs[idx].auEvents for idx in idxAU], axis=0)
            events = events[np.argsort(events[:, 2])]
            

            newau = AU(events, self.lx, self.ly, self.dAdd, self.auSize, self.lines, self.ways)
            newau.auBox = bbox(newau.auEvents[:, 0], newau.auEvents[:, 1])
            

            if any([self.AUs[idx].auNumber[0] > 0 for idx in idxAU]):
                idxAU = idxAU[[self.AUs[idx].auNumber[0] > 0 for idx in idxAU]]
            idxNum = idxAU[np.argmin([self.AUs[idx].auNumber[1] for idx in idxAU])]
            newau.auNumber = self.AUs[idxNum].auNumber
            
            self.AUs.append(newau)


        if len(idxDel) > 0:
            for idx in sorted(idxDel, reverse=True):
                self.AUs[idx].delete()
                self.AUs.pop(idx)

    def Kill(self, ts):
        idxDel = np.argwhere([ts - au.auEvents[-1, 2] > self.tDel or bbArea(au.auBox) < self.areaDel for au in self.AUs]).flatten()


        if idxDel.size > 0:
            for idx in sorted(idxDel, reverse=True):
                self.AUs[idx].delete()
                self.AUs.pop(idx)


        for au in self.AUs:
            if not au.auNumber[0] and ts - au.auNumber[1] > self.tLive and bbArea(au.auBox) > self.areaLive and au.auEvents.shape[0] > self.numLive:
                self.globalID += 1
                au.auNumber[0] = self.globalID


    def animation(self, x, y, ts, f):

        if f > self.pFrame and self.pFrame >= 0:
            if len(self.AUs) >= 1:
                idxC = np.array([au.auNumber[0] % 7 for au in self.AUs], 'int32')
                auColors = self.cmap[idxC]
                idxW = np.array([au.auNumber[0] == 0 for au in self.AUs])
                auColors[idxW] = auColors[idxW] * 0 + 1

                idxVis = []
                boxes = []
                IDs = []

                for j, au in enumerate(self.AUs):
                    if au.auNumber[0] > 0:
                        sauEvents = au.auEvents
                        idxEvt = sauEvents[:, 2] >= ts - self.tFrame
                        if any(idxEvt):
                            idxVis.append(j)
                            boxes.append(bbox(sauEvents[idxEvt, 0], sauEvents[idxEvt, 1]))
                            IDs.append(au.auNumber[0])
                
                self.tkBoxes.append(boxes)
                self.tkIDs.append(IDs)

                if len(idxVis) > 0:
                    for j, k in enumerate(idxVis):
                        self.iFrame = cv2.rectangle(self.iFrame, (boxes[j][0], boxes[j][1]), (boxes[j][2], boxes[j][3]), auColors[k].tolist(), 1)

                if len(idxVis) > 0:
                    for j, k in enumerate(idxVis):
                        self.iFrame = cv2.putText(self.iFrame, '{}'.format(IDs[j]), (boxes[j][0], boxes[j][1]), cv2.FONT_HERSHEY_PLAIN, 1., auColors[k].tolist(), 1)
                
            self.iFrame = cv2.putText(self.iFrame, 'Frame:{}/{}, MaxID:{}'.format(self.pFrame, self.nFrame - 1, self.globalID), (0, 15), cv2.FONT_HERSHEY_PLAIN, 1., [255, 255, 255], 1)

            self.frames[self.pFrame] = self.iFrame

        if f > self.pFrame:
            self.pFrame = f
            self.iFrame = self.frame0.copy()


        self.iFrame[y, x] = [255, 255, 255]

    def run(self):
        for i in tqdm(range(self.events.shape[0])):
            x  = self.events[i, 0]
            y  = self.events[i, 1]
            ts = self.events[i, 3]
            f  = self.events[i, 4]

 
            if ts > self.t and len(self.AUs) >= 1:
                self.t = self.t + self.tPause
                


                for idx in reversed(range(len(self.AUs))):
                    if not self.AUs[idx].is_alive():
                        self.AUs.pop(idx)

                self.Shrink()
                self.Split()
                self.Merge()
                self.Kill(ts)
    
            self.animation(x, y, ts, f)
            
            intAU = np.argwhere([au.is_alive() and au.interested(x, y) for au in self.AUs]).flatten()
            if intAU.size == 0:
                idxAU = -1
            elif intAU.size == 1:
                idxAU = intAU[0]
            else:
                idxAU = intAU[np.argmin([pbdist([x, y], self.AUs[j].auBox) for j in intAU])]


            if idxAU >= 0:
                self.AUs[idxAU].run(x, y, ts)


            if intAU.size == 0 and len(self.AUs) < self.auNum:
                newau = AU(np.array([[x, y, ts]]), self.lx, self.ly, self.dAdd, self.auSize, self.lines, self.ways)
                newau.auBox = [x, y, x + 1, y + 1]
                newau.auNumber = [0, ts]
                self.AUs.append(newau)


    def save_result(self):
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        videoWriter = cv2.VideoWriter('output.avi', fourcc, 5, (self.lx, self.ly), True)
        for i in range(self.pFrame):
            videoWriter.write(self.frames[i])
        videoWriter.release()   
    
        self.tkBoxes = np.array(self.tkBoxes, dtype='object')
        self.tkIDs = np.array(self.tkIDs, dtype='object')
        for boxes in self.tkBoxes:
            if len(boxes) == 0:
                continue
            for box in boxes:
                box[2] = box[2] - box[0]
                box[3] = box[3] - box[1]

        folder = self.folder

        if not os.path.exists(folder):
            os.mkdir(folder)       

        tkbox_name = self.name + 'tkBoxes.mat'
        tkIDs_name = self.name + 'tkIDs.mat'
        print(os.path.join(folder, tkbox_name))
        tkbox_dir = os.path.join(folder, tkbox_name)
        tkid_dir = os.path.join(folder, tkIDs_name)
        gt_dir = "gt.mat"
        io.savemat(tkbox_dir, {'tkBoxes': self.tkBoxes})
        io.savemat(tkid_dir, {'tkIDs': self.tkIDs})
        
        eng = matlab.engine.start_matlab()
#         HOTA, DETA, ASSA = eng.evalhota(tkbox_dir, tkid_dir, nargout=3)
        HOTA, DETA, ASSA, hacc, dacc, aacc = eng.evalhota(tkbox_dir, tkid_dir, gt_dir, nargout=6)
        print('Name={}, HOTA={:4f}, DETA={:4f}, ASSA={:4f}\n'.format(self.name, HOTA, DETA, ASSA))
        output_dir = os.path.join(self.folder, "result.csv")
        with open(output_dir, 'a') as csv_file:  
            writer = csv.writer(csv_file)
            writer.writerow([tkbox_dir, HOTA])




class AU():
    def __init__(self, events, lx, ly, dAdd, auSize, lines, ways):
        super().__init__()

        self.switch = True
        self.lx = lx
        self.ly = ly
        self.dAdd = dAdd
        self.auSize = auSize

        self.auEvents = events
        #important change on hash
        self.auMap = set_associate_hash_table(lines, ways, ly, lx)
        self.auBox = [0, 0, 0, 0]
        self.auNumber = [0, 0]
        self.alive_flag = True
        for i in range(events.shape[0]):
            self.expand(events[i, 0], events[i, 1])

    def is_alive(self):
        return self.alive_flag

    def interested(self, x, y):
        return self.auMap.interested(x, y)

    def expand(self, x, y):
        x = int(x)
        y = int(y)
        idxx = np.arange(max(x - self.dAdd, 0), min(x + self.dAdd + 1, self.lx))
        idxy = np.arange(max(y - self.dAdd, 0), min(y + self.dAdd + 1, self.ly))
        idxxx, idxyy = np.meshgrid(idxx, idxy)

        #import change on hash
        for i in range(idxxx.shape[0]):
            for j in range(idxxx.shape[1]):
                self.auMap.add(idxxx[i, j], idxyy[i, j])

    def shrink(self, x, y):
        x = int(x)
        y = int(y)
        idxx = np.arange(max(x - self.dAdd, 0), min(x + self.dAdd + 1, self.lx))
        idxy = np.arange(max(y - self.dAdd, 0), min(y + self.dAdd + 1, self.ly))
        idxxx, idxyy = np.meshgrid(idxx, idxy)

        #import change on hash
        for i in range(idxxx.shape[0]):
            for j in range(idxxx.shape[1]):  
                self.auMap.sub(idxxx[i, j], idxyy[i, j])

    def delete(self):
        self.alive_flag = False

    def run(self, x, y, ts):
        self.expand(x, y)
        self.auEvents = np.insert(self.auEvents, self.auEvents.shape[0], [x, y, ts], axis=0)

        if self.auEvents.shape[0] > self.auSize:
            idxFade = range(self.auEvents.shape[0] - self.auSize)
            for i in idxFade:
                self.shrink(self.auEvents[i, 0], self.auEvents[i, 1])
            self.auEvents = np.delete(self.auEvents, idxFade, axis=0)


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("--folder", default='hash_result')
    parser.add_argument("--input", default="shapes_6dof.mat")
    parser.add_argument("--name", default='shapes_6dof')
    parser.add_argument("--lines", type=int, default="4096")
    parser.add_argument("--ways", type=int, default="2")

    args = parser.parse_args()
    print(args)
    controller = AU_Controller(args)
    controller.run()

    controller.save_result()


if __name__ == "__main__":
    main()
    
        




