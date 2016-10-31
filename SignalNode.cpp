/***** SignalNode.cpp *****/


#include "SignalNode.h"
#include <Bela.h>
//#include <string.h>


AudioBuffer::AudioBuffer(unsigned channels, uint64_t frames)
    : _channels(channels), _frames(frames)
{
    _data = (float *)calloc(_channels * _frames, sizeof(float));
}


AudioBuffer::AudioBuffer(BelaContext *context)
    : _channels(context->audioOutChannels), _frames(1)
{
    _data = (float *)calloc(_channels * _frames, sizeof(float));
}


AudioBuffer::~AudioBuffer()
{
    if (_data)
        free(_data);
}


AudioBuffer::AudioBuffer(const AudioBuffer& s)
    : _channels(s._channels), _frames(s._frames)
{
    _data = (float *)calloc(_channels * _frames, sizeof(float));
    //memcpy(_data, s._data, _channels * _frames * sizeof(float));
}


AudioBuffer& AudioBuffer::operator=(const AudioBuffer& s)
{
    _channels = s._channels;
    _frames = s._frames;
    
    if (_data)
        free(_data);
    _data = (float *)calloc(_channels * _frames, sizeof(float));
    //memcpy(_data, s._data, _channels * _frames * sizeof(float));
    return *this;    
}


float AudioBuffer::getSample(unsigned channel, uint64_t offset)
{
    return _data[offset * _channels + channel];
}


void AudioBuffer::setSample(unsigned channel, uint64_t offset, float val)
{
    _data[offset * _channels + channel] = val;
}


float* AudioBuffer::getFrame(uint64_t offset)
{
    return _data + offset;
}


void AudioBuffer::setFrame(uint64_t offset, float *values)
{
    for (unsigned ch=0; ch<_channels; ch++) {
        _data[offset * _channels + ch] = values[ch];
    }

}

SignalNode::SignalNode(unsigned channels, float sampleRate) 
    : outputChannels(channels), 
    outputSampleRate(sampleRate), 
    userData(NULL), userDataSize(0),
    outputBuffer(outputChannels,1)
{
    //_inputBuffer = (float *)calloc(_inputBufferChannels * _inputBufferFrames, sizeof(float));
}


SignalNode::SignalNode(BelaContext* context) 
    : outputChannels(context->audioOutChannels), 
    outputSampleRate(context->audioSampleRate), 
    userData(NULL), userDataSize(0),
    outputBuffer(outputChannels,1)
{ 
    //_inputBuffer = (float *)calloc(_inputBufferChannels * _inputBufferFrames, sizeof(float));
}


SignalNode::~SignalNode()
{ 
    if (userData)
        free(userData);

}


SignalNode::SignalNode(const SignalNode& s) 
    : upstreamNodes(s.upstreamNodes),
    outputChannels(s.outputChannels),
    outputSampleRate(s.outputSampleRate),
    outputBuffer(s.outputBuffer)
{ 
    userDataSize = s.userDataSize;    
    userData = malloc(userDataSize);
    //memcpy(userData, s.userData, userDataSize);
    
    //_inputBuffer = (float *)calloc(_inputBufferChannels * _inputBufferFrames, sizeof(float));

}


SignalNode& SignalNode::operator=(const SignalNode& s)
{
    upstreamNodes = s.upstreamNodes;
    outputChannels = s.outputChannels;
    outputSampleRate = s.outputSampleRate;
    if (userData)
        free(userData);

    userDataSize = s.userDataSize;    
    userData = malloc(userDataSize);
    //memcpy(userData, s.userData, userDataSize);    

    // _inputBufferChannels = s._inputBufferChannels;
    // _inputBufferFrames = s._inputBufferFrames;
    // if (_inputBuffer)
    //     free(_inputBuffer);
    // _inputBuffer = (float *)calloc(_inputBufferChannels * _inputBufferFrames, sizeof(float));

    return *this;
}


uint64_t SignalNode::nextFrame()
{
    for (unsigned i=0; i<upstreamNodes.size(); i++)
        upstreamNodes.at(i)->nextFrame();
        
    process();    
    return outputChannels;        
}


ParameterAutomationNode::ParameterAutomationNode(float sampleRate)
    : SignalNode(1,sampleRate),
    _value(0.0f), _target(0.0f), _framesToTarget(0)
{
    // Squiddlypop
}


ParameterAutomationNode::~ParameterAutomationNode()
{ }


ParameterAutomationNode::ParameterAutomationNode(const ParameterAutomationNode& s)
    : SignalNode(s), _value(s._value), _target(s._target), _framesToTarget(s._framesToTarget)
{ }


ParameterAutomationNode& ParameterAutomationNode::operator=(const ParameterAutomationNode& s)
{
    SignalNode::operator=(s);
    _value = s._value;
    _target = s._target;
    _framesToTarget = s._framesToTarget;
    return *this;
}


float ParameterAutomationNode::getValue()
{
    return _value;        
}


void ParameterAutomationNode::setValue(float val)
{
    _value = val;
    recalculateDelta();
}


float ParameterAutomationNode::getTarget()
{
    return _target;
}


void ParameterAutomationNode::setTarget(float target)
{
    _target = target;
    recalculateDelta();
}


uint64_t ParameterAutomationNode::getFramesToTarget()
{
    return _framesToTarget;
}


void ParameterAutomationNode::setFramesToTarget(uint64_t framesToTarget)
{
    _framesToTarget = framesToTarget;
    recalculateDelta();
}


void ParameterAutomationNode::setMillisecondsToTarget(uint64_t millis)
{
    setFramesToTarget ((millis * 1000) / ((int)outputSampleRate));
}


void ParameterAutomationNode::recalculateDelta()
{
    if (_framesToTarget > 0) {
        _delta = (_value - _target) / ((float)_framesToTarget);    
    } else {
        _delta = 0;
    }
}


uint64_t ParameterAutomationNode::process()
{
    if (_framesToTarget <= 0) {
        _value = _target;
    } else {
        _value += _delta;
        _framesToTarget--;
    }
    return 1;
}


void ParameterAutomationNode::prepare()
{
    // Nothing to be done
}


FilePlayerNode::FilePlayerNode(const char* filename, unsigned channels, uint64_t frames)
    : SignalNode(channels, frames), stream(NULL)
{
    stream = new SampleStream(filename, (uint64_t)outputSampleRate);
}


FilePlayerNode::~FilePlayerNode()
{
    if (stream)
        delete(stream);
    
}


FilePlayerNode::FilePlayerNode(const FilePlayerNode& s)
    : SignalNode(s)
{
    char* filename = s.stream->getFilename();    
    stream = new SampleStream(filename, (uint64_t)outputSampleRate);
}


FilePlayerNode& FilePlayerNode::operator=(const FilePlayerNode& s)
{
    SignalNode::operator=(s);
    if (stream)
        delete(stream);
    
    char* filename = s.stream->getFilename();    
    stream = new SampleStream(filename, (uint64_t)outputSampleRate);
    
    return *this;
}


uint64_t FilePlayerNode::process()
{
    // FilePlayer node has no inputs, just grab samples from the stream
    stream->processFrame();
    for (unsigned ch=0; ch<outputChannels; ch++) 
    {
        outputBuffer.setSample(ch,0,stream->getSample(ch));
    }

    return outputChannels;
    
}


void FilePlayerNode::prepare()
{
    if(stream->bufferNeedsFilled())
        stream->fillBuffer();
}
