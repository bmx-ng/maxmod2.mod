#include "maxmod2.h"

double MaxMod_GlobalVolume = 1.0;
bool   FillScope_Active = false;
float* FillScope_Buffer = NULL;
int    FillScope_Size = 0;

void MaxMod_SetGlobalVolume(double vol){
	MaxMod_GlobalVolume = vol;
}
double MaxMod_GetGlobalVolume(){
	return MaxMod_GlobalVolume;
}

// ______________________________________________________________________________________________________________

static list<IMaxModChannel*> ChannelList;
static MMMutex ChannelList_Mutex;

void MaxMod_FillScope(float* buf, int size){
	ChannelList_Mutex.Lock();
	FillScope_Buffer = buf;
	FillScope_Size = size;
	FillScope_Active = true;
	ChannelList_Mutex.Unlock();
//	mmPrint("MaxMod_FillScope");
}

void MaxMod_AddChannel(IMaxModChannel* chn){
	ChannelList_Mutex.Lock();
	ChannelList.push_back(chn);
	ChannelList_Mutex.Unlock();
}

void MaxMod_RemoveChannel(IMaxModChannel* chn){
	ChannelList_Mutex.Lock();
	ChannelList.remove(chn);
	ChannelList_Mutex.Unlock();
}

void MaxMod_SetChannelRateAdjuster(IMaxModChannel* chn, double adjuster) {
	chn->AudioWrite_Mutex->Lock();
	if(adjuster<0 and chn->RS->outRateAdjuster>0) {
		chn->RS->outRateAdjuster=adjuster;
	} else if(adjuster>0 and chn->RS->outRateAdjuster<0) {
		chn->RS->outRateAdjuster=adjuster;
	} else {
		chn->RS->outRateAdjuster+=adjuster;
	}
	chn->AudioWrite_Mutex->Unlock();
}


void MaxMod_FreeChannel(IMaxModChannel* chn){
	chn->TERMINATE=1;
}

int  MaxMod_ChannelCount(){
	ChannelList_Mutex.Lock();
	int size = ChannelList.size();
	ChannelList_Mutex.Unlock();
	return size;
}

void MaxMod_ChannelLock(){
	ChannelList_Mutex.Lock();
}

void MaxMod_ChannelUnlock(){
	ChannelList_Mutex.Unlock();
}

// ______________________________________________________________________________________________________________

IMaxModChannel* MaxMod_AllocChannel(){
//	mmPrint("AllocChannel");
	IMaxModChannel* chn = new IMaxModChannel;
	MaxMod_AddChannel(chn);
	mmPrint("AllocChannel #=",MaxMod_ChannelCount());
	return chn;
}
// ______________________________________________________________________________________________________________


bool MaxMod_ChannelFinished(const IMaxModChannel* chn) {
	if(chn->TERMINATE==1 and chn->STATUS==0) {delete chn; return true;}
	return false;
}

void MaxMod_ProcessChannels(void *outputBuffer,unsigned int nBufferFrames){

	list<IMaxModChannel*>::iterator it;
	IMaxModChannel* chn;
	memset(outputBuffer,0,nBufferFrames*8);

	ChannelList_Mutex.Lock();
  	ChannelList.remove_if(MaxMod_ChannelFinished);
	for ( it=ChannelList.begin() ; it != ChannelList.end(); it++ ) {
		chn = (IMaxModChannel*)*it;
		chn->Fill(outputBuffer,nBufferFrames);
	}
	ChannelList_Mutex.Unlock();

	// apply clipping
	float* buf = (float*)outputBuffer;
	for(int i=0;i!=nBufferFrames*2;i++) {
		*buf*=MaxMod_GlobalVolume;
		if(*buf>1.0f) {
			*buf=1.0f;
		} else if(*buf<-1.0f) {
			*buf=-1.0f;
		}
		buf++;
	}

	if (FillScope_Active==true){
//		mmPrint("memcpy!");
		int size;
		if(FillScope_Size>nBufferFrames) {size=nBufferFrames;} else {size=FillScope_Size;}
		memcpy(FillScope_Buffer,outputBuffer,8*size);
		FillScope_Active = false;
	}

}

















// ================================================================================================================

IMaxModChannel::IMaxModChannel(){
	MUSIC			= NULL;
	AudioWrite_Mutex 	= new MMMutex();
	RS                  = new MaxMod_Resampler(2,32,44100);
	SAMPLERATE		= 0;
	CHANNELS			= 0;
	BITS				= 0;
	SIZE				= 0;
	SEEKABLE			= 0;
	POS				= 0;
	LOOP				= 0;
	STATUS			= 0;
	TERMINATE			= 0;
	VOLUME			= 1.0f;
	RATE				= 1.0f;
	DEPTH			= 0.0f;
	PAN				= 0.0f;
}
// _________________________________________________________________________________

IMaxModChannel::~IMaxModChannel(){
//	mmPrint("~IMaxModChannel");
	delete AudioWrite_Mutex;
	delete RS;
	if(MUSIC) MUSIC->RemoveRef();
}
// _________________________________________________________________________________

