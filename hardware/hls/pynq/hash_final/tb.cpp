#include "top.h"
#include <iostream>
using namespace std;


int main(){

	event e1 = { .x = 100, .y = 100, .t = 432786, .p = 1 };
	event e2 = { .x = 50, .y = 50, .t = 1239847, .p = 1 };
	event_axi e_pack;

	event_stream in;

	for(int i = 0; i < 4; i++){
#pragma HLS PIPELINE
		event_pack_t event_data;
		event_data.range(XBITS - 1, 0) = e1.x;
		event_data.range(XBITS + YBITS - 1, XBITS) = e1.y;
		event_data.range(XBITS + YBITS + TBITS - 1, XBITS + YBITS) = e1.t;
		event_data.range(PBITS + YBITS + TBITS, XBITS + YBITS + TBITS) = e1.p;
		e_pack.data = event_data;
		in.write(e_pack);
	}

	for(int i = 0; i < 4; i++){
#pragma HLS PIPELINE
		event_pack_t event_data;
		event_data.range(XBITS - 1, 0) = e2.x;
		event_data.range(XBITS + YBITS - 1, XBITS) = e2.y;
		event_data.range(XBITS + YBITS + TBITS - 1, XBITS + YBITS) = e2.t;
		event_data.range(PBITS + YBITS + TBITS, XBITS + YBITS + TBITS) = e2.p;
		e_pack.data = event_data;
		in.write(e_pack);
	}

	static ap_uint<OUTBITS> out_hash[T_SIZE * T_WIDTH];
	static ap_uint<OUTBITS> init_hash[T_SIZE * T_WIDTH];
	static ap_uint<OUTBITS> out_fifo[FIFO_DEPTH];
	static ap_uint<OUTBITS> init_fifo[FIFO_DEPTH];

	
	for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE
		ap_uint<OUTBITS> pack;
		pack.range(XBITS - 1, 0) = 1;
		pack.range(YBITS + XBITS - 1, XBITS) = 1;
		pack.range(TBITS + YBITS + XBITS - 1, YBITS + XBITS) = 1;
		pack.range(PBITS + TBITS + YBITS + XBITS - 1, YBITS + XBITS + TBITS) = 1;
		init_fifo[i] = pack;
	}



	int N = 8;
	int return_n = 0;

	int init_n = 16;
	int count = 0;
	int out_fifo_depth = 0;
	int in_fifo_depth = 100;
	status_type status_in;
	status_type status_out;


	for(int i = 0 ; i < 32; i++){
		status_in.range(i, i) = 1;
	}

	


	top(
		in,
		out_hash,
		init_hash,
		out_fifo,
		init_fifo,
		N,
		return_n,
		init_n,
		in_fifo_depth,
		status_in,
		status_out
	);

	for(int i = 0; i < N_AU; i++){
		cout<<i<<" empty_out:"<<status_out.range(i, i)<<endl;;
	}

	cout<<"result:"<<endl;
	for(int i = 0; i < T_SIZE; i++){
		for(int j = 0; j < T_WIDTH; j++){
			ap_uint<OUTBITS> out = out_hash[i * T_WIDTH + j];
			ap_uint<KBITS> a_key = out.range(KBITS - 1, 0);
			ap_uint<VBITS> a_val = out.range(KBITS + VBITS - 1, KBITS);
			if (a_val > 0){
				cout<<count++<<"\t";
				cout<<"i: "<<i<<" ";
				cout<<"out "<<out<< " akey: "<<a_key<<" aval: "<<a_val<<endl;
			}
		}
	}


	cout<<"outfifo_depth: "<<out_fifo_depth<<endl;
	for(int i = 0; i < FIFO_DEPTH; i++){
		event event_read;
		ap_uint<OUTBITS> pack = out_fifo[i];
		cout<<i<<"\t";
		event_read.x = pack.range(XBITS - 1, 0);
		event_read.y = pack.range(YBITS + XBITS - 1, XBITS);
		event_read.t = pack.range(TBITS + YBITS + XBITS - 1, YBITS + XBITS);
		event_read.p = pack.range(PBITS + TBITS + YBITS + XBITS - 1, YBITS + XBITS + TBITS);
		cout<<" x:"<<event_read.x<<" y:"<<event_read.y<<" t:"<<event_read.t<<" p:"<<event_read.p<<endl;
	}

	return 0;
}





