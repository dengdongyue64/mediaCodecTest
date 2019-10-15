#include <stdio.h>
#include "avplayer.h"
#include "encode.h"
#include <sys/time.h>
class VideoBuffer
{
public:
	VideoBuffer() {
	}
	
	void SetBuffer(unsigned char* buffer) {
		mBuffer = buffer;
		mTotalLength = 0;
	}
	
	void AppendBuffer(unsigned char* buffer, int len) {
		memcpy(mBuffer + mTotalLength, buffer, len);
		mTotalLength += len;
	}
	
	void DisposeOneFrame(int len) {
		memmove(mBuffer, mBuffer + len, mTotalLength - len);
		mTotalLength -= len;
	}
	
	int SearchStartCode() {
		int count = 0;
		for (int i = 4; i < mTotalLength; i++) {
			switch(count) {
				case 0:
				case 1:
			//	case 2:
					if (mBuffer[i] == 0) {
                                                count++;
                                        }else
                                                count = 0;
				break;
				case 2:
			                if (mBuffer[i] == 0) {
                                                count++;
                                        }else if(mBuffer[i] == 1)
						return i-2;
					else
                                                count = 0;
				break;
			
				case 3:
					if (mBuffer[i] == 1) {
						return i - 3;
					} else {
						count = 0;
					}
			}
		}
		
		return 0;
	}
	
	unsigned char* GetBuffer() {
		return mBuffer;
	}

private:
	unsigned char* mBuffer;
	int mTotalLength;
};
enum {
    DEQUEUE_INFO_TRY_AGAIN_LATER            = -1,
    DEQUEUE_INFO_OUTPUT_FORMAT_CHANGED      = -2,
    DEQUEUE_INFO_OUTPUT_BUFFERS_CHANGED     = -3,
};
#define MAX_BUFFER_SIZE (1280 * 1280) 
#define BULK_SIZE 640 * 480 * 3 / 2 //1382400
void * avplayerPreocess(void * argc)
{
	FILE* video_fp = fopen("/sdcard/app_camera720p.h264", "rb");
	unsigned char *data_buffer =(unsigned char *) malloc(MAX_BUFFER_SIZE);
	unsigned char *y;
	AVPlayer * avplayer = (AVPlayer *)argc;
	VideoBuffer buffer;
	buffer.SetBuffer(data_buffer);
	y=buffer.GetBuffer();
	while(true) {
		unsigned char data[BULK_SIZE];
		int len = fread(data, 1, BULK_SIZE, video_fp);
		if (len <= 0)
			break;
		
		buffer.AppendBuffer(data, len);
		
		while(true) {
			int i = buffer.SearchStartCode();
			if (i == 0)
				break;
			printf("%x %x %x %x %x \n",y[0],y[1],y[2],y[3],y[4]);
			avplayer->FeedOneH264Frame(buffer.GetBuffer(), i);
			buffer.DisposeOneFrame(i);
		}
	}
	free(data_buffer);
	fclose(video_fp);
	return NULL;
}

#define PICTURE_SIZE 1280*720*3/2  //(460800) 

void * encodePreocess(void * argc)
{
	Encode * encode = (Encode *)argc;
	int len=0;
	//int i =0;
	struct timeval tv;
	size_t inputIndex =0;
	size_t outputIndex = 0;
	static long timeus = 0;
	status_t err=0;
	size_t size, offset;
    int64_t timeUs =0;
    uint32_t flags;
	FILE* yuv_fp = fopen("/sdcard/app_camera720p.yuv", "rb");
	//FILE* yuv_fp = fopen("/mnt/external_sd/bbb_352x288_420p_30fps_32frames.yuv", "rb");
	printf("start encode \n");	
	unsigned char * data=(unsigned char *) malloc(PICTURE_SIZE+4);
	unsigned char * outdata = NULL;
(void) outdata;
	while(true){
		len = fread(data, 1, PICTURE_SIZE, yuv_fp);
	    printf("******len = %d \n",len);
		gettimeofday(&tv,NULL);
		printf("input second:%ld ,millisecond :%ld \n",tv.tv_sec,tv.tv_usec/1000);
		err = encode->mCodec->dequeueInputBuffer(&inputIndex, -1ll);
		printf("dequeueInputBuffer err= %d \n",err);
		printf("****input Index = %d \n",inputIndex);

		const sp<ABuffer> &in = encode->mBuffers[0].itemAt(inputIndex);
		CHECK_LE(PICTURE_SIZE, in->capacity());
		in->setRange(0, len);

		memcpy(in->data(), data, len);
		timeus++;

		err = encode->mCodec->queueInputBuffer(inputIndex,0,len,timeus,0);
        printf("queueInputBuffer err= %d \n",err);
		
			while(true){
		        err = encode->mCodec->dequeueOutputBuffer(&outputIndex, &offset, &size, &timeUs, &flags, 200 * 1000);
				printf("dequeueOutputBuffer err= %d \n",err);
				printf("****out Index = %d \n",outputIndex);
				gettimeofday(&tv,NULL);
               			 printf("getoutdata second:%ld ,millisecond :%ld \n",tv.tv_sec,tv.tv_usec/1000);
				while(err == OK){
				/*	const sp<ABuffer> &out =encode->mBuffers[1].itemAt(outputIndex); 
					printf("****hhhhhhhhhhhhhhhhhhhhhhhhsize = %d  \n",size);
					outdata= (unsigned char *)malloc(size);

					memcpy(outdata,out->data(),size);
					for(i=0;i<33;i++){
						if(i%10==0)
							printf("\n");
						printf(" %02x ",outdata[i]);

					}*/
					printf("\n");
					encode->mCodec->releaseOutputBuffer(outputIndex);
		            err = encode->mCodec->dequeueOutputBuffer(&outputIndex, &offset, &size, &timeUs, &flags, timeUs);
					//free(outdata);
				}
				break;
			}
		}
	free(data);
	fclose(yuv_fp);
    return NULL;
}
int main()
{
//	AVPlayer avplayer;
//	avplayer.InitVideo();
//	pthread_t pid_t;
	size_t stack_size = 0;
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr);
	ret = pthread_attr_getstacksize(&attr,&stack_size);
	printf("stack_size = %dB,%dk  \n",stack_size,stack_size/1024);
	
      //  pthread_create(&pid_t, NULL, avplayerPreocess, &avplayer);

	Encode encode ;
	encode.InitEnVideo();
        pthread_t enpid_t;
	if(pthread_create(&enpid_t, NULL, encodePreocess, &encode)==-1)
	{	
		printf("create encodePreocess failed \n  ");
		return -2;
	}
	
/*	if (pthread_join(pid_t, NULL))                  
    {
        printf("thread is not exit...\n");
        return -2;
    }
 	avplayer.Dispose(); */

	if (pthread_join(enpid_t, NULL))                  
  	  {
            printf("thread is not exit...\n");
            return -2;
          }
	encode.Dispose();
	return 0;	
}
