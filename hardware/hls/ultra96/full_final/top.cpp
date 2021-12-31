#include "top.h"


template<int Number>
void Au_full_wrapper(
		local_stream& in_data,
		amap_type* out_amap,
		amap_type* init_amap,
		efifo_type* out_fifo,
		efifo_type* init_fifo,
		status_type status_in,
		status_type& status_out,
		int N,
		int return_n,
		int init_n,
		int in_fifo_depth
){
#pragma HLS INLINE OFF

	static amap_type amap[N_AU][HEIGHT * WIDTH];
#pragma HLS ARRAY_PARTITION variable=amap dim=1 complete

	static pointer_type fifo_pointer[N_AU];
	static event_pack_t event_fifo[N_AU][FIFO_DEPTH];  
	static bool status[N_AU];
DO_PRAGMA( HLS ARRAY_PARTITION variable=event_fifo dim=1 complete)
DO_PRAGMA( HLS ARRAY_PARTITION variable=status dim=0 complete)
DO_PRAGMA( HLS ARRAY_PARTITION variable=fifo_pointer dim=0 complete)



	init_status(status_in, status, init_n);
	init_amap_array<Number>(init_n, init_amap, amap);
	init_efifo<Number>(init_n, init_fifo, event_fifo, in_fifo_depth, fifo_pointer);

	au_full<Number>(in_data, amap, event_fifo, fifo_pointer, status, N);

	return_status<Number>(status_out, status);
	return_fifo<Number>(return_n, event_fifo, out_fifo);
	return_amap<Number>(return_n, amap, out_amap);
	
}




void top(
		event_stream& in_stream,
		amap_type* out_amap,
		amap_type* init_amap,
		efifo_type* out_fifo,
		efifo_type* init_fifo,
		int N,
		int return_n,
		int init_n,
		int in_fifo_depth,
		status_type status_in,
		status_type& status_out
){

#pragma HLS INTERFACE axis port=in_stream
DO_PRAGMA(HLS INTERFACE m_axi port=out_amap bundle=gmem0 depth=HEIGHT*WIDTH)
DO_PRAGMA(HLS INTERFACE m_axi port=init_amap bundle=gmem1 depth=HEIGHT*WIDTH)
DO_PRAGMA(HLS INTERFACE m_axi port=out_fifo bundle=gmem2 depth=FIFO_DEPTH)
DO_PRAGMA(HLS INTERFACE m_axi port=init_fifo bundle=gmem3 depth=FIFO_DEPTH)

#pragma HLS INTERFACE s_axilite port=N bundle=control
#pragma HLS INTERFACE s_axilite port=return_n bundle=control
#pragma HLS INTERFACE s_axilite port=init_n bundle=control
#pragma HLS INTERFACE s_axilite port=in_fifo_depth bundle=control
#pragma HLS INTERFACE s_axilite port=status_in bundle=control
#pragma HLS INTERFACE s_axilite port=status_out bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

#pragma HLS DATAFLOW

	local_stream broadcast_stream;
#pragma HLS stream variable=broadcast_stream depth=1024

	broadcast<N_AU>(in_stream, broadcast_stream, N);

	Au_full_wrapper<0>(
		broadcast_stream,
		out_amap,
		init_amap,
		out_fifo,
		init_fifo,
		status_in,
		status_out,
		N,
		return_n,
		init_n,
		in_fifo_depth
	);


}



