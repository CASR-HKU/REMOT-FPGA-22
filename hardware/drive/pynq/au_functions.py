import numpy as np
class ARGS:
    def __init__(self, config):
        for key in config.keys():
            setattr(self, key, config[key])

def bbox(xs, ys):
    return [int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1]


def bbArea(b):
    return (b[2] - b[0]) * (b[3] - b[1])


def bbox2points(b):
    xmin = b[0]
    ymin = b[1]
    xmax = b[2]
    ymax = b[3]

    return [[xmin, ymin], [xmax, ymin], [xmax, ymax], [xmin, ymax]]


def bboxOverlapRatio(boxA, boxB):
    xA = max(boxA[0], boxB[0])
    yA = max(boxA[1], boxB[1])
    xB = min(boxA[2], boxB[2])
    yB = min(boxA[3], boxB[3])

    interArea = max(0, xB - xA + 1) * max(0, yB - yA + 1)

    boxAArea = (boxA[2] - boxA[0] + 1) * (boxA[3] - boxA[1] + 1)
    boxBArea = (boxB[2] - boxB[0] + 1) * (boxB[3] - boxB[1] + 1)

    return float(interArea) / min(boxAArea, boxBArea)

def clusterAu(boxes, iom):
    sz = boxes.shape[0]
    idxGroup = np.zeros(sz, 'int') - 1
    n = 0
    
    for i in range(sz):
        if idxGroup[i] >= 0:
            continue
        else:
            idxGroup[i] = n
            n += 1
        
        for j in range(i + 1, sz):
            if idxGroup[j] >= 0:
                continue
            
            if bboxOverlapRatio(boxes[i], boxes[j]) >= iom:
                idxGroup[j] = idxGroup[i]
    
    return idxGroup