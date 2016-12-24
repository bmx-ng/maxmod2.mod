SuperStrict

Rem
bbdoc: MaxMod2.MaxMod2
End Rem
Module MaxMod2.MaxMod2
ModuleInfo "MaxMod2: Audio Driver V1.0"
ModuleInfo "Author: REDi - Cliff Harman"
ModuleInfo "License: MIT"

Import BRL.Audio
Import BRL.LinkedList
Import BRL.Map
Import BRL.Bank
Import BRL.System
Import BRL.StandardIO

Import "code/*.h"
Import "code/music.cpp"
Import "code/file.cpp"
Import "code/fileoutstream.cpp"
Import "code/threads.cpp"
Import "code/custom.cpp"
Import "code/memorystream.cpp"
Import "code/resampler.cpp"
Import "code/sound.cpp"
Import "code/channel.cpp"

' POSITION CONSTANCE
Const MM_BYTES		:Int = 0
Const MM_SAMPLES	:Int = 1
Const MM_MILLISECS	:Int = 2
Const MM_TRACK		:Int = 3
Const MM_LINE		:Int = 4
Const MM_SEQUENCE	:Int = 5

' ---------------------------------------------------------------------------------------

Extern "C"
?bmxng
	Interface IMaxModAudioDriver
?Not bmxng
	Type IMaxModAudioDriver
?
		Method SetAPI:Int				(api:Int)
		Method Startup:Int				()
		Method Shutdown:Int				()
		Method CreateSound:IMaxModMusic	(sample:TAudioSample, flags:Int)
		Method AllocChannel:IMaxModChannel	()
?bmxng
	End Interface
?Not bmxng
	EndType
?

?bmxng
	Interface IMaxModMusic
?Not bmxng
	Type IMaxModMusic
?
		Method FillBuffer:Int			(buffer:Byte Ptr, length:Int)
		Method Seek:Int				(position:Int, Mode:Int)
		Method GetLength:Int			(Mode:Int)
		Method GetPosition:Int			(Mode:Int)
		Method GetStatus:Int			()
		Method SetStatus				(state:Int)
		Method GetChannels:Int			()
		Method GetBits:Int				()
		Method GetSampleRate:Int			()
		Method GetLoopMode:Int			()
		Method SetLoopMode				(Mode:Int)
		Method IsSeekable:Int			()
		Method AddRef					()
		Method RemoveRef				()
		Method RefCount:Int				()
		Method Cue:IMaxModChannel		(alloced_channel:IMaxModChannel)
?bmxng
	End Interface
?Not bmxng
	EndType
?

?bmxng
	Interface IMaxModEffect
?Not bmxng
	Type IMaxModEffect
?
		Method FillBuffer:Int			(buffer:Byte Ptr, length:Int)
		Method Seek:Int				(position:Int, Mode:Int)
		Method GetLength:Int			(Mode:Int)
		Method GetPosition:Int			(Mode:Int)
		Method GetStatus:Int			()
		Method SetStatus				(state:Int)
		Method GetChannels:Int			()
		Method GetBits:Int				()
		Method GetSampleRate:Int			()
		Method GetLoopMode:Int			()
		Method SetLoopMode				(Mode:Int)
		Method IsSeekable:Int			()
		Method AddRef					()
		Method RemoveRef				()
		Method RefCount:Int				()
		Method Cue:IMaxModChannel		(alloced_channel:IMaxModChannel)
?bmxng
	End Interface
?Not bmxng
	EndType
?

?bmxng
	Interface IMaxModChannel
?Not bmxng
	Type IMaxModChannel
?
		Method Stop					()
		Method SetPaused				(paused:Int)
		Method SetVolume				(volume:Float)
		Method SetPan					(pan:Float)
		Method SetDepth				(depth:Float)
		Method SetRate					(rate:Float)
		Method Playing:Int				()
		Method GetPosition:Int			(Mode:Int)
		Method GetLength:Int               (Mode:Int)
		Method Seek:Int				(position:Int, Mode:Int)
		Method GetLoopMode:Int			()
		Method SetLoopMode				(bool:Int)
		Method SetLoopPoints			(startsample:Int,endsample:Int)
		Method GetUV					(l:Double Var,r:Double Var)
		Method IsSeekable:Int              ()
?bmxng
	End Interface
