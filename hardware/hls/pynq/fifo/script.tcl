############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project fifo_proj
set_top top
add_files top.cpp
add_files -tb tb.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xc7z020-clg484-1}
create_clock -period 4.5 -name default
config_interface -m_axi_offset slave
config_export -format sysgen -rtl verilog
set_directive_top -name top "top"
# csim_design
csynth_design
# cosim_design
export_design -rtl verilog -format sysgen
quit