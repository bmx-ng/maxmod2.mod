#ifndef __MAXMOD_H__
#define __MAXMOD_H__
#include <stdio.h>
#include <cmath>
#include <string.h>
#include <iostream>
#include <list>
using namespace std;

#if __APPLE__ && __BIG_ENDIAN__
static int endian = 1;
#else
static int endian = 0;
#endif

const int MM_BYTES 		= 0;
const int MM_SAMPLES	= 1;
const int MM_MILLISECS	= 2;
const int MM_TRACK		= 3;
const int MM_LINE		= 4;
const int MM_SEQUENCE	= 5;

const int MM_COMPLETE	= -1;
const int MM_STOPPED	= 0;
const int MM_PLAYING	= 1;

void mmPrint(string text);
void mmPrint(string text,char value);
void mmPrint(string text,short value);
void mmPrint(string text,int value);
void mmPrint(string text,long value);
void mmPrint(string text,float value);
void mmPrint(string text,double value);
void mmPrint(string text,string text1);

class MMThread;
class MMMutex;
class IMaxModMusic;
class MaxMod_Resampler;
class IMaxModChannel;
class IMaxModSound;
class IMaxModEffect;
class IMaxModStream;

// ===============================================================================================================

IMaxModChannel* MaxMod_AllocChannel();
void MaxMod_AddChannel(IMaxModChannel* chn);
void MaxMod_RemoveChannel(IMaxModChannel* chn);
int  MaxMod_ChannelCount();
void MaxMod_ChannelLock();
void MaxMod_ChannelUnlock();
void MaxMod_ProcessChannels(void *outputBuffer,unsigned int nBufferFrames);

extern "C" {

	void MaxMod_SetChannelRateAdjuster(IMaxModChannel* channel, double adjuster);

	void MaxMod_AddChannelEffect(IMaxModChannel* channel, IMaxModEffect* effect);
	void MaxMod_RemoveChannelEffect(IMaxModEffect* effect);

	void MaxMod_SetGlobalVolume(double vol);
	double MaxMod_GetGlobalVolume();
	void MaxMod_FillScope(float* buf, int size);
	void MaxMod_FreeChannel(IMaxModChannel* chn);

	IMaxModSound* MaxMod_CreateSound( int channels, int bits, int samplerate, int size, void* samples, int flags );
	void MaxMod_CloseSound(IMaxModSound* sound);

	IMaxModStream* MaxMod_CreateStream(char* fname);
	IMaxModStream* MaxMod_CreateMemStream(void* buf, int length);
	void MaxMod_CloseStream(IMaxModStream*);

	void MaxMod_OutputWavFile(IMaxModMusic* music, char* dst);

	double MaxMod_GetGlobalVolume();


}

// ===============================================================================================================

#ifdef _WIN32

	#include <windows.h>

//	DWORD WINAPI threadProc( void* p );

	class MMThread {
		HANDLE handle;
		DWORD id;
		bool status;
		public:
		~MMThread();
		void Start(void* data);
		int Stop();
		void SetPriority(int Priority);
	};

	class MMMutex {
		HANDLE handle;
		public:
		MMMutex();
		~MMMutex();
		void Lock();
		void Unlock();
	};

#else

	#include <pthread.h>

	class MMThread {
		pthread_t   handle;
		sched_param param;
		bool        status;
		public:
		~MMThread();
		void Start(void* data);
		void * Stop();
		void SetPriority(int Priority);
	};

	class MMMutex {
		pthread_mutex_t handle;
		public:
		MMMutex();
		~MMMutex();
		void Lock();
		void Unlock();
	};

#endif

// ===============================================================================================================

class IMaxModMusic {

	public:

	int SAMPLERATE;		// samples per second
	int CHANNELS;			// number of channels 				: 1=mono, 2=stereo
	int BITS;				// bits per channel
	int BYTESPERSAMPLE;		// number of bytes per sample			: mono8=1, mono16=2, stereo8=2, stereo16=4
	int SIZE;				// length of audio in bytes
	int SEEKABLE;			// seekability						: 1=seekable
	int POS;				// position of the play cursor in bytes
	int LOOP;				// loop mode						: 0=single, 1=loop
	int LOOPSTART;			// position of start of loop in bytes
	int LOOPLENGTH;		// length of audio to loop in bytes
	int STATUS;			// audio status					: 0=finished, 1=active
	int REFCOUNT;			// reference count for this audio
	double RATE;			// mix rate
	int REVERSABLE;		// set to true only if audio can play backwards (IMaxModSound only ATM)

	void(*FreeFunction)(IMaxModMusic* music);

	IMaxModMusic() {
		SAMPLERATE	= 0;
		CHANNELS		= 0;
		BITS			= 0;
		BYTESPERSAMPLE	= 0;
		SIZE			= 0;
		SEEKABLE		= 0;
		POS			= 0;
		LOOP			= 0;
		LOOPSTART		= 0;
		LOOPLENGTH	= 0;
		STATUS		= 0;
		REFCOUNT       = 0;
		RATE			= 0.0;
		REVERSABLE	= 0;
		FreeFunction   = NULL;
	}

// Virtual methods that must be implemented ====================================================

