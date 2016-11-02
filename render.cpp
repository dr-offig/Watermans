/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
  Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
  Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/


#include <Bela.h>
#include <cmath>
#include <Scope.h>
#include <SampleStream.h>

#include <rtdk.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include "serial.h"
#include "SoundAnimation.h"
#include "SignalNode.h"
//#include "OSC.h"

#define NUM_CHANNELS 2    // NUMBER OF CHANNELS IN THE FILE
#define BUFFER_LEN 44100   // BUFFER LENGTH
//#define NUM_STREAMS 0

using namespace std;

// For debugging, to help see what is happening
Scope scope;

//SampleStream *sampleStream[NUM_STREAMS];
AnimationTimer *stopwatch;
SoundAnimation *animation;
//vector<SignalNode*> *pSignals;
//uint64_t numActiveSignalNodes;


AuxiliaryTask ttyTask;		// Auxiliary task to read serial data
float freq, amp, phase, azimuth;

AuxiliaryTask gFillBuffersTask;
int gStopThreads = 0;
int gTaskStopped = 0;
int gCount = 0;

AuxiliaryTask checkScheduleTask;

void fillBuffers() {

    /*
    for(unsigned i=0;i<animation->soundObjects.size();i++) {
        if(animation->soundObjects.at(i)->stream->bufferNeedsFilled())
            animation->soundObjects.at(i)->stream->fillBuffer();
    }
    
    for(unsigned i=0;i<animation->soundBeds.size();i++) {
        if(animation->soundBeds.at(i)->stream->bufferNeedsFilled())
            animation->soundBeds.at(i)->stream->fillBuffer();
    }
    */
    
    unsigned n = animation->outputNodes.size();
    for (unsigned i=0; i<n; i++)
    {
        animation->outputNodes.at(i)->prepare();
    }

}



/****** OSC is currently disabled *****
AuxiliaryTask oscTask;
void readOSC()
{
    // receive OSC messages, parse them, and send back an acknowledgment
    while (oscServer.messageWaiting()){
        parseMessage(oscServer.popMessage());
        oscClient.queueMessage(oscClient.newMessage.to("/osc-acknowledge").add(5).add(4.2f).add(std::string("OSC message received")).end());
    }
}
***************************************/


void checkSchedule()
{
    while ( (animation->schedule.size() > 0) 
            && (animation->schedule.nextTime() < stopwatch->currentTimeInSeconds()) )
    {
        Action* action = animation->schedule.nextAction();
        action->perform();
        animation->schedule.pop();
    }
    

}



float gaussian(float x, float sigma)
{
    return expf(-1.0f * ( x * x  / (2.0f * sigma * sigma))) * (1.0f / (sigma * sqrtf(2.0f * M_PI))); 
}


float triangle(float x, float sigma)
{
    float width = 0.25;
    float output = 0.0f;
    if (fabs(x) > width)
        output = 0.0f;
    else if (width > 0.0)
        output = 1.0 - (x / width);
    else
        output = 1.0;
    
    return output;
}




void directionalPanning(float* output, float azi, float angle, float width)
{
    // Assuming an azimuth measured in 256 units around the full circle
    float w = width / 360.0f;
    float q = angle / 360.0f;
    float d = fmod(q - azi, 1.0f);
    float e = 2.0f * (d - 0.5f);
    float left_gain = sinf(2.0f * M_PI * d);
    float right_gain = cosf(2.0f * M_PI * d);
    //float master_gain = gaussian(e,0.1f);
    float master_gain = 1.0f;
    output[0] = left_gain * master_gain;
    output[1] = right_gain * master_gain;

}


void updateAzimuth()
{
    azimuth = readSerialPort();
}


