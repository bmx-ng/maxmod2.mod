#include <maxmod2.mod/maxmod2.mod/code/maxmod2.h>
#include "include/vorbisfile.h"
//#include <pub.mod\oggvorbis.mod\libvorbis-1.1.2\include\vorbis\vorbisfile.h>

static int quiet = 0;
static int bits = 16;
static int raw = 0;
static int sign = 1;
   
// -----------------------------------------------------------------------------------

size_t 	readfunc( void* buf,size_t size,size_t nmemb,void* src );
int 		seekfunc( void* src, ogg_int64_t off, int whence );
int 		closefunc( void* src );
long int 	tellfunc( void* src );

// -----------------------------------------------------------------------------------

class OggPlayer : public IMaxModMusic {

	public:

	IMaxModStream* Stream;
	OggVorbis_File	vf;
	ov_callbacks	cb;

	OggPlayer();
	~OggPlayer();
	
	// -----------------------------------------------------------------------------------
	int 	FillBuffer(void* buffer,int Length);
 	void Stop();
	int 	Seek(int position,int mode);

};

// -----------------------------------------------------------------------------------
