/***** SampleStream.cpp *****/
#include <SampleStream.h>
#include <stdlib.h>
#include <string.h>

SampleStream::SampleStream(const char* filename, int bufferLength) {
    
    gSampleBuf[0] = NULL;
    gSampleBuf[1] = NULL;
    gFilename = NULL;

    sndfile = NULL;    
    openFile(filename, bufferLength);
    
}

SampleStream::~SampleStream() {
    
    if (sndfile)
        sf_close(sndfile);
        
    for(int ch=0;ch<gNumChannelsInFile;ch++) {
        for(int i=0;i<2;i++) {
        if(gSampleBuf[i][ch].samples != NULL)
            delete[] gSampleBuf[i][ch].samples;
        }
    }
    free(gSampleBuf[0]);
    free(gSampleBuf[1]);
    
    free(gFilename);
    
}


SampleStream::SampleStream(const SampleStream& s)
{
    gSampleBuf[0] = NULL;
    gSampleBuf[1] = NULL;
    gFilename = NULL;

    sndfile = NULL;    
    openFile(s.gFilename,s.gBufferLength);

}
    
SampleStream& SampleStream::operator=(const SampleStream& s)
{
    openFile(s.gFilename,s.gBufferLength);
    return *this;
}


int SampleStream::openFile(const char* filename, int bufferLength) {
    
    gBusy = 1;
    
    if (sndfile) 
        sf_close(sndfile);

    if (gFilename)
        free(gFilename);
        
    gFilename = (char *)malloc(strlen(filename)+1);
    strcpy(gFilename,filename);


    if (!(sndfile = sf_open (gFilename, SFM_READ, &sfinfo))) {
		cout << "Couldn't open file " << filename << endl;
		return 1;
	}

    for(int i=0;i<2;i++) {
        if(gSampleBuf[i] != NULL) {
            cout << "I AM NOT A NULL" << endl;
            for(int ch=0; ch<gNumChannelsInFile; ch++) {
                if(gSampleBuf[i][ch].samples != NULL)
                    delete[] gSampleBuf[i][ch].samples;
            }
            free(gSampleBuf[i]);
        }
    }

    gBufferLength = bufferLength;
    gNumChannelsInFile = sfinfo.channels;
    
    gSampleBuf[0] = (SampleData*)calloc(bufferLength*gNumChannelsInFile,sizeof(SampleData));
    gSampleBuf[1] = (SampleData*)calloc(bufferLength*gNumChannelsInFile,sizeof(SampleData));
    
    gReadPtr = bufferLength;
    gBufferReadPtr = 0;
    gActiveBuffer = 0;
    gDoneLoadingBuffer = 1;
    gBufferToBeFilled = 0;
    
    gInputSampleRate = 44100;
    gOutputSampleRate = 44100;
    

    gPlaying = 0;
    gLooping = 0;
    gFadeAmount = 0;
    gFadeLengthInSeconds = 0.1;
    gFadeDirection = -1;
    
    gGain = 1.0f;
    
    gNumFramesInFile = sfinfo.frames;
    gLoopStartFrame = 0;
    gLoopEndFrame = gNumFramesInFile;
    
    if(gNumFramesInFile <= gBufferLength) {
        printf("Sample needs to be longer than buffer size. This example is intended to work with long samples.");
        return 1;
    }
    
    for(int ch=0;ch<gNumChannelsInFile;ch++) {
        for(int i=0;i<2;i++) {
            gSampleBuf[i][ch].sampleLen = gBufferLength;
        	gSampleBuf[i][ch].samples = new float[gBufferLength];
            if(getSamples(gFilename,gSampleBuf[i][ch].samples,ch,0,gBufferLength)) {
                printf("error getting samples\n");
                return 1;
            }
        }
    }
    
    cout << "Loaded " << filename << endl;
    
    gBusy = 0;
    
    return 0;
    
}

