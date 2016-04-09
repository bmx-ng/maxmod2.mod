SuperStrict
Import MaxMod2.RtAudio
Import MaxMod2.ModPlayer

SetAudioStreamDriver("MaxMod RtAudio")
MaxModVerbose True

Graphics 640,480,0

Local path$ = RequestDir( "Choose a folder containing tracker modules" )
Local dir% = ReadDir(path)
If Not dir RuntimeError "failed to read directory"

Local failList:TList = New TList

Local Channel:TChannel, t$
Repeat
	t$=NextFile( dir )
	If t="" Exit
	If t="." Or t=".." Continue

'	If t.tolower().endswith(".mod") Continue  ' skip all mod files
'	If t.tolower().endswith(".xm") Continue   ' skip all xm files

	If FileType(path+"\"+t)=1
		Print "file=~q"+t+"~q"
		channel = CueMusic(path+"\"+t)
		If channel
			ResumeChannel(channel)
			Repeat
				Cls
				SetColor(255,255,255)
				DrawText "Press space to skip to next track",10,10
				DrawText "Playing ~q"+t+"~q",10,25

				Local f$,i%
				SetColor(255,0,0)
				For f=EachIn failList
					DrawText f,10,45+(15*i)
					i:+1
				Next
				Flip
				If KeyHit(KEY_ESCAPE) Or AppTerminate() Then CloseDir(dir); End
			Until ChannelPlaying(Channel)=False Or KeyHit(KEY_SPACE)
			StopChannel(channel)
		Else
			failList.AddLast(t)
			If failList.count()>29 Then failList.Remove(failList.first())
		EndIf
	EndIf
Forever
Print "End of directory!"
CloseDir dir

