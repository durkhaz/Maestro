# Maestro
A piano aimbot for the Overwatch map "Paris".
Video of it in action: 
https://youtu.be/AARFZbtWe_Q

### What it does
The map "Paris" has a playable piano. You shoot the keys to hear a tone.
This application can open MIDI files and allows you to play songs automatically. 
Does not hook into the Overwatch process, it is essentially blind.
Uses predefined view-offsets and injects mouse events using WinAPI.
Parses MIDI files using [Midifile](https://github.com/craigsapp/midifile).
Interfaces with MIDI keyboards using [RtMidi](http://www.music.mcgill.ca/~gary/rtmidi/index.html#intro).


### Features
- Can listen to MIDI keyboards
- Supports playback of any MIDI track combination
- Shows number of track events
- Adjustable playback speed

### How to use
1. Launch Overwatch
2. Run Maestro
3. Open a MIDI file or connect keyboard
4. Set your Overwatch mouse sensitivity 
5. If MIDI-file, select a track
6. Focus Overwatch window, and assume position
7. Press INSERT key to play track

### Important
Make sure you're in the correct position before playing the song. Otherwise it won't work.
Use this for reference: https://i.imgur.com/SIXngZg.jpg

Stand in the spot shown at the bottom, and look at the spot shown at the top.
I recommend using stick movement (controller) to get in position, because it's more precise.

Heroes it works well with: Mei, Soldier76

### Download
**Disclaimer: THIS TOOL MIGHT GET YOU BANNED, USE AT YOUR OWN RISK!**

https://github.com/durkhaz/Maestro/releases

**Disclaimer: THIS TOOL MIGHT GET YOU BANNED, USE AT YOUR OWN RISK!**

### Setup the project
**Clone and initialize the repository**

```
git clone https://github.com/durkhaz/Maestro.git
cd Maestro
git submodule update --init
```
