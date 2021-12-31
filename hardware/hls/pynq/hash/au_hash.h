
template<class T>
event pack2e(T in_pack){
#pragma HLS INLINE
	event out_event;
	out_event.x = in_pack.range(XBITS - 1, 0);
	out_event.y = in_pack.range(YBITS + XBITS - 1, XBITS);
	out_event.t = in_pack.range(TBITS + YBITS + XBITS - 1, YBITS + XBITS);
	out_event.p = in_pack.range(PBITS + TBITS + YBITS + XBITS - 1, YBITS + XBITS + TBITS);
	return out_event;
}


template<int Number>
void au_hash(
		local_stream &in_data,
		key_type hashA_key[N_AU][T_SIZE],
		value_type hashA_value[N_AU][T_SIZE],
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		pointer_type fifo_pointer[N_AU],
		bool status[N_AU],
		int N
){
//#pragma HLS inline

	event event_in;
	event_pack_t old_event[N_AU];
	event old[N_AU];

	bool found_interested = 0;
	static bool interested[N_AU];
	x_type update_x[N_AU];
	y_type update_y[N_AU];

#pragma HLS ARRAY_PARTITION variable=interested complete dim=0
#pragma HLS ARRAY_PARTITION variable=update_x complete dim=0
#pragma HLS ARRAY_PARTITION variable=update_y complete dim=0
#pragma HLS ARRAY_PARTITION variable=old complete dim=0


	for(int n = 0; n < N; n++){
		event_pack_t in_read = in_data.read();
		event event_in =  pack2e(in_read);
		query<Number>(event_in.x, event_in.y, hashA_key, hashA_value, interested);
		
		found_interested = 0;
		for(int i = 0; i <= N_AU - 1; i++){
#pragma HLS UNROLL
			found_interested |= interested[i];
		}

		if(!found_interested){
			int next_empty = -1;
			for(int i = 0; i <= N_AU - 1; i++){
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

		for(int i = 0; i <= N_AU - 1; i++){
#pragma HLS UNROLL
			update_x[i] = event_in.x;
			update_y[i] = event_in.y;
		}

		update_square<Number>(update_x, update_y, 1, hashA_key, hashA_value, interested);

		for(int i = 0; i <= N_AU - 1; i++){
#pragma HLS UNROLL 
			if(interested[i]){
				pointer_type last_i = fifo_pointer[i];
				old_event[i] = event_fifo[i][last_i];
				if(old_event[i] == 0){
					update_x[i] = 255;
					update_y[i] = 255;
				}
				else{
					old[i] = pack2e(old_event[i]);
					update_x[i] = old[i].x;
					update_y[i] = old[i].y;
				}

				event_fifo[i][last_i++] = in_read;
				last_i %= FIFO_DEPTH;
				fifo_pointer[i] = last_i;
			}
		}

		update_square<Number>(update_x, update_y, -1, hashA_key, hashA_value, interested);
	}
}


template<int Number>
void return_hash(
		int return_n,
		key_type hashA_key[N_AU][T_SIZE],
		value_type hashA_value[N_AU][T_SIZE],
		hash_type* out_hash
){
#pragma HLS INLINE


	for(int i = 0; i < T_SIZE; i++){
#pragma HLS PIPELINE
			if(return_n < 0 || return_n >= N_AU) break;
			key_type a_key = hashA_key[return_n][i];
			value_type a_val = hashA_value[return_n][i];
			out_type out_pack = 0;
			out_pack.range(KBITS - 1, 0) = a_key;
			out_pack.range(KBITS + VBITS - 1, KBITS) = a_val;
			out_hash[i] = out_pack;
	}
	
}


template<int Number>
void return_all(
		int retrive_n,
		key_type hashA_key[N_AU][T_SIZE][T_WIDTH],
		value_type hashA_value[N_AU][T_SIZE][T_WIDTH],
		hmap_stream& out_stream,
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		fifo_stream& out_fifo_stream,
		pointer_type fifo_pointer[N_AU],
		depth_stream& out_depth_stream
){

	if(retrive_n >= 0 && retrive_n <= N_AU - 1){

		for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE	
			event_pack_t event_read = event_fifo[retrive_n][i];
			out_fifo_stream.write(event_read);
		}

		for(int i = 0; i < T_SIZE; i++){
			for(int j = 0; j < T_WIDTH; j++){
#pragma HLS PIPELINE
				key_type a_key = hashA_key[retrive_n][i][j];
				value_type a_val = hashA_value[retrive_n][i][j];
				out_type out_pack = 0;
				out_pack.range(KBITS - 1, 0) = a_key;
				out_pack.range(KBITS + VBITS - 1, KBITS) = a_val;
				out_stream.write(out_pack);
			}
		}
		out_depth_stream.write(fifo_pointer[retrive_n]);
	}
}



template<int Number>
void init_hash_array(
		int init_n,
		hash_type* init_hash,
		key_type hashA_key[N_AU][T_SIZE],
		value_type hashA_value[N_AU][T_SIZE]
){

	if(init_n >= 0 && init_n <= N_AU - 1){
		for(int i = 0; i < T_SIZE; i++){
#pragma HLS PIPELINE
				hash_type init_pack = init_hash[i];
				key_type a_key = init_pack.range(KBITS - 1, 0);
				value_type a_val = init_pack.range(KBITS + VBITS - 1, KBITS);
				hashA_key[init_n][i] = a_key;
				hashA_value[init_n][i] = a_val;
		}
	}
}



template<int Number>
void init_efifo(
		int init_n,
		efifo_type* fifo_in,
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		int in_fifo_depth,
		pointer_type fifo_pointer[N_AU]
){

	if(init_n >= 0 && init_n <= N_AU - 1){
		for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE
			event event_read;
			efifo_type pack = fifo_in[i]; 
			event_fifo[init_n][i] = pack;
		}
		fifo_pointer[init_n] = in_fifo_depth;
	}
}