	/*-----------------------------------------------------------------------------------------
	   The FillBuffer function is called by the audio driver when it requires more audio data.
	   The return value is the number of bytes actually written to the output buffer.
	   The audio driver will finish the stream if the return value is less than "length"
	-----------------------------------------------------------------------------------------*/
	virtual int  FillBuffer(void* buffer,int length)=0;

	/*-----------------------------------------------------------------------------------------
	   The Seek function is called either by the user or by the audio driver.
	   The return value should be the new stream position after the seek is done.
	-----------------------------------------------------------------------------------------*/
	virtual int  Seek(int position,int mode)=0;

// Standard methods ============================================================================

	virtual int GetLength(int mode)	{
		switch(mode) {
			case MM_BYTES:		return SIZE;
			case MM_SAMPLES:	return SIZE/((BITS/8)*CHANNELS);
			case MM_MILLISECS:	return ((double)SIZE/((BITS/8)*CHANNELS))/(SAMPLERATE/1000.0);
		}
	}

	virtual int GetPosition(int mode)	{
		switch(mode) {
			case MM_BYTES:		return POS;
			case MM_SAMPLES:	return POS/((BITS/8)*CHANNELS);
			case MM_MILLISECS:	return ((double)POS/((BITS/8)*CHANNELS))/(SAMPLERATE/1000.0);
		}
	}

	virtual int  GetStatus()			{return STATUS;}
	virtual void SetStatus(int state)	{STATUS=state;}
	virtual int  GetChannels()		{return CHANNELS;}
	virtual int  GetBits()			{return BITS;}
	virtual int  GetSampleRate()		{return SAMPLERATE;}
	virtual int  GetLoopMode()		{return LOOP;}
	virtual void SetLoopMode(int mode)	{LOOP=mode;}
	virtual int  IsSeekable()		{return SEEKABLE;}
	virtual void AddRef()			{REFCOUNT++;/*mmPrint("Ref+ =",REFCOUNT);*/}
	virtual void RemoveRef()			{REFCOUNT--;/*mmPrint("Ref- =",REFCOUNT);*/if(REFCOUNT<=0) FreeFunction(this);}
	virtual int  RefCount()			{return REFCOUNT;}

	virtual IMaxModChannel* Cue( IMaxModChannel* chn );

};

// ===============================================================================================================

class IMaxModEffect : public IMaxModMusic {

	public:

	IMaxModMusic* MUSIC;

	// -----------------------------------------------------------------------------------

	virtual int FillBuffer(void* buffer,int length)=0;

	virtual void Setup();
};

// ===============================================================================================================


class IMaxModChannel {

	public:

	IMaxModMusic* 		MUSIC;
	MMMutex* 			AudioWrite_Mutex;
	MaxMod_Resampler* 	RS;

	int   SAMPLERATE,
	      CHANNELS,
	      BITS,
	      SIZE,
	      SEEKABLE,
	      POS,
	      LOOP,
	      LOOPSTART,
	      LOOPLENGTH,
	      STATUS,
	      TERMINATE;

	float VOLUME,
	      RATE,
	      DEPTH,
	      PAN;

	IMaxModChannel();
	~IMaxModChannel();
	virtual void Stop();
	virtual void SetPaused( int paused );
	virtual void SetVolume( float vol );
	virtual void SetPan( float pan );
	virtual void SetDepth( float depth );
	virtual void SetRate( float rate );
	virtual int  Playing();
	virtual int  GetPosition(int mode);
	virtual int  GetLength(int mode);
	virtual int  Seek(int position, int mode);
	virtual int  GetLoopMode();
	virtual void SetLoopMode(int mode);
	virtual void SetLoopPoints(int firstsample,int length);
	virtual void GetUV(double* L, double* R);
	virtual int  IsSeekable();
	virtual int  Fill(void* outputBuffer,int nBufferFrames);
	virtual int  threadProc();

};

// ===============================================================================================================

class IMaxModSound : public IMaxModMusic {

	public:

	char* 	AUDIOBUFFER;
	MMMutex* 	AudioWrite_Mutex;

	IMaxModSound();
	~IMaxModSound();

	int 	FillBuffer(void* buffer,int Length);
	int 	Seek(int position,int mode);

};

// ===============================================================================================================

class IMaxModAudioDriver {

	public:

	int SAMPLERATE;			// samples per second
	int CHANNELS;				// number of channels 				: 1=mono, 2=stereo
	int BITS;					// bits per channel

	IMaxModAudioDriver() {
		SAMPLERATE	= 44100;
		CHANNELS		= 2;
		BITS			= 16;
	}

	//ron: if driver can use different APIs
	virtual int SetAPI(int api)=0;

	virtual int Startup()=0;
	virtual int Shutdown()=0;

	virtual IMaxModSound* CreateSound( int samplerate, int channels, int bits, int flags, void* data, int size )=0;
	virtual IMaxModChannel* AllocChannel() {return MaxMod_AllocChannel();}

};