?Not bmxng
	EndType
?

?bmxng
	Interface IMaxModStream
?Not bmxng
	Type IMaxModStream
?
		Method Close					()
		Method Eof:Int					()
		Method Size:Long				()
		Method Position:Long			()
		Method Seek:Long				(pos:Long)
		Method SeekFrom:Long			(pos:Long, Mode:Int)
		Method Flush:Int				()
		Method Read:Long				(buf:Byte Ptr, count:Long)
?bmxng
	End Interface
?Not bmxng
	EndType
?

	Function MaxMod_SetGlobalVolume(vol:Double)
	Function MaxMod_FreeChannel(channel:IMaxModChannel)

	Function MaxMod_CreateSound:IMaxModMusic( channels:Int, bits:Int, samplerate:Int, size:Int, samples:Byte Ptr, flags:Int)
	Function MaxMod_CloseSound(stream:IMaxModMusic)

	Function MaxMod_CreateStream:IMaxModStream(file$z)
	Function MaxMod_CreateMemStream:IMaxModStream(buf:Byte Ptr, length:Int)
	Function MaxMod_CloseStream(stream:IMaxModStream)

	Function MaxMod_SetVerbose(bool:Int)
	Function MaxMod_FillScope(buf:Byte Ptr,size:Int)

	Function LoadMusic_Custom:IMaxModMusic(MusicCustom:TCustomMusic,SampleRate:Int,Channels:Int,Bits:Int)
	Function CloseMusic_Custom(Music:IMaxModMusic)

	Function CreateAudioStream_Memory:IMaxModChannel(Music:IMaxModMusic,data:Byte Ptr)
	Function CloseAudioStream_Memory(Stream:IMaxModChannel)

	Function MaxMod_SetChannelRateAdjuster(channel:IMaxModChannel, adjuster:Double)

	Function MaxMod_AddChannelEffect(IMaxModChannel:Byte Ptr, IMaxModEffect:Byte Ptr)
'	Function MaxMod_RemoveChannelEffect(IMaxModEffect:Byte Ptr)
'	Function MaxMod_LockChannel(IMaxModChannel:Byte Ptr)
'	Function MaxMod_UnlockChannel(IMaxModChannel:Byte Ptr)

	Function MaxMod_OutputWavFile(Music:IMaxModMusic, outputfile$z)

EndExtern

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Set the verbose debug mode for all maxmod modules.
about: Calling this function with a value of True will cause maxmod to spew debug infomation during execution.
End Rem
Function MaxModVerbose(bool:Int)
	MaxMod_SetVerbose(bool)
EndFunction

Function MaxModFillScope(buf:Byte Ptr,size:Int)
	MaxMod_FillScope(buf,size)
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Initialize MaxMods audio stream for playback.
about: You can use this function to start an audio stream driver without using it for sample playback, allowing you to still use other audio drivers and still take advantage of maxmod2 audio streaming.
End Rem
Function SetAudioStreamDriver(name$)
	TMaxModStreamDriver.SetDriver(name$)
EndFunction

Type TMaxModStreamDriver

	Global Drivers:TMap = New TMap

	Function AddDriver(Driver:TMaxModDriver,name$)
		If Not driver Or name$="" Return
		MapInsert(Drivers,name.tolower(),Driver)
	EndFunction

	Function SetDriver(name$)
		Local this:TMaxModDriver = TMaxModDriver(MapValueForKey( Drivers, name.tolower() ))
		If this=Null Then Notify "Unable to find maxmod2 audio driver ~q"+name+"~q" ; Return
		If Not this.Startup() Then Notify "Unable to start maxmod2 audio stream driver ~q"+name+"~q"; Return
	EndFunction

EndType

' ---------------------------------------------------------------------------------------

'ron: better getter for current audio driver
Function GetCurrentAudioDriver:TMaxModDriver()
	Return TMaxModDriver.Active
End Function

