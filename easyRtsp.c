#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>

char cmd_describe[]=
		"RTSP/1.0 200 OK\r\n"
		"Server: Rtsp Server/2.0\r\n"
		"Cseq: %d\r\n"
		"Content-Type: application/sdp\r\n"
		"Content-Base: rtsp://192.168.1.138:554\r\n"
		"Content-Length: %d\r\n"
		"Cache-Control: must-revalidate\r\n"
		"x-Accept-Dynamic-Rate: 1\r\n"
		"\r\n"
		"%s"
		;
char const* const sdp =
		"v=0\r\n"
		"o=StreamingServer 3433055887 %ld%06ld IN IP4 %s\r\n"
		"s=RTSP Session/2.0\r\n"
		"e=NONE\r\n"
		"c=IN IP4 0.0.0.0\r\n"
		"t=0 0\r\n"
		"m=video 0 RTP/AVP %d\r\n"//96
		"b=AS:70\r\n"
		"a=control:trackID=0\r\n"
		"a=rtpmap:%d %s/90000\r\n"
		"a=fmtp:%d profile-level-id=%s;packetization-mode=1;sprop-parameter-sets=%s\r\n";

char cmd_option[] =
		"RTSP/1.0 200 OK\r\n"
		"Server: Rtsp Server/2.0\r\n"
		"CSeq: %d\r\n"
		"Public: OPTIONS,DESCRIBE,SETUP,PLAY\r\n"//SET_PARAMETER,GET_PARAMETER
		"\r\n";
char cmd_setup[] = 
		"RTSP/1.0 200 OK\r\n"
		"Server: Rtsp Server/2.0\r\n"
		"CSeq: %d\r\n"
		"Session: 546385326158;timeout=60\r\n"
		"Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=20010-20011;ssrc=%x08x\r\n"
		"x-dynamic-rate: 1\r\n"
		"\r\n";
char cmd_play[] =
		"RTSP/1.0 200 OK\r\n"
		"Server: Rtsp Server/2.0\r\n"
		"CSeq: %d\r\n"
		"Session: 546385326158\r\n"
		"RTP-Info: url=trackID=0;seq=242;rtptime=420293225,url=trackID=0;seq=26323;rtptime=1019536746\r\n"
		"Range: npt=0.000000-\r\n"
		"\r\n";


int stop=0;
void stopMe()
{
	stop=1;
}


#define UINT_MAX 0xFFFFFFFF
const unsigned char Map2[] =
{
	0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
	0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
};
/*****************************************************************************
 * 
 * 
 * 
 *****************************************************************************/
int Av_Base64_Decode(unsigned char * out, const char *in, int out_length)
{
	int i, v;
	unsigned char *dst = out;

	v = 0;
	for (i = 0; in[i] && in[i] != '='; i++) {
		unsigned int index= in[i]-43;
		if (index>=(sizeof(Map2)/sizeof(Map2[0])) || Map2[index] == 0xff)
			return -1;
		v = (v << 6) + Map2[index];
		if (i & 3) {
			if (dst - out < out_length) {
				*dst++ = v >> (6 - 2 * (i & 3));
			}
		}
	}

	return dst - out;
}

/*****************************************************************************
 * b64_encode: stolen from VLC's http.c
 * 
 * 
 *****************************************************************************/