void IMaxModChannel::Stop(){
	STATUS=0;
	if(MUSIC) MUSIC->SetStatus(0);
}
// _________________________________________________________________________________

void IMaxModChannel::SetPaused( int paused ){
	if(paused==1){
		STATUS=0;
		if(MUSIC) MUSIC->SetStatus(0);
	} else {
		STATUS=1;
		if(MUSIC) MUSIC->SetStatus(1);
	}
}
// _________________________________________________________________________________

void IMaxModChannel::SetVolume( float vol ){
	if(vol<=0.0f) {VOLUME=0.0f;}
	else {VOLUME=vol;}
	RS->outVolume=VOLUME;
}
// _________________________________________________________________________________

void IMaxModChannel::SetPan( float pan ){
	if (pan<-1.0f) {pan=-1.0f;}
	else if (pan>1.0f) {pan=1.0f;}
	RS->setPan(pan);
}
// _________________________________________________________________________________

void IMaxModChannel::SetDepth( float depth ){};
// _________________________________________________________________________________

void IMaxModChannel::SetRate( float rate ){
	if(MUSIC and rate<0) {if(MUSIC->REVERSABLE=0) return;}
	RATE = rate;
	RS->outRate=rate;
	if(MUSIC) MUSIC->RATE=rate;
}
// _________________________________________________________________________________

int IMaxModChannel::Playing(){
	return STATUS;
}
// _________________________________________________________________________________

int IMaxModChannel::GetPosition(int mode){
	switch(mode) {
		case MM_BYTES:		return POS;
		case MM_SAMPLES:	return POS/((BITS/8)*CHANNELS);
		case MM_MILLISECS:	return ((double)POS/((BITS/8)*CHANNELS))/(SAMPLERATE/1000.0);
	}
	return MUSIC->GetPosition(mode);
}
// _________________________________________________________________________________

int IMaxModChannel::GetLength(int mode){
	switch(mode) {
		case MM_BYTES:		return SIZE;
		case MM_SAMPLES:	return SIZE/((BITS/8)*CHANNELS);
		case MM_MILLISECS:	return ((double)SIZE/((BITS/8)*CHANNELS))/(SAMPLERATE/1000.0);
	}
	return MUSIC->GetLength(mode);
}
// _________________________________________________________________________________

int IMaxModChannel::Seek(int position, int mode){
	if(MUSIC) {
		AudioWrite_Mutex->Lock();
		MUSIC->Seek(position,mode);
		POS=MUSIC->POS;
		AudioWrite_Mutex->Unlock();
	}
}
// _________________________________________________________________________________

int IMaxModChannel::GetLoopMode(){
	return LOOP;
}
// _________________________________________________________________________________

void IMaxModChannel::SetLoopMode(int mode){
	if(mode) {LOOP=1;} else {LOOP=0;}
}
// _________________________________________________________________________________

void IMaxModChannel::SetLoopPoints(int firstsample,int length){
	LOOPSTART  = firstsample*((BITS/8)*CHANNELS);
	LOOPLENGTH = length*((BITS/8)*CHANNELS);
}
// _________________________________________________________________________________

void IMaxModChannel::GetUV(double* L, double* R){
	if(MUSIC and STATUS==1) {
		*L=RS->outLevelLeft;
		*R=RS->outLevelRight;
	} else {
		*L=0;
		*R=0;
	}
}
// _________________________________________________________________________________

int IMaxModChannel::IsSeekable(){
	if(MUSIC) return MUSIC->SEEKABLE;
}

// _________________________________________________________________________________

int IMaxModChannel::Fill(void* outputBuffer,int nBufferFrames){

//	mmPrint("IMaxModChannel::Fill");

	if(!MUSIC) return 0;
	if(!STATUS) {if(RS->outRateAdjuster==0.0) return 0;}

	int   size        = nBufferFrames*8;
	char* inbuf       = NULL;
	int   insize      = 0;

	AudioWrite_Mutex->Lock();

	MUSIC->POS        = POS;
	MUSIC->STATUS     = STATUS;
	MUSIC->LOOPSTART  = LOOPSTART;
	MUSIC->LOOPLENGTH = LOOPLENGTH;
	MUSIC->LOOP       = LOOP;

	RS->STATUS = STATUS;
	RS->requestOutput(outputBuffer,size);
	while( RS->process()==1 ) {
		inbuf = (char*)(RS->inBuffer)+RS->inStart;
		insize = RS->inBufferSize-RS->inStart;
		memset(inbuf,0,insize);
		MUSIC->RATE = RS->outAdjusterRealRate;						// required for rate adjuster
		RS->pushInput(MUSIC->FillBuffer(inbuf,insize));
	}
	POS = MUSIC->POS;
	AudioWrite_Mutex->Unlock();

	if(MUSIC->STATUS==0) {STATUS=0;}
	return 0;

}
// _________________________________________________________________________________

int IMaxModChannel::threadProc() {}


