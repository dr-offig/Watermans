/***** SampleStream.h *****/

// TODO: seek/rewind functions

#ifndef SAMPLESTREAM_H_
#define SAMPLESTREAM_H_

#include <SampleData.h>
//#include <string>
#include <Bela.h>

#include <sndfile.h>	// to load audio files
#include <iostream>
#include <cstdlib>

using namespace std;

class SampleStream
{

public:
    
    SampleStream(const char* filename, int bufferLength);
    ~SampleStream();
    SampleStream(const SampleStream&);
    SampleStream& operator=(const SampleStream&);
    int openFile(const char* filename, int bufferLength);
    void fillBuffer();
    void processFrame();
    float getSample(int channel);
    int bufferNeedsFilled();
    void togglePlayback();
    void togglePlaybackWithFade(float fadeLengthInSeconds);
    void togglePlayback(int toggle);
    void togglePlaybackWithFade(int toggle, float fadeLengthInSeconds);
    int isPlaying();
    int shouldLoop();
    int loopStartFrameOffset();
    int loopEndFrameOffset();
    int loopStartTimeOffset();
    int loopEndTimeOffset();
    void setLoopStartTimeOffset(int a);
    void setLoopEndTimeOffset(int b);
    void setShouldLoop(int shouldLoop);
    float fadeLengthInSeconds();
    char* getFilename();
    void setGain(float g);
    void rewind();

    int frames2Milliseconds(int frames);
    int milliseconds2Frames(int millis);

private:

    // private libsndfile wrappers
    int getSamples(const char* file, float *buf, int channel, int startFrame, int endFrame);
    //int getNumChannels(const char* file);
    //int getNumFrames(const char* file);

    // Two buffers for each channel:
    // one of them loads the next chunk of audio while the other one is used for playback
    SampleData *gSampleBuf[2];
    // read pointer relative current buffer (range 0-BUFFER_LEN)
    // initialise at BUFFER_LEN to pre-load second buffer (see render())
    int gReadPtr;
    // read pointer relative to file, increments by BUFFER_LEN (see fillBuffer())
    int gBufferReadPtr;
    // keeps track of which buffer is currently active (switches between 0 and 1)
    int gActiveBuffer;
    // this variable will let us know if the buffer doesn't manage to load in time
    int gDoneLoadingBuffer;
    int gBufferLength;
    int gNumChannelsInFile;
    //int gNumOutputChannels;
    
    int gInputSampleRate;
    int gOutputSampleRate;
    
    int gNumFramesInFile;
    char* gFilename;
    int gBufferToBeFilled;
    int gPlaying;
    int gLooping;
    int gLoopStartFrame;
    int gLoopEndFrame;

    float gFadeAmount;
    float gFadeLengthInSeconds;
    int gFadeDirection;
    float gGain;
    
    int gBusy;
    
    SNDFILE *sndfile ;
	SF_INFO sfinfo ;
    
};

#endif // SAMPLESTREAM_H_