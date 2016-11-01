/***** SignalNode.h *****/

#ifndef SIGNAL_NODE_H
#define SIGNAL_NODE_H

#include "SampleStream.h"
#include <vector>

using namespace std;


class AudioBuffer
{
public:
    AudioBuffer(unsigned channels, uint64_t frames);
    AudioBuffer(BelaContext *context);
    ~AudioBuffer();
    AudioBuffer(const AudioBuffer&);
    AudioBuffer& operator=(const AudioBuffer&);

    float getSample(unsigned channel, uint64_t offset);
    void setSample(unsigned channel, uint64_t offset, float val);
    float *getFrame(uint64_t offset);
    void setFrame(uint64_t offset, float *values);


private:
    unsigned _channels;
    uint64_t _frames;
    float *_data;
    
};


class SignalNode
{
public:
    SignalNode(unsigned channels, float sampleRate);
    SignalNode(BelaContext *context);
    ~SignalNode();
    SignalNode(const SignalNode&);
    SignalNode& operator=(const SignalNode&);
        
    uint64_t nextFrame();
    virtual uint64_t process() = 0;
    virtual void prepare() = 0;
    vector<SignalNode*>upstreamNodes;

    unsigned outputChannels;
    float outputSampleRate;
    void *userData;
    uint64_t userDataSize;

    AudioBuffer outputBuffer;

private:
    
    // uint64_t _inputBufferChannels;  // should be equal to upstreamNode.outputChannels 
    // uint64_t _inputBufferFrames;    // should be equal to 1 unless there is sample rate conversion happening
    // float *_inputBuffer;
};


class ParameterAutomationNode : public SignalNode
{
public:
    ParameterAutomationNode(float sampleRate);
    ~ParameterAutomationNode();
    ParameterAutomationNode(const ParameterAutomationNode&);
    ParameterAutomationNode& operator=(const ParameterAutomationNode&);

    virtual uint64_t process();
    virtual void prepare();

    float getValue();
    void setValue(float val);
    float getTarget();
    void setTarget(float target);
    uint64_t getFramesToTarget();
    void setFramesToTarget(uint64_t framesToTarget);
    void setSecondsToTarget(float secs);


private:
    void recalculateDelta();

    float _value;
    float _target;
    uint64_t _framesToTarget;
    float _delta;
};


class FilePlayerNode : public SignalNode
{
public:
    FilePlayerNode(const char* filename, unsigned channels, uint64_t frames);
    FilePlayerNode(const char* filename, BelaContext *context);
    ~FilePlayerNode();
    FilePlayerNode(const FilePlayerNode&);
    FilePlayerNode& operator=(const FilePlayerNode&);

    virtual uint64_t process();
    virtual void prepare();

    void play();
    void stop();
    void setGain(float g);
    
    char *getFilename();

    SampleStream *stream;

};


#endif


