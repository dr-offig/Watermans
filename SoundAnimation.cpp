/***** SoundAnimation.cpp *****/
#define MAX_LINE_LENGTH 256

#include "SoundAnimation.h"
#include <stdio.h>
#include <string.h>

//enum AnimationObjectType { ANIM_OBJ_TYPE_INVALID, ANIM_OBJ_TYPE_DIRECTIONAL, ANIM_OBJ_TYPE_FIXED };


uint64_t Clock::calculate(int hh, int mm, int ss, int mss)
{
    return ((hh * 60 + mm) * 60 + ss) * 1000 + mss;
}

uint64_t Clock::set(int hh, int mm, int ss, int mss)
{
    _ticks = Clock::calculate(hh,mm,ss,mss);
    return _ticks;
}


int Clock::milli()
{
    return (_ticks % 1000);
}


int Clock::sec()
{
    return (_ticks - (_ticks / 1000) * 1000);
}


int Clock::hour()
{
    return (_ticks / 3600000);
}


int Clock::min()
{
    return ((_ticks % 3600000) / 60000);
}



void Action::perform()
{
    param->setTarget(targetValue);
    param->setMillisecondsToTarget(timeToTarget);
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
        _time = Clock::calculate(hour, minute, second, millisecond);
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
        _action = new Action();
        _action->timeToTarget = _time;

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
                    if (strncmp(value,"ON",2) == 0)
                        cout << "PLAY ON" << endl; //obj->setTargetFor(ANIM_OBJ_PLAY,Param(true));
                    else
                        cout << "PLAY OFF" << endl;

                } else if (strcmp(name,"GAIN") == 0) {
                    float g = 0.0f;
                    if (sscanf(value, "%f", &g) == 1) {
                        ParameterAutomationNode* param = new ParameterAutomationNode(44100);
                        automations.push_back(param);
                        _action->param = param;
                        _action->targetValue = g;
                    }
                        
                }
                
                if (_action)
                    schedule.push_back(make_pair(_time,_action));

                // Schedule the morph to these parameter values
                
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