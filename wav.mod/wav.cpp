#include "wav.h"

void WavFreeFunction(IMaxModMusic* music){
	delete static_cast<WavPlayer*>(music);
}


// -----------------------------------------------------------------------------------


WavPlayer::WavPlayer(){
	Stream         = NULL;
	FreeFunction	= WavFreeFunction;
	
}

WavPlayer::~WavPlayer(){
	mmPrint("~WavPlayer");
	Stop();
	if (Stream!=NULL) {
		Stream->Close();
		delete Stream;
	}
}

// -----------------------------------------------------------------------------------

int WavPlayer::FillBuffer(void* buffer,int Length) {

	if (Length==0 or STATUS==0) {return 0;}
	short* buf = (short*)buffer;
	int	res,bs,got=0;

	while (Length>0){
	
		res=Stream->Read(buf,Length);
		if (res<=0) {
			POS=0; 
			Stream->Seek(StartPos);
			if (LOOP==0) {STATUS=0; return got;}
		} else {
			got+=res;
			buf+=(res/(BITS/8));
			Length-=res;
			POS+=res;
		}

		if (Stream->Eof()) {
			POS=0; 
			Stream->Seek(StartPos);
			if (LOOP==0) {STATUS=0; return got;}
		}
		
	}
	return got;

}
 
// -----------------------------------------------------------------------------------

void WavPlayer::Stop() {
	POS=0;
}
	
// -----------------------------------------------------------------------------------

int WavPlayer::Seek(int Pos,int mode) {
	// convert Pos into samples
	int position = 0;
	
	switch(mode) {
		case MM_BYTES:		position=Pos/(BITS/8)/CHANNELS	; break;
		case MM_SAMPLES:	position=Pos					; break;
//		case MM_MILLISECS:	position=Pos*(SAMPLERATE/1000.0)	; break;
	}
	position+=StartPos;
	mmPrint("Wav seek to:",position);
	int res = Stream->Seek(position);
	STATUS=1;
	POS = res*((BITS/8)*CHANNELS);
	return res;

}

// -----------------------------------------------------------------------------------

extern "C" {

	IMaxModMusic* LoadMusic_Wav(IMaxModStream* Stream) {
	
		mmPrint("Wav Loader...");
	
		WavPlayer* This 	= new WavPlayer;
	
		Stream->Seek(0);

		string tag = Stream->ReadString(4);
		int ts = Stream->ReadInt();
		string tag1 = Stream->ReadString(4);
		string tag2 = Stream->ReadString(4);
		if (tag!="RIFF" or tag1!="WAVE" or tag2!="fmt ") {
			mmPrint("Wav rejected");
			delete This; 
			return NULL;
		}
		
		int fmtchunksize = Stream->ReadInt();
		int comp = (int)Stream->ReadShort();
		if (comp!=1) {
			mmPrint("Wav rejected - compression not supported");
			delete This; 
			return NULL;
		}

		This->Stream 		= Stream;
		This->CHANNELS 	= Stream->ReadShort();
		This->SAMPLERATE 	= Stream->ReadInt();
		This->BytesPerSec	= Stream->ReadInt();
		This->Pad			= Stream->ReadShort();
		This->BITS		= Stream->ReadShort();
		This->SEEKABLE		= 1;
		This->STATUS		= 1;

		if (fmtchunksize>16) {Stream->ReadString( fmtchunksize-16 );}

		string tag3;
		while (Stream->Eof()==false) {

			tag3=Stream->ReadString(4);
			if (tag3!="data") {
				int sz=Stream->ReadInt();
				Stream->ReadString( sz );
				continue;
			}

			int w_sizebytes=Stream->ReadInt();
			This->SIZE = w_sizebytes; ///((This->BITS)/8);
			This->StartPos = Stream->Position();
			
			mmPrint("Wav accepted! samples=",This->SIZE);
			mmPrint("channels=",This->CHANNELS);
			mmPrint("freq=",This->SAMPLERATE);
			mmPrint("start@=",This->StartPos);
	
			return This;
		
		}

	}

}
