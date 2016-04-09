SuperStrict

Framework MaxGUI.Drivers
Import BRL.GLMax2D
Import BRL.EventQueue
Import BRL.Timer
Import BRL.WavLoader
Import BRL.RamStream
Import BRL.Audio
'Import BRL.FreeAudioAudio

Import MaxMod2.RtAudio
'Import MaxMod2.Flac
Import MaxMod2.OGG
Import MaxMod2.WAV
'Import MaxMod2.Dumb
'Import MaxMod2ex.MP3

Incbin "eject.wav"
Incbin "click.wav"
?Linux
TMaxModRtAudioDriver.Init("LINUX_PULSE")
?macos
TMaxModRtAudioDriver.Init("MACOS_CORE")
?

SetAudioDriver("MaxMod RtAudio")
MaxModVerbose True

New TApp
Repeat
	WaitEvent()
Forever

Type TApp

	Field ejectsound:TSound = LoadSound("incbin::eject.wav")
	Field clicksound:TSound = LoadSound("incbin::click.wav")
	Field Stream    :TChannel
	Field Win       :TGadget = CreateWindow("MaxMod2 MiniPlayer",0,0,320,220,Null,WINDOW_TITLEBAR|WINDOW_CENTER|WINDOW_CLIENTCOORDS|WINDOW_STATUS)
	Field Eject     :TGadget = CreateButton("Eject",5,5,60,20,Win)
	Field Play      :TGadget = CreateButton("Play",65,5,60,20,Win)
	Field SeekZero  :TGadget = CreateButton("|<",125,5,30,20,Win)
	Field Loop      :TGadget = CreateButton("Loop",190,5,55,20,Win,BUTTON_CHECKBOX|BUTTON_PUSH)
	Field PosGroup  :TGadget = CreatePanel(5,25,310,50,Win,PANEL_GROUP|PANEL_BORDER,"Position")
	Field Slider    :TGadget = CreateSlider(0,0,ClientWidth(PosGroup)-75,ClientHeight(PosGroup),PosGroup,SLIDER_TRACKBAR|SLIDER_HORIZONTAL)
	Field TimeText  :TGadget = CreateLabel("00:00:00.000",ClientWidth(PosGroup)-75,0,70,12,PosGroup,LABEL_CENTER)
	Field SizeText  :TGadget = CreateLabel("00:00:00.000",ClientWidth(PosGroup)-75,11,70,12,PosGroup,LABEL_CENTER)
	Field VolGroup  :TGadget = CreatePanel(5,75,100,40,Win,PANEL_GROUP|PANEL_BORDER,"Volume")
	Field VolSlider :TGadget = CreateSlider(0,0,ClientWidth(VolGroup),ClientHeight(VolGroup),VolGroup,SLIDER_TRACKBAR|SLIDER_HORIZONTAL)
	Field RateGroup :TGadget = CreatePanel(110,75,100,40,Win,PANEL_GROUP|PANEL_BORDER,"Pitch")
	Field RateSlider:TGadget = CreateSlider(0,0,ClientWidth(RateGroup),ClientHeight(RateGroup),RateGroup,SLIDER_TRACKBAR|SLIDER_HORIZONTAL)
	Field PanGroup  :TGadget = CreatePanel(215,75,100,40,Win,PANEL_GROUP|PANEL_BORDER,"Pan")
	Field PanSlider :TGadget = CreateSlider(0,0,ClientWidth(PanGroup),ClientHeight(PanGroup),PanGroup,SLIDER_TRACKBAR|SLIDER_HORIZONTAL)

	Field WavGroup  :TGadget = CreatePanel(5,115,310,100,Win,PANEL_GROUP|PANEL_BORDER,"Waveform")
	Field WavCanvas :TGadget = CreateCanvas(0,0,ClientWidth(WavGroup),ClientHeight(WavGroup),WavGroup,PANEL_BORDER)

	Field Timer	 :TTimer  = CreateTimer(30)
	Field LoopMode	 :Int
	Field Status	 :Int
	Field L:Double,R:Double
	Field Pitch     :Double = 1

	Field scopesize:Int = 512+35
	Field scope:Float[scopesize*2]

	Method New()
		SetSliderRange(VolSlider,0,256)
		SetSliderValue(VolSlider,128)
		SetSliderRange(RateSlider,0,256)
		SetSliderValue(RateSlider,128)
		SetSliderRange(PanSlider,0,256)
		SetSliderValue(PanSlider,128)
		AddHook EmitEventHook,eventhook,Self
		If AppArgs.Length=>2
			Print AppArgs[1]
			OpenAudio(AppArgs[1])
		EndIf
	EndMethod

	Function eventhook:Object(id:Int,data:Object,context:Object)
		If TApp(context) Then TApp(context).OnEvent(TEvent(data))
		Return data
	End Function

	Method OnEvent(event:TEvent)

		Select event.id

			Case EVENT_WINDOWCLOSE
				If Event.Source=Win Then End

			Case EVENT_GADGETACTION
				Select Event.Source
					Case Eject
						PlaySound(ejectsound)
						OpenAudio( RequestFile( "", MusicExtensions() ) )

					Case Play
						PlaySound(clicksound)
						If Stream
							If Status=0
								Status=1
								ResumeChannel Stream
								SetGadgetText(Play,"Pause")
							Else
								Status=0
								PauseChannel Stream
								SetGadgetText(Play,"Play")
							EndIf
						EndIf

					Case Loop
						PlaySound(clicksound)
						If LoopMode=0 Then LoopMode=1 Else LoopMode=0
						If Stream SetChannelLoop(Stream,LoopMode)
						SetStatusText Win,"LoopMode="+LoopMode

					Case Slider
						Local pos:Int = Event.Data*512
						SetStatusText Win,"Seek="+pos
						If Stream ChannelSeek(Stream,pos,MM_SAMPLES)

					Case VolSlider
						Local vol! = (1!/128)*Event.Data
						SetStatusText Win,"Volume="+Float(vol)
						If Stream SetChannelVolume(Stream,Float(vol))

					Case RateSlider
						Pitch = 1+((1!/128)*(Event.Data-128))
						SetStatusText Win,"Rate="+Float(pitch)
						If Stream SetChannelRate(Stream,Float(pitch))

					Case PanSlider
						Local pan! = (1!/128)*(Event.Data-128)
						SetStatusText Win,"Pan="+Float(pan)
						If Stream SetChannelPan(Stream,Float(pan))

					Case SeekZero
						If Stream ChannelSeek(Stream,0)

				EndSelect

			Case EVENT_TIMERTICK
				RedrawGadget(WavCanvas)
				If Event.Source=Timer And Stream
					If Not ChannelPlaying(Stream)
						If Status=1 Then Status=0
						SetGadgetText(Play,"Play")
						Return
					EndIf
					SetGadgetText(SizeText,Millisecs2Time(GetChannelLength(Stream,MM_MILLISECS)))
					SetSliderValue(Slider,GetChannelPosition(Stream,MM_SAMPLES)/512)
					SetGadgetText(TimeText,Millisecs2Time(GetChannelPosition(Stream,MM_MILLISECS)))
				EndIf

			Case EVENT_GADGETPAINT
				If Event.Source=WavCanvas
					MaxModFillScope(scope,scopesize)
					SetGraphics CanvasGraphics(WavCanvas)
					Cls

					SetBlend(LIGHTBLEND)
					Local h:Int = ClientHeight(WavCanvas)/2
					Local w:Int = ClientWidth(WavCanvas)
					Local n:Int,x:Int
					SetColor(0,255,255)
					For n=0 Until scopesize Step 2
						Plot x,h+(h*scope[n])
						x:+1
					Next
					x=0
					SetColor(255,255,0)
					For n=1 Until scopesize Step 2
						Plot x,h+(h*scope[n])
						x:+1
					Next

					GetChannelUV(Stream,L,R)
					h = ClientHeight(WavCanvas)
					L = (h-2)*L
					R = (h-2)*R
					SetColor(0,255,255)
					DrawRect w-20,Float((h-1)-L),9,Float(L)
					SetColor(255,255,0)
					DrawRect w-10,Float((h-1)-R),9,Float(R)

					Flip
				EndIf

		EndSelect
	End Method

	Method OpenAudio(file$,inc:Int=False)
		If FileType(file)<>FILETYPE_FILE And inc=False Then Return
		If Stream Then StopChannel(Stream)
		Local TempStream:TChannel = CueMusic(File,LoopMode)
		If Not TempStream Notify "Unable to Stream Music" ; Return
		Stream = TempStream
		SetGadgetText(Win,StripAll(File)+" - "+"MaxMod2 MiniPlayer")
		SetGadgetText(SizeText,Millisecs2Time(GetChannelLength(Stream,MM_MILLISECS)))
		SetGadgetText(Play,"Pause")	; Status=1
		SetSliderRange(Slider,0,GetChannelLength(Stream,MM_SAMPLES)/512)
		SetChannelVolume(Stream,Float((1!/128)*SliderValue(VolSlider)))
		SetChannelRate(Stream,Float((1!/128)*SliderValue(RateSlider)))
		If Not ChannelSeekable(Stream)
			DisableGadget(Slider)
		Else
			EnableGadget(Slider)
		EndIf
		ResumeChannel Stream
	EndMethod

	Function Millisecs2Time$(Mill:Int)

		Local time$

		Local hours:Int = (Mill/(60000*60))
		If hours<10 Then time$:+"0"
		time$:+hours+":"

		Local mins:Int = (Mill/60000) Mod 60
		If mins<10 Then time$:+"0"
		time$:+mins+":"

		Local secs:Int = (Mill/1000) Mod 60
		If secs<10 Then time$:+"0"
		time$:+secs+"."

		Local mills:Int = Mill Mod 1000
		If mills<100 Then time:+"0"
		If mills<10 Then time:+"0"
		time:+mills

		Return time$

	EndFunction

EndType

