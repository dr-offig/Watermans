/***** SoundAnimation.cpp *****/
#define MAX_LINE_LENGTH 256

#include "SoundAnimation.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

//enum AnimationObjectType { ANIM_OBJ_TYPE_INVALID, ANIM_OBJ_TYPE_DIRECTIONAL, ANIM_OBJ_TYPE_FIXED };

AnimationTimer::AnimationTimer(float sampleRate)
{
    if (sampleRate > 0.0)
        secondsPerFrame = 1.0f / sampleRate;
    else
        secondsPerFrame = 1.0f / 44100.0f;
    
}

float AnimationTimer::calculate(int hh, int mm, int ss, int mss)
{
    int millis = ((hh * 60 + mm) * 60 + ss) * 1000 + mss;
    float mf = (float)millis;
    return mf / 1000.0f;
}


float AnimationTimer::set(int hh, int mm, int ss, int mss)
{
    _timeInSeconds = AnimationTimer::calculate(hh,mm,ss,mss);
    return _timeInSeconds;
}


float AnimationTimer::set(float t)
{
    _timeInSeconds = t;
    return _timeInSeconds;
}


/*
int AnimationTimer::milli()
{
    return _timeInSeconds - floor(_timeInSeconds);
}


int AnimationTimer::sec()
{
    return 0.0f;
}


int AnimationTimer::hour()
{
    return 0.0f;
}


int AnimationTimer::min()
{
    return 0.0f;
}
*/

float AnimationTimer::tick()
{
    //int a = floor(_timeInSeconds);
    _timeInSeconds += secondsPerFrame;
    //int b = floor(_timeInSeconds);
    
    //if (a < b)
    //    printf("Time: %d\n", b);
        
    return _timeInSeconds;
}


float AnimationTimer::currentTimeInSeconds()
{
    return _timeInSeconds;
}



Action::~Action()
{
    // nothing special to be done here
}


void Action::perform()
{
    // shouldn't actually be calling from this abstract class
    printf("Error: Action is an abstract class, shouldn't reach this function\n");
}


char *Action::toString()
{
    char *f = (char *)malloc(32);
    sprintf(f,"Error: Action is an abstract class\n");    
    return f;
}


ParameterAutomationAction::~ParameterAutomationAction()
{
    // nothing special to be done here
}


void ParameterAutomationAction::perform()
{
    ParameterAutomationNode *param = dynamic_cast<ParameterAutomationNode*>(_node);
    param->setTarget(_targetValue);
    param->setSecondsToTarget(_timeToTarget);
}


char *ParameterAutomationAction::toString()
{
    char *output = (char *)malloc(64);
    sprintf(output, "Target: %2.2f\tTime To Target: %2.2f", _targetValue, _timeToTarget);
    return output;
}


PlayFileAction::PlayFileAction(FilePlayerNode* node, bool flag)
{
    _node = node;
    _playing = flag;
}


PlayFileAction::PlayFileAction(const PlayFileAction& src) : Action(src), _playing(src._playing)
{

}


PlayFileAction& PlayFileAction::operator=(const PlayFileAction& src)
{
    _node = src._node;
    _playing = src._playing;
    return *this;
}


PlayFileAction::~PlayFileAction()
{
    // nothing special to be done here
}

void PlayFileAction::perform()
{
    FilePlayerNode *player = dynamic_cast<FilePlayerNode*>(_node);
    _playing ? player->play() : player->stop();
    
    char *fname = player->getFilename();
    if (_playing)
        printf("Playing file %s", fname);
    else
        printf("Stopping file %s", fname);
    
}


char *PlayFileAction::toString()
{
    FilePlayerNode* pfnode = dynamic_cast<FilePlayerNode*>(_node);
    char *fname = pfnode->getFilename();
    char *output = (char *)malloc(strlen(fname) + 32);
    if (_playing)
        sprintf(output, "Play file %s", fname);
    else
        sprintf(output, "Stop file %s", fname);
        
    return output;
}


