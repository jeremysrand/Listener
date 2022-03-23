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

Once you have the software installed and configured on your Apple IIGS, you should launch an desktop application that accepts text.  The Teach application is an example of an application that would work.  Make sure there is a window open which you can type into and then open the Listener NDA from under the Apple menu.  The Listener NDA window will say it is waiting for a connection.

In the iOS/macOS app, tap the "+" button (if you are on an iPad in portrait orientation, you may need to tap a "&lt; GS Destinations" button to reveal the side bar and the "+" button) and enter the IP address or hostname of your Apple IIGS.  You can enter multiple IP addresses and hostnames if you have multiple machines.  Your destinations are synced through iCloud so if you have multiple modern devices, you should find the IP addresses are mirrored to those other devices.

Select one of these destinations and tap the "Connect" button to bring up a network connection to your Apple IIGS.  On the GS, you should find the NDA window also indicates that the connection is up.  Then tap the "Listen and Send Text" button.  Speak clearly and you should find that your words are typed into the window on your GS.  If the NDA window was top-most when you started speaking, you should find that it goes to the back.

Tap "Stop Listening" when you want to stop entering text through speech and "Disconnect" when you are done using the app.

## Emulators

In theory, this NDA should work on any GS emulator that supports networking.  In practice unfortunately, that is not true.  The reason is that the NDA needs to be reachable by IP address from your modern device.  Emulators usually have a "fake" IP address for the GS and your iPhone won't be able to reach that address.  Your iPhone may be able to communicate with the Mac or PC that is running the emulator but the GS is often behind some kind of network address translation (NAT).  With NAT, your GS can general make connections out to other devices on the network but other devices on the network cannot make connections to your GS.  And given how Listener works, the modern device must connect to the GS.

However, I was able to make this work using mame/Ample.  The first thing I did is go the Apple Menu > Control Panel in the emulated GS and scroll down the TCP/IP control panel.  Inside TCP/IP, I clicked the "Connect to network" button and then clicked on "Disconnect from network".  I did this just to ensure that Marinetti has gotten its IP address.  Then, I click on "Setup connection..." and then "Configure...".  Inside the dialog that appears, you will see your IP address.  Mine is 192.168.64.5.  Make a note of this.

You also need the IP address of your Mac where the emulator is running.  I will not describe how to find that but I know mine is 192.168.1.2.  Also, you will need to have "Remote Login" enabled in Sharing inside of System Preferences for this to work.  With all this in place, I run this command from a Terminal:

```
ssh -L 192.168.1.2:19026:192.168.64.5:19026 localhost
```

Make sure to replace the two IP addresses with the correct IP addresses for your situation.  You will likely be asked for your password and you should enter that.  You should find you are back at a shell.  For as long as this shell is open, you will also be forwarding traffic that arrives at port 19026 on your Mac to 19026 on your GS emulator.  That port number is the port number that Listener uses.  In my testing, I was able to get Listener to work in mame/Ample with this tunnel.  Once you are done, you should exit from the shell you started with ssh.

## Compatibility

Listener should work with any desktop application that supports NDAs and accepts text input.  I have tested the following applications and they seem to work well:

* Teach
* AppleWorks GS (pretty slow though)
* EGOed
* Hermes
* coolwriter
* Lost Treasures of Infocom (although once you say "new line", the recognizer cannot change its mind anymore and edit previous text)
* HyperCard IIGS (although it can be hard to send the NDA to the background)
* DeluxeWrite
* Medley (very slow)
* GraphicWriter
* ORCA Prism (although I doubt you could dictate code)
* TMLPascal (again, not sure you can code this way)

## Future Improvements

There are a number of improvements to make:

* The iOS app needs to be "good enough" to pass Apple's review process.
* Perhaps a Siri shortcut so you can just tell your phone to dictate to your GS without even starting the ListenerApp.
* Maybe a mode where instead of dictating text, you can dictate commands like "close window", "copy" and "paste" and the right standard commands are sent as key strokes.
* Instead of injecting keyboard events through the Event Manager on the GS, look at injecting fake keypresses into ADB itself.  This should improve compatibility with applications that do not get their key events from the Event Manager.  Based on what I have read, the SendInfo toolbox routine with the keyCode command should be able to do this.  It should be possible based on what things like Video Keyboard does in the System 6.0.1 distribution.
* Investigate if some minimal amount of bonjour support could be introduced on the GS side so that the modern device could detect its presence on the network.  That way, you wouldn't need to set the IP address of the GS in the app.
* There have been suggestions to turn this into a general solution for integrating a modern machine with a GS.  This might involve things like file transfer in either direction, clipboard syncing and perhaps other things along those lines.
* Add a mode in the app where you can just type instead of using speech.  And this would let you paste also.
