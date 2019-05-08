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
extern void protocol_init(int argc, char **argv);//协议环境的初始化，该函数的参数必须传递main()函数的两个同名函数

/* Event Driver */
extern int wait_for_event(int *arg);  //进程等待，直到有一个“事件”发生，下面是可能的 5种事件，参数arg用于获得已发生的事件的相关信息，仅用于事件 4 和 5

#define NETWORK_LAYER_READY  0   //事件1    //网络层有新的分组需要发送并且未被链路层disable会产生该事件
#define PHYSICAL_LAYER_READY 1	 //事件2    //
#define FRAME_RECEIVED       2	 //事件3    //物理层收到了一整帧
#define DATA_TIMEOUT         3	 //事件4    //定时器超时，参数arg返回超时的定时器编号
#define ACK_TIMEOUT          4	 //事件5    //ACK定时器超时

/* Network Layer functions */
#define PKT_LEN 256

extern void enable_network_layer(void);  //数据链路层在缓存区满等条件下无法发送分组时通过该函数通知网络层
extern void disable_network_layer(void);  //在能够承接新的发送任务时执行该函数
extern int  get_packet(unsigned char *packet);  //如果网络层需要发送分组但是被数据链路层disable，则网络层自行缓存待发送的分组，
												//数据链路层调用该函数，该函数将分组拷贝到指针packet指定的缓存区中，因为分组长度不等，所以该函数
												//返回值为分组长度
extern void put_packet(unsigned char *packet, int len);//两个参数分别是收到的分组缓存区首地址和分组长度

/* Physical Layer functions */
extern int  recv_frame(unsigned char *buf, int size);//从物理层接收一帧，size为用于存放接受帧的缓存区buf的空间大小，返回值为收到帧的实际长度
extern void send_frame(unsigned char *frame, int len);//用于将内存frame处长度为len的缓存区块向物理层发送为一帧，每字节发送需要1ms，帧与帧之间的边界保留1ms

extern int  phl_sq_len(void);//返回当前物理层队列的长度，如果队列长度大于数据链路层需要发送的数据长度，则产生事件2，否则需等待直到队列足够才会产生事件2

/* CRC-32 polynomium coding function */
extern unsigned int crc32(unsigned char *buf, int len);//CRC校验，指针buf指向的缓存区中有len长度字节的数据，缓存区至少有247字节的有效空间，
														//验证校验和，只需要判断crc32(p,len+?),p为指向缓存区的指针，len为数据长度，?为需要补充的长度，
														//如果len=243,则?=4，因为要保证长度总和为247，返回值为32比特的校验和

/* Timer Management functions */
extern unsigned int get_ms(void);//获取当前的时间坐标，单位为ms
extern void start_timer(unsigned int nr, unsigned int ms);//启动一个定时器，参数分别为定时器编号和超时时间值（即定时器长度），编号只允许在0-63中，超时发生后产生事件4
extern void stop_timer(unsigned int nr);//终止一个定时器，参数为定时器编号
extern void start_ack_timer(unsigned int ms);//ACK定时器，从ms开始计时，参数为开始计时时间
extern void stop_ack_timer(void);//停止计时

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
