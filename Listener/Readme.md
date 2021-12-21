#  Listener

Listener allows you to use an iOS device running the ListenerApp to do speech dictation and send that text to a network capable Apple IIgs.

## Installation

Copy the Listener file to your System folder on your GS.  It is a new desk accessory and the Finder will put it in the right place for you.

You must have Marinetti installed:

http://www.apple2.org/marinetti/

on your GS to provide software support for TCP/IP.  And you must setup the correct link layer for your network card or whatever network connection you may have.

If you have networking up and running and working in apps like Webber:

https://speccie.uk/software/webber/

then Listener should also work.

You need an iOS device running 14.4 or newer.  And you need to have the ListenerApp:

https://github.com/jeremysrand/ListenerApp

At the moment, I don't have a good solution for distributing this app but I am looking into it.  Once I get to something that I feel might pass Apple's review, I will see if I can get it into the App Store.  Failing that, I will look at alternatives like ad-hoc distribution.  More details to come.

## Usage

Once you have all of the pieces, you should launch the application you want to dictate into on the GS.  An application like Teach is a good option.  It needs to be a GUI application and support new desk accessories.

Start the Listener new desk accessory.  You should find it says "Waiting for connection".  Make sure your application window is the frontmost window.  The Listener window must be open but it doesn't need to be the top window nor even visible.  If it is topmost, any text heard will be sent to the Listener window and it doesn't do anything with keystrokes so you don't want that.

Start the ListenerApp on your iOS device.  At the top of the screen is a text box where you need to enter the IP address of your Apple IIgs.  Once you have typed it in and pressed enter, the iOS device will attempt to connect to the GS.  You should see the Listener desk accessary now says "Connected to device".

Make sure your document window is on top on your GS and press the "Listen" button in the iOS app.  The button will turn red to indicate that your device is listening to you.  Also, the desk accessory on the GS will say "Listening...".

Start speaking.  Best to speak slowly and clearly to get the best results.  Voice transcription is only as good (bad?) as your iOS can do normally.  You should see the text appearing in your document window on your GS.

## Future Improvements

There are a number of improvements to make:

* The speech dictation on iOS is often indecisive which leads to lots of backspacing and re-writing of text.  There are a couple of things that can be done about this.  First, I think the speech APIs provides a confidence value with the transcription so the app could ignore low quality speech and not send anything until iOS is more confident.  Second, I could have the GS tell the app when it is ready for more text and in the meantime, if iOS has changed its mind about the text four or more times, it will only send the latest best transcription.
* The Swift code in the iOS app is terrible.  This is my first real project in Swift and it shows.  I need to clean it up.
* The UI in the iOS app is also terrible and should be improved.
* The iOS app needs to be "good enough" to pass Apple's review process.
* Perhaps a Siri shortcut so you can just tell your phone to dictate to your GS without even starting the ListenerApp.
* Maybe a mode where instead of dictating text, you can dictate commands like "close window", "copy" and "paste" and the right standard commands are sent as key strokes.
