#include "maxmod2.h"

MaxMod_Resampler::MaxMod_Resampler(int channels,int bits,int rate) {
	
	STATUS           = 0;
	
	inBufferSize	  = 128*((bits/8)*channels);
	inBuffer		  = new char[inBufferSize];
	inStart		  = 0;
	inSamples		  = 0;
	inChannels	  = 0;
	inBits		  = 0;
	inSampleRate	  = 0;
	inCursor		  = 0;
	memset(inBuffer,0,inBufferSize);
	
	outBufferSize	  = 0;
	outBuffer		  = NULL;
	outSamples	  = 0;
	outComplete	  = 0;
	outChannels	  = channels;
	outBits		  = bits;
	outSampleRate    = rate;
	outRate		  = 1.0;
	outVolume		  = 1.0;
	outPanL		  = 1.0;
	outPanR		  = 1.0;
	outCursor		  = 0;
	outl			  = 0;
	outr			  = 0;
	outLevelLeft	  = 0;
	outLevelRight	  = 0;
	outRequestSize   = 0;
	outRateAdjuster  = 0.0;
	outAdjusterCount = 0;
	outAdjusterTick  = 512;
	outAdjusterRealRate = 0;

}

MaxMod_Resampler::~MaxMod_Resampler(){
	delete[] inBuffer;
}

void MaxMod_Resampler::setInputFormat(int channels, int bits, int rate) {
	inChannels=channels;
	inBits=bits;
	inSampleRate=rate;
}
	
void MaxMod_Resampler::requestOutput(void* buffer,int size) {
	outBuffer=buffer;
	outSamples=size/(outBits/8)/outChannels;
	outRequestSize=outSamples;
	outComplete=0;
	outCursor=0;
	outl=0;
	outr=0;
}

void MaxMod_Resampler::pushInput(int size) {
	inSamples=(size+inStart)/(inBits/8)/inChannels;
	if(outAdjusterRealRate<0) {
//		mmPrint("neg feed");
		inCursor=inSamples-2;
	}
}

void MaxMod_Resampler::setPan(double pan) {
	if(pan==0) {outPanL = 1.0; outPanR = 1.0;}
	else if(pan<0) {outPanL = 1.0;outPanR = 1.0-fabs(pan);} 
	else {outPanL = 1.0-pan;outPanR = 1.0;}
}

int MaxMod_Resampler::process() {

	if(inBufferSize==0)		return 1;	// more input is required
	if(inCursor>inSamples)	return 1;	// more input is required
	if(outComplete==1)		return 0;	// output is available

	return outStereo32();

//	switch(outChannels){
//		case 1:
//			switch(outBits){
//				case 8:	return outMono8();
//				case 16:	return outMono16();
//				case 32:	return outMono32();
//			}
//			break;
//		case 2:
//			switch(outBits){
//				case 8:	return outStereo8();
//				case 16:	return outStereo16();
//				case 32:	return outStereo32();
//			}
//			break;
//	}
//	return -1;	// something is wrong

}


