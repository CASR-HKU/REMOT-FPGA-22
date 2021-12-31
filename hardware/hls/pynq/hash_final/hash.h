
template<int Number>
key_type hash_map(
		x_type x,
		y_type y
){
	key_type out;
	out = x + y * WIDTH + 1;
	return out;
}

template<int Number>
ap_uint<TABLE_BITS> hash_forward_A(
		key_type key
){

	const key_type A = 72189;
	ap_uint<2 * KBITS> mul = key * A;
	ap_uint<TABLE_BITS> out = mul.range(KBITS - 1, KBITS - TABLE_BITS);
	return out;
}



template<int Number>
 void insert_2way(
 		x_type x,
 		y_type y,
 		value_type value,
 		key_type hashA_key[T_SIZE],
 		value_type hashA_value[T_SIZE]
 ){
 #pragma HLS INLINE

 	key_type key = hash_map<Number>(x, y);
 	ap_uint<TABLE_BITS> idx_a = hash_forward_A<Number>(key);


 	key_type key_a;
 	value_type val_a;
 	key_type new_key_a;
 	value_type new_val_a;
 	bool match;

 	key_a = hashA_key[idx_a];
 	val_a = hashA_value[idx_a];


 	bool full = 1;
 	

 	match = (key_a == key) ? 1 : 0;
 	
	if (match){
		if ((val_a + value) >= 0){
			new_val_a = val_a + value;
		}
		else{
			new_val_a = 0;
		}
		new_key_a = key_a;
 	}
	 else if(value > 0){
	 	new_val_a = value;
	 	new_key_a = key;
	 }
 	 else {
	 	new_val_a = val_a;
	 	new_key_a = key_a;
 	 }

 	hashA_key[idx_a] = new_key_a;
 	hashA_value[idx_a] = new_val_a;
}


template<int Number>
void query(
		x_type x,
		y_type y,
		key_type hashA_key[N_AU][T_SIZE],
		value_type hashA_value[N_AU][T_SIZE],
		bool interested[N_AU]
){


	key_type key = hash_map<Number>(x, y);
	ap_uint<TABLE_BITS> idx_a = hash_forward_A<Number>(key);

	for(int j = 0; j < N_AU; j++){
#pragma HLS UNROLL
		interested[j] = 0;
	}

	for(int n = 0; n < N_AU; n++){
#pragma HLS UNROLL
		value_type out = 0; 
		
		if(hashA_key[n][idx_a] == key){
			out = hashA_value[n][idx_a];
		}
		if(out > 0){
			interested[n] = 1;
		}
	}

}



template<int Number>
void update_square(
		x_type x[N_AU],
		y_type y[N_AU],
		value_type value,
		key_type hashA_key[N_AU][T_SIZE],
		value_type hashA_value[N_AU][T_SIZE],
		bool interested[N_AU]
){
#pragma HLS DATAFLOW
		hls::stream<ap_int<XBITS + YBITS + VBITS> > queue[N_AU];
#pragma HLS stream variable=queue depth=121
		ap_int<XBITS + YBITS + VBITS> pack = 0;
		ap_int<XBITS + YBITS + VBITS> read = 0;


QL1:	for(int i = -d; i <= d; i++){
QL2:		for(int j = -d; j <= d; j++){
#pragma HLS PIPELINE II=1
				for(int n = 0; n < N_AU; n++){
#pragma HLS UNROLL
					if(interested[n]){
						int x_index = x[n] + i;
						int y_index = y[n] + j;
						if(x_index >= 0 && x_index <= WIDTH - 1 && y_index >= 0 && y_index <= HEIGHT - 1){
							pack.range(VBITS - 1, 0) = (value_type) value;
							pack.range(VBITS + YBITS - 1, VBITS) = (y_type) y_index;
							pack.range(VBITS + YBITS + XBITS - 1, VBITS + YBITS) = (x_type) x_index;
						}
						else{
							pack = 0;
						}
						queue[n].write(pack);
					}
				}
			}
		}

IL1:	for(int i = 0; i < (2 * d + 1) * (2 * d + 1); i++){
#pragma HLS PIPELINE
			for(int n = 0; n < N_AU; n++){
#pragma HLS UNROLL
				if(interested[n]){
					read = queue[n].read();
					if (read == 0) continue;
					value_type v = read.range(VBITS - 1, 0);
					y_type y_i = read.range(VBITS + YBITS - 1, VBITS);
					x_type x_i = read.range(VBITS + YBITS + XBITS - 1, VBITS + YBITS);
					
					insert_2way<Number>(x_i, y_i, v, hashA_key[n], hashA_value[n]);
					#pragma HLS dependence variable=hashA_key inter false
					#pragma HLS dependence variable=hashA_value inter false
				}
			}
		}
}