LoopFileAction::LoopFileAction(FilePlayerNode* node, bool flag)
{
    _node = node;
    _looping = flag;
}


LoopFileAction::LoopFileAction(const LoopFileAction& src) : Action(src), _looping(src._looping)
{

}


LoopFileAction& LoopFileAction::operator=(const LoopFileAction& src)
{
    _node = src._node;
    _looping = src._looping;
    return *this;
}


LoopFileAction::~LoopFileAction()
{
    // nothing special to be done here
}

void LoopFileAction::perform()
{
    FilePlayerNode *player = dynamic_cast<FilePlayerNode*>(_node);
    player->setLooping(_looping);
}


char *LoopFileAction::toString()
{
    FilePlayerNode* pfnode = dynamic_cast<FilePlayerNode*>(_node);
    char *fname = pfnode->getFilename();
    char *output = (char *)malloc(strlen(fname) + 32);
    if (_looping)
        sprintf(output, "Looping file %s", fname);
    else
        sprintf(output, "Not looping file %s", fname);
        
    return output;
}




RestartAction::RestartAction(SoundAnimation* animation)
{
    _animation = animation;
}


void RestartAction::perform()
{
    
    _animation->restart();
    // _animation->schedule = _animation->score;
    // _animation->_clock->set(0.0f);
    // printf("Restarting animation now\n");
    // _animation->schedule.printSchedule();
}


char *RestartAction::toString()
{
    char *output = (char *)malloc(32);
    sprintf(output, "Restart animation\n");
    return output;
}


AnimationSchedule::AnimationSchedule()
{
    // nothing to do here    
}

AnimationSchedule::~AnimationSchedule()
{
    // nothing to do here
}


AnimationSchedule::AnimationSchedule(const AnimationSchedule& src) : _agenda(src._agenda)
{
    // nothing to do here
}



AnimationSchedule& AnimationSchedule::operator=(const AnimationSchedule& src)
{
    _agenda = src._agenda;
    return *this;
}


void AnimationSchedule::add(float t, Action *action)
{
    _agenda.push_back(Agendum(t,action));
    _agenda.sort(AnimationSchedule::order);
    
}


float AnimationSchedule::nextTime()
{
    return _agenda.front()._time;
}


Action* AnimationSchedule::nextAction()
{
    return _agenda.front()._action;
}


void AnimationSchedule::pop()
{
    _agenda.pop_front();
}


unsigned AnimationSchedule::size()
{
    return _agenda.size();
}


bool AnimationSchedule::order (const Agendum& a, const Agendum& b)
{
  return ( a._time < b._time );
}


void AnimationSchedule::printSchedule()
{
    
    printf ("---------- Schedule ----------\n");
    list<Agendum>::iterator p;
    for (p = _agenda.begin(); p != _agenda.end(); p++) 
    {
        char *f = p->_action->toString();
        printf("Time: %2.2f\t Action: %s\n", p->_time, f);
        if (f) 
            free (f);
        
    }

}


SoundAnimation::SoundAnimation(const char *instructions, AnimationTimer *stopwatch) : _clock(stopwatch)
{
    // read in the file
    int result = parseSoundAnimationFile(instructions);
    if (result < 0)
        printf("Problem reading in instructions\n");
    
}


SoundAnimation::~SoundAnimation()
{
    list<Agendum>::iterator p;
    for (p = score._agenda.begin(); p != score._agenda.end(); p++) 
    {
        delete(p->_action);
    }
    
    vector<FilePlayerNode*>::iterator q;
    for (q = outputNodes.begin(); q != outputNodes.end(); q++) 
    {
        delete(*q);
    }
}


void SoundAnimation::restart()
{
    // Rewind all the files
    vector<FilePlayerNode*>::iterator q;
    for (q = outputNodes.begin(); q != outputNodes.end(); q++) 
    {
        FilePlayerNode *node = *q;
        node->rewind();
    }

    schedule = score;
    _clock->set(0.0f);
    printf("Restarting animation now\n");
    schedule.printSchedule();
    
    
}


