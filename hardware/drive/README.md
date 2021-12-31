## AU hardware demo drive

The two folder contains the drive code and bitstreams to run REMOT on PYNQ-Z2 and Ultra96 with [PYNQ overlay](https://pynq.readthedocs.io/en/v2.6.1/index.html).

The bitfile folder already contains the prebuilt bitstreams for the demo. If you build the bitstreams by yourself running **make** in the hls folder, the newly generated bitstreams will be automatically exported into pynq/bitfile and ultra96/bitfile.

When you have the bitstreams ready, pack everything and uploaded to the development boards by using:

```
    tar -cvzf REMOT_pynq.tar.gz ./pynq
    tar -cvzf REMOT_ultra96.tar.gz ./ultra96
```

After you unpack the folder on the development board, run  

- shapes_6dof_demo.ipynb
- test_speed.ipynb

for real hardware demo and throughputs tests.



