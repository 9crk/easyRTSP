#include<string.h>
#include<stdio.h>
char cmd_option[]="\
RTSP/1.0 200 OK\r\n\
Server: Rtsp Server/2.0\r\n\
CSeq: 5\r\n\
Public: OPTIONS,DESCRIBE,SETUP,PLAY,PAUSE,TEARDOWN,SET_PARAMETER,GET_PARAMETER\r\n\
\r\n";
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
int main()
{
char ss=
"xxxxxxxx%dxxx"
"rrrrrrrrrrr%stt";
sprintf(ss,100, "yyy");
printf("%s", ss);
//	unsigned int i;
//i = getSeqNum(cmd_option);
	//char buff[1000];
//	sscanf("sdfawf<123>waef","sdf%sf<%d>waef",buff, &i);
	printf("%d", i);
	//printf("%d\n", myCmp("1234abcde", "1234abcd"));
}
