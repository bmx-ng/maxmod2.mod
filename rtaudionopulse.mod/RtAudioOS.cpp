
#ifdef _WIN32
#define __WINDOWS_DS__
//#define __WINDOWS_ASIO__
#endif

#ifdef __linux__
#define __LINUX_ALSA__
//add by ron : add pulse support
//#define __LINUX_PULSE__
//#define __UNIX_JACK__
#define __LINUX_OSS__
#endif

#ifdef __APPLE__
#define __MACOSX_CORE__
#endif

#include "../rtaudio.mod/rtaudio/RtAudio.cpp"
