# REMOT

REMOT: A Hardware-Software Architecture for Attention-Guided Multi-Object Tracking with Dynamic Vision Sensors on FPGAs

## Citation

```
@article{yizhao2021remot,
    title = "{REMOT: A Hardware-Software Architecture for Attention-Guided Multi-Object Tracking with Dynamic Vision Sensors on FPGAs}",
    author = {Yizhao, Gao and Song, Wang and So, Hayden K.-H.},
    journal = {The 2022 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays},
    year = {2022}
}
```


## Introduction 

REMOT is a hardware/software architecture for Multi-Object Tracking using Dynamic Vision Sensors on FPGA. It's designed around the concept of an attention unit (AU). Each AU will only pay attention to a specific region of interest, which is designed to changed as the object moves. In REMOT, a layer of parallel AUs is implemented on FPGA to collectively process the stream of asynchronous events from a DVS.



## Outlines
- software: Python implementation of AU for Multi-Object Tracking 
- hardware: HLS code and vivado scripts for 3 different implementations: FUll-AMAP, HASH-AMAP, and FIFO-ONLY. Python drive for [PYNQ overlay](https://pynq.readthedocs.io/en/v2.6.1/index.html) is also provided for PYNQ-Z2 and Ultra96 development boards. 


## Dependency

The repo has been verified on:

- Ubuntu 18.04 
- [Vivado 2020.2](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2020-2.html)
- Matlab R2021b
- python 3.7
- [PYNQ v2.6](https://pynq.readthedocs.io/en/v2.6.1/index.html) image


### Vivado 2020.2

- We provide scripts with [Vivado 2020.2](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/2020-2.html) to build bitstream. If using a different version, some mannul intervention might be needed. 

```
    source <vivado_root/2020.2/settings64.sh>
```

### Python Environment Installation
- Use [conda](https://docs.conda.io/projects/conda/en/latest/) to install python libraries:
```
    conda create -n au python=3.7
    conda activate au
    pip install -r requirements.txt
```
- Install [Matlab](https://ww2.mathworks.cn/help/matlab/matlab_external/install-the-matlab-engine-for-python.html) Engine API for python
```
    cd <matlab_root/extern/engines/python>
    python setup.py install
```