Type TMaxModDriver Extends TAudioDriver

	Field _driver:IMaxModAudioDriver
	Field _name:String
	'Ron: store the active driver here for easier access
	Global Active:TMaxModDriver

	'ron: to access currently set api
	'default is automatic
	Field _currentAPI:Int = 0
	'ron: amount of apis the driver handles
	Global APIs:TMap = CreateMap()

	Method Name$()
		Return _name
	EndMethod

	Method AddAPI(id:Int, name:String)
		Self.APIs.Insert(String(id), name)
	End Method

	Method RemoveAPI(id:Int)
		Self.APIs.Remove(String(id))
	End Method

	'ron - setter if driver has different APIs
	Method SetAPI:Int(api:Int)
		Self._currentAPI = api
		Return _driver.SetAPI(api)
	End Method

	'ron - setter if driver has different APIs
	'default is first api (0)
	Method SetNamedAPI:Int(namedAPI:String)
		Return Self.SetApi( GetAPIid(namedAPI) )
	End Method

	Method GetAPIid:Int(namedAPI:String)
		namedAPI = namedAPI.toUpper()
		For Local key:String = EachIn Self.APIs.Keys()
			If namedAPI = String( Self.APIs.ValueForKey(key) )
				Return Int(key)
			EndIf
		Next
		'set to default
		Return 0
	End Method

	Method GetAPIName:String( api:Int = -1)
		For Local key:String = EachIn Self.APIs.Keys()
			If Int(key) = api Then Return String( Self.APIs.ValueForKey(key) )
		Next
		Return "AUTOMATIC"
	End Method

	'ron: get a array of accepted API names
	Method GetAPINames:Object[]()
		Local res:TList = CreateList()
		res.addLast("AUTOMATIC")
		For Local str:String = EachIn Self.APIs.Values()
			res.addLast( str )
		Next
		Return res.toArray()
	End Method

	Method registerAPIs()
		Self.AddAPI(0, "AUTOMATIC")	'= "UNSPECIFIED"
	End Method

	Method Startup:Int()
		Return _driver.Startup()
	End Method

	Method Shutdown()
		_driver.Shutdown()
	End Method

	Method CreateSound:TMaxModSound( sample:TAudioSample,flags:Int )
		Local snd:TMaxModSound = New TMaxModSound
		Local chns:Int = ChannelsPerSample[sample.format]
		Local bits:Int = BytesPerSample[sample.format]*(8/chns)
		Local size:Int = sample.length*BytesPerSample[sample.format]
		snd._sound = MaxMod_CreateSound( chns, bits, sample.hertz, size, sample.samples, flags)
		snd._sound.AddRef()
		snd._audiosample = sample
		If snd._sound Return snd
	End Method

	Method AllocChannel:TMaxModChannel()
		Local chn:TMaxModChannel = New TMaxModChannel
		chn._channel =	_driver.AllocChannel()
		If chn._channel Return chn
	End Method

	Function Create:TMaxModDriver( name$ ) Abstract

End Type

' ---------------------------------------------------------------------------------------

Type TMaxModSound Extends TSound

	Field _audiosample:TAudioSample
	Field _sound:IMaxModMusic

	Method Delete()
		MaxMod_CloseSound(_sound)
		_sound.RemoveRef() '; Print "(TMaxModSound.Delete _sound)"
	End Method

	Method Play:TMaxModChannel( alloced_channel:TChannel )
		Local chn:TMaxModChannel = Cue(alloced_channel)
		ResumeChannel(chn)
		Return chn
	End Method

	Method Cue:TMaxModChannel( alloced_channel:TChannel )
		Local chn:TMaxModChannel = TMaxModChannel(alloced_channel)
		If Not chn
			chn=New TMaxModChannel
		Else
			If chn._sound Then chn._sound._sound.RemoveRef()
			If chn._music Then chn._music.Music.RemoveRef()
		EndIf
		chn._sound   = Self
		chn._channel = _sound.Cue(chn._channel)
		_sound.AddRef()
		Return chn
	End Method

End Type

' ---------------------------------------------------------------------------------------

