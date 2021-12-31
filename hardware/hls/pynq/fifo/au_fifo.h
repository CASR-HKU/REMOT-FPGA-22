template<class T>
T ceil_div(T A, T B){
	T d = A / B;
	if (A > d * B){
		return d + 1;
	}
	else{
		return d;
	}
}


template<int PI>
void au_comparator(
	event_pack_t event_fifo[N_AU][FIFO_DEPTH],
	event_pack_t in_event,
	bool interested[N_AU]
){
//#pragma HLS INLINE

	event_pack_t event_to_compare[N_AU];
	ap_int<XBITS + 1> c_x[N_AU];
	ap_int<YBITS + 1> c_y[N_AU];
	ap_int<XBITS + 1> x_diff[N_AU];	
	ap_int<YBITS + 1> y_diff[N_AU];
#pragma HLS ARRAY_PARTITION variable=c_x complete dim=0
#pragma HLS ARRAY_PARTITION variable=c_y complete dim=0
#pragma HLS ARRAY_PARTITION variable=x_diff complete dim=0
#pragma HLS ARRAY_PARTITION variable=y_diff complete dim=0

	for(int j = 0; j < N_AU; j++){
#pragma HLS UNROLL
		interested[j] = 0;
	}

	for(ap_uint<10> i = 0; i < FIFO_DEPTH/PI; i++){
#pragma HLS PIPELINE II=1
		for(ap_uint<10> pi = 0; pi < PI; pi++){
#pragma HLS UNROLL
			ap_int<XBITS + 1> in_x = in_event.range(XBITS, 0);
			ap_int<YBITS + 1> in_y = in_event.range(YBITS + XBITS - 1, XBITS);
			for(int j = 0; j < N_AU; j++){
#pragma HLS UNROLL
				event_to_compare[j] = event_fifo[j][pi + i * PI];
				c_x[j] = event_to_compare[j].range(XBITS, 0);
				c_y[j] = event_to_compare[j].range(YBITS + XBITS - 1, XBITS);
				x_diff[j] = in_x - c_x[j];
				y_diff[j] = in_y - c_y[j];
				if ((x_diff[j]<=dadd) && (x_diff[j]>=-dadd) && (y_diff[j]<=dadd) && (y_diff[j]>=-dadd)){
					interested[j] |= 1;
				}
			}
		}
	}
}



template<int Number>
void au_fifo(
		local_stream& in_data,
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		pointer_type fifo_pointer[N_AU],
		bool status[N_AU],
		int N
){
//#pragma HLS inline

	event_pack_t in_event;
	event_pack_t old_event;
	const event_pack_t zero_event = 0;
	
	bool found_interested = 0;
	bool interested[N_AU];


#pragma HLS ARRAY_PARTITION variable=interested complete dim=0

	for(int n = 0; n < N; n++){
		//cout<<"n: "<<n<<endl;
		in_event = in_data.read();
		au_comparator<PI_0>(event_fifo, in_event, interested);

		found_interested = 0;
		for(int i = 0; i < N_AU; i++){
#pragma HLS UNROLL
			found_interested |= interested[i];
		}

		if(!found_interested){
			int next_empty = -1;
			for(int i = 0; i < N_AU; i++){
#pragma HLS UNROLL
				if (status[i] == 1){ 
					next_empty = i;
					status[i] = 0;
					break;
				}	
			}
			if (next_empty != -1){
				interested[next_empty] = 1;
			}
		}

		for(int i = 0; i < N_AU; i++){
#pragma HLS UNROLL 
			if(interested[i]){
				pointer_type last_i = fifo_pointer[i];
				event_fifo[i][last_i++] = in_event;
				last_i %= FIFO_DEPTH;
				fifo_pointer[i] = last_i;
			}
		}
	}
}


template<int Number>
void return_fifo(
		int return_n,
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		efifo_type* out_fifo
){

	for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE II=1
		if(return_n < 0 || return_n >= N_AU) break;
		out_fifo[i] = event_fifo[return_n][i];
	}
}

template<int Number>
void return_status(
	ap_uint<32>& status_out,
	bool status[N_AU]
){
	status_type pack = 0;
	for(int i = 0; i < N_AU; i++){
#pragma HLS UNROLL
		pack.range(i, i) = status[i];
	}
	status_out = pack;
}


template<int Number>
void init_fifo(
		int init_n,
		efifo_type* in_fifo,
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		int in_fifo_depth,
		pointer_type fifo_pointer[N_AU]
){	
	if(init_n >= 0 && init_n < N_AU){
		for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE
			event_fifo[init_n][i] = in_fifo[i]; 
		}
		fifo_pointer[init_n] = in_fifo_depth; 
	}
}


template<int Number>
void broadcast(
		event_stream &in_data,
		local_stream &out_data,
		int N
){
	event_axi event_read;
	event_pack_t event_data;

	for(int i = 0; i < N; i++){
#pragma HLS PIPELINE
		event_read = in_data.read();
		out_data.write(event_read.data);
	}
}

template<class T>
void init_status(
	status_type status_in,
	T status[N_AU],
	int init_n
){
	static bool inited_flag = 0;
	if(!inited_flag){
		for(int i = 0; i < N_AU; i++){
#pragma HLS UNROLL
			status[i] = 1;
		}
		inited_flag = 1;
	}
	else if(init_n >= 0 && init_n < N_AU){
		for(int i = 0; i < N_AU; i++){
#pragma HLS UNROLL
			status[i] = status_in.range(i, i);
		}
	}
}
