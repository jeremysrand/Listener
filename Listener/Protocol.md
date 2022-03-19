#  Listener Protocol

The connection is made by the modern machine to TCP port 19026.  The GS is listening and accepts the connection, acting as the server.

Once the connection comes up, neither side is required to send any kind of message today.  There is no "HELLO" style message but one may be defined in the future.

Each message starts with a 16 bit integer in little endian format (native for the GS) which identies the message type.

## Listen State Message

This is message type 1 and is sent by the modern machine to the GS.  It consists of two 16 bit integers.  The first is the message type (1) and the second is 0 or 1.  The second integer indicates if the modern machine has started or stopped listening for speech.  When the second integer is 1, that is an indication that the modern machine is now listening for speech.  When the second integer is 0, that is an indication that the modern machine has stopped listening for speech.

This message is entirely optional.  The GS just uses this to update its window to indicate whether speech is being recognized or not.  The GS doesn't actually care if it never gets these messages.

## Listen Text Message

This is message type 2 and is sent by the modern machine to the GS.  It consists of two 16 bit integers followed by 0 or more MacRoman characters.  The first integer is the message type (2).  The second integer is the length of the text which follows.  Right after that is N bytes of MacRoman encoded text, where N is the value of that second integer.

The GS doesn't care when it receives this message.  It doesn't care if the modern machine has said it isn't listening at the time or not.  The GS today just accepts the text and puts it in the queue of fake keystrokes to generate.

## Send More Message

This is message type 3 and is sent by the GS to the modern machine.  It is just a single 16 bit integer and that integer is 3, indicating the message type.

The GS sends this to the modern machine when it runs out of text to send to event manager.  This is used for pacing.  Today, the modern machine will only send a single Listen Text Message and then wait for the GS to send the Send More Message before sending more text.  This reduces the amount of retyping that might happen on the GS when the modern machine is indecisive about what it is hearing.
