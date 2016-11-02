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
    virtual ~SignalNode();
    SignalNode(const SignalNode&);
    SignalNode& operator=(const SignalNode&);
        
    uint64_t nextFrame();
    virtual uint64_t process() = 0;
    virtual void prepare();
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
    ParameterAutomationNode();
    ParameterAutomationNode(float sampleRate);
    virtual ~ParameterAutomationNode();
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
typedef ParameterAutomationNode PAM;






class FilePlayerNode : public SignalNode
{
public:
    FilePlayerNode(const char* filename, unsigned channels, uint64_t frames);
    FilePlayerNode(const char* filename, BelaContext *context);
    virtual ~FilePlayerNode();
    FilePlayerNode(const FilePlayerNode&);
    FilePlayerNode& operator=(const FilePlayerNode&);

    virtual uint64_t process();
    virtual void prepare();

    void play();
    void stop();
    void rewind();
    bool setLooping(bool);
    //void setGain(float g);
    //void automateGain(ParameterAutomationNode *param);
    char *getFilename();
    SampleStream *stream;

    PAM sigGain;
    //PAM sigPan;
    PAM sigAngle;
    PAM sigWidth;

};


/***
class DirectionalPanningNode : public SignalNode
{
public:
    DirectionalPanningNode();
    virtual ~DirectionalPanningNode();
    DirectionalPanningNode(const DirectionalPanningNode&);
    DirectionalPanningNode& operator=(const FilePlayerNode&);

    virtual uint64_t process();
    virtual void prepare();

    void play();
    void stop();
    void rewind();
    bool setLooping(bool);
    //void setGain(float g);
    //void automateGain(ParameterAutomationNode *param);
    char *getFilename();
    SampleStream *stream;

    PAM sigGain;
    PAM sigPan;

};
*/



#endif