int SoundAnimation::parseKeyFrame(char *keyFrame)
{

    // entries are separated by spaces
    int hour, minute, second, millisecond;
    
    // First entry is the time
    char *time = strtok(keyFrame," ");
    if (sscanf(time,"%d:%d:%d.%d", &hour, &minute, &second, &millisecond) == 4) {
        // Do sequencing stuff
        _time = AnimationTimer::calculate(hour, minute, second, millisecond);
        //printf("Key Frame at %02d:%02d:%02d.%03d\t", hour, minute, second, millisecond);
        printf("Key Frame at %2.2f\n",_time);
    } else {
        printf("Couldn't understand key frame time description - %s", time);
        return -1;
    }
    
    //AnimationObjectType objectType = ANIM_OBJ_TYPE_INVALID;
    //AnimationObject *obj = NULL;
    _soundObject = NULL;
    _action = NULL;

    // Second entry should be either a SOUND_OBJECT or the RESTART instructions
    char *element = strtok(NULL," ");
    if (strncmp(element,"OBJEC",5) == 0)
    {
        //unsigned slot;
        if (sscanf(element,"OBJECT[%d]", &_slot) == 1)
        {
            //objectType = ANIM_OBJ_TYPE_DIRECTIONAL;
            _slot--;
            if (0 <= _slot) 
            {
                printf("Object Slot read in as #%d\t", _slot);                    
                if (_slot < outputNodes.size())
                {
                    printf("Sound Object #%d\t", _slot);                    
                    _soundObject = outputNodes[_slot];
                    //_previousTime = previousKeyFrameTimes[slot];
                    //obj = (AnimationObject *)_soundObject;
                } 
                else if (_slot == outputNodes.size()) 
                {
                    printf("Creating new object slot #%d\t", _slot);                    
                    // Create a new sound object - must look for filename
                    char *field = strtok(NULL," ");
                    char *tmp;
                    if ((tmp = strchr(field,'=')))
                    {
                        *tmp = '\0';
                        char *name = field;
                        char *value = tmp + 1;
                        
                        if (strcmp(name,"FILE")==0) 
                        {
                            if (strlen(value) < 200)
                            {
                                char *filename = (char *)malloc(256);
                                strcpy(filename, "./samples/");
                                filename = strcat(filename,value);
                                _soundObject = new FilePlayerNode(filename,2,44100);
                                outputNodes.push_back(_soundObject);
                                previousKeyFrameTimes.push_back(_time);
                                //_previousTime = _time;
                                //obj = (AnimationObject *)_soundObject;
                                free(filename);
                            }
                            else
                            {
                                printf("Filename too long: %s\n", value);
                            }
                        } 
                        else
                        {
                            _soundObject = NULL;
                            printf("Couldn't understand filename: %s\n", field);
                        }
                        printf("%s=%s\t",name,value);
                    } 
                    else 
                    {
                        printf("Sound file setting needs an 'equals' sign: %s\n", tmp);
                    }

                }
                else
                {
                    printf ("Bad slot %d\n", _slot);
                }

            }
            else
            { 
                printf ("Bad slot %d\n", _slot);
            } 
        } 
        else
        {
            printf("Couldn't understand instruction: %s\n", element);
            return -1;
        }
    } 
    else if (strncmp(element,"RESTA",5) == 0) 
    {
        // Start the whole animation again. i.e. reset the clock
        printf("Looping back to the beginning of the animation\n");
        RestartAction *raction = new RestartAction(this);
        score.add(_time,raction);
        _clock->set(0.0f);
    }
    else
    {
        printf("Unrecognised command: %s\n", element);
        return -1;
    }
    
    if (_soundObject) 
    {
        //_action = new Action();
        //_action->timeToTarget = _time;

        char *field;
        while ((field = strtok(NULL," ")))
        {
            char *tmp;
            if ((tmp = strchr(field,'=')))
            {
                *tmp = '\0';
                char *name = field;
                char *value = tmp + 1;
                printf("%s=%s\t",name,value);
                                
                if (strcmp(name,"PLAY") == 0) {
                    bool flag = (strncmp(value,"ON",2) == 0);
                    PlayFileAction *playFileAction = new PlayFileAction(_soundObject,flag);
                    score.add(_time,playFileAction);
                    
                } else if (strcmp(name,"GAIN") == 0) {
                    float g = 0.0f;
                    if (sscanf(value, "%f", &g) == 1) {
                        ParameterAutomationAction *paction = new ParameterAutomationAction();
                        paction->_node = &(_soundObject->sigGain);
                        paction->_targetValue = g;
                        float _previousTime = previousKeyFrameTimes[_slot];
                        paction->_timeToTarget = _time - _previousTime;
                        score.add(_previousTime, paction);
                        previousKeyFrameTimes[_slot] = _time;
                        
                        //_soundObject->sigGain.setTarget(g);
                        //_soundObject->sigGain.setFramesToTarget(0);
                        //printf("Setting gain for %s to %2.2f\n", _soundObject->getFilename(),g);
                        //_action->param = param;
                        //_action->targetValue = g;
                    }
                        
                } else if (strcmp(name,"LOOP") == 0) {
                    bool flag = (strncmp(value,"ON",2) == 0);
                    LoopFileAction *loopFileAction = new LoopFileAction(_soundObject,flag);
                    score.add(_time,loopFileAction);
                
                    
                } else if (strcmp(name,"ANGLE") == 0) {
                    float theta = 0.0f;
                    if (sscanf(value, "%f", &theta) == 1) {
                        ParameterAutomationAction *paction = new ParameterAutomationAction();
                        paction->_node = &(_soundObject->sigAngle);
                        paction->_targetValue = theta;
                        float _previousTime = previousKeyFrameTimes[_slot];
                        paction->_timeToTarget = _time - _previousTime;
                        score.add(_previousTime, paction);
                        previousKeyFrameTimes[_slot] = _time;
                        
                        //_soundObject->sigGain.setTarget(g);
                        //_soundObject->sigGain.setFramesToTarget(0);
                        //printf("Setting gain for %s to %2.2f\n", _soundObject->getFilename(),g);
                        //_action->param = param;
                        //_action->targetValue = g;
                    }
                        
                } else if (strcmp(name,"WIDTH") == 0) {
                    float delta = 5.0f;
                    if (sscanf(value, "%f", &delta) == 1) {
                        ParameterAutomationAction *paction = new ParameterAutomationAction();
                        paction->_node = &(_soundObject->sigWidth);
                        paction->_targetValue = delta;
                        float _previousTime = previousKeyFrameTimes[_slot];
                        paction->_timeToTarget = _time - _previousTime;
                        score.add(_previousTime, paction);
                        previousKeyFrameTimes[_slot] = _time;
                        
                        //_soundObject->sigGain.setTarget(g);
                        //_soundObject->sigGain.setFramesToTarget(0);
                        //printf("Setting gain for %s to %2.2f\n", _soundObject->getFilename(),g);
                        //_action->param = param;
                        //_action->targetValue = g;
                    }
                        
                } 

            } else {
                printf("Parameter setting needs an 'equals' sign: %s\n", tmp);
            }
        }        
        
    }
    
    return 0;
}


int SoundAnimation::parseSoundAnimationFile(const char * filename) 
{
    FILE *instructions;
    char line[MAX_LINE_LENGTH];

    instructions = fopen(filename,"r");
    while(fgets(line, MAX_LINE_LENGTH, instructions))
    {
        parseKeyFrame(line);
    }
    fclose(instructions);
    
    schedule = score;
    //schedule.sort(SoundAnimation::order);
    
    return 0;    
    
}


vector<FilePlayerNode*>* SoundAnimation::getOutputNodes()
{
    return &outputNodes;
}

