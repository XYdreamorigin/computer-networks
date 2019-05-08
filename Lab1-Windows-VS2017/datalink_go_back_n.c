/*#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "protocol.h"
#include "datalink.h"

#define DATA_TIMER  2000
#define ACK_TIMER	240

#define MAX_SEQ 31
#define NR_BUFS ((MAX_SEQ + 1) / 2)
#define inc(k) (k = k < MAX_SEQ ? k + 1 : 0)

static unsigned char buffer[MAX_SEQ + 1][PKT_LEN], nbuffered = 0;//���ʹ��ڴ�С*֡��С�� ���ʹ����Ѿ����͵�֡����
static unsigned char frame_expected = 0, ack_expected = 0, next_frame_to_send = 0;
//��һ�����յ�֡����š���¼��Ҫ���ܵ�ACK����š���¼��һ����Ҫ���͵�֡����š�
static int phl_ready = 0, no_nak = true;


struct FRAME
{
	unsigned char kind; //֡����
	unsigned char ack;  //���� ACK ���
	unsigned char seq;  //֡���
	unsigned char data[PKT_LEN];  //��������ݰ�����
	unsigned int  padding;  //����ֶΣ�CRC У�飩
};



static void put_frame(unsigned char* frame, int len)
{
	*(unsigned int*)(frame + len) = crc32(frame, len);
	send_frame(frame, len + 4);
	phl_ready = 0;
}

static void send_data_frame(void)
{
	struct FRAME s;

	s.kind = FRAME_DATA;
	s.seq = next_frame_to_send;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
	memcpy(s.data, buffer[next_frame_to_send], PKT_LEN); //ָ����c��c++ʹ�õ��ڴ濽��������memcpy�����Ĺ����Ǵ�Դ�ڴ��ַ����ʼλ�ÿ�ʼ�������ɸ��ֽڵ�Ŀ���ڴ��ַ�С�
	//��˼�Ǵ��ڴ�Ϊbuffer��ָ�ĵ�ַ�п���PKT_LEN�����ݵ�s.data��ʼ�ĵ�ַ��

	dbg_frame("Send DATA %d %d, ID %d\n", s.seq, s.ack, *(short*)s.data);

	put_frame((unsigned char*)& s, 3 + PKT_LEN);
	start_timer(next_frame_to_send, DATA_TIMER);//����֮֡�󼴿�ʼ��ʱ
}

static void send_ack_frame(void)
{
	struct FRAME s;

	s.kind = FRAME_ACK;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	dbg_frame("Send ACK  %d\n", s.ack);
	start_ack_timer(ACK_TIMER); //ACK��ʱ����ʼ��ʱ
	put_frame((unsigned char*)& s, 2);
}
static void send_nak_frame(void)
{
	struct FRAME s;

	s.kind = FRAME_NAK;
	s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);

	no_nak = false;
	dbg_frame("Send NAK with ACK %d\n", s.ack);
	stop_ack_timer();
	put_frame((unsigned char*)& s, 2);

}

static int between(unsigned char a, unsigned char b, unsigned char c)
{
	// a <= b < c circularly
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

	while (1)
	{
		event = wait_for_event(&arg);

		switch (event)
		{
		case NETWORK_LAYER_READY:
			get_packet(buffer[next_frame_to_send]);
			nbuffered++;
			send_data_frame();
			inc(next_frame_to_send);
			break;

		case PHYSICAL_LAYER_READY:
			phl_ready = 1;
			break;

		case FRAME_RECEIVED:
			len = recv_frame((unsigned char*)& f, sizeof f);
			if (len < 5 || crc32((unsigned char*)& f, len) != 0)
			{
				dbg_event("**** Receiver Error, Bad CRC Checksum\n");
				break;
			}
			if (f.kind == FRAME_ACK)
				dbg_frame("Recv ACK  %d\n", f.ack);
			if (f.kind == FRAME_DATA)
			{
				dbg_frame("Recv DATA %d %d, ID %d\n", f.seq, f.ack, *(short*)f.data);
				if (f.seq == frame_expected)
				{
					put_packet(f.data, len - 7);
					inc(frame_expected);
				}
				send_ack_frame();
			}
			while (between(ack_expected, f.ack, next_frame_to_send))//f.ack֮ǰ��֡�����յ���ֹͣ��ʱ
			{
				nbuffered--;
				stop_timer(ack_expected);
				inc(ack_expected);
			}
			break;

		case DATA_TIMEOUT:
			dbg_event("---- DATA %d timeout\n", arg);
			next_frame_to_send = ack_expected;
			for (int i = 1; i <= nbuffered; i++)//�ط�֮���֡
			{
				send_data_frame();
				inc(next_frame_to_send);
			}
			break;

		case ACK_TIMEOUT:
			send_ack_frame();
			break;
		}

		if (nbuffered < MAX_SEQ && phl_ready)
			enable_network_layer();
		else
			disable_network_layer();
	}
}

*/