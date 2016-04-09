#include "rtaudiodriver.h"

extern "C" {
	//Ron: added int api as param
	IMaxModAudioDriver* CreateAudioDriver_RtAudio( int api, int showWarnings )          {return new RtAudioDriver(api, showWarnings);}
	//Ron: added showWarnings
	void RtAudio_showWarnings(IMaxModAudioDriver* Driver, int onoff) {return static_cast<RtAudioDriver*>(Driver)->audio->showWarnings(onoff);}
	void CloseAudioDriver_RtAudio(IMaxModAudioDriver* Driver) {delete static_cast<RtAudioDriver*>(Driver);}
}

int RtCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,double streamTime, RtAudioStreamStatus status, void *data ) {
	MaxMod_ProcessChannels(outputBuffer,nBufferFrames);
	return 0;
}

// ______________________________________________________________________________________________________________

//Ron: added int api and int showWarnings so warnings could be disabled
//     before initialization
RtAudioDriver::RtAudioDriver(int api, int showWarnings){
	//std::cout<<"api:"<<api<<endl;
	//create new audio object with correct api
	RtAudioDriver::SetAPI(api);
	audio->showWarnings(showWarnings);

	mmPrint("RtAudioDriver::RtAudioDriver");
	Terminate = 0;
	Active = 0;
	//unsigned int devices = dac.getDeviceCount();
	unsigned int devices = audio->getDeviceCount();
	if ( devices<1 ) {mmPrint("No audio output device found!"); return;}
	else {mmPrint("Audio drivers found=",(int)devices);}

	//parameters.deviceId     = dac.getDefaultOutputDevice();
	parameters.deviceId     = audio->getDefaultOutputDevice();
	parameters.nChannels    = 2;
	parameters.firstChannel = 0;
	bufferFrames            = 1024;
}

RtAudioDriver::~RtAudioDriver(){
	mmPrint("~RtAudioStream");
	if (!Terminate) Shutdown();
}

//Ron: make used API configurable
int RtAudioDriver::SetAPI( int api) {
	//std::cout<<"SetAPI:"<<api;

	switch(api) {
		case 1:		audio = new RtAudio(RtAudio::LINUX_ALSA);break;
		case 2:		audio = new RtAudio(RtAudio::LINUX_PULSE);break;
		case 3:		audio = new RtAudio(RtAudio::LINUX_OSS);break;
		case 4:		audio = new RtAudio(RtAudio::UNIX_JACK);break;
		case 5:		audio = new RtAudio(RtAudio::MACOSX_CORE);break;
		case 6:		audio = new RtAudio(RtAudio::WINDOWS_ASIO);break;
		case 7:		audio = new RtAudio(RtAudio::WINDOWS_DS);break;
		default:	audio = new RtAudio(RtAudio::UNSPECIFIED);break;

	}
	return 1;
}

int RtAudioDriver::Startup(){
	if (Active==1) return 1;
	mmPrint("RtAudioDriver::Startup");

	options.flags = RTAUDIO_SCHEDULE_REALTIME;
	options.numberOfBuffers = 3;
	options.priority = 1;

	//Ron: wrap try-catch around so "false" can get returned
	try {
		audio->openStream( &parameters, NULL, RTAUDIO_FLOAT32, 44100, &bufferFrames, &RtCallback, this, &options );
		audio->startStream();
	}
	catch( RtError& e ) {
		//e.printMessage();
		return 0;
	}

	Active=1;
	return 1;
}

int RtAudioDriver::Shutdown(){
	mmPrint("RtAudioDriver::Shutdown");
	Terminate=1;
	audio->closeStream();
	return 1;
}

IMaxModSound* RtAudioDriver::CreateSound( int samplerate, int channels, int bits, int flags, void* data, int size ){
}

