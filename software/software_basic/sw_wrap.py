from scipy import io
from scipy.spatial.distance import squareform, directed_hausdorff
from itertools import combinations
from sklearn.cluster import DBSCAN, AgglomerativeClustering
import numpy as np

import cv2
from tqdm import tqdm
from matplotlib import pyplot as plt
import matlab.engine
import argparse
import os
import csv
import yaml
from au_functions import *

__all__ = ['Controller', 'AU']


class Controller():
    def __init__(self, args):
        self.input = args.input
        self.Init(self.input)
        self.tkBoxes = []
        self.tkIDs = []

        self.auFifo = args.auFifo
        self.auNum = args.auNum
        self.dAdd = args.dAdd
        self.folder = args.outfolder
        self.name = args.name

        self.tDel = args.tDel * self.tFrame
        self.areaDel = args.areaDel
        self.tLive = args.tLive * self.tFrame
        self.areaLive = args.areaLive
        self.numLive = args.numLive

        self.t = self.tFrame

        self.split = args.split
        self.epsDiv = args.epsDiv
        self.minptsDiv = 1

        self.merge = args.merge
        self.iomMer = args.iomMer
        self.dsMer = args.dsMer
        self.minptsMer = 1
        self.epsMer = 1

        if "inbound" in self.input:
            self.bdspawn1 = 40
            self.bdspawn2 = 40
        else:
            self.bdspawn1 = -1
            self.bdspawn2 = -1

        if "outbound" in self.input:
            self.bdkill = 20
        else:
            self.bdkill = -1

        self.globalID = -1
        self.AUs = []

        self.eng = matlab.engine.start_matlab()

    def Init(self, file):
        if "bound" in self.input:
            self.tFrame = 40000
        else:
            self.tFrame = 44065

        self.cmap = io.loadmat('cmap.mat')['cmap'] * 255
        self.events = io.loadmat(file)['events']
        self.lx, self.ly = self.events[:, :2].max(0) + 1
        self.pFrame = -1
        self.nFrame = self.events[:, 4][-1] + 1
        self.frame0 = np.zeros((self.ly, self.lx, 3), 'uint8') + 255
        self.iFrame = self.frame0.copy()
        self.frames = np.tile(self.frame0, (self.nFrame, 1, 1, 1))

    def Update(self):
        for au in self.AUs:
            idxFade = np.argwhere(
                au.auEvents[:, 2] < au.auEvents[-1, 2] - self.tFrame).flatten()

            if idxFade.size == 0:
                continue

            for ievt in idxFade:
                au.shrink(au.auEvents[ievt, 0], au.auEvents[ievt, 1])

            au.auEvents = np.delete(au.auEvents, idxFade, axis=0)

        for au in self.AUs:
            au.auBox = bbox(au.auEvents[:, 0], au.auEvents[:, 1])

    def Split(self):
        idxDel = []

        if len(self.AUs) >= self.auNum:
            return
        
        for j, au in enumerate(self.AUs):
            if self.split == 'DBSCAN':
                idxGroup = DBSCAN(eps=self.epsDiv, min_samples=self.minptsDiv).fit_predict(
                    au.auEvents[:, :2])
                idxGroup[idxGroup < 0] = 0
            elif self.split == 'HAC':
                if au.auEvents.shape[0] <= 1:
                    continue
                clustering = AgglomerativeClustering(
                    linkage='average', affinity='euclidean').fit(au.auEvents[:, :2])
                idxGroup = clustering.labels_
                idxGroup[idxGroup < 0] = 0
                if max(directed_hausdorff(au.auEvents[idxGroup == 0, :2], au.auEvents[idxGroup == 1, :2])[0],
                       directed_hausdorff(au.auEvents[idxGroup == 1, :2], au.auEvents[idxGroup == 0, :2])[0]) < self.dsMer:
                    continue
            else:
                raise Exception('Split algorithm error!')
                
            if max(idxGroup) <= 0:
                continue
            else:
                idxDel.append(j)

            idxTk = np.argmax([sum(idxGroup == idx)
                              for idx in np.unique(idxGroup)])

            for k in range(max(idxGroup) + 1):
                idxEvents = np.argwhere(idxGroup == k).flatten()

                newau = AU(au.auEvents[idxEvents], self.lx,
                           self.ly, self.dAdd, self.auFifo)
                newau.auBox = bbox(newau.auEvents[:, 0], newau.auEvents[:, 1])

                if k == idxTk:
                    newau.auNumber = au.auNumber
                else:
                    newau.auNumber = [0, min(newau.auEvents[:, 2])]

                self.AUs.append(newau)

        if len(idxDel) > 0:
            for idx in sorted(idxDel, reverse=True):
                self.AUs.pop(idx)

    def Merge(self):
        idxDel = []

        if len(self.AUs) < 2:
            return

        idxnk = list(combinations(range(len(self.AUs)), 2))
        if self.merge == 'IOM':
            idxGroup = clusterAu(np.array([self.AUs[idx].auBox for idx in range(len(self.AUs))]), self.iomMer)
        elif self.merge == 'DIST':
            auPoints = [bbox2points(au.auBox) for au in self.AUs]
            dist = [max(directed_hausdorff(auPoints[idx[0]], auPoints[idx[1]])[0],
                        directed_hausdorff(auPoints[idx[1]], auPoints[idx[0]])[0]) > self.dsMer for idx in idxnk]
            idxGroup = DBSCAN(eps=self.epsMer, min_samples=self.minptsMer).fit_predict(
                squareform(dist))
        else:
            raise Exception('Merge algorithm error!')

        for j in range(max(idxGroup) + 1):
            idxAU = np.argwhere(idxGroup == j).flatten()
            if idxAU.size < 2:
                continue
            idxDel.extend(idxAU)

            events = np.concatenate(
                [self.AUs[idx].auEvents for idx in idxAU], axis=0)
            events = events[np.argsort(events[:, 2])]

            newau = AU(events, self.lx, self.ly, self.dAdd, self.auFifo)
            newau.auBox = bbox(newau.auEvents[:, 0], newau.auEvents[:, 1])

            if any([self.AUs[idx].auNumber[0] > 0 for idx in idxAU]):
                idxAU = idxAU[[self.AUs[idx].auNumber[0] > 0 for idx in idxAU]]
            idxNum = idxAU[np.argmin(
                [self.AUs[idx].auNumber[1] for idx in idxAU])]
            newau.auNumber = self.AUs[idxNum].auNumber

            self.AUs.append(newau)

        if len(idxDel) > 0:
            for idx in sorted(idxDel, reverse=True):
                self.AUs.pop(idx)

    def Kill(self, ts):
        idxDel = np.argwhere([ts - au.auEvents[-1, 2] > self.tDel or bbArea(
            au.auBox) < self.areaDel or (au.auBox[1] + au.auBox[3]) / 2 < self.bdkill for au in self.AUs]).flatten()

        if idxDel.size > 0:
            for idx in sorted(idxDel, reverse=True):
                self.AUs.pop(idx)

        for au in self.AUs:
            if not au.auNumber[0] and ts - au.auNumber[1] > self.tLive and bbArea(au.auBox) > self.areaLive and au.auEvents.shape[0] > self.numLive and au.auBox[2] / 2 > self.bdspawn1 and au.auBox[3] / 2 > self.bdspawn2:
                self.globalID += 1
                au.auNumber[0] = self.globalID

    def Animation(self, x, y, p, ts, f):
        if f > self.pFrame and self.pFrame >= 0:
            if len(self.AUs) >= 1:
                idxC = np.array([au.auNumber[0] %
                                7 for au in self.AUs], 'int32')
                auColors = self.cmap[idxC]
                idxW = np.array([au.auNumber[0] == 0 for au in self.AUs])
                auColors[idxW] = auColors[idxW] * 0 + 1

                idxVis = []
                boxes = []
                IDs = []

                for j, au in enumerate(self.AUs):
                    if au.auNumber[0] > 0:
                        idxEvt = au.auEvents[:, 2] >= ts - self.tFrame
                        if any(idxEvt):
                            idxVis.append(j)
                            boxes.append(
                                bbox(au.auEvents[idxEvt, 0], au.auEvents[idxEvt, 1]))
                            IDs.append(au.auNumber[0])

                self.tkBoxes.append(boxes)
                self.tkIDs.append(IDs)

                if len(idxVis) > 0:
                    for j, k in enumerate(idxVis):
                        self.iFrame = cv2.rectangle(
                            self.iFrame, (boxes[j][0], boxes[j][1]), (boxes[j][2], boxes[j][3]), auColors[k].tolist(), 1)

                if len(idxVis) > 0:
                    for j, k in enumerate(idxVis):
                        self.iFrame = cv2.putText(self.iFrame, '{}'.format(
                            IDs[j]), (boxes[j][0], boxes[j][1]), cv2.FONT_HERSHEY_PLAIN, 1., auColors[k].tolist(), 1)

            self.iFrame = cv2.putText(self.iFrame, 'Frame:{}/{}, MaxID:{}'.format(
                self.pFrame, self.nFrame - 1, self.globalID), (0, 15), cv2.FONT_HERSHEY_PLAIN, 1., [255, 255, 255], 1)

            self.frames[self.pFrame] = self.iFrame

        if f > self.pFrame:
            self.pFrame = f
            self.iFrame = self.frame0.copy()

        if p > 0:
            self.iFrame[y, x] = [255, 0, 0]
        else:
            self.iFrame[y, x] = [0, 0, 255]

    def SaveResults(self):
        if not os.path.exists(self.folder):
            os.mkdir(self.folder)

        tkbox_dir = os.path.join(self.folder, self.name + 'tkBoxes.mat')
        tkid_dir = os.path.join(self.folder, self.name + 'tkIDs.mat')
        gt_dir = self.input.replace('.mat', 'GT.mat')
        video_dir = os.path.join(self.folder, self.name + 'output.avi')

        for boxes in self.tkBoxes:
            if len(boxes) == 0:
                continue
            for box in boxes:
                box[2] = box[2] - box[0]
                box[3] = box[3] - box[1]

        self.tkBoxes = np.array(self.tkBoxes, dtype='object')
        self.tkIDs = np.array(self.tkIDs, dtype='object')
        io.savemat(tkbox_dir, {'tkBoxes': self.tkBoxes})
        io.savemat(tkid_dir, {'tkIDs': self.tkIDs})

        HOTA, DETA, ASSA, hacc, dacc, aacc = self.eng.evalhota(
            tkbox_dir, tkid_dir, gt_dir, nargout=6)

        print('Name={}, HOTA={:4f}, DETA={:4f}, ASSA={:4f}\n'.format(self.name, HOTA, DETA, ASSA))
        plt.plot(hacc, color='red', label='HOTA')
        plt.plot(dacc, color='green', label='Detection Accuracy')
        plt.plot(aacc, color='blue', label='Association Accuracy')
        plt.legend()

        result_csv_name = "result.csv"
        with open(os.path.join(self.folder, result_csv_name), 'a') as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow([tkbox_dir, HOTA])

        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        videoWriter = cv2.VideoWriter(
            video_dir, fourcc, 5, (self.lx, self.ly), True)
        for i in range(self.pFrame):
            videoWriter.write(self.frames[i])
        videoWriter.release()

    def Process(self, animation=True):
        print("Split: ", self.split)
        print("Merge: ", self.merge)
        
        for i in tqdm(range(self.events.shape[0])):
            x = self.events[i, 0]
            y = self.events[i, 1]
            p = self.events[i, 2]
            ts = self.events[i, 3]
            f = self.events[i, 4]

            if ts > self.t and len(self.AUs) >= 1:
                self.t = self.t + self.tFrame

                # self.Shrink()
                self.Update()
                self.Split()
                self.Merge()
                self.Kill(ts)

            if animation:
                self.animation = True
                self.Animation(x, y, p, ts, f)

            exist_interested = False

            if len(self.AUs) >= 1:
                for au in self.AUs:
                    if au.interested(x, y):
                        exist_interested = True
                        au.process(x, y, ts)

            if not exist_interested and len(self.AUs) < self.auNum:
                newau = AU(np.array([[x, y, ts]]), self.lx,
                           self.ly, self.dAdd, self.auFifo)
                newau.auBox = [x, y, x + 1, y + 1]
                newau.auNumber = [0, ts]
                self.AUs.append(newau)


