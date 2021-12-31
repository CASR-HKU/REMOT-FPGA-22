############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2021 Xilinx, Inc. All Rights Reserved.
############################################################
open_project hash_proj
set_top top
add_files top.cpp
add_files -tb tb.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xczu3eg-sbva484-2-i}
create_clock -period 3 -name default
config_interface -m_axi_offset slave
config_export -format ip_catalog -rtl verilog
set_directive_top -name top "top"
# csim_design
csynth_design
# cosim_design
export_design -rtl verilog -format ip_catalog
quit
