#ifndef __PROTOCOL_fr12hn_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define VERSION "4.0"

#ifndef	_CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>

#include "lprintf.h"

/* Initalization */ 
extern void protocol_init(int argc, char **argv);//Э�黷���ĳ�ʼ�����ú����Ĳ������봫��main()����������ͬ������

/* Event Driver */
extern int wait_for_event(int *arg);  //���̵ȴ���ֱ����һ�����¼��������������ǿ��ܵ� 5���¼�������arg���ڻ���ѷ������¼��������Ϣ���������¼� 4 �� 5

#define NETWORK_LAYER_READY  0   //�¼�1    //��������µķ�����Ҫ���Ͳ���δ����·��disable��������¼�
#define PHYSICAL_LAYER_READY 1	 //�¼�2    //
#define FRAME_RECEIVED       2	 //�¼�3    //������յ���һ��֡
#define DATA_TIMEOUT         3	 //�¼�4    //��ʱ����ʱ������arg���س�ʱ�Ķ�ʱ�����
#define ACK_TIMEOUT          4	 //�¼�5    //ACK��ʱ����ʱ

/* Network Layer functions */
#define PKT_LEN 256

extern void enable_network_layer(void);  //������·���ڻ����������������޷����ͷ���ʱͨ���ú���֪ͨ�����
extern void disable_network_layer(void);  //���ܹ��н��µķ�������ʱִ�иú���
extern int  get_packet(unsigned char *packet);  //����������Ҫ���ͷ��鵫�Ǳ�������·��disable������������л�������͵ķ��飬
												//������·����øú������ú��������鿽����ָ��packetָ���Ļ������У���Ϊ���鳤�Ȳ��ȣ����Ըú���
												//����ֵΪ���鳤��
extern void put_packet(unsigned char *packet, int len);//���������ֱ����յ��ķ��黺�����׵�ַ�ͷ��鳤��

/* Physical Layer functions */
extern int  recv_frame(unsigned char *buf, int size);//����������һ֡��sizeΪ���ڴ�Ž���֡�Ļ�����buf�Ŀռ��С������ֵΪ�յ�֡��ʵ�ʳ���
extern void send_frame(unsigned char *frame, int len);//���ڽ��ڴ�frame������Ϊlen�Ļ�������������㷢��Ϊһ֡��ÿ�ֽڷ�����Ҫ1ms��֡��֮֡��ı߽籣��1ms

extern int  phl_sq_len(void);//���ص�ǰ�������еĳ��ȣ�������г��ȴ���������·����Ҫ���͵����ݳ��ȣ�������¼�2��������ȴ�ֱ�������㹻�Ż�����¼�2

/* CRC-32 polynomium coding function */
extern unsigned int crc32(unsigned char *buf, int len);//CRCУ�飬ָ��bufָ��Ļ���������len�����ֽڵ����ݣ�������������247�ֽڵ���Ч�ռ䣬
														//��֤У��ͣ�ֻ��Ҫ�ж�crc32(p,len+?),pΪָ�򻺴�����ָ�룬lenΪ���ݳ��ȣ�?Ϊ��Ҫ����ĳ��ȣ�
														//���len=243,��?=4����ΪҪ��֤�����ܺ�Ϊ247������ֵΪ32���ص�У���

/* Timer Management functions */
extern unsigned int get_ms(void);//��ȡ��ǰ��ʱ�����꣬��λΪms
extern void start_timer(unsigned int nr, unsigned int ms);//����һ����ʱ���������ֱ�Ϊ��ʱ����źͳ�ʱʱ��ֵ������ʱ�����ȣ������ֻ������0-63�У���ʱ����������¼�4
extern void stop_timer(unsigned int nr);//��ֹһ����ʱ��������Ϊ��ʱ�����
extern void start_ack_timer(unsigned int ms);//ACK��ʱ������ms��ʼ��ʱ������Ϊ��ʼ��ʱʱ��
extern void stop_ack_timer(void);//ֹͣ��ʱ

/* Protocol Debugger */
extern char *station_name(void);

extern void dbg_event(char *fmt, ...);
extern void dbg_frame(char *fmt, ...);
extern void dbg_warning(char *fmt, ...);

#define MARK lprintf("File \"%s\" (%d)\n", __FILE__, __LINE__)

#ifdef  __cplusplus
}
#endif

#endif
