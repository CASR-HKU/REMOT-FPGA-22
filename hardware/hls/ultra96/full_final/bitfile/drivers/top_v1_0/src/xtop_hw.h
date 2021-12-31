// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2020.2 (64-bit)
// Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - enable ap_done interrupt (Read/Write)
//        bit 1  - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - ap_done (COR/TOW)
//        bit 1  - ap_ready (COR/TOW)
//        others - reserved
// 0x10 : Data signal of out_amap
//        bit 31~0 - out_amap[31:0] (Read/Write)
// 0x14 : Data signal of out_amap
//        bit 31~0 - out_amap[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of init_amap
//        bit 31~0 - init_amap[31:0] (Read/Write)
// 0x20 : Data signal of init_amap
//        bit 31~0 - init_amap[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of out_fifo
//        bit 31~0 - out_fifo[31:0] (Read/Write)
// 0x2c : Data signal of out_fifo
//        bit 31~0 - out_fifo[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of init_fifo
//        bit 31~0 - init_fifo[31:0] (Read/Write)
// 0x38 : Data signal of init_fifo
//        bit 31~0 - init_fifo[63:32] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of N
//        bit 31~0 - N[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of return_n
//        bit 31~0 - return_n[31:0] (Read/Write)
// 0x4c : reserved
// 0x50 : Data signal of init_n
//        bit 31~0 - init_n[31:0] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of in_fifo_depth
//        bit 31~0 - in_fifo_depth[31:0] (Read/Write)
// 0x5c : reserved
// 0x60 : Data signal of status_in
//        bit 31~0 - status_in[31:0] (Read/Write)
// 0x64 : reserved
// 0x68 : Data signal of status_out
//        bit 31~0 - status_out[31:0] (Read)
// 0x6c : Control signal of status_out
//        bit 0  - status_out_ap_vld (Read/COR)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XTOP_CONTROL_ADDR_AP_CTRL            0x00
#define XTOP_CONTROL_ADDR_GIE                0x04
#define XTOP_CONTROL_ADDR_IER                0x08
#define XTOP_CONTROL_ADDR_ISR                0x0c
#define XTOP_CONTROL_ADDR_OUT_AMAP_DATA      0x10
#define XTOP_CONTROL_BITS_OUT_AMAP_DATA      64
#define XTOP_CONTROL_ADDR_INIT_AMAP_DATA     0x1c
#define XTOP_CONTROL_BITS_INIT_AMAP_DATA     64
#define XTOP_CONTROL_ADDR_OUT_FIFO_DATA      0x28
#define XTOP_CONTROL_BITS_OUT_FIFO_DATA      64
#define XTOP_CONTROL_ADDR_INIT_FIFO_DATA     0x34
#define XTOP_CONTROL_BITS_INIT_FIFO_DATA     64
#define XTOP_CONTROL_ADDR_N_DATA             0x40
#define XTOP_CONTROL_BITS_N_DATA             32
#define XTOP_CONTROL_ADDR_RETURN_N_DATA      0x48
#define XTOP_CONTROL_BITS_RETURN_N_DATA      32
#define XTOP_CONTROL_ADDR_INIT_N_DATA        0x50
#define XTOP_CONTROL_BITS_INIT_N_DATA        32
#define XTOP_CONTROL_ADDR_IN_FIFO_DEPTH_DATA 0x58
#define XTOP_CONTROL_BITS_IN_FIFO_DEPTH_DATA 32
#define XTOP_CONTROL_ADDR_STATUS_IN_DATA     0x60
#define XTOP_CONTROL_BITS_STATUS_IN_DATA     32
#define XTOP_CONTROL_ADDR_STATUS_OUT_DATA    0x68
#define XTOP_CONTROL_BITS_STATUS_OUT_DATA    32
#define XTOP_CONTROL_ADDR_STATUS_OUT_CTRL    0x6c

