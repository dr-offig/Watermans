/***** SoundAnimation.h *****/

#ifndef SOUND_ANIMATION_H
#define SOUND_ANIMATION_H

//#include <KeyFrame.h>
#include <SignalNode.h>
#include <utility>
#include <vector>
#include <list>



using namespace std;

class Clock
{
public:
    int hour();
    int min();
    int sec();
    int milli();
    
    static uint64_t calculate(int hh, int mm, int ss, int mss);
    uint64_t set(int hh, int mm, int ss, int mss);

private:
    uint64_t _ticks; 
};



class Action
{
public:
    void perform();
    ParameterAutomationNode* param;
    float targetValue;
    uint64_t timeToTarget;
};


class SoundAnimation
{
public:
    SoundAnimation(const char* instructions);
    //~SoundAnimation();
    
    //vector<SoundObject*>soundObjects;
    //vector<SoundBed*>soundBeds;
    vector<FilePlayerNode*>outputNodes;
    vector<ParameterAutomationNode*> automations;
    
    //vector<KeyFrame*>keyFrames;
    list<pair<uint64_t,Action*> >schedule;
    vector<FilePlayerNode*>* getOutputNodes();


private:
    int parseSoundAnimationFile(const char *filename);
    int parseKeyFrame(char *keyFrame);
    
    uint64_t _time;
    //SoundObject* _soundObject;
    //SoundBed* _soundBed;
    FilePlayerNode* _soundObject;
    Action *_action;
    bool _loop;
    bool _playing;
    float _gain;
    float _angle;
    float _distance;
    float _width;

};


#endif