int MaxMod_Resampler::outStereo16(){

	short* outL = (short*)outBuffer; outL+=(outCursor*outChannels);
	short* outR = outL+1;
	double char2short = 32767/128;
	double float2short= 32767;
	int    s=(inBits/8)*inChannels;
	double add=(inSampleRate/outSampleRate)*outRate;
	double frac=0;
	double av;
	
	char*  inC;
	short* inS;
	float* inF;
	
//	mmPrint("MaxMod_Resampler",inBits);
	
	for(int i=outCursor;i<outSamples;i++){

		frac = inCursor-int(inCursor);

		switch(inBits){
			case 8:
				inC = (char*)inBuffer;	
				switch(inChannels){
					case 1:	
						inC+=(int)inCursor;
						av=(Interpolate(*inC,*(inC+1),frac)*outVolume)*char2short;
						*outL += av*outPanL;
						*outR += av*outPanR;
						break;
					case 2:
						inC+=((int)inCursor)*2;
						*outL += (Interpolate(*inC,*(inC+2),frac)*outVolume*outPanL)*char2short;
						*outR += (Interpolate(*(inC+1),*(inC+3),frac)*outVolume*outPanR)*char2short;
						break;
				}
				break;

			case 16:
				inS = (short*)inBuffer;	
				switch(inChannels){
					case 1:	
						inS+=(int)inCursor;
						av=Interpolate(*inS,*(inS+1),frac)*outVolume;
						*outL += av*outPanL;
						*outR += av*outPanR;
						break;
					case 2:
						inS+=((int)inCursor)*2;
						*outL += Interpolate(*inS,*(inS+2),frac)*outVolume*outPanL;
						*outR += Interpolate(*(inS+1),*(inS+3),frac)*outVolume*outPanR;
						break;
				}
				break;
				
			case 32:
				inF = (float*)inBuffer;	
				switch(inChannels){
					case 1:	
						inF+=(int)inCursor;
						av=(Interpolate(*inF,*(inF+1),frac)*outVolume)*float2short;
						*outL += av*outPanL;
						*outR += av*outPanR;
						break;
					case 2:
						inF+=((int)inCursor)*2;
						*outL += (Interpolate(*inF,*(inF+2),frac)*outVolume*outPanL)*float2short;
						*outR += (Interpolate(*(inF+1),*(inF+3),frac)*outVolume*outPanR)*float2short;
						break;
				}
				break;
			
		}

		outL+=2; 
		outR+=2;
		outCursor++;
		inCursor+=add;
		if(inCursor+1>=inSamples) {
			memcpy(inBuffer,(char*)(inBuffer)+(inBufferSize-s),s);
			inStart=s;
			inCursor=inCursor-(int)inCursor;
			return 1;
		}

	} //next

	outComplete=1;
	return 0;
		
}


