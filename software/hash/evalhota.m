function [HOTA, DETA, ASSA, hacc, dacc, aacc] = evalhota(tkbox_dir, tkid_dir, gt_dir)

load(tkbox_dir)
load(tkid_dir)
load(gt_dir)

[HOTA, DETA, ASSA, hacc, dacc, aacc] = hota(tkBoxes, tkIDs, gtBoxes, gtIDs);