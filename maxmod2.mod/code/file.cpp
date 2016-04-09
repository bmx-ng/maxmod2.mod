#include "maxmod2.h"

// =================================================================================================================
// cfile methods

void 	cfile::Close() 					{fclose(pFile);}
int 		cfile::Eof() 						{return feof(pFile);}
long int 	cfile::Size() 						{return SIZE;}
long int 	cfile::Position() 					{if (!Eof()) {return ftell(pFile);}; return -1;}
long int 	cfile::Seek(long int pos)			{SeekFrom(pos,SEEK_SET); return ftell(pFile);}
long int 	cfile::SeekFrom(long int pos,int mode)	{fseek(pFile,pos,mode); return ftell(pFile);}
int 		cfile::Flush()						{return fflush(pFile);}
long int 	cfile::Read( void* buf, size_t count)	{if (Eof()) {return 0;}; return fread(buf,1,count,pFile);}
char 	cfile::ReadByte()					{char v; fread(&v,1,1,pFile); return v;}
short 	cfile::ReadShort()					{short v; fread(&v,1,2,pFile); return v;}
short 	cfile::ReadShortBE() {
	int temp 	= ReadShort();
	#if __APPLE__ && __BIG_ENDIAN__
		return (short)temp;
	#endif
	int temp1 	= (temp&255)	; temp1<<=8;
	int temp2 	= (temp&65280)	; temp2>>=8;
	return (short)(temp1+temp2);
}
int 		cfile::ReadInt()					{int v; fread(&v,1,4,pFile); return v;}
long		cfile::ReadLong()					{long v; fread(&v,1,8,pFile); return v;}
float	cfile::ReadFloat()					{float v; fread(&v,1,4,pFile); return v;}
double	cfile::ReadDouble()					{double v; fread(&v,1,8,pFile); return v;}
string	cfile::ReadString(int size)			{char v[size+1]; fread(&v,1,size,pFile);v[size]=0; return (string)v;}

// =================================================================================================================
// cmem methods

void 	cmem::Close() 						{}
int 		cmem::Eof() 						{return END;}
long int 	cmem::Size() 						{return SIZE;}
long int 	cmem::Position() 					{if (END==0) {return POS;};}

long int 	cmem::Seek(long int pos)	{
	if (pos>=SIZE) {POS=SIZE; END=1; return POS;}
	return SeekFrom(pos,SEEK_SET);
//	END=0;
//	POS=pos; return POS;
}

long int 	cmem::SeekFrom(long int pos,int mode)	{
	//mmPrint("CMEM SEEK");
	switch(mode){
  		case SEEK_SET:
			//seek to start of the "file"
			if(pos>SIZE)
				POS = SIZE;
			else
				POS = pos;
			break;

		case SEEK_CUR:
			//Seek from current position
			//-> do not go past end
			//-> available space = SIZE - POS
			if (pos < (SIZE - POS))
				POS+= pos;
			else
				POS = SIZE;
			break;

		case SEEK_END:
			//Seek to end
			POS = SIZE + 1;
			break;
	};
	//adjust if END is reached
	END = (POS>SIZE);

	//return current pos
	return POS;

/*
	switch(mode){
  		case SEEK_SET:
			if (pos>SIZE) {POS=SIZE; END=1; return POS;}
			END=0;
			POS=pos; return POS;
		case SEEK_CUR:
			if (POS+pos>SIZE) {POS=SIZE; END=1; return POS;}
			END=0;
			POS+=pos; return POS;
		case SEEK_END:
			if (POS-pos<0) {POS=0; END=0; return POS;}
			END=0;
			POS-=pos; return POS;
	};
*/
}

int 		cmem::Flush()						{}
long int 	cmem::Read( void* buf, size_t count)	{
	char* buff = (char*)buf;
	for(int n=0;n<count;n++) {
		*buff++=pFile[POS++];
		if (POS>=SIZE) {END=1;return n;}
	}
	return count;
}

char 	cmem::ReadByte()					{char v; Read(&v,1); return v;}
short 	cmem::ReadShort()					{short v; Read(&v,2); return v;}
short 	cmem::ReadShortBE() {
	int temp 	= ReadShort();
	#if __APPLE__ && __BIG_ENDIAN__
		return (short)temp;
	#endif
	int temp1 	= (temp&255)	; temp1<<=8;
	int temp2 	= (temp&65280)	; temp2>>=8;
	return (short)(temp1+temp2);
}
int 		cmem::ReadInt()					{int v; Read(&v,4); return v;}
long		cmem::ReadLong()					{long v; Read(&v,8); return v;}
float	cmem::ReadFloat()					{float v; Read(&v,4); return v;}
double	cmem::ReadDouble()					{double v; Read(&v,8); return v;}
string	cmem::ReadString(int size)			{char v[size+1]; Read(&v,size);v[size]=0; return (string)v;}





extern "C" {

	IMaxModStream* MaxMod_CreateStream(char* fname) {
		FILE* pFile;
		pFile = fopen(fname,"rb");
		if (!pFile) return NULL;
		cfile* cf = new cfile;
		cf->pFile = pFile;
		fseek(pFile,0,SEEK_END);
		cf->SIZE = ftell(pFile);
		fseek(pFile,0,SEEK_SET);
		cf->TypeID = 0;
		return cf;
	}

	IMaxModStream* MaxMod_CreateMemStream(void* buf, int length) {
		cmem* cf = new cmem;
		cf->pFile = (char*)buf;
		cf->SIZE = length;
		cf->TypeID = 1;
		cf->Seek(0);
		return cf;
	}

	void MaxMod_CloseStream(IMaxModStream* stream) {
		stream->Close();
		delete stream;
	}

}