static char *Av_Base64_Encode(char * buf, int buf_len, const unsigned char * src, int len)
{
	static const char b64[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *ret, *dst;
	unsigned i_bits = 0;
	int i_shift = 0;
	int bytes_remaining = len;

	if (len >= UINT_MAX / 4 ||
			buf_len < len * 4 / 3 + 12)
		return NULL;
	ret = dst = buf;
	while (bytes_remaining) {
		i_bits = (i_bits << 8) + *src++;
		bytes_remaining--;
		i_shift += 8;

		do {
			*dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
			i_shift -= 6;
		} while (i_shift > 6 || (bytes_remaining == 0 && i_shift > 0));
	}
	while ((dst - ret) & 3)
		*dst++ = '=';
	*dst = '\0';

	return ret;
}

int myCmp(char *CMD, char *buff)
{
    char *p1, *p2;
    p1 = CMD;
    p2 = buff;
    while(*p1 != '\0'){
        if(*p1 != *p2){
            return -1;
        }
        p1++;
        p2++;
    }
    return 0;
}
int getSeqNum(char* buff)
{
    char* p, *q;
    char tmp[10];
    int i;
    p = buff;
    while(*p != '\0'){
        if(*p == 'C' && *(p+1) == 'S' && *(p+2) == 'e' && *(p+3) == 'q'){
            break;
        }else{
            p++;
        }
    }
    q = p;
    i = 0;
    memset(tmp, 0, 10);
    while(*p != '\r'){
        tmp[i] = *p;
        p++;
    }
    sscanf(tmp, " %d", &i);
    return i;
}

int main(int argc,char **argv)
{
	char buff[8192];
char tmpBuff[2000];
char cmdBuff[2000];
int on, ret;
struct timeval timeNow;
#if 1
char out[100];
char in[100] = "YWRtaW5hZG1pbg==";
memset(out, 0, 100);
Av_Base64_Decode(out, in, 100);
printf("%s\n", out);
memset(out, 0, 100);
Av_Base64_Encode(out, 100, "admin;admin",11);
printf("%s\n", out);
return;
#endif

    struct sockaddr_in sevAddr;  
    struct sockaddr_in cliAddr;
	signal(SIGINT, stopMe);
//以上三个sockaddr_in结构体代表的IPV4协议簇,
    bzero(&sevAddr,sizeof(sevAddr));
//bzero是专门设计的一个对给定结构体中的元素置0的接口，类似于初始化
    sevAddr.sin_family = AF_INET;  //AF_INET代表的是传输协议是tcp,udp,etc
    sevAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr函数是将字符串转换成网络字节序的ip地址格式
//对于IP地址的设定还可以不用固定的ip绑定，而让操作系统自己去找，这部分以后在说
    sevAddr.sin_port = htons(atoi(argv[1]));

    int sev_fd,cli_fd;    
    int resault;
    pid_t childPid;
    socklen_t cliAddrSize = sizeof(cliAddr);
    sev_fd = socket(AF_INET,SOCK_STREAM,0);   //创建套接字，这里注意到AF_INET和SOCK_STREAM搭配，这表示用的是IPV4的传输协议
    if(-1 == sev_fd)
    {
        printf("socket set up fail,please check it\n");
        return 0;
    }
    resault = bind(sev_fd,(const struct sockaddr*)(&sevAddr),sizeof(sevAddr)); //绑定套接字到本机地址 
    if(-1 == resault)
    {
        printf("bind socket error ,please check it\n");
        return 0;
    }
    resault = listen(sev_fd,3);  //监听改服务器套接字，看有否有客户端链接上来
    if(-1 == resault)
    {
        printf("listen the socket fail,please check it\n");
        return 0;
    }
//打开端口复用，防止退出占用
on = 1;
ret = setsockopt( sev_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
printf("ret == %d\n", ret);
    while(1)
    {
        cli_fd = accept(sev_fd,(struct sockaddr*)(&cliAddr),&(cliAddrSize));  //接受客户端的请求，这个接口可以顺便获得client的套接字，地址信息，端口号信息等等
	    if(-1 == cli_fd)
        {
            printf("accept client connect error,please check it\n");
        }
        if((childPid = fork()) == 0)   //构建子进程为了实现并发服务器的作用
        {
			printf("ONE!\n");
			while(!stop){
				memset(buff, 0, 8192);
				memset(cmdBuff, 0, sizeof(cmdBuff));
				memset(tmpBuff, 0, sizeof(tmpBuff));
				if(recv(cli_fd, buff, 8192,0) == -1)
				{
					perror("Error in receiving response from server \n");
					exit(1);
				}
				printf("收到！\n");
				if(myCmp("OPTIONS", buff) == 0){
						printf("OPTION命令收到\n");	
						snprintf(cmdBuff, sizeof(cmdBuff), cmd_option, getSeqNum(buff));//返回的CSeq必须和请求的对应
						printf("%s", cmdBuff);
						ret = send(cli_fd, cmdBuff, strlen(cmdBuff), 0);
					}else
				if(myCmp("DESCRIBE", buff) == 0){
						printf("DESCRIBE命令收到\n");
						gettimeofday(&timeNow, NULL);
						snprintf(tmpBuff, sizeof(tmpBuff), sdp, timeNow.tv_sec,timeNow.tv_usec, "0.0.0.0", 96, 96, "H264", 96, "4D0029", "Z00AKdoB4AifllSAAAADAIAAAAMBMCAAfP4AACD1b3vfC8IhGoA=,aM48gA==");
						snprintf(cmdBuff, sizeof(cmdBuff), cmd_describe, getSeqNum(buff), strlen(tmpBuff), tmpBuff);
						printf("%s", cmdBuff);
						ret = send(cli_fd, cmdBuff, strlen(cmdBuff), 0);
					}else
				if(myCmp("SETUP", buff) == 0){
						printf("SETUP命令收到\n");
						snprintf(cmdBuff, sizeof(cmdBuff), cmd_setup, getSeqNum(buff), 8002, 8003, 0);
						printf("%s", cmdBuff);
						ret = send(cli_fd, cmdBuff, strlen(cmdBuff), 0);
					}else
				if(myCmp("PLAY", buff) == 0){
						printf("PLAY命令收到\n");
						snprintf(cmdBuff, sizeof(cmdBuff), cmd_play, getSeqNum(buff));
						printf("%s", cmdBuff);
						ret = send(cli_fd, cmdBuff, strlen(cmdBuff), 0);
					}else
				if(myCmp("GET_FRAMERATE", buff) == 0){
						printf("GET_FPS-------\n");
					}

					
				//printf("%s", buff);
				//close(sev_fd);    //关闭监听套接字
				//exit(0);
			}
        }
		
		if(stop)break;
        close(cli_fd);  //关闭客户端套接字
    }
	close(sev_fd);
}
