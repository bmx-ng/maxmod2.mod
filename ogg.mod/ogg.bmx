SuperStrict
'NoDebug

Rem  
bbdoc: MaxMod2.Ogg
about: This module provides cross platform ogg streaming via libvorbis<p>

<h2>libvorbis License</h2>
<table><table width=100%><td>
Copyright (c) 2002-2004 Xiph.org Foundation<p>
Redistribution and use in source and binary forms, with or without<br>
modification, are permitted provided that the following conditions<br>
are met:<p>
- Redistributions of source code must retain the above copyright<br>
notice, this list of conditions and the following disclaimer.<p>
- Redistributions in binary form must reproduce the above copyright<br>
notice, this list of conditions and the following disclaimer in the<br>
documentation and/or other materials provided with the distribution.<p>
- Neither the name of the Xiph.org Foundation nor the names of its<br>
contributors may be used to endorse or promote products derived from<br>
this software without specific prior written permission.<p>
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS<br>
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT<br>
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR<br>
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION<br>
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,<br>
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT<br>
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,<br>
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY<br>
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT<br>
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE<br>
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.<br>
</td></table>

End Rem
Module MaxMod2.Ogg
ModuleInfo "MaxMod: Ogg streaming"
ModuleInfo "Author: REDi - Cliff Harman"

Import MaxMod2.MaxMod2
Import Pub.OggVorbis
Import "ogg.cpp"
 
Extern
	Function LoadMusic_Ogg:IMaxModMusic(Stream:IMaxModStream)
EndExtern

MaxModLoader.Create(TMusicOgg.Loader,"OggVorbis ~q.ogg~q","ogg")
Type TMusicOgg Extends TMusic

	Function Loader:TMusic(Stream:IMaxModStream,Filename$)
		If Not Stream Return Null
		Local IMS:IMaxModMusic = LoadMusic_Ogg(Stream)
		If Not IMS Return Null
		IMS.AddRef()
		Local This:TMusicOgg = New TMusicOgg
		This.Music = IMS
		This.Stream = Stream
		Return This
	EndFunction

	Method Delete()
		If Music Music.RemoveRef()
	EndMethod

EndType
