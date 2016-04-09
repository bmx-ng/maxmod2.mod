#include <maxmod2.mod/maxmod2.mod/code/maxmod2.h>

class WavPlayer : public IMaxModMusic {

	public:

	int BytesPerSec;
	int Pad;
	int StartPos;

	IMaxModStream* Stream;

	WavPlayer();
	~WavPlayer();
	
	// -----------------------------------------------------------------------------------
	int 	FillBuffer(void* buffer,int Length);
 	void Stop();
	int 	Seek(int position,int mode);

};

