#include "ogg.h"

void OGGFreeFunction(IMaxModMusic* music){
	delete static_cast<OggPlayer*>(music);
}


// -----------------------------------------------------------------------------------

size_t readfunc( void* buf,size_t size,size_t nmemb,void* src ){
	IMaxModStream* stream = (IMaxModStream*)src;
	long int bytes = stream->Read(buf,size*nmemb);
	return bytes/size;
}

int seekfunc( void* src, ogg_int64_t off, int whence ){
//	cout << "seekfunc " << whence << endl;
	IMaxModStream* stream=(IMaxModStream*)src;
	int res=-1;
	switch(whence) {
//		case 0: res=stream->SeekFrom(off,SEEK_SET); cout << "seekfunc SET res=" << res << " offset=" << off << endl; break;
//		case 1: res=stream->SeekFrom(off,SEEK_CUR); cout << "seekfunc CUR res=" << res << " offset=" << off << endl; break;
//		case 2: res=stream->SeekFrom(off,SEEK_END); cout << "seekfunc END res=" << res << " offset=" << off << endl; break;
		case 0: res=stream->SeekFrom(off,SEEK_SET); break;
		case 1: res=stream->SeekFrom(off,SEEK_CUR); break;
		case 2: res=stream->SeekFrom(off,SEEK_END); break;
	}
	if (res>=0) return 0;
	return -1;
//	return res;
}

int closefunc( void* src ){}

long int tellfunc( void* src ){
	IMaxModStream* stream=(IMaxModStream*)src;
	return stream->Position();
}

// -----------------------------------------------------------------------------------

OggPlayer::OggPlayer(){
	Stream         = NULL;
	FreeFunction	= OGGFreeFunction;

}

OggPlayer::~OggPlayer(){
	mmPrint("~OggPlayer");
	Stop();
	if (Stream!=NULL) {
		Stream->Close();
		delete Stream;
	}
}

// -----------------------------------------------------------------------------------

int OggPlayer::FillBuffer(void* buffer,int Length) {

	if (Length==0 or STATUS==0) {return 0;}
	short* buf = (short*)buffer;
	int	res,bs,got=0;
	while (Length>0){

		res=ov_read(&vf,(char*)buf,Length,endian,bits/8,sign,&bs);
		if(bs) return 0;
		if (res<=0) {
			POS=0;
			if (LOOP==0) {STATUS=0;return got;}
		} else {
			got+=res;
			buf+=(res/(bits/8));
			Length-=res;
			POS+=res;
		}

		if (Stream->Eof()) {
			Stream->Seek(0);
		}

	}
	return got;

}

// -----------------------------------------------------------------------------------

void OggPlayer::Stop() {
	ov_clear(&vf);
	POS=0;
}

// -----------------------------------------------------------------------------------

int OggPlayer::Seek(int Pos,int mode) {
	// convert Pos into samples
	int position = 0;
	switch(mode) {
		case MM_BYTES:		position=Pos/(BITS/8)/CHANNELS	; break;
		case MM_SAMPLES:	position=Pos					; break;
//		case MM_MILLISECS:	position=Pos*(SAMPLERATE/1000.0)	; break;
	}
	mmPrint("Ogg seek to:",position);
	ogg_int64_t pos = position;
	int res = ov_pcm_seek_lap(&vf,pos);
	STATUS=1;
	POS = pos*((BITS/8)*CHANNELS);
	return res;
}

// -----------------------------------------------------------------------------------

extern "C" {

	IMaxModMusic* LoadMusic_Ogg(IMaxModStream* Stream) {

		mmPrint("OGG Loader...");

		OggPlayer* This 	= new OggPlayer;
		This->cb.read_func	= readfunc;
		This->cb.seek_func	= seekfunc;
		This->cb.close_func	= closefunc;
		This->cb.tell_func	= tellfunc;

		Stream->Seek(0);

		if (ov_open_callbacks(Stream,&This->vf,NULL,0,This->cb)<0) {
			mmPrint("OGG decoder rejected");
			delete This;
			return NULL;
		}

		This->Stream 		= Stream;
		This->SAMPLERATE 	= ov_info(&This->vf,-1)->rate;
		This->CHANNELS 	= ov_info(&This->vf,-1)->channels;
		This->BITS		= 16;
		This->SIZE		= ov_pcm_total(&This->vf,-1)*((This->BITS/8)*This->CHANNELS);
		This->SEEKABLE		= 1;
		This->STATUS		= 1;

		mmPrint("OGG accepted! samples=",This->SIZE);
		mmPrint("channels=",This->CHANNELS);
		mmPrint("freq=",This->SAMPLERATE);

		return This;

	}

}