void SampleStream::processFrame() {
    
    if(gFadeAmount<1 && gFadeAmount>0) {
        gFadeAmount += (gFadeDirection*((1.0/gFadeLengthInSeconds)/44100.0));
    }
    else if(gFadeAmount < 0)
        gPlaying = 0;
    
    if(gPlaying) {
        // Increment read pointer and reset to 0 when end of file is reached
        if(++gReadPtr >= gBufferLength) {
            // if(!gDoneLoadingBuffer)
            //     rt_printf("Couldn't load buffer in time :( -- try increasing buffer size!");
            gDoneLoadingBuffer = 0;
            gReadPtr = 0;
            gActiveBuffer = !gActiveBuffer;
            gBufferToBeFilled = 1;
        }
    }
    
}

float SampleStream::getSample(int channel) {
    if(gPlaying) {
        // Wrap channel index in case there are more audio output channels than the file contains
        float out = gSampleBuf[gActiveBuffer][channel%gNumChannelsInFile].samples[gReadPtr];
    	return out * gFadeAmount * gFadeAmount * gGain;
    }
    return 0;
	
}

void SampleStream::fillBuffer() {
    
    if(!gBusy) {
    
        // increment buffer read pointer by buffer length
        gBufferReadPtr+=gBufferLength;
        
        // reset buffer pointer if it exceeds the number of frames in the file
        if(gBufferReadPtr>=gNumFramesInFile) 
        {
            gBufferReadPtr=0;
            // if (gLooping == 0) {
            //     printf("Finished playing file %s\n", gFilename);
            //     togglePlaybackWithFade(-1,0.001);
            // } else {
            //     printf("Looping file %s\n", gFilename);
            // }
        }
        
        int endFrame = gBufferReadPtr + gBufferLength;
        int zeroPad = 0;
        
        // if reaching the end of the file take note of the last frame index
        // so we can zero-pad the rest later
        if((gBufferReadPtr+gBufferLength)>=gNumFramesInFile-1) {
              endFrame = gNumFramesInFile-1;
              zeroPad = 1;
        }
        
        for(int ch=0;ch<gNumChannelsInFile;ch++) {
            
            // fill (nonactive) buffer
            getSamples(gFilename,gSampleBuf[!gActiveBuffer][ch].samples,ch
                        ,gBufferReadPtr,endFrame);
                        
            // zero-pad if necessary
            if(zeroPad) {
                int numFramesToPad = gBufferLength - (endFrame-gBufferReadPtr);
                for(int n=0;n<numFramesToPad;n++)
                    gSampleBuf[!gActiveBuffer][ch].samples[n+(gBufferLength-numFramesToPad)] = 0;
            }
            
        }
        
        gDoneLoadingBuffer = 1;
        gBufferToBeFilled = 0;
        
        // printf("done loading buffer!\n");
        
    }
    
}

int SampleStream::bufferNeedsFilled() {
    return gBufferToBeFilled;
}

void SampleStream::togglePlayback() {
    gPlaying = !gPlaying;
    gFadeAmount = gPlaying;
}

void SampleStream::togglePlaybackWithFade(int toggle, float fadeLengthInSeconds) {

    gFadeDirection = toggle;
    if(fadeLengthInSeconds<=0)
        fadeLengthInSeconds = 0.00001;
    gFadeLengthInSeconds = fadeLengthInSeconds;
    gFadeAmount += (gFadeDirection*((1.0/gFadeLengthInSeconds)/44100.0));
    if(gFadeDirection)
        gPlaying = 1;
}

void SampleStream::togglePlayback(int toggle) {
    gPlaying = toggle;
    gFadeAmount = gPlaying;
}

