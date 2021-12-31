# REMOT Hardware
- This folder contains the hls and vivado project tcl for REMOT. The ./pynq and ./ultra96 folder both contains 3 implementations: FULL-AMAP, HASH-AMAP and FIFO-ONLY for PYNQ-Z2 and Ultra96. 

- For each implementation, the one with postfix "_final" are the final implmentation configurations with maximum number of AU deployed, which correspond to the results of Fig.7c in the paper. The other ones are designed for running hardware demo on shapes_6dof datasets. 

## Generate bitstreams 

If you want to reproduce all the 12 bitstreams, go to ./pynq and ./ultra96 folder and run **make all**.

```
    cd pynq
    make all

    cd ultra96
    make all
```

If you only want to play with a specific design, you can go the folder and run **make all**.

The output (.bit and .hwh) will automatically be exported to the corresponding folders in hardware/drive. The hardware/drive folder already contains the prebuilt bitstreams. Once you are ready, you can pack the entire folder and upload to development board for test.  