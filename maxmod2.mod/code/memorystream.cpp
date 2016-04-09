#include "maxmod2.h"

#ifdef __linux
#include <unistd.h>
#endif

// ______________________________________________________________________________________________________________

extern "C" {

	IMaxModChannel* CreateAudioStream_Memory(IMaxModMusic* Music,char* data) {
		return new MemAudioStream(Music,data);
	}

	void CloseAudioStream_Memory(IMaxModChannel* Stream) {
		delete static_cast<MemAudioStream*>(Stream);
	}

};

// ______________________________________________________________________________________________________________

MemAudioStream::MemAudioStream(IMaxModMusic* Music,char* outputdata) {

	mmPrint("MemAudioStream create!");

	SEEKABLE		= 1;
	MUSIC 		= Music;
	CHANNELS		= MUSIC->GetChannels();
	BITS			= MUSIC->GetBits();
	SAMPLERATE	= MUSIC->GetSampleRate();
	Terminate		= 0;
	WPOS			= 0;

	data			= outputdata;
	buffersize	= 44100;

	MUSIC->SetLoopMode(0);
	MUSIC->SetStatus(1);

	Thread = new MMThread;
	Thread->Start(this);
	SetPaused(0);

}

// ______________________________________________________________________________________________________________

MemAudioStream::~MemAudioStream() {
	mmPrint("~MemAudioStream");
	if (!Terminate) Stop();
	delete Thread;
	delete AudioWrite_Mutex;
}

// ___________________________________________________________________________________________________________

void MemAudioStream::FillEntireBuffer() {}

// ___________________________________________________________________________________________________________

void MemAudioStream::Update() {

	if (STATUS==0 or MUSIC->GetStatus()==0) return;
//	if (WPOS>=MUSIC->SIZE) {Terminate=1; return;}

//mmPrint("want=",buffersize);
	AudioWrite_Mutex->Lock();
	int size = MUSIC->FillBuffer( data, buffersize );
	AudioWrite_Mutex->Unlock();
	data+=size;
//	WPOS+=size;
//mmPrint("WPOS=",WPOS);
	if(size<buffersize) Terminate=1;

}

// ___________________________________________________________________________________________________________

void MemAudioStream::Stop() {
	Terminate=1;
	Thread->Stop();
	mmPrint("Memory stream stopped!");
}
// ___________________________________________________________________________________________________________

void MemAudioStream::SetPaused(int paused) {

	if (paused) {
		STATUS = 0;
	} else {
		STATUS = 1;
		MUSIC->SetStatus(1);
	}

}
// ___________________________________________________________________________________________________________

void MemAudioStream::SetVolume(float volume) {}

// ___________________________________________________________________________________________________________

void MemAudioStream::SetPan(float pan) {}

// ___________________________________________________________________________________________________________

void MemAudioStream::SetDepth(float depth) {};

// ___________________________________________________________________________________________________________

void MemAudioStream::SetRate(float rate) {}

// ___________________________________________________________________________________________________________

int  MemAudioStream::Playing() {
	return (!Terminate);
}

// ___________________________________________________________________________________________________________

int  MemAudioStream::GetPosition(int Mode) {
	return MUSIC->GetPosition(Mode);
}

// ___________________________________________________________________________________________________________

int  MemAudioStream::Seek(int Pos, int Mode) {
//	if(MUSIC) {
//		AudioWrite_Mutex->Lock();
//		MUSIC->Seek(Pos,Mode);
//		POS=MUSIC->POS;
//		AudioWrite_Mutex->Unlock();
//	}
}

// ___________________________________________________________________________________________________________

int MemAudioStream::threadProc() {
	Thread->SetPriority(1);

	mmPrint("MemAudioStream ThreadProc!");
	while(Terminate==0) {
#ifdef __linux
		sleep(1);
#endif


#ifdef _win32
		Sleep(1);
#endif
		Update();
	}

	mmPrint("exiting thread!");
	return 0;
}

// ______________________________________________________________________________________________________________