void SampleStream::togglePlaybackWithFade(float fadeLengthInSeconds) {

    gFadeDirection = gFadeDirection*-1;
    if(fadeLengthInSeconds<=0)
        fadeLengthInSeconds = 0.00001;
    gFadeLengthInSeconds = fadeLengthInSeconds;
    gFadeAmount += (gFadeDirection*((1.0/gFadeLengthInSeconds)/44100.0));
    if(gFadeDirection)
        gPlaying = 1;
}

int SampleStream::isPlaying() {
    return gPlaying;
}



int SampleStream::shouldLoop() {
    return gLooping;
}


void SampleStream::setShouldLoop(int shouldLoop) {
    gLooping = shouldLoop;
}


int SampleStream::loopStartFrameOffset() {
    return gLoopStartFrame;
}


int SampleStream::loopEndFrameOffset() {
    return gLoopEndFrame;
}


int SampleStream::loopStartTimeOffset() {
    return frames2Milliseconds(gLoopStartFrame);
}


int SampleStream::loopEndTimeOffset() {
    return frames2Milliseconds(gLoopEndFrame);
}


void SampleStream::setLoopStartTimeOffset(int a) {
    gLoopStartFrame = milliseconds2Frames(a);
}


void SampleStream::setLoopEndTimeOffset(int b) {
    gLoopEndFrame = milliseconds2Frames(b);
}


int SampleStream::frames2Milliseconds(int frames) {
    return ((frames * gOutputSampleRate) / 1000); 
}
    
    
int SampleStream::milliseconds2Frames(int millis) {
    return ((millis * 1000) / gOutputSampleRate);
}


float SampleStream::fadeLengthInSeconds()
{
    return gFadeLengthInSeconds;
}


void SampleStream::rewind()
{
    bool wasPlaying = gPlaying;
    if (wasPlaying) 
        togglePlaybackWithFade(-1,0.01);
    sf_seek(sndfile,0,SEEK_SET);
    if (wasPlaying)
        togglePlaybackWithFade(1,0.01);
}


// private libsndfile wrappers (previously in SampleLoader.h)

int SampleStream::getSamples(const char* file, float *buf, int channel, int startFrame, int endFrame)
{

	int numChannelsInFile = sfinfo.channels;
	if(numChannelsInFile < channel+1)
	{
		cout << "Error: " << file << " doesn't contain requested channel" << endl;
		return 1;
	}
    
    int frameLen = endFrame-startFrame;
    
    if(frameLen <= 0 || startFrame < 0 || endFrame <= 0 || endFrame > sfinfo.frames)
	{
	    cout << "Error: " << file << " invalid frame range requested" << endl;
		return 1;
	}
    
    sf_seek(sndfile,startFrame,SEEK_SET);
    
    float* tempBuf = new float[frameLen*numChannelsInFile];
    
	int subformat = sfinfo.format & SF_FORMAT_SUBMASK;
	int readcount = sf_read_float(sndfile, tempBuf, frameLen*numChannelsInFile); //FIXME

	// Pad with zeros in case we couldn't read whole file
	for(int k = readcount; k <frameLen*numChannelsInFile; k++)
		tempBuf[k] = 0;

	if (subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE) {
		double	scale ;
		int 	m ;

		sf_command (sndfile, SFC_CALC_SIGNAL_MAX, &scale, sizeof (scale)) ;
		if (scale < 1e-10)
			scale = 1.0 ;
		else
			scale = 32700.0 / scale ;
		cout << "File samples scale = " << scale << endl;

		for (m = 0; m < frameLen; m++)
			tempBuf[m] *= scale;
	}
	
	for(int n=0;n<frameLen;n++)
	    buf[n] = tempBuf[n*numChannelsInFile+channel];

    delete[] tempBuf;

	return 0;
}


char* SampleStream::getFilename()
{
    return gFilename;
}


void SampleStream::setGain(float g)
{
    gGain = g;
}


/*
int SampleStream::getNumChannels(const char* file) {

	return sfinfo.channels;
}

int SampleStream::getNumFrames(const char* file) {

	return sfinfo.frames;
}
*/