int MaxMod_Resampler::outStereo32(){

	float* outL = (float*)outBuffer; outL+=(outCursor*outChannels);
	float* outR = outL+1;
	int    s=(inBits/8)*inChannels;
	double add;						//=(inSampleRate/outSampleRate)*outRate;
	double frac=0;
	double av;
	
	double char2float = 1.0/127;
	double short2float = 1.0/32767;
	unsigned char* inC;	
	short* inS;	
	float* inF;	

	if(STATUS==1) {
		add=(inSampleRate/outSampleRate)*(outRate+outRateAdjuster);
	} else {
		add=(inSampleRate/outSampleRate)*outRateAdjuster;
	}
	outAdjusterRealRate=add;
	
	for(int i=outCursor;i<outSamples;i++){

		if(inCursor<0) break;
		frac = inCursor-int(inCursor);

		if(outAdjusterCount==outAdjusterTick) {
			outAdjusterCount=0;
			outRateAdjuster*=0.9;
			//if(fabs(outRateAdjuster)<0.001) outRateAdjuster=0;
			if(STATUS==1) {
				add=(inSampleRate/outSampleRate)*(outRate+outRateAdjuster);
			} else {
				add=(inSampleRate/outSampleRate)*outRateAdjuster;
			}
			outAdjusterRealRate=add;
		} else {
			outAdjusterCount++;
		}

		switch(inBits){
			case 8:
				inC = (unsigned char*)inBuffer;	
				switch(inChannels){
					case 1:	
						inC += (int)inCursor;
						av = (InterpolateU8(*inC,*(inC+1),frac)*outVolume)*char2float;
						*outL += av*outPanL;
						if(fabs(av)*outPanL>outl) outl=fabs(av)*outPanL;
						*outR += av*outPanR;
						if(fabs(av)*outPanR>outr) outr=fabs(av)*outPanR;
						break;
					case 2:
						inC += ((int)inCursor)*2;
						av = (InterpolateU8(*inC,*(inC+2),frac)*outVolume*outPanL)*char2float;
						*outL += av;
						if(fabs(av)>outl) outl=fabs(av);
						av = (InterpolateU8(*(inC+1),*(inC+3),frac)*outVolume*outPanR)*char2float;
						*outR += av;
						if(fabs(av)>outr) outr=fabs(av);
						break;
				}
				break;

			case 16:
				//mmPrint("MaxMod_Resampler::outStereo32 in16");
				inS = (short*)inBuffer;	
				switch(inChannels){
					case 1:	
						inS+=(int)inCursor;
						av=(Interpolate(*inS,*(inS+1),frac)*outVolume)*short2float;
						*outL += av*outPanL;
						if(fabs(av)*outPanL>outl) outl=fabs(av)*outPanL;
						*outR += av*outPanR;
						if(fabs(av)*outPanR>outr) outr=fabs(av)*outPanR;
						break;
					case 2:
						inS+=((int)inCursor)*2;
						av = (Interpolate(*inS,*(inS+2),frac)*outVolume*outPanL)*short2float;
						*outL += av;
						if(fabs(av)>outl) outl=fabs(av);
						av = (Interpolate(*(inS+1),*(inS+3),frac)*outVolume*outPanR)*short2float;
						*outR += av;
						if(fabs(av)>outr) outr=fabs(av);
						break;
				}
				break;
				
			case 32:
				//mmPrint("MaxMod_Resampler::outStereo32 in32");
				inF = (float*)inBuffer;	
				switch(inChannels){
					case 1:	
						inF+=(int)inCursor;
						av=Interpolate(*inS,*(inS+1),frac)*outVolume;
						*outL += av*outPanL;
						if(fabs(av)*outPanL>outl) outl=fabs(av)*outPanL;
						*outR += av*outPanR;
						if(fabs(av)*outPanR>outr) outr=fabs(av)*outPanR;
						break;
					case 2:
						inF+=((int)inCursor)*2;
						av = Interpolate(*inF,*(inF+2),frac)*outVolume*outPanL;
						*outL += av;
						if(fabs(av)>outl) outl=fabs(av);
						av = Interpolate(*(inF+1),*(inF+3),frac)*outVolume*outPanR;
						*outR += av;
						if(fabs(av)>outr) outr=fabs(av);
						break;
				}
				break;
			
		}
			
		outL+=2; 
		outR+=2;
		outCursor++;
		inCursor+=add;
		if(add>0) {
			if(inCursor+1>=inSamples) {
				memcpy(inBuffer,(char*)(inBuffer)+(inBufferSize-s),s);
				inStart=s;
				inCursor=inCursor-(int)inCursor;
				return 1;
			}
		} else if(add<0) {
			if(inCursor-1<0) {
				//mmPrint("inCursor<0");
				return 1;
			}
		
		}

	} //next

	outl*=MaxMod_GetGlobalVolume();
	if (outl>1.0) outl=1.0;
	if (outl>outLevelLeft) {outLevelLeft = outl;} 
	else {outLevelLeft += (outl-outLevelLeft)*0.25;}
	
	outr*=MaxMod_GetGlobalVolume();
	if (outr>1.0) outr=1.0;
	if (outl>outLevelRight) {outLevelRight = outr;} 
	else {outLevelRight += (outr-outLevelRight)*0.25;}

	outComplete=1;
	return 0;
		
}


	// Linear
//double MaxMod_Resampler::Interpolate(double a,double b,double frac){
//	return a+((b-a)*frac);
//}

// Cosine
double MaxMod_Resampler::Interpolate(double y1,double y2,double mu){
	mu-=(int)mu;
	double mu2;
	mu2 = (1-cos(mu*M_PI))/2;
	return(y1*(1-mu2)+y2*mu2);
}

// Cosine
double MaxMod_Resampler::InterpolateU8(double y1,double y2,double mu){
	y1-=128;
	y2-=128;
	double mu2=(1.0-cos(mu*M_PI))/2;
	return(y1*(1.0-mu2)+y2*mu2);
}
