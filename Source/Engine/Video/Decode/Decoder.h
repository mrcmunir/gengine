//
// Based very heavily off of the "Decoder" struct in ffmpeg's ffplay example.
//
// A decoder uses a codec to decode packets from a packet queue.
// Decoded packets are called "Frames" and are put into an appropriate FrameQueue.
//
// A decoder can potentially block indefinitely, so decoding occurs on a separate thread.
//
#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS

#pragma once

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/frame.h>
    #include <libavformat/avformat.h>
}
#include <SDL.h>

struct FrameQueue;
struct PacketQueue;
struct VideoState;

struct Decoder
{
    void Init(AVCodecContext* codecContext, PacketQueue* packetQueue, SDL_cond* emptyQueueCond);
    void Destroy();

    int Start(VideoState* is);
    void Abort(FrameQueue* frameQueue);

    int DecodeFrame(AVFrame* frame, AVSubtitle* sub);

    void SetFlushPts(int64_t flushPts, const AVRational& flushPtsTimeBase)
    {
        mFlushPts = flushPts;
        mFlushPtsTimeBase = flushPtsTimeBase;
    }

    bool ReachedEOF() const;
    AVCodecContext* GetCodecContext() const { return mCodecContext; }
    int GetSerial() const { return mSerial; }

private:
    AVCodecContext* mCodecContext = nullptr;
    PacketQueue* mPacketQueue = nullptr;
    AVPacket mPendingPacket { };
    bool mPacketPending = false;
    SDL_cond* mEmptyQueueCondition = nullptr;
    SDL_Thread* mDecoderThread = nullptr;
    int64_t mFlushPts = AV_NOPTS_VALUE;
    AVRational mFlushPtsTimeBase { 0, 0 };
    int64_t mNextPts = AV_NOPTS_VALUE;
    AVRational mNextPtsTimeBase { 0, 0 };
    int mSerial = -1;
    int mEofSerial = 0;
};
