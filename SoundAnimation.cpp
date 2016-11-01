/***** SoundAnimation.cpp *****/
#define MAX_LINE_LENGTH 256

#include "SoundAnimation.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

//enum AnimationObjectType { ANIM_OBJ_TYPE_INVALID, ANIM_OBJ_TYPE_DIRECTIONAL, ANIM_OBJ_TYPE_FIXED };


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
    int a = floor(_timeInSeconds);
    _timeInSeconds += secondsPerFrame;
    int b = floor(_timeInSeconds);
    
    if (a < b)
        printf("Time: %d\n", b);
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


ParameterAutomationAction::~ParameterAutomationAction()
{
    // nothing special to be done here
}


PlayFileAction::~PlayFileAction()
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



SoundAnimation::SoundAnimation(const char *instructions)
{
    // read in the file
    int result = parseSoundAnimationFile(instructions);
    if (result < 0)
        printf("Problem reading in instructions\n");
    
}


/*
SoundAnimation::~SoundAnimation() { }
*/

int SoundAnimation::parseKeyFrame(char *keyFrame)
{

    // entries are separated by spaces
    int hour, minute, second, millisecond;
    
    // First entry is the time
    char *time = strtok(keyFrame," ");
    if (sscanf(time,"%d:%d:%d.%d", &hour, &minute, &second, &millisecond) == 4) {
        // Do sequencing stuff
        _time = AnimationTimer::calculate(hour, minute, second, millisecond);
        printf("Key Frame at %02d:%02d:%02d.%03d\t", hour, minute, second, millisecond);
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
        unsigned slot;
        if (sscanf(element,"OBJECT[%d]", &slot) == 1)
        {
            //objectType = ANIM_OBJ_TYPE_DIRECTIONAL;
            slot--;
            if (0 <= slot) 
            {
                printf("Object Slot read in as #%d\t", slot);                    
                if (slot < outputNodes.size())
                {
                    printf("Sound Object #%d\t", slot);                    
                    _soundObject = outputNodes[slot];
                    //obj = (AnimationObject *)_soundObject;
                } 
                else if (slot == outputNodes.size()) 
                {
                    printf("Creating new object slot #%d\t", slot);                    
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
                    printf ("Bad slot %d\n", slot);
                }

            }
            else
            { 
                printf ("Bad slot %d\n", slot);
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
                    schedule.push_back(make_pair(_time,playFileAction));
                    
                } else if (strcmp(name,"GAIN") == 0) {
                    float g = 0.0f;
                    if (sscanf(value, "%f", &g) == 1) {
                        //ParameterAutomationNode* param = new ParameterAutomationNode(44100);
                        //automations.push_back(param);
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
    return 0;    
    
}


vector<FilePlayerNode*>* SoundAnimation::getOutputNodes()
{
    return &outputNodes;
}


void SoundAnimation::printSchedule()
{
    
    printf ("---------- Schedule ----------\n");
    list<pair<float,Action*> >::iterator p;
    for (p = schedule.begin(); p != schedule.end(); p++) 
    {
        pair<float, Action*> x = *p;
        char *f = x.second->toString();
        printf("Time: %2.2f\t Action: %s\n", x.first, x.second->toString());
        if (f) 
            free (f);
        
    }


}