class AU():
    def __init__(self, events, lx, ly, dAdd, auFifo):
        self.lx = lx
        self.ly = ly
        self.dAdd = dAdd
        self.auFifo = auFifo

        self.auEvents = events
        self.auMap = np.zeros((self.ly, self.lx), 'int32')
        self.auBox = [0, 0, 0, 0]
        self.auNumber = [0, 0]
        for i in range(events.shape[0]):
            self.expand(events[i, 0], events[i, 1])

    def interested(self, x, y):
        return self.auMap[y, x] > 0

    def expand(self, x, y):
        x = int(x)
        y = int(y)
        idxx = np.arange(max(x - self.dAdd, 0),
                         min(x + self.dAdd + 1, self.lx))
        idxy = np.arange(max(y - self.dAdd, 0),
                         min(y + self.dAdd + 1, self.ly))
        idxxx, idxyy = np.meshgrid(idxx, idxy)
        self.auMap[idxyy, idxxx] += 1

    def shrink(self, x, y):
        x = int(x)
        y = int(y)
        idxx = np.arange(max(x - self.dAdd, 0),
                         min(x + self.dAdd + 1, self.lx))
        idxy = np.arange(max(y - self.dAdd, 0),
                         min(y + self.dAdd + 1, self.ly))
        idxxx, idxyy = np.meshgrid(idxx, idxy)
        self.auMap[idxyy, idxxx] -= 1

    def process(self, x, y, ts):
        self.expand(x, y)

        self.auEvents = np.insert(
            self.auEvents, self.auEvents.shape[0], [x, y, ts], axis=0)

        if self.auEvents.shape[0] > self.auFifo:
            idxFade = range(self.auEvents.shape[0] - self.auFifo)
            for i in idxFade:
                self.shrink(self.auEvents[i, 0], self.auEvents[i, 1])
            self.auEvents = np.delete(self.auEvents, idxFade, axis=0)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config",   default='./config/shape_6dof.yml')
    parser.add_argument("--auFifo", type=int)
    parser.add_argument("--auNum", type=int)
    parser.add_argument("--dAdd", type=int)
    parser.add_argument("--tDel", type=int)
    parser.add_argument("--areaDel", type=int)
    parser.add_argument("--tLive", type=int)
    parser.add_argument("--areaLive", type=int)
    parser.add_argument("--numLive", type=int)
    parser.add_argument("--epsDiv", type=int)
    parser.add_argument("--iomMer", type=float)
    parser.add_argument("--dsMer", type=int)
    parser.add_argument("--split", choices=['DBSCAN', 'HAC'])
    parser.add_argument("--merge", choices=['IOM', 'DIST'])
    parser.add_argument("--input")
    parser.add_argument("--name")
    parser.add_argument("--outfolder")

    args = parser.parse_args()
    with open(args.config, "r") as file:
        config = yaml.safe_load(file)

    args = vars(args)
    config.update({k: v for k, v in args.items() if v is not None})

    config = ARGS(config)
    controller = Controller(config)
    controller.Process()
    controller.SaveResults()


if __name__ == "__main__":
    main()

