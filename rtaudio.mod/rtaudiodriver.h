#ifndef __MAXMOD_RTAUDIODRIVER_H__
#define __MAXMOD_RTAUDIODRIVER_H__

#include <maxmod2.mod/maxmod2.mod/code/maxmod2.h>
#include <rtaudio/RtAudio.h>

class RtAudioDriver : public IMaxModAudioDriver {

	public:

	//we want a pointer
	RtAudio *audio;

//	RtAudio dac;
	RtAudio::StreamParameters parameters;
	RtAudio::StreamOptions options;
	unsigned int bufferFrames;
	RtAudioFormat format;
	int Terminate;
	int Active;

	RtAudioDriver(int api, int showWarnings);
	virtual ~RtAudioDriver();
	virtual int SetAPI(int api);

	virtual int Startup();
	virtual int Shutdown();

	virtual IMaxModSound*   CreateSound( int samplerate, int channels, int bits, int flags, void* data, int size );
//	virtual IMaxModChannel* AllocChannel();

};

#endif // __MAXMOD_RTAUDIODRIVER_H__
