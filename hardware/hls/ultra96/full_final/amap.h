

template<int Number>
void query(
		x_type x,
		y_type y,
		amap_type AMAP[N_AU][HEIGHT * WIDTH],
		bool interested[N_AU]
){


	for(int j = 0; j < N_AU; j++){
#pragma HLS UNROLL
		interested[j] = 0;
	}

	for(int n = 0; n < N_AU; n++){
#pragma HLS UNROLL
		amap_type out = AMAP[n][y * WIDTH + x]; 
		if(out > 0){
			interested[n] = 1;
		}
	}

}


template<int Number>
void update_square(
		x_type x[N_AU],
		y_type y[N_AU],
		amap_type value,
		amap_type AMAP[N_AU][HEIGHT * WIDTH],
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

IL1:	for(int i = 0; i < (2*d+1)*(2*d+1); i++){
#pragma HLS PIPELINE
			for(int n = 0; n < N_AU; n++){
#pragma HLS UNROLL
				if(interested[n]){
					read = queue[n].read();
					if (read == 0) continue;
					value_type v = read.range(VBITS - 1, 0);
					y_type y_i = read.range(VBITS + YBITS - 1, VBITS);
					x_type x_i = read.range(VBITS + YBITS + XBITS - 1, VBITS + YBITS);
					AMAP[n][y_i * WIDTH + x_i] += v; 
					#pragma HLS dependence variable=AMAP inter false
				}
			}
		}
}





