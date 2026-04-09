//
// Based very heavily off of the "FrameQueue" struct in ffmpeg's ffplay example.
//
// A queue of "frames" from ffmpeg.
//
// A frame contains decoded (decompressed) video or audio data.
// Frames are obtained by decoding packets using the appropriate codec.
//
#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS

#pragma once

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/frame.h>
    #include <libavutil/rational.h>
    #include <libavformat/avformat.h>
}
#include <SDL.h>

struct PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

struct Frame
{
    AVFrame* frame = nullptr;
    AVSubtitle sub;
    int serial = 0;
    double pts = 0.0;
    double duration = 0.0;
    int64_t pos = 0L;
    int width = 0;
    int height = 0;
    AVRational aspectRatio;
    bool uploaded = false;
    bool flipVertical = false;

    Frame();
    ~Frame();
    void Unref();
};

struct FrameQueue
{
    int Init(PacketQueue* pktq, int max_size, bool keep_last);
    void Destroy();
    void Enqueue();
    void Dequeue();
    Frame* Peek() { return &mQueue[(mReadIndex + mReadIndexOffset) % mMaxSize]; }
    Frame* PeekNext() { return &mQueue[(mReadIndex + mReadIndexOffset + 1) % mMaxSize]; }
    Frame* PeekLast() { return &mQueue[mReadIndex]; }
    Frame* PeekWritable();
    Frame* PeekReadable();
    int GetUndisplayedCount() { return mSize - mReadIndexOffset; }
    bool HasReadableFrame() const { return mSize > 0 || mReadIndexOffset != 0; }
    void Signal();
    void LockMutex() { SDL_LockMutex(mMutex); }
    void UnlockMutex() { SDL_UnlockMutex(mMutex); }

private:
    Frame mQueue[FRAME_QUEUE_SIZE];
    int mSize = 0;
    int mMaxSize = 0;
    int mReadIndex = 0;
    int mWriteIndex = 0;
    bool mKeepLastFrame = false;
    int mReadIndexOffset = 0;
    SDL_cond* mSizeChangedCondition = nullptr;
    SDL_mutex* mMutex = nullptr;
    PacketQueue* mPacketQueue = nullptr;
};