template<int NA>
void in_hash_m2s(
		int init_N,
		out_type* in_hash,
		hmap_stream& in_hash_stream
){

	int index = 0;
	out_type hmap_read;
	if(init_N >= 0 && init_N <= (NA - 1)){
		for(int i = 0; i < T_SIZE * T_WIDTH; i++){
#pragma HLS PIPELINE
			hmap_read = in_hash[i];
			in_hash_stream.write(hmap_read);
		}
	}
}

template<int NA>
void in_fifo_m2s(
		int init_N,
		out_type* in_fifo,
		hmap_stream& in_fifo_stream,
		int in_fifo_depth
){

	int index = 0;
	out_type value;
	if(init_N >= 0 && init_N <= (NA - 1)){
		for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE
			value = in_fifo[i];
			in_fifo_stream.write(value);
		}
	}
}


template<int NA>
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
		event_data = event_read.data;
		out_data.write(event_data);
	}
}


template<int Number>
void arbiter_hash(
		hmap_stream& hmap,
		out_type* out_data,
		int retrive_n

){

	
	for(int i = 0; i < T_SIZE; i++){
		if(retrive_n < 0 || retrive_n >= N_AU) break;
#pragma HLS PIPELINE II=1
		out_data[i] = hmap.read();			
	}
}



template<int Number>
void arbiter_fifo(
		fifo_stream& out_fifo_stream,
		out_type* out_fifo,
		int return_n

){
	int index = 0;

	for(int i = 0; i < FIFO_DEPTH; i++){
#pragma HLS PIPELINE
		if(return_n < 0 || return_n >= N_AU) break;
		event_pack_t event_read = out_fifo_stream.read();			
		out_fifo[i] = event_read;
	}
	
	
}


template<class T>
void init_status(
	status_type status_in,
	T status[N_AU],
	int init_n
){
	static bool init_flag = 0;
	if(!init_flag){
		init_flag = 1;
		for(int i = 0; i <= N_AU - 1; i++){
#pragma HLS UNROLL
			status[i] = 1;
		}
	}
	else if(init_n >= 0 && init_n <= N_AU - 1){
		for(int i = 0; i <= N_AU - 1; i++){
#pragma HLS UNROLL
			status[i] = status_in.range(i, i);
		}
	}
}




template<int Number>
void return_status(
	status_type& status_out,
	bool status[N_AU]
){
	#pragma HLS INLINE
	status_type empty_pack = 0;
	for(int i = 0; i <= N_AU - 1; i++){
#pragma HLS UNROLL
		empty_pack.range(i, i) = status[i];
	}
	status_out = empty_pack;

}


template<int Number>
void return_fifo(
		int return_n,
		event_pack_t event_fifo[N_AU][FIFO_DEPTH],
		efifo_type* out_fifo
){
#pragma HLS INLINE
	

	for(int i = 0; i < FIFO_DEPTH; i++){	
#pragma HLS PIPELINE II=1
		if(return_n < 0 || return_n >=  N_AU) break;
		event_pack_t event_read = event_fifo[return_n][i];
		out_fifo[i] = event_read;
	}		
}



template<int Number>
void return_depth(
		int retrive_n,
		pointer_type fifo_pointer[N_AU],
		depth_stream& out_fifo_depth
){

	if(retrive_n >= 0 && retrive_n <= N_AU - 1){
		out_fifo_depth.write(fifo_pointer[retrive_n]);	
	}
}