Type TMaxModChannel Extends TChannel

	Field _sound:TMaxModSound
	Field _channel:IMaxModChannel
	Field _music:TMusic

	Method Delete()
		If _sound Then _sound._sound.RemoveRef()
		If _music Then _music.Music.RemoveRef()
		MaxMod_FreeChannel(_channel)
	End Method

	Method Stop()
		_channel.Stop()
	End Method

	Method SetPaused( paused:Int )
		_channel.SetPaused paused
	End Method

	Method SetVolume( volume# )
		_channel.SetVolume volume
	End Method

	Method SetPan( pan# )
		_channel.SetPan pan
	End Method

	Method SetDepth( depth# )
		_channel.SetDepth depth
	End Method

	Method SetRate( rate# )
		_channel.SetRate rate
	End Method

	Method Playing:Int()
		Return _channel.Playing()
	End Method

	Method Lock()
'		MaxMod_ChannelLock(_channel)
	EndMethod

	Method Unlock()
'		MaxMod_ChannelUnlock(_channel)
	EndMethod

	Method SetChannelRateAdjuster(adjuster!)
		MaxMod_SetChannelRateAdjuster(_channel, adjuster)
	EndMethod

End Type
' ---------------------------------------------------------------------------------------

Type TMusic

	Field Music:IMaxModMusic
	Field Stream:IMaxModStream
	Field Bank:TBank

	Function Loader:TMusic(Stream:IMaxModStream,Filename$="") Abstract

	Method FillBuffer:Int(buffer:Byte Ptr,length:Int)
		Return Music.FillBuffer(buffer,length)
	EndMethod

	Method Seek:Int(position:Int,Mode:Int)
		Return Music.Seek(position,Mode)
	EndMethod

	Method IsSeekable:Int()
		Return Music.IsSeekable()
	EndMethod

	Method GetLength:Int(Mode:Int=MM_BYTES)
		Return Music.GetLength(Mode)
	EndMethod

	Method GetStatus:Int()
		Return Music.GetStatus()
	EndMethod

	Method SetStatus:Int(state:Int)
		Music.SetStatus(state)
	EndMethod

	Method GetPosition:Int(Mode:Int=MM_BYTES)
		Return Music.GetPosition(Mode)
	EndMethod

	Method GetChannels:Int()
		Return Music.GetChannels()
	EndMethod

	Method GetBits:Int()
		Return Music.GetBits()
	EndMethod

	Method GetSampleRate:Int()
		Return Music.GetSampleRate()
	EndMethod

	Method GetLoopMode:Int()
		Return Music.GetLoopMode()
	EndMethod

	Method SetLoopMode(Mode:Int)
		Music.SetLoopMode(Mode)
	EndMethod

	Method GetFormat:Int()
		If GetChannels()=2
			If GetBits()=8
				Return 4 'SF_Stereo8
			Else
				?BIGENDIAN
					Return 6 'SF_Stereo16BE
				?LITTLEENDIAN
					Return 5 'SF_Stereo16LE
				?
			EndIf
		Else
			If GetBits()=8
				Return 1 'SF_Mono8
			Else
				?BIGENDIAN
					Return 3 'SF_Mono16BE
				?LITTLEENDIAN
					Return 2 'SF_Mono16LE
				?
			EndIf
		EndIf
	EndMethod

EndType

' --------------------------------------------------------------------------------------------------

Type TMaxModEffect

	Field _effect:IMaxModEffect

EndType

' ---------------------------------------------------------------------------------------

Function MaxModMainCustomThreadProc:Int(Source:TCustomMusic, buffer:Byte Ptr, size:Int)
	Return Source.FillBuffer(buffer,size)
EndFunction


Rem
bbdoc: Start a new audio callback stream.
returns: A TChannel object
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Music</TD><TD>A TCustomMusic object to use for audio stream</TD></TR>
<TR><TD>Loop</TD><TD>True to loop the stream forever<BR>False to play the stream only once (defaults to False)</TD></TR>
<TR><TD>Paused</TD><TD>True to create the stream in a paused state<BR>False to start playback on creation of the stream (defaults to False)</TD></TR>
</TABLE><P>
EndRem
Function CreateAudioCallbackStream:TChannel(Music:TCustomMusic,Loop:Int=False,Paused:Int=False)
	?Threaded
		If Not Music Return Null
'		If Music.Music Then CloseMusic_Custom(Music.Music)
		Music.Music = LoadMusic_Custom(Music,Music.SampleRate,Music.Channels,Music.Bits)
		Return PlayMusic(Music,Loop)
	?Not Threaded
		Notify "CreateAudioCallbackStream will only work if threading is enabled"
	?
EndFunction

Type TCustomMusic Extends TMusic

	Field SampleRate:Int = 44100
	Field Bits:Int       = 16
	Field Channels:Int   = 2

	Function Loader:TMusic(Stream:IMaxModStream,Filename$)
	EndFunction

	Method FillBuffer:Int(buffer:Byte Ptr,length:Int) Abstract

	Method Delete()
'		CloseMusic_Custom(Music)
	EndMethod

EndType

' --------------------------------------------------------------------------------------------------

Type MaxModLoader

	Global MMLoader:TMusic(Stream:IMaxModStream,Filename$)
	Global List:TList = CreateList()

	Field Name$
	Field Func:Byte Ptr
	Field Ext$

	Method New()
		List.AddLast(Self)
	EndMethod

	Function Create:MaxModLoader(Func:Byte Ptr,Description$,Extension$)
		Local Mod_Loader:MaxModLoader = New MaxModLoader
		Mod_Loader.Func  = Func
		Mod_Loader.Name$ = Description$
		Mod_Loader.Ext$  = Extension$
	EndFunction

	Function GetExtensions$(AllSupported:Int=True,AllFiles:Int=True)
		Local Txt$
		If AllSupported
			Txt$:+"All supported:"
			For Local This:MaxModLoader = EachIn List
				Txt$:+","+This.Ext.tolower()
			Next
		EndIf
		For Local This:MaxModLoader = EachIn List
			If Txt$<>"" Then Txt$:+";"
			Txt$:+This.Name$+":"+This.Ext.tolower()
		Next
		If AllFiles
			If Txt$<>"" Then Txt$:+";"
			Txt$:+"All Files:*"
		EndIf
		Return txt$
	EndFunction

	Function Load:TMusic(URL:Object)
		Local NewSong:TMusic
		Local str:String = String(URL)
		Local Bank:TBank = TBank(URL)
		Local Stream:IMaxModStream

		If Str
			If Str.find("incbin::")=>0
				Local arr$[] = Str.Split( "::" )
				Stream = MaxMod_CreateMemStream(IncbinPtr(arr[1]),IncbinLen(arr[1]))
			Else
				Stream = MaxMod_CreateStream(Str)
			EndIf
		ElseIf Bank
			Stream = MaxMod_CreateMemStream(Bank.Buf(), Int(Bank.Size()))
			URL = "Bank"
		EndIf

		If Not Stream Return Null
		For Local This:MaxModLoader = EachIn List
			Stream.Seek(0)
			If This.Func
				MaxModLoader.MMLoader = This.Func
				NewSong = MaxModLoader.MMLoader(Stream,String(URL))
				If NewSong
					If Bank NewSong.Bank = Bank
					Return NewSong
				EndIf
			EndIf
		Next
		MaxMod_CloseStream(Stream)

	EndFunction

EndType

Function LoadTMusic:TMusic(File:Object)
	Return MaxModLoader.Load(File:Object)
EndFunction

Rem
bbdoc: Query supported music formats for streaming.
returns: a string for use with RequestFile() ect
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>AllSupported</TD><TD>adds an "all supported" item to the filter list</TD></TR>
<TR><TD>AllFiles</TD><TD>adds an "all files" item to the filter list</TD></TR>
<TR><TD></TD><TD></TD></TR>
</TABLE><P>
End Rem
Function MusicExtensions:String(AllSupported:Int=True,AllFiles:Int=True)
	Return MaxModLoader.GetExtensions(AllSupported,AllFiles)
EndFunction

' --------------------------------------------------------------------------------------------------

Rem
bbdoc: Streams audio outputs to a TAudioSample for use with the BlitzMax LoadSound command.
returns: A TAudioSample object
End Rem
Function LoadMusic:TAudioSample( URL:String )
	Local Music:TMusic
	Music = LoadTMusic(URL)
	If Not Music Return Null
	Local length   :Int = Music.GetLength(MM_SAMPLES)
	Local lengthb  :Int = Music.GetLength(MM_BYTES)
	Local rate     :Int = Music.GetSampleRate()
	Local format   :Int = Music.GetFormat()
	Local Sample	:TAudioSample = CreateAudioSample(length, rate, format)
	If Not Sample Return Null
	Music.Fillbuffer(Sample.samples,lengthb)
	Music=Null
	GCCollect()
	Return Sample
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Cue a new music stream.
returns: A TChannel object
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>URL</TD><TD>URL to the audio you want to stream</TD></TR>
<TR><TD>Loop</TD><TD>True to loop the stream forever<BR>False to play the stream only once (defaults to False)</TD></TR>
<TR><TD>Paused</TD><TD>True to create the stream in a paused state<BR>False to start playback on creation of the stream (defaults to False)</TD></TR>
</TABLE><P>
EndRem
Function CueMusic:TChannel(URL:Object,Loop:Int=False)
	Local mus:TMusic=TMusic(URL)
	If Not mus mus=LoadTMusic(URL)
	If Not mus Return Null
	mus.music.SetLoopMode(Loop)
	mus.music.AddRef()
	Local chn:TMaxModChannel = New TMaxModChannel
	chn._channel = mus.music.Cue(Null)
	chn._channel.SetLoopMode(Loop)
	chn._music = mus
	Return chn
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Start a new music stream.
returns: A TChannel object
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>URL</TD><TD>URL to the audio you want to stream</TD></TR>
<TR><TD>Loop</TD><TD>True to loop the stream forever<BR>False to play the stream only once (defaults to False)</TD></TR>
<TR><TD>Paused</TD><TD>True to create the stream in a paused state<BR>False to start playback on creation of the stream (defaults to False)</TD></TR>
</TABLE><P>
EndRem
Function PlayMusic:TChannel(URL:Object,Loop:Int=False)
	Local chn:TChannel = CueMusic(URL,Loop)
	If Not chn Return Null
	ResumeChannel(chn)
	Return chn
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Streams music to memory using a thread.
returns: A TChannel object
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>URL</TD><TD>URL to the audio you want to stream</TD></TR>
<TR><TD>buf</TD><TD>a pointer to the memory where to write</TD></TR>
</TABLE><P>
EndRem
Function CreateMemoryStream:TChannel(URL:Object,buf:Byte Ptr)
	Local mus:TMusic = TMusic(URL)
	If Not mus mus:TMusic = LoadTMusic(String(URL))
	If Not mus Return Null
	mus.music.Seek(0,MM_SAMPLES)
	Local chn:TMaxModChannel = New TMaxModChannel
	chn._channel = CreateAudioStream_Memory(mus.music,buf)
	chn._music = mus
	mus.music.AddRef()
	If Not chn._channel Return Null
	Return chn
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Start a new audio callback stream.
returns: A TChannel object
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Music</TD><TD>A TCustomMusic object to use for audio stream</TD></TR>
<TR><TD>Loop</TD><TD>True to loop the stream forever<BR>False to play the stream only once (defaults to False)</TD></TR>
<TR><TD>Paused</TD><TD>True to create the stream in a paused state<BR>False to start playback on creation of the stream (defaults to False)</TD></TR>
</TABLE><P>
EndRem
'Function CreateAudioCallbackStream:TChannel(Music:TCustomMusic,Loop:Int=False,Paused:Int=False)
'	?Threaded
'		If Not Music Return Null
'		If Music.Music Then CloseMusic_Custom(Music.Music)
'		Music.Music = LoadMusic_Custom(Music,Music.SampleRate,Music.Channels,Music.Bits)
'		Return MaxModAudioStreamer.Load(Music,Loop,Paused)
'	?Not Threaded
'		Notify "CreateAudioCallbackStream will only work if threading is enabled"
'	?
'EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Returns the total length of audio.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Channel</TD><TD>Channel to retrieve length from</TD></TR>
<TR><TD>Mode</TD><TD>Format of the length returned, can be either MM_SAMPLES, MM_BYTES or MM_MILLISECS<br>Defaults to MM_SAMPLES</TD></TR>
</TABLE><P>
End Rem
Function GetChannelLength:Int(Channel:TChannel,Mode:Int=MM_SAMPLES)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return 0
	Return chn._channel.GetLength(Mode)
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Seek the audio to the given position, if available.
returns: True if seeking was successful, else False.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Channel</TD><TD>Channel to perform seek on</TD></TR>
<TR><TD>Pos</TD><TD>Position to seek too</TD></TR>
<TR><TD>Mode</TD><TD>Format of the Pos value, can be either MM_SAMPLES, MM_BYTES or MM_MILLISECS<br>Defaults to MM_SAMPLES</TD></TR>
</TABLE><P>
End Rem
Function ChannelSeek:Int(Channel:TChannel,Pos:Int,Mode:Int=MM_SAMPLES)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return -1
	Return chn._channel.Seek(Pos,Mode)
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Query if audio is seekable.
returns: True if seekable, else False.
End Rem
Function ChannelSeekable:Int(Channel:TChannel)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return -1
	Return chn._channel.IsSeekable()
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Retrieve the current stream position.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Channel</TD><TD>Channel to retrieve position from</TD></TR>
<TR><TD>Mode</TD><TD>Format of the returned position, can be either MM_SAMPLES, MM_BYTES or MM_MILLISECS<br>Defaults to MM_SAMPLES</TD></TR>
</TABLE><P>
End Rem
Function GetChannelPosition:Int(Channel:TChannel,Mode:Int=MM_SAMPLES)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return 0
	Return chn._channel.GetPosition(Mode)
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Get the channels loop mode.
returns: True if the audio is in looping mode, else False
End Rem
Function GetChannelLoop:Int(Channel:TChannel)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return 0
	Return chn._channel.GetLoopMode()
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Set the channels loop mode.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Channel</TD><TD>Channel to set looping mode on</TD></TR>
<TR><TD>Bool</TD><TD>True to enable looping on this channel, False to disable it</TD></TR>
</TABLE><P>
End Rem
Function SetChannelLoop(Channel:TChannel,Bool:Int=True)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return
	chn._channel.SetLoopMode(Bool)
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Set the channels loop points.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Channel</TD><TD>Channel to change loop point on</TD></TR>
<TR><TD>Start</TD><TD>Position in samples to start loop at</TD></TR>
<TR><TD>EndSample</TD><TD>Position in samples to end loop at</TD></TR>
</TABLE><P>
End Rem
Function SetChannelLoopPoints(Channel:TChannel,Start:Int,Length:Int)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return
	chn._channel.SetLoopPoints(Start,Length)
EndFunction

' ---------------------------------------------------------------------------------------

Rem
bbdoc: Set the channels loop points.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Channel</TD><TD>Channel to change loop point on</TD></TR>
<TR><TD>Start</TD><TD>Position in samples to start loop at</TD></TR>
<TR><TD>EndSample</TD><TD>Position in samples to end loop at</TD></TR>
</TABLE><P>
End Rem
Function GetChannelUV(Channel:TChannel,L:Double Var, R:Double Var)
	Local chn:TMaxModChannel = TMaxModChannel(Channel)
	If Not chn Return
	chn._channel.GetUV(L,R)
EndFunction

' ---------------------------------------------------------------------------------------
Rem
bbdoc: Add an effect filter to a channel.
returns: True if successfull
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>channel</TD><TD>TChannel object to apply the effect with</TD></TR>
<TR><TD>effect</TD><TD>TMaxModEffect filter object to apply to the channel</TD></TR>
</TABLE><P>
End Rem
Function AddChannelEffect:Int(channel:TChannel, effect:TMaxModEffect)
	Local chn:TMaxModChannel = TMaxModChannel(channel)
	If Not chn Or Not effect Return 0
	MaxMod_AddChannelEffect(chn._channel, effect._effect)
	Return 1;
EndFunction

Rem
bbdoc: Set the global volume.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Volume</TD><TD>Volume to be applied to all audio output, <br>0=silent, 0.5=half, 1=normal, 2=double etc</TD></TR>
</TABLE><P>
End Rem
Function SetGlobalVolume(Volume:Double)
	MaxMod_SetGlobalVolume(Volume)
EndFunction

'Function SetChannelEffect(Channel:TChannel,Effect:TMaxModEffect)
'EndFunction

' ---------------------------------------------------------------------------------------
Rem
bbdoc: Stream audio output to file.
about:
<TABLE>
<TR><TH><B>Parameters:</B></TH><TH>Description:</TH></TR>
<TR><TD>Volume</TD><TD>Volume to be applied to all audio output, <br>0=silent, 0.5=half, 1=normal, 2=double etc</TD></TR>
</TABLE><P>
End Rem
Function OutputWavFile:TChannel(source:Object, destination:Object)
	Local src:TMusic = LoadTMusic(source:Object)
	Local dst:String = String(destination)
	If Not (src And dst) Return Null
	MaxMod_OutputWavFile(src.Music, dst)
EndFunction

