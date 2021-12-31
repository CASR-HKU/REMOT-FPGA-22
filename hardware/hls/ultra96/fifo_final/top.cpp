#include "top.h"


template<int Number>
void Au_hash_wrapper(
		local_stream& in_data,
		efifo_type* in_fifo,
		efifo_type* out_fifo,
		status_type status_in,
		status_type& status_out,
		int N,
		int return_n,
		int init_n,
		int in_fifo_depth
		
){
//#pragma HLS INLINE

	static pointer_type fifo_pointer[N_AU];
	static event_pack_t event_fifo[N_AU][FIFO_DEPTH];
	static bool status[N_AU];

DO_PRAGMA( HLS ARRAY_PARTITION variable=status dim=0 complete)
DO_PRAGMA( HLS ARRAY_PARTITION variable=event_fifo dim=2 factor=PI_0/2 cyclic)
DO_PRAGMA( HLS ARRAY_PARTITION variable=event_fifo dim=1 complete)

	init_status(status_in, status, init_n);
	init_fifo<Number>(init_n, in_fifo, event_fifo, in_fifo_depth, fifo_pointer);
	au_fifo<Number>(in_data, event_fifo, fifo_pointer, status, N);
	return_fifo<Number>(return_n, event_fifo, out_fifo);
	return_status<Number>(status_out, status);
}

void top(
		event_stream& in_stream,
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
DO_PRAGMA(HLS INTERFACE m_axi port=out_fifo bundle=gmem2 depth=FIFO_DEPTH*N_AU)
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

   	Au_hash_wrapper<0>(broadcast_stream, init_fifo, out_fifo, status_in, status_out, N, return_n, init_n, in_fifo_depth);
}
