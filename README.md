It's based on one of Victor Lazzarini's audio programming blog on android section:  
	http://audioprograming.wordpress.com/2012/03/03/android-audio-streaming-with-opensl-es-and-the-ndk/

The original project demonstrates streaming audio IO on android with OpenSL ES, by plugging the mono input 
from the audio device into its stereo output.

I put the test code for Android system audio deplay test. Using a 48kHz audio sample file to play out, and
in the same time, recording the input from the microphone. And put them together into an output file as a 
"stereo" audio file: playing audio on left channel, and the recording audio on the right channnel, much easy 
for me to read the exact delay between "playing" operation and "recording" operation.

The test on my Nexus 5 device show about 170ms delay, which is quite big for me! It's far from a good "low-latency"
system!  Also the recording quality is very bad, a lot of "clips" during the talking.

Reason is not clear yet. But I believe OpenSL-ES is not being good supported on Android. I just suspend it here, and
maybe in one day of future I will come back to this topic if having new hints.

Some test result is uploaded to one of my Google Group: https://plus.google.com/u/0/communities/100224842778848673689


Build Instructions:

The build and run is quite simple. For me, I need these steps for build and run this example:

1. Install SWIG: swig-3.0.2 . Install it by 3 steps:

$ configure $ make $ sudo make install.

2. Edit the build.sh to reflect the location of your NDK. And run the build.sh script at the top-level directory.
Generate build.xml by:
$ android update project --name opensl-es-audiotest --target 1 --path ./

3. Build with ant: $ ant debug

Then, you can install it to your device and test it:
$ adb install bin/opensl-es-audiotest-debug.apk



