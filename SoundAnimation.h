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
    AnimationTimer(float sampleRate);
    
    // int hour();
    // int min();
    // int sec();
    // int milli();
    
    static float calculate(int hh, int mm, int ss, int mss);
    float set(float t);
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
    Action() {}
    Action(const Action& src) : _node(src._node) { }
    Action& operator=(const Action& src) { _node = src._node; return *this; }
    virtual ~Action();
    virtual void perform();
    virtual char *toString();

    SignalNode* _node;
};


class ParameterAutomationAction : public Action
{
public:
    ParameterAutomationAction() {}
    ParameterAutomationAction(const ParameterAutomationAction& src) 
        : Action(src), _targetValue(src._targetValue), _timeToTarget(src._timeToTarget) { }
    ParameterAutomationAction& operator=(const ParameterAutomationAction& src) 
        { _node = src._node; _targetValue = src._targetValue; _timeToTarget = src._timeToTarget; return *this; }
    virtual ~ParameterAutomationAction();
    
    virtual void perform();
    virtual char *toString();

    float _targetValue;
    float _timeToTarget;
};


class PlayFileAction : public Action
{
public:
    
    PlayFileAction(FilePlayerNode*, bool);
    PlayFileAction(const PlayFileAction&);
    PlayFileAction& operator=(const PlayFileAction&);
    virtual ~PlayFileAction();
    virtual void perform();
    virtual char *toString();
    
    bool _playing;
};


class LoopFileAction : public Action
{
public:
    
    LoopFileAction(FilePlayerNode*, bool);
    LoopFileAction(const LoopFileAction&);
    LoopFileAction& operator=(const LoopFileAction&);
    virtual ~LoopFileAction();
    virtual void perform();
    virtual char *toString();
    
    bool _looping;
};


class SoundAnimation;

class RestartAction : public Action
{
public:
    RestartAction(SoundAnimation *anim);
    RestartAction(const RestartAction& src) : Action(src), _animation(src._animation) { }
    RestartAction& operator=(const RestartAction& src) 
        { _node = src._node; _animation = src._animation; return *this; }
    virtual ~RestartAction() {}

    virtual void perform();
    virtual char *toString();

    SoundAnimation* _animation;
};


class Agendum
{
public:
    Agendum() { }
    Agendum(float t, Action *p) { _time = t; _action = p; }
    ~Agendum() { }
    Agendum(const Agendum& src) : _time(src._time), _action(src._action) { } 
    Agendum& operator=(const Agendum& src) { _time = src._time; _action = src._action; return *this; }
    float _time;
    Action *_action;
};


class AnimationSchedule
{
public:
    AnimationSchedule();
    ~AnimationSchedule();
    AnimationSchedule(const AnimationSchedule&);
    AnimationSchedule& operator=(const AnimationSchedule&);
    
    void add(float t, Action *action);
    void pop();
    static bool order (const Agendum& a, const Agendum& b);
    void printSchedule();
    unsigned size();
    float nextTime();
    Action* nextAction();
    
    list<Agendum> _agenda;
    
};


class SoundAnimation
{
public:
    SoundAnimation(const char* instructions, AnimationTimer *stopwatch);
    ~SoundAnimation();
    
    void restart();
    //vector<SoundObject*>soundObjects;
    //vector<SoundBed*>soundBeds;
    vector<FilePlayerNode*>outputNodes;
    vector<ParameterAutomationNode*> automations;
    
    //vector<KeyFrame*>keyFrames;
    //list<agendum>schedule;
    AnimationSchedule schedule;
    AnimationSchedule score;
    vector<FilePlayerNode*>* getOutputNodes();
    
    //void scheduleAction(float time, Action* action);
    
    //void printSchedule();
    AnimationTimer *_clock;


private:
    int parseSoundAnimationFile(const char *filename);
    int parseKeyFrame(char *keyFrame);

    vector<float>previousKeyFrameTimes;    
    float _time;
    //SoundObject* _soundObject;
    //SoundBed* _soundBed;
    FilePlayerNode* _soundObject;
    Action *_action;
    
    unsigned _slot;
    // bool _loop;
    // bool _playing;
    // float _gain;
    // float _angle;
    // float _distance;
    // float _width;
    


};

#endif
