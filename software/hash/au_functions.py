from scipy.spatial.distance import directed_hausdorff, pdist

def bbox(xs, ys):
    # get bounding box
    return [int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1]


def pbdist(point, box):
    # calculate distance between point and box
    return max([pdist([point, [box[0], box[1]]]),
                pdist([point, [box[2], box[1]]]),
                pdist([point, [box[2], box[3]]]),
                pdist([point, [box[0], box[3]]])])


def bbArea(b):
    # calculate area of bounding box
    return (b[2] - b[0]) * (b[3] - b[1])


def bbox2points(b):
    # convert bounding box to corner points
    xmin = b[0]
    ymin = b[1]
    xmax = b[2]
    ymax = b[3]

    return [[xmin, ymin], [xmax, ymin], [xmax, ymax], [xmin, ymax]]


def bboxOverlapRatio(boxA, boxB):
    # get overlap ratio between bounding boxes
    xA = max(boxA[0], boxB[0])
    yA = max(boxA[1], boxB[1])
    xB = min(boxA[2], boxB[2])
    yB = min(boxA[3], boxB[3])

    interArea = max(0, xB - xA + 1) * max(0, yB - yA + 1)

    boxAArea = (boxA[2] - boxA[0] + 1) * (boxA[3] - boxA[1] + 1)
    boxBArea = (boxB[2] - boxB[0] + 1) * (boxB[3] - boxB[1] + 1)

    return float(interArea) / min(boxAArea, boxBArea)