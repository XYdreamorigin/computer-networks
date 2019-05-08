#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "protocol.h"
#include "datalink.h"

#define DATA_TIMER  2000
#define ACK_TIMER	260

#define MAX_SEQ 31
#define NR_BUFS ((MAX_SEQ + 1) / 2)
#define inc(k) (k = k < MAX_SEQ ? k + 1 : 0)

static unsigned char out_buffer[NR_BUFS][PKT_LEN], in_buffer[NR_BUFS][PKT_LEN], nbuffered = 0;//缓存发送的帧。缓存接收的帧。 发送窗口已经发送的帧数量
static unsigned char frame_expected = 0, ack_expected = 0, next_frame_to_send = 0, up_boundary = NR_BUFS;
//下一个接收的帧的序号。记录需要接受的ACK的序号。记录下一个需要发送的帧的序号。标记目前接收的最大上界。
static int phl_ready = 0;
bool received[NR_BUFS],no_nak = true;;//用于标记该帧是否已经收到


struct FRAME
{
	unsigned char kind; //帧种类
	unsigned char ack;  //搭载 ACK 序号
	unsigned char seq;  //帧序号
	unsigned char data[PKT_LEN];  //网络层数据包内容
	unsigned int  padding;  //填充字段（CRC 校验）
};



static void put_frame(unsigned char* frame, int len)
{
	*(unsigned int*)(frame + len) = crc32(frame, len);
	send_frame(frame, len + 4);
	phl_ready = 0;
}

static void send_data_frame(unsigned char frame_nr)
{
	struct FRAME s;

	s.kind = FRAME_DATA;
	s.seq = frame_nr;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
	memcpy(s.data, out_buffer[frame_nr % NR_BUFS], PKT_LEN); //指的是c和c++使用的内存拷贝函数，memcpy函数的功能是从源内存地址的起始位置开始拷贝若干个字节到目标内存地址中。
	//意思是从内存为buffer所指的地址中拷贝PKT_LEN个数据到s.data开始的地址中

	dbg_frame("Send DATA %d %d, ID %d\n", s.seq, s.ack, *(short*)s.data);
	put_frame((unsigned char*)& s, 3 + PKT_LEN);

	start_timer(frame_nr % NR_BUFS, DATA_TIMER);//发送帧之后即开始计时
	stop_ack_timer();
}

//生成ack确认帧并发送到物理层
static void send_ack_frame(void)
{
	struct FRAME s;

	s.kind = FRAME_ACK;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	dbg_frame("Send ACK  %d\n", s.ack);

	put_frame((unsigned char*)& s, 2);
	stop_ack_timer();

}

//生成nak确认帧并发送到确认帧
static void send_nak_frame(void)
{
	struct FRAME s;

	s.kind = FRAME_NAK;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	no_nak = false;
	dbg_frame("Send NAK with ACK %d\n", s.ack);

	put_frame((unsigned char*)& s, 2);
	stop_ack_timer();
}

//如果b在a和c组成的窗口之间，则返回true，否则返回false
static int between(unsigned char a, unsigned char b, unsigned char c)
{
	return (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)));
}

int main(int argc, char** argv)
{
	int event, arg;
	struct FRAME f;
	int len = 0;

	protocol_init(argc, argv);
	lprintf("Designed by Jiang Yanjun, build: " __DATE__"  "__TIME__"\n");

	disable_network_layer();

	for (int i = 0; i < NR_BUFS; i++)
		received[i] = false;

	while (1)
	{
		event = wait_for_event(&arg);

		switch (event)
		{
		case NETWORK_LAYER_READY:
			get_packet(out_buffer[next_frame_to_send % NR_BUFS]);//从网络层获取一帧放入输出缓存中
			nbuffered++;//缓存序号+1
			send_data_frame(next_frame_to_send);//发送该帧
			inc(next_frame_to_send);// 将发送窗口帧序号+1
			break;

		case PHYSICAL_LAYER_READY://当物理层ready时，将phy_ready置为1以便之后enable网络层
			phl_ready = 1;
			break;

		case FRAME_RECEIVED://当物理层收到一帧时
			len = recv_frame((unsigned char*)& f, sizeof f);//从物理层获取一帧

			//校验和出错，发送nak请求重传
			if (len < 5 || crc32((unsigned char*)& f, len) != 0)//如果接收坏帧或CRC校验失败，则返回nak帧
			{
				dbg_event("**** Receiver Error, Bad CRC Checksum\n");
				if (no_nak)
				{
					send_nak_frame();
				}
				break;
			}
			if (f.kind == FRAME_ACK)//如果是ack帧的话，由于所有帧都含有ack帧，因此统一处理
				dbg_frame("Recv ACK  %d\n", f.ack);

			if (f.kind == FRAME_DATA)//如果是data帧
			{
				if ((f.seq != frame_expected) && no_nak)//如果收到的是不需要的帧则返回nak
					send_nak_frame();
				else
					start_ack_timer(ACK_TIMER);

				//如果收到的帧在接收方窗口内且这一帧未被接收过
				if (between(frame_expected, f.seq, up_boundary) && (received[f.seq % NR_BUFS] == false))
				{
					dbg_frame("Recv DATA %d %d, ID %d\n", f.seq, f.ack, *(short*)f.data);
					received[f.seq % NR_BUFS] = true;//标记该帧为已接受
					memcpy(in_buffer[f.seq % NR_BUFS], f.data, PKT_LEN);//将接收到的帧的data域复制至输入缓存中
					while (received[frame_expected % NR_BUFS])//当接收方收到窗口下界的一帧时，对这一帧以及之后收到的帧进行处理
					{
						put_packet(in_buffer[frame_expected % NR_BUFS], len - 7);//将输入缓存送至网络层
						no_nak = true;
						received[frame_expected % NR_BUFS] = false;
						inc(frame_expected);//窗口下界前移一位
						inc(up_boundary);//窗口上界前移一位
						start_ack_timer(ACK_TIMER); //如果ack_timer超时则发送ack帧
					}
				}
			}

			
			if ((f.kind == FRAME_NAK) && between(ack_expected, (f.ack + 1) % (MAX_SEQ + 1), next_frame_to_send))
			{
				send_data_frame((f.ack + 1) % (MAX_SEQ + 1));
				dbg_frame("Recv NAK  %d\n", f.ack);
			}

			//累计确认，只要ack在发送方窗口内就不断地将窗口前移直至未确认的一帧
			while (between(ack_expected, f.ack, next_frame_to_send))
			{
				nbuffered--;
				stop_timer(ack_expected % NR_BUFS);
				inc(ack_expected);
			}
			break;

		case DATA_TIMEOUT://如果收到的帧在接收方窗口内且这一帧未被接收过
			dbg_event("---- DATA %d timeout\n", arg);
			if (!between(ack_expected, arg, next_frame_to_send))
				arg = arg + NR_BUFS;
			send_data_frame(arg);
			break;

		case ACK_TIMEOUT:
			dbg_event("---- ACK %d timeout\n", arg);
			send_ack_frame();
			break;
		}

		if (nbuffered < NR_BUFS && phl_ready)
			enable_network_layer();
		else
			disable_network_layer();
	}
}

