# Extract Vivado HLS include path
VHLS_PATH := $(dir $(shell which vivado_hls))/..
VHLS_INC ?= ${VHLS_PATH}/include
proj_dir = $(shell pwd)
bifile_dir = ../../../../drive/pynq/bitfile
name = full_1024_5_2
hls_name = full_proj
tcl_name = full_proj
xsa_dir = bitfile

CFLAGS = -g -I${VHLS_INC}

all: hls bitstream unpack

hls:
	vitis_hls -f script.tcl

bitstream:
	
	cp ./vivado/design_1_wrapper.v ./$(hls_name)/solution1/impl/ip/
	cp ./vivado/$(tcl_name).tcl ./$(hls_name)/solution1/impl/ip/
	cd ./$(hls_name)/solution1/impl/ip/; vivado -mode tcl -source $(tcl_name).tcl

unpack:
	cd ./$(xsa_dir); unzip $(xsa_dir).xsa; mv design_1.hwh $(name).hwh; mv $(xsa_dir).bit $(name).bit; cp $(name).bit $(bifile_dir); cp $(name).hwh $(bifile_dir)
	
clean:
	rm -rf $(hls_name) $(xsa_dir) *.prj *.log