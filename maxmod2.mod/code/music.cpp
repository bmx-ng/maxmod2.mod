#import "maxmod2.h"

int MAXMOD_VERBOSE       = 0;

extern "C" {
	void MaxMod_SetVerbose(int mode) {MAXMOD_VERBOSE=mode;}

	void MaxMod_AddChannelEffect(IMaxModChannel* channel, IMaxModEffect* effect) {
		MaxMod_ChannelLock();
		effect->MUSIC = channel->MUSIC;
		channel->MUSIC = effect;
		effect->Setup();
		MaxMod_ChannelUnlock();
	}

	void MaxMod_RemoveChannelEffect(IMaxModEffect* effect) {}

}

void mmPrint(string text)              {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << endl;}
void mmPrint(string text,char value)   {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << value << endl;}
void mmPrint(string text,short value)  {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << value << endl;}
void mmPrint(string text,int value)    {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << value << endl;}
void mmPrint(string text,long value)   {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << value << endl;}
void mmPrint(string text,float value)  {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << value << endl;}
void mmPrint(string text,double value) {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << value << endl;}
void mmPrint(string text,string text1) {if (MAXMOD_VERBOSE==1) cout << "MaxMod2: " << text << " " << text1 << endl;}

// ______________________________________________________________________________________________________________

IMaxModChannel* IMaxModMusic::Cue( IMaxModChannel* chn ){

//	mmPrint("IMaxModMusic::Cue");
	MaxMod_ChannelLock();
	SetStatus(1);
	bool add=false;
	if(!chn) {chn = new IMaxModChannel; add=true;} else if(chn->MUSIC) {chn->MUSIC->RemoveRef();}
	chn->MUSIC      = this;
	AddRef();
	chn->CHANNELS   = CHANNELS;
	chn->BITS       = BITS;
	chn->SAMPLERATE = SAMPLERATE;
	chn->STATUS     = 0;
	chn->SIZE       = SIZE;
	chn->POS        = 0;
	chn->LOOPSTART	 = 0;
	chn->LOOPLENGTH = SIZE;
	chn->TERMINATE  = 0;
	chn->RS->setInputFormat(CHANNELS, BITS, SAMPLERATE);
	MaxMod_ChannelUnlock();
	if(add) {
		MaxMod_AddChannel(chn);
//		mmPrint("IMaxModMusic::Cue AllocChannel #=",MaxMod_ChannelCount());
	}
	return chn;

}

