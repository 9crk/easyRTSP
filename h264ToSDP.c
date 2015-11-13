#include<stdio.h>
#include<string.h>

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
const char heads[]={0x00, 0x00, 0x00, 0x01};
char head[4];
int readHeader(FILE* tmpFp)
{
    fread(head, 4, 1, tmpFp);
    if(strcmp(head, heads) != 0){
        printf("error header!\n");
        return -1;
    }
    return 0;
}
int readFrame(FILE* tmpFp, char* pBuffer)
{
    int ch;
    int index;
    unsigned int count;

    count = 0;
    index = 0;
    memcpy((void *)pBuffer, (void*)heads, 4);
    pBuffer += 4;
    while(!feof(tmpFp)){
        ch = fgetc(tmpFp);
        *pBuffer = ch;
        pBuffer++;
        count ++;
        if(ch == 0x00 || ch == 0x01){
            head[index] = ch;
            index++;
            if(index == 4 && strcmp(head, heads) == 0){
                break;
            }
        }else{
            index=0;
        }
    }
    return count;
}

int main(int argc, char *argv[])
{
	FILE *fp; 
	if(argc < 2){
		printf("%s xxx.264\n", argv[0]);
		return -1;
	}
	fp = fopen(argv[1], "rb");

	char out[100];	
	char in[100];// = "YWRtaW5hZG1pbg==";
//	Av_Base64_Decode(out, in, 100);

memset(in, 0, 100);
memset(out, 0, 100);
readHeader(fp);

int cnt = readFrame(fp, in);
Av_Base64_Encode(out, 100, &in[4],cnt-4);
printf("%s,", out);
memset(in, 0, 100);
memset(out, 0, 100);
cnt = readFrame(fp, in);
Av_Base64_Encode(out, 100, &in[4],cnt-4);
printf("%s\n", out);
	return;

}
