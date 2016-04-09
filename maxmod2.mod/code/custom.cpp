#include "maxmod2.h"
//#include <brl.mod/blitz.mod/bdwgc/include/gc.h>

extern "C" {
	int maxmod2_maxmod2_MaxModMainCustomThreadProc(void* source, void* buffer, int length);
}

CustomPlayer::CustomPlayer() {};
CustomPlayer::~CustomPlayer() {mmPrint("~CustomPlayer");};

// -----------------------------------------------------------------------------------

int CustomPlayer::FillBuffer(void* buffer,int length) {
	if (length<=0 or STATUS==0) return 0;
	return maxmod2_maxmod2_MaxModMainCustomThreadProc(Source, buffer, length);
}

// -----------------------------------------------------------------------------------

int CustomPlayer::Seek(int position,int mode){};

// -----------------------------------------------------------------------------------

extern "C" {
	
	IMaxModMusic* LoadMusic_Custom(void* source,int samplerate,int channels,int bits) {
	
		CustomPlayer* This 	= new CustomPlayer;
		This->SAMPLERATE	= samplerate;
		This->CHANNELS		= channels;
		This->BITS		= bits;
		This->SIZE		= -1;
		This->STATUS		= 1;
		This->Source		= source;
		return This;
	
	}

	void CloseMusic_Custom(IMaxModMusic* music) {delete static_cast<CustomPlayer*>(music);}
	
}