// ===============================================================================================================

class MemAudioStream : public IMaxModChannel {

	public:
	MMThread* Thread;
	int  Terminate;
	int 	buffersize;
	char* data;
	int  WPOS;

// ______________________________________________________________________________________________________________

	MemAudioStream(IMaxModMusic* Music,char* data);
	virtual ~MemAudioStream();

	virtual void Stop();
	virtual void SetPaused(int paused);
	virtual void SetVolume(float volume);
	virtual void SetPan(float pan);
	virtual void SetDepth(float depth);
	virtual void SetRate(float rate);
	virtual int  Playing();
	virtual int  GetPosition(int Mode);
	virtual int  Seek(int Pos, int Mode);

	virtual void Update();
	virtual void FillEntireBuffer();
	virtual int threadProc();



};

// ===============================================================================================================

class CustomPlayer : public IMaxModMusic {

	public:

	void*	Source;

	// -----------------------------------------------------------------------------------

	CustomPlayer();
	virtual ~CustomPlayer();

	// -----------------------------------------------------------------------------------

	virtual int FillBuffer(void* buffer,int Length);
	virtual int Seek(int position,int mode);

	// -----------------------------------------------------------------------------------

//	virtual void SetSampleRate(float rate);
//	virtual void SetBits(int bits);
//	virtual void SetChannels(int channels);

};

// ===============================================================================================================

class IMaxModStream {

	public:

	int TypeID;

	virtual void 		Close()=0;
	virtual int 		Eof()=0;
	virtual long int	Size()=0;
	virtual long int 	Position()=0;
	virtual long int 	Seek( long int pos )=0;
	virtual long int	SeekFrom(long int pos,int mode)=0;
	virtual int 		Flush()=0;
	virtual long int 	Read( void* buf, size_t count)=0;
	virtual char 		ReadByte()=0;
	virtual short 		ReadShort()=0;
	virtual short 		ReadShortBE()=0;
	virtual int 		ReadInt()=0;
	virtual long 		ReadLong()=0;
	virtual float 		ReadFloat()=0;
	virtual double		ReadDouble()=0;
	virtual string		ReadString(int size)=0;

};

class cfile : public IMaxModStream{

	public:

	FILE* pFile;
	long int SIZE;

	~cfile() {
		Close();
	}

	void 	Close();
	int 		Eof();
	long int	Size();
	long int 	Position();
	long int 	Seek(long int pos);
	long int	SeekFrom(long int pos,int mode);
	int 		Flush();
	long int 	Read(void* buf, size_t count);
	char 	ReadByte();
	short	ReadShort();
	short 	ReadShortBE();
	int 		ReadInt();
	long 	ReadLong();
	float 	ReadFloat();
	double	ReadDouble();
	string	ReadString(int size);

};

class cmem : public IMaxModStream{

	public:

	char* 	pFile;
	long int 	SIZE;
	int		POS;
	int		END;

	cmem() {
		pFile=NULL;
		SIZE=0;
		END=0;
		POS=0;
	}

	~cmem() {
		Close();
	}

	void 	Close();
	int 		Eof();
	long int	Size();
	long int 	Position();
	long int 	Seek(long int pos);
	long int	SeekFrom(long int pos,int mode);
	int 		Flush();
	long int 	Read(void* buf, size_t count);
	char 	ReadByte();
	short	ReadShort();
	short 	ReadShortBE();
	int 		ReadInt();
	long 	ReadLong();
	float 	ReadFloat();
	double	ReadDouble();
	string	ReadString(int size);

};

// ===============================================================================================================

class MaxMod_Resampler {

	public:

	int    STATUS;

	int    inBufferSize;
	void*  inBuffer;
	int    inStart;
	int    inSamples;
	int    inChannels;
	int    inBits;
	int    inCharsPerSample;
	double inSampleRate;
	double inCursor;

	int    outBufferSize;
	void*  outBuffer;
	int    outSamples;
	int    outComplete;
	int    outChannels;
	int    outBits;
	int    outCharsPerSample;
	double outSampleRate;
	double outRate;
	double outVolume;
	double outPanL;
	double outPanR;
	int    outCursor;
	double outl;
	double outr;
	double outLevelLeft;
	double outLevelRight;
	int    outRequestSize;
	double outRateAdjuster;
	int    outAdjusterCount;
	int    outAdjusterTick;
	double outAdjusterRealRate;

	MaxMod_Resampler(int channels,int bits,int rate);
	~MaxMod_Resampler();

	void setInputFormat(int channels, int bits, int rate);
	void requestOutput(void* buffer,int size);
	void pushInput(int size);

	void setPan(double pan);

	int process();

	// -------------------------------------------------------------------------

	int outMono8(){};
	int outMono16(){};
	int outMono32(){};
	int outStereo8(){};
	int outStereo16();
	int outStereo32();
	double Interpolate(double y1,double y2,double mu);
	double InterpolateU8(double y1,double y2,double mu);

};

// ===============================================================================================================




#endif // __MAXMOD_AUDIODRIVER_H__
