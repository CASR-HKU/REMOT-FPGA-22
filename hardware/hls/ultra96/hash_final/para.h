#define VBITS 16
#define XBITS 10
#define YBITS 10
#define TBITS 32
#define HEIGHT 180
#define WIDTH 240
#define T_SIZE 8192
#define TABLE_BITS 13 //log2(T_SIZE)
#define T_WIDTH 1
#define KBITS 16 //log2(hight*witdth)
#define OUTBITS 64
#define N_AU 18
#define FIFO_DEPTH 1024
#define PBITS 1
#define d 5

typedef ap_uint<XBITS> x_type;
typedef ap_uint<YBITS> y_type;
typedef ap_uint<TBITS> t_type;
typedef ap_uint<PBITS> p_type;
typedef ap_uint<32> status_type;


typedef struct event{
	x_type x;
	y_type y;
	t_type t;
	p_type p;
} event;

typedef ap_uint<64> event_pack_t;
typedef ap_uint<OUTBITS> out_type;
typedef ap_uint<OUTBITS> hash_type;
typedef ap_uint<OUTBITS> efifo_type;
typedef ap_uint<16> pointer_type;

typedef hls::axis<event_pack_t, 1, 1, 1> event_axi;
typedef hls::stream<event_axi> event_stream;
typedef hls::stream<event_pack_t > local_stream;
typedef hls::stream<out_type > hmap_stream;
typedef hls::stream<out_type > fifo_stream;
typedef hls::stream<int> depth_stream;
typedef ap_uint<KBITS> key_type; 
typedef ap_int<VBITS> value_type; 

