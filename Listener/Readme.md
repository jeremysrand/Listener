#  Listener

Listener allows you to use an iOS device running the ListenerApp to do speech dictation and send that text to a network capable Apple IIgs.

## Installation

Copy the Listener file to your System folder on your GS.  It is a new desk accessory and the Finder will put it in the right place for you.

You must have Marinetti installed:

http://www.apple2.org/marinetti/

on your GS to provide software support for TCP/IP.  And you must setup the correct link layer for your network card or whatever network connection you may have.

If you have networking up and running and working in apps like Webber:

https://speccie.uk/software/webber/

then Listener should also work.  If you are trying to use Listener on an emulator, see the section below about issues with emulators.

You need an iOS device running 14.4+ or a Mac running macOS v11.3+.  And you need to have the ListenerApp:

https://github.com/jeremysrand/ListenerApp

At the moment, I don't have a good solution for distributing this app but I am looking into it.  Once I get to something that I feel might pass Apple's review, I will see if I can get it into the App Store.  Failing that, I will look at alternatives like ad-hoc distribution.  More details to come.

## Usage

Once you have all of the pieces, you should launch the application you want to dictate into on the GS.  An application like Teach is a good option.  It needs to be a GUI application and support new desk accessories.

Start the Listener new desk accessory.  You should find it says "Waiting for connection".  Make sure your application window is the frontmost window.  The Listener window must be open but it doesn't need to be the top window nor even visible.  If it is topmost, any text heard will be sent to the Listener window and it doesn't do anything with keystrokes so you don't want that.

Start the ListenerApp on your iOS device.  At the top of the screen is a text box where you need to enter the IP address of your Apple IIgs.  Once you have typed it in and pressed enter, the iOS device will attempt to connect to the GS.  You should see the Listener desk accessary now says "Connected to device".

Make sure your document window is on top on your GS and press the "Listen" button in the iOS app.  The button will turn red to indicate that your device is listening to you.  Also, the desk accessory on the GS will say "Listening...".

Start speaking.  Best to speak slowly and clearly to get the best results.  Voice transcription is only as good (bad?) as your iOS can do normally.  You should see the text appearing in your document window on your GS.

## Emulators

In theory, this NDA should work on any GS emulator that supports networking.  In practice unfortunately, that is not true.  The reason is that the NDA needs to be reachable by IP address from your modern device.  Emulators usually have a "fake" IP address for the GS and your iPhone won't be able to reach that address.  Your iPhone may be able to communicate with the Mac or PC that is running the emulator but the GS is often behind some kind of network address translation (NAT).  With NAT, your GS can general make connections out to other devices on the network but other devices on the network cannot make connections to your GS.  And given how Listener works, the modern device must connect to the GS.

However, I was able to make this work using mame/Ample.  The first thing I did is go the Apple Menu > Control Panel in the emulated GS and scroll down the TCP/IP control panel.  Inside TCP/IP, I clicked the "Connect to network" button and then clicked on "Disconnect from network".  I did this just to ensure that Marinetti has gotten its IP address.  Then, I click on "Setup connection..." and then "Configure...".  Inside the dialog that appears, you will see your IP address.  Mine is 192.168.64.5.  Make a note of this.

You also need the IP address of your Mac where the emulator is running.  I will not describe how to find that but I know mine is 192.168.1.2.  Also, you will need to have "Remote Login" enabled in Sharing inside of System Preferences for this to work.  With all this in place, I run this command from a Terminal:

```
ssh -L 192.168.1.2:19026:192.168.64.5:19026 localhost
```

Make sure to replace the two IP addresses with the correct IP addresses for your situation.  You will likely be asked for your password and you should enter that.  You should find you are back at a shell.  For as long as this shell is open, you will also be forwarding traffic that arrives at port 19026 on your Mac to 19026 on your GS emulator.  That port number is the port number that Listener uses.  In my testing, I was able to get Listener to work in mame/Ample with this tunnel.  Once you are done, you should exit from the shell you started with ssh.

## Future Improvements

There are a number of improvements to make:

* The iOS app needs to be "good enough" to pass Apple's review process.
* Perhaps a Siri shortcut so you can just tell your phone to dictate to your GS without even starting the ListenerApp.
* Maybe a mode where instead of dictating text, you can dictate commands like "close window", "copy" and "paste" and the right standard commands are sent as key strokes.
* Instead of injecting keyboard events through the Event Manager on the GS, look at injecting fake keypresses into ADB itself.  This should improve compatibility with applications that do not get their key events from the Event Manager.  Based on what I have read, the SendInfo toolbox routine with the keyCode command should be able to do this.  It should be possible based on what things like Video Keyboard does in the System 6.0.1 distribution.
* Investigate if some minimal amount of bonjour support could be introduced on the GS side so that the modern device could detect its presence on the network.  That way, you wouldn't need to set the IP address of the GS in the app.
* There have been suggestions to turn this into a general solution for integrating a modern machine with a GS.  This might involve things like file transfer in either direction, clipboard syncing and perhaps other things along those lines.
