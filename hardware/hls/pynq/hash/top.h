#define PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) PRAGMA_SUB(x)

#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "ap_int.h"
#include <iostream>
using namespace std;


#include "para.h"
#include "hash.h"
#include "au_hash.h"

void top(
		event_stream& in_stream,
		hash_type* out_hash,
		hash_type* init_hash,
		efifo_type* out_fifo,
		efifo_type* init_fifo,
		int N,
		int return_n,
		int init_n,
		int in_fifo_depth,
		status_type status_in,
		status_type& status_out
);
