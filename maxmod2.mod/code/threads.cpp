#include "maxmod2.h"
#include <brl.mod/blitz.mod/blitz.h>

#ifdef _WIN32
	
	#include <windows.h>

	DWORD WINAPI threadProc( void* lpParameter ){
		mmPrint("Entered ThreadProc");
		IMaxModChannel* as = (IMaxModChannel*)lpParameter;
		if (as==NULL) {return 0;};
		return as->threadProc();		
	}

	MMThread::~MMThread(){
		if (status==true) Stop();
		CloseHandle(handle);
	}
	
	void MMThread::Start(void* data){
		handle = CreateThread(0, 0, threadProc, data ,0 ,&id);
		status = true;
	}

	int MMThread::Stop(){
		DWORD p = 0;
		WaitForSingleObject( handle,INFINITE );
		GetExitCodeThread( handle,&p );
		status = false;
		return p;
	}

	void MMThread::SetPriority(int Priority){
		SetThreadPriority(handle,Priority);
		mmPrint("thread priority=",GetThreadPriority(handle));
	}


	MMMutex::MMMutex() 		{handle = CreateMutex( 0,0,0 );}
	MMMutex::~MMMutex()		{CloseHandle( handle );}
	void MMMutex::Lock()	{WaitForSingleObject( handle,INFINITE );}
	void MMMutex::Unlock()	{ReleaseMutex( handle );}

#else 
	
	#include <pthread.h>

	int threadProc( void* lpParameter ){
		mmPrint("Entered ThreadProc");
		IMaxModChannel* as = (IMaxModChannel*)lpParameter;
		if (as==NULL) {return 0;};
		return as->threadProc();
	}

	MMThread::~MMThread(){
		if (status==true) Stop();
		pthread_detach( handle );
	}

	void MMThread::Start(void* data){
		pthread_create( &handle,0,(void*(*)(void*))threadProc,data );
		status = true;
	}

	void* MMThread::Stop(){
		void* p=0;
		pthread_join( handle,&p );
		return p;
	}

	void MMThread::SetPriority(int Priority){
		switch(Priority) {
			case -2: param.sched_priority = 2; 	break;
			case -1: param.sched_priority = 1; 	break;
			case 0:  param.sched_priority = 0; 	break;
			case 1:  param.sched_priority = -1; 	break;
			case 2:  param.sched_priority = -2; 	break;
		}
		pthread_setschedparam(handle, SCHED_OTHER, &param);
	}

	MMMutex::MMMutex() 		{pthread_mutex_init( &handle,0 );}
	MMMutex::~MMMutex()		{pthread_mutex_destroy( &handle );}
	void MMMutex::Lock()	{pthread_mutex_lock( &handle );}
	void MMMutex::Unlock()	{pthread_mutex_unlock( &handle );}
	
#endif
	