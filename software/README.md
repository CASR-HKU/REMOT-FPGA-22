# Software-only implementation of REMOT

## software_basic
The "software_evalution" notebook basically contains all the necessary software simulation results on three different dataset:

- [shapes_6dof](http://rpg.ifi.uzh.ch/davis_data.html)
- inbound traffic
- outbound traffic

The datasets data and their ground truth are put in the "dataset" folder. 
We mannully labeled [shapes_6dof](http://rpg.ifi.uzh.ch/davis_data.html) dataset for MOT by ourselves. If you want to use this dataset, please cite their paper: 

*E. Mueggler, H. Rebecq, G. Gallego, T. Delbruck, D. Scaramuzza, The Event-Camera Dataset and Simulator: Event-based Data for Pose Estimation, Visual Odometry, and SLAM, International Journal of Robotics Research, Vol. 36, Issue 2, pages 142-149, Feb. 2017.*

## hash
The "hash_acc_experiment" notebook contains scripts for running the HASH-AMAP simulation with different hash table configurations on shapes_6dof dataset, which corresponds to the fig.7f in the paper.


