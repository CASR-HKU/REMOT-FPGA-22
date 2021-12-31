#define VBITS 16
#define XBITS 10
#define YBITS 10
#define TBITS 32
#define HEIGHT 180
#define WIDTH 240
#define KBITS 16 //log2(hight*witdth)
#define OUTBITS 64
#define N_AU 12
#define FIFO_DEPTH 128
#define PBITS 1
#define PI_0 4
#define dadd 2




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
typedef ap_uint<64> efifo_type;
typedef ap_uint<16> pointer_type;

typedef hls::axis<event_pack_t, 1, 1, 1> event_axi;
typedef hls::stream<event_axi> event_stream;
typedef hls::stream<event_pack_t> local_stream;
typedef hls::stream<efifo_type > fifo_stream;
typedef hls::stream<int> depth_stream;





