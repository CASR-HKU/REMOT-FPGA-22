
template<int Number>
 void insert_2way(
 		x_type x,
 		y_type y,
 		value_type value,
 		key_type hashA_key[T_SIZE][T_WIDTH],
 		value_type hashA_value[T_SIZE][T_WIDTH]
 ){
 #pragma HLS INLINE

 	key_type key = hash_map<Number>(x, y);
 	ap_uint<TABLE_BITS> idx_a = hash_forward_A<Number>(key);


 	key_type key_a[T_WIDTH];
 	value_type val_a[T_WIDTH];
 	key_type new_key_a[T_WIDTH];
 	value_type new_val_a[T_WIDTH];
 	bool match[T_WIDTH];


 #pragma HLS ARRAY_PARTITION variable=key_a complete dim=0
 #pragma HLS ARRAY_PARTITION variable=val_a complete dim=0
 #pragma HLS ARRAY_PARTITION variable=new_key_a complete dim=0
 #pragma HLS ARRAY_PARTITION variable=new_val_a complete dim=0
 #pragma HLS ARRAY_PARTITION variable=match complete dim=0

 	//cout<<"\n\n new\n";


 	key_a[0] = hashA_key[idx_a][0];
 	val_a[0] = hashA_value[idx_a][0];
 	key_a[1] = hashA_key[idx_a][1];
 	val_a[1] = hashA_value[idx_a][1];


 	bool full = 1;
 	// cout<<" match ";

 	match[0] = (key_a[0] == key) ? 1 : 0;
 	match[1] = (key_a[1] == key) ? 1 : 0;
 	full = full & (key_a[0] != 0) & (key_a[1] != 0);

 	bool matched = match[0] | match[1];
	
 	if (matched){
 		if(match[0] == 1){
 			new_val_a[0] = val_a[0] + value;
 			new_key_a[0] = key_a[0];
 			new_key_a[1] = key_a[1];
 			new_val_a[1] = val_a[1];
 		}
 		else{
 			new_val_a[1] = val_a[1] + value;
 			new_key_a[1] = key_a[0];
 			new_key_a[0] = key_a[0];
 			new_val_a[0] = val_a[0];
 		}
 	}
 	else if (value > 0) { //hardcoded code for 2 way associated
 		if(val_a[0] <= val_a[1]){
 			new_val_a[0] = value;
 			new_key_a[0] = key;
 			new_val_a[1] = val_a[1];
 			new_key_a[1] = key_a[1];
 		}
 		else{
 			new_val_a[1] = value;
 			new_key_a[1] = key;
 			new_val_a[0] = val_a[0];
 			new_key_a[0] = key_a[0];
 		}
 	}

 	else {
 			new_val_a[0] = val_a[0];
 			new_key_a[0] = key_a[0];
 			new_val_a[1] = val_a[1];
 			new_key_a[1] = key_a[1];
 	}

 	hashA_key[idx_a][0] = new_key_a[0];
 	hashA_value[idx_a][0] = new_val_a[0];
 	hashA_key[idx_a][1] = new_key_a[1];
 	hashA_value[idx_a][1] = new_val_a[1];

}


template<int Number>
value_type query(
		x_type x,
		y_type y,
		key_type hashA_key[T_SIZE][T_WIDTH],
		value_type hashA_value[T_SIZE][T_WIDTH]
){
	key_type key = hash_map<Number>(x, y);
	ap_uint<TABLE_BITS> idx_a = hash_forward_A<Number>(key);
	value_type result = 0;

	for(int i = 0; i < T_WIDTH; i++){
#pragma HLS UNROLL
		if(hashA_key[idx_a][i] == key){
			return hashA_value[idx_a][i];
		}
	}
	return (value_type) 0;

}



template<int Number>
void update_square(
		x_type x,
		y_type y,
		value_type value,
		key_type hashA_key[T_SIZE][T_WIDTH],
		value_type hashA_value[T_SIZE][T_WIDTH]
){
#pragma HLS DATAFLOW
		hls::stream<ap_int<XBITS + YBITS + VBITS> > queue;
#pragma HLS stream variable=queue depth=121
		ap_int<XBITS + YBITS + VBITS> pack = 0;
		ap_int<XBITS + YBITS + VBITS> read = 0;


QL1:	for(int i = -5; i <= 5; i++){
QL2:		for(int j = -5; j <= 5; j++){
#pragma HLS PIPELINE
				int x_index = x + i;
				int y_index = y + j;
				if(x_index >= 0 && x_index <= WIDTH - 1 && y_index >= 0 && y_index <= HEIGHT - 1){
					pack.range(VBITS - 1, 0) = (value_type) value;
					pack.range(VBITS + YBITS - 1, VBITS) = (y_type) y_index;
					pack.range(VBITS + YBITS + XBITS - 1, VBITS + YBITS) = (x_type) x_index;
				}
				else{
					pack = 0;
				}
				queue.write(pack);
			}
		}

IL1:	for(int i = 0; i < 121; i++){
#pragma HLS PIPELINE
			read = queue.read();
			if (read == 0) continue;
			value_type v = read.range(VBITS - 1, 0);
			y_type y_i = read.range(VBITS + YBITS - 1, VBITS);
			x_type x_i = read.range(VBITS + YBITS + XBITS - 1, VBITS + YBITS);
			
			insert_2way<Number>(x_i, y_i, v, hashA_key, hashA_value);
			#pragma HLS dependence variable=hashA_key inter false
			#pragma HLS dependence variable=hashA_value inter false
		}
}





