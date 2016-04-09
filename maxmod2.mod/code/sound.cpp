#include "maxmod2.h"

void SOUNDFreeFunction(IMaxModMusic* music){
	delete static_cast<IMaxModSound*>(music);
}

// ______________________________________________________________________________________________________________

IMaxModSound::IMaxModSound(){
	REVERSABLE       = 1;
	FreeFunction	  = SOUNDFreeFunction;
	AudioWrite_Mutex = new MMMutex;
}


IMaxModSound::~IMaxModSound(){
	mmPrint("~IMaxModSound");
	delete AUDIOBUFFER;
	delete AudioWrite_Mutex;
	AUDIOBUFFER=NULL;
}

// ______________________________________________________________________________________________________________

int IMaxModSound::FillBuffer(void* buffer,int Length) {

	if(RATE<0) {POS-=Length;}

	AudioWrite_Mutex->Lock();

	if (POS>SIZE) {
		if(LOOP==0) {
			STATUS=0;
			AudioWrite_Mutex->Unlock();
			return 0;
		} else {
			POS=LOOPSTART;
		}
	}

	if (POS+Length>=LOOPSTART+LOOPLENGTH and LOOP==1) {
		int chunk=(LOOPSTART+LOOPLENGTH)-POS;
		memcpy(buffer,AUDIOBUFFER+POS,chunk);
		POS=LOOPSTART;
		AudioWrite_Mutex->Unlock();
		chunk+=FillBuffer(((char*)buffer)+chunk,Length-chunk);
		return chunk;

	} else if (POS+Length>=SIZE) {
		int chunk=SIZE-POS;
		memcpy(buffer,AUDIOBUFFER+POS,chunk);
		POS=0;
		AudioWrite_Mutex->Unlock();
		if(LOOP==0) {
			STATUS=0;
		} else {
			chunk+=FillBuffer(((char*)buffer)+chunk,Length-chunk);
		}
		return chunk;
		
	} else {
		memcpy(buffer,AUDIOBUFFER+POS,Length);
		if(RATE>0) POS+=Length;
		AudioWrite_Mutex->Unlock();
		return Length;
	}


}
 
// ______________________________________________________________________________________________________________

int IMaxModSound::Seek(int position,int mode){
	POS=position*((BITS/8)*CHANNELS);
	if (POS>SIZE) {
		if(LOOP==0) {
			STATUS=0;
			return 0;
		} else {
			POS=LOOPSTART;
		}
	}
	return POS;
}

// ______________________________________________________________________________________________________________

extern "C" {
	
	IMaxModSound* MaxMod_CreateSound( int channels, int bits, int samplerate, int size, void* samples, int flags ) {
		mmPrint("MaxMod_CreateSound ",size);
		IMaxModSound* snd = new IMaxModSound;
		snd->CHANNELS     = channels;
		snd->BITS         = bits;
		snd->SAMPLERATE   = samplerate;
		snd->SIZE         = size;
		snd->AUDIOBUFFER  = (char*)samples;
		snd->STATUS       = 0;
		snd->SetStatus( 0 );
		if(flags&1) snd->LOOP = 1;
		return snd;
	}
	
	void MaxMod_CloseSound(IMaxModSound* sound){
		if(sound) delete sound;
	}

}
// ______________________________________________________________________________________________________________