bool setup(BelaContext *context, void *userData)
{
    // Set clock to run at audioSampleRate
    stopwatch = new AnimationTimer(context->audioSampleRate);
    //stopwatch->secondsPerFrame = 1.0f / (context->audioSampleRate);
 
 
    // Read in the sound animation files
    animation = new SoundAnimation("sound_animation.txt", stopwatch);
    printf("Animation has %d signal nodes and %d scheduled actions", animation->outputNodes.size(),animation->schedule.size());
    animation->schedule.printSchedule();
 
    /*
    for(unsigned i=0;i<animation->soundObjects.size();i++) {
        // Start all streams playing
        animation->soundObjects.at(i)->stream->togglePlayback(1);
    }

    for(unsigned i=0;i<animation->soundBeds.size();i++) {
        // Start all streams playing
        animation->soundBeds.at(i)->stream->togglePlayback(1);
    }

    unsigned ns = animation->soundObjects.size() + animation->soundBeds.size() + 1;
    scope.setup(ns, context->audioSampleRate);
    */
 
    //unsigned ns = animation->outputNodes.size();
    //scope.setup(ns, context->audioSampleRate);
    scope.setup(3,context->audioSampleRate);


 
    /***** Prepare Streams *****
    for(int i=0;i<NUM_STREAMS;i++) {
        //sampleStream[i] = new SampleStream("../../samples/waves.wav",NUM_CHANNELS,BUFFER_LEN);
        sampleStream[i] = new SampleStream("./samples/noise.wav",NUM_CHANNELS,BUFFER_LEN);
    }
    ****************************/
    
    // Start serial port
    initSerial("/dev/ttyACM0", B9600);


    /****** OSC is currently disabled *****
    // setup the OSC server to receive on port 8875
    oscServer.setup(8875);
    // setup the OSC client to send on port 8876
    oscClient.setup(8876);
    
    // the following code sends an OSC message to address /osc-setup
    // then waits 1 second for a reply on /osc-setup-reply
    bool handshakeReceived = false;
    oscClient.sendMessageNow(oscClient.newMessage.to("/osc-setup").end());
    oscServer.receiveMessageNow(1000);
    while (oscServer.messageWaiting()){
        if (oscServer.popMessage().match("/osc-setup-reply")){
            handshakeReceived = true;
        }
    }
    
    if (handshakeReceived) rt_printf("handshake received!\n");
    else rt_printf("timeout!\n");
    ****************************************/
    
    // Initialise auxiliary tasks
	if((gFillBuffersTask = Bela_createAuxiliaryTask(&fillBuffers, 90, "fill-buffer")) == 0)
		return false;

    if ((ttyTask = Bela_createAuxiliaryTask(updateAzimuth, 50, "bela-arduino-comms")) == 0)
        return false;

    if ((checkScheduleTask = Bela_createAuxiliaryTask(checkSchedule, 30, "check-schedule")) == 0)
        return false;


    /****** OSC is currently disabled *****
    if ((oscTask = Bela_createAuxiliaryTask(readOSC, 50, "bela-osc")) == 0)
        return false;
    ****************************************/    
 
 
	return true;
}


void render(BelaContext *context, void *userData)
{

    // check if buffers need filling
    Bela_scheduleAuxiliaryTask(gFillBuffersTask);

    // Read in sensor data
    Bela_scheduleAuxiliaryTask(ttyTask);

    // Check the action schedule
    Bela_scheduleAuxiliaryTask(checkScheduleTask);
    
    
    
    /*****
    // random playback toggling
    for(unsigned i=0;i<animation->soundObjects.size();i++) {
        // randomly pauses/unpauses + fades in/out streams
        if((rand() / (float)RAND_MAX)>0.9999)
            animation->soundObjects.at(i)->stream->togglePlaybackWithFade(0.1);
            // the above function can also be overloaded specifying the state
            // of playback to toggle - i.e. togglePlaybackWithFade(1,0.1)
            // same applies to togglePlayback()
        
         // demonstrates dynamically reloading samples
         // (TODO: this should really be done in a separate thread)
         //
        // if((rand() / (float)RAND_MAX)>0.9999) {
        //     // only change sample if sampleStream isn't playing
        //     if(!sampleStream[i]->isPlaying())
        //         sampleStream[i]->openFile("chimes_stereo.wav",NUM_CHANNELS,BUFFER_LEN);
        // }
    }
    ******/// Random playback toggling demo
    
    
    for(unsigned int n = 0; n < context->audioFrames; n++) {

        // Read in audio data from streams
        for(unsigned i=0;i<animation->outputNodes.size();i++) {
            //animation->outputNodes.at(i)->stream->processFrame();
            animation->outputNodes.at(i)->nextFrame();
            //animation->soundObjects.at(i)->update(1);
        }

        // for(unsigned i=0;i<animation->outputNodes.size();i++) {
        //     animation->outputNodes.at(i)->stream->processFrame();
        //     //animation->soundBeds.at(i)->update(1);
        // }

        float outputFrame[2]; outputFrame[0] = 0.0f; outputFrame[1] = 0.0f;
        float panning[2]; panning[0] = 0.5f; panning[1] = 0.5f;
        
        for(unsigned i=0;i<animation->outputNodes.size();i++) {
            
            // Output audio
    	    //for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
    	    
            //float out = 0.0f;
            // get samples for each channel from each sampleStream object
            //out += animation->outputNodes.at(i)->stream->getSample(channel);
            float angle = animation->outputNodes.at(i)->sigAngle.getValue();
            float width = animation->outputNodes.at(i)->sigWidth.getValue();
            //float tmpFrame[2];                
            directionalPanning(panning,azimuth,angle,width);                
            if (i==0)
                scope.log(azimuth, panning[0],panning[1]);
            outputFrame[0] += panning[0] * animation->outputNodes.at(i)->stream->getSample(0);
            outputFrame[1] += panning[1] * animation->outputNodes.at(i)->stream->getSample(1);

            //out += animation->outputNodes.at(i)->outputBuffer.getSample(channel,0);
            //    if (channel == 0 && i == 0)
            //        scope.log(animation->outputNodes.at(i)->sigGain.getValue(),animation->outputNodes.at(i)->stream->getSample(channel));
                //}

            // you may need to attenuate the output depending on the amount of streams playing back
//            float compressedOut = tanh(out);
//            audioWrite(context, n, channel, compressedOut);
            
    	}
        audioWrite(context, n, 0, outputFrame[0]);
        audioWrite(context, n, 1, outputFrame[1]);
        
    
        stopwatch->tick();
    	
    }
    
    
    
}


void cleanup(BelaContext *context, void *userData)
{
    delete(animation);
    delete(stopwatch);
}