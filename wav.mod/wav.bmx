SuperStrict
'NoDebug

Rem  
bbdoc: MaxMod2.Wav
about: This module provides cross platform wav streaming<p>
End Rem
Module MaxMod2.Wav
ModuleInfo "MaxMod: Wav streaming"
ModuleInfo "Author: REDi - Cliff Harman"

Import MaxMod2.MaxMod2
Import "wav.cpp"
 
Extern
	Function LoadMusic_Wav:IMaxModMusic(Stream:IMaxModStream)
EndExtern

MaxModLoader.Create(TMusicWav.Loader,"Waveform audio ~q.wav~q","wav")
Type TMusicWav Extends TMusic

	Function Loader:TMusic(Stream:IMaxModStream,Filename$)
		If Not Stream Return Null
		Local IMS:IMaxModMusic = LoadMusic_Wav(Stream)
		If Not IMS Return Null
		IMS.AddRef()
		Local This:TMusicWav = New TMusicWav
		This.Music = IMS
		This.Stream = Stream
		Return This
	EndFunction

	Method Delete()
		If Music Music.RemoveRef()
	EndMethod

EndType
