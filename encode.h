#ifndef _ENCODE_H_
#define _ENCODE_H_
#include <unistd.h>
#include <pthread.h>
#include <queue>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaDefs.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>
//#include <media/stagefright/NativeWindowWrapper.h>


#include <time.h>

#define FRAME_SIZE 32768
#define FRAME_COUNT 8

using namespace android;

class Encode
{
public:
	Encode() {
		mVideoFrameCount = 0;
		mBeginTime = 0;
    }
	
	int InitEnVideo();
	int FeedOneH264Frame(unsigned char* frame, int size);

	sp<MediaCodec> mCodec;
	Vector<sp<ABuffer> > mBuffers[2];
	sp<ALooper> mLooper;
	sp<AMessage> mFormat;
	
	int mWidth;
	int mHeight;
	
	void CheckIfFormatChange();

	void Dispose();
	
private:
	void* mPrivate;
	sp<Surface> mSurfaceTextureClient;
	int mVideoFrameCount;
	clock_t mBeginTime;
};
#endif
