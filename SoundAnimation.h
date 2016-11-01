/***** SoundAnimation.h *****/

#ifndef SOUND_ANIMATION_H
#define SOUND_ANIMATION_H

//#include <KeyFrame.h>
#include <SignalNode.h>
#include <utility>
#include <vector>
#include <list>


using namespace std;

class AnimationTimer
{
public:
    // int hour();
    // int min();
    // int sec();
    // int milli();
    
    static float calculate(int hh, int mm, int ss, int mss);
    float set(int hh, int mm, int ss, int mss);
    float tick();
    float currentTimeInSeconds();
    
    float secondsPerFrame;


private:
    float _timeInSeconds; 
    unsigned _counter;
};



class Action
{
public:
    virtual ~Action();
    virtual void perform() = 0;
    virtual char *toString() = 0;

    SignalNode* _node;
};


class ParameterAutomationAction : public Action
{
public:
    virtual void perform();
    virtual ~ParameterAutomationAction();
    virtual char *toString();

    float _targetValue;
    float _timeToTarget;
};


class PlayFileAction : public Action
{
public:
    PlayFileAction(FilePlayerNode*, bool);
    virtual ~PlayFileAction();
    virtual void perform();
    virtual char *toString();
    
    bool _playing;
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
    list<pair<float,Action*> >schedule;
    vector<FilePlayerNode*>* getOutputNodes();
    
    void printSchedule();


private:
    int parseSoundAnimationFile(const char *filename);
    int parseKeyFrame(char *keyFrame);
    
    float _time;
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
