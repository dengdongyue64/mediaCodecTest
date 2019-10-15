#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <android/native_window.h>

#include "encode.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int Encode::InitEnVideo()
{
	mWidth = SCREEN_WIDTH;
	mHeight = SCREEN_HEIGHT;
	//mSurfaceTextureClient.clear();
	ProcessState::self()->startThreadPool();
	status_t err;
	mFormat = new AMessage;
	
	mLooper = new ALooper;
	mLooper->setName("my_encode");
	mLooper->start();
	
	mCodec = MediaCodec::CreateByType(mLooper, "video/avc", true);
	if (mCodec == NULL) {
        printf("ERROR: unable to create %s codec instance\n","video/avc");
        return UNKNOWN_ERROR;
    }
    
    sp<AMessage> format = new AMessage;
    format->setString("mime", "video/avc");
	format->setInt32("encoder",true);  //new  add 
    format->setInt32("width", mWidth);
    format->setInt32("height", mHeight);
	format->setInt32("bitrate", 600000);
	format->setInt32("frame-rate", 15);
	format->setInt32("color-format", 19);
	format->setInt32("i-frame-interval", 30);

   // err=mCodec->configure(format,NULL, NULL, MediaCodec::CONFIGURE_FLAG_ENCODE);
   err=mCodec->configure(format,NULL, NULL, 0);  //new  add
	if (err != NO_ERROR) {
        fprintf(stderr, "ERROR: unable to configure %s codec at %dx%d (err=%d)\n",
                "video/avc",mWidth , mHeight, err);
        mCodec->release();
        return err;
    }
				
    err= mCodec->start();
	if (err != NO_ERROR) {
        fprintf(stderr, "ERROR: unable to start codec (err=%d)\n", err);
        mCodec->release();
        return err;
    }

	err = mCodec->getInputBuffers(&mBuffers[0]);
    CHECK_EQ(err, (status_t)OK);
    
    err = mCodec->getOutputBuffers(&mBuffers[1]);
    CHECK_EQ(err, (status_t)OK);	
	
    return 0;
}
/*
void Encode::CheckIfFormatChange()
{
	mCodec->getOutputFormat(&mFormat);
		
	int width, height;
	if (mFormat->findInt32("width", &width) &&
		mFormat->findInt32("height", &height)) {
		float scale_x = (SCREEN_WIDTH + 0.0) / width;
		float scale_y = (SCREEN_HEIGHT + 0.0) / height;
		float scale = (scale_x < scale_y) ? scale_x : scale_y;
		
		scale = (scale > 1) ? 1 : scale;
		
		if (scale < 1) {
			int new_width = width * scale;
			int new_height = height * scale;
			
			new_width = (new_width > SCREEN_WIDTH) ? SCREEN_WIDTH : new_width;
			new_height = (new_height > SCREEN_HEIGHT) ? SCREEN_HEIGHT : new_height;
			
			width = new_width;
			height = new_height;
		}
		
		if (width > SCREEN_WIDTH)
			width = SCREEN_WIDTH;
		
		if (height > SCREEN_HEIGHT)
			height = SCREEN_HEIGHT;
		
		if (width != mWidth || height != mHeight) {
			mWidth = width;
			mHeight = height;
			
			int x = (SCREEN_WIDTH - width) / 2;
			int y = (SCREEN_HEIGHT - height) / 2;
		}
	}	
}*/
/*
void Encode::RenderFrames()
{
	size_t index, offset, size;
	int64_t pts;
	uint32_t flags;
	
	int err;
	
	do {
		CheckIfFormatChange();
		
		err = mCodec->dequeueOutputBuffer(
			&index, &offset, &size, &pts, &flags);

		if (err == OK) {
			mCodec->renderOutputBufferAndRelease(
					index);
			
			mVideoFrameCount++;
			if (mBeginTime == 0) {
				mBeginTime = clock();
			} else {
				float fps = mVideoFrameCount / (float(clock() - mBeginTime) / CLOCKS_PER_SEC);
				printf("### %f\n", fps);
			}
		}
	} while(err == OK
                || err == INFO_FORMAT_CHANGED
                || err == INFO_OUTPUT_BUFFERS_CHANGED);
}*/

int Encode::FeedOneH264Frame(unsigned char* frame, int size)
{
	size_t index;

	int err = mCodec->dequeueInputBuffer(&index, -1ll);
	
	CHECK_EQ(err, (status_t)OK);
	
	const sp<ABuffer> &dst = mBuffers[0].itemAt(index);

	CHECK_LE(size, dst->capacity());
	
	dst->setRange(0, size);
	memcpy(dst->data(), frame, size);
	
	err = mCodec->queueInputBuffer(
					index,
					0,
					size,
					0ll,
					0);
	return err;
}
	
void Encode::Dispose()
{
	mCodec->stop();
	mCodec->reset();
	mCodec->release();
	mLooper->stop();
}

