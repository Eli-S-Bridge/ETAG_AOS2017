ETAG RFID reader Version1
(Distrubted at the American Ornithological Society Meeting, Lansing, MI, August 2017.

This repository contains:
  1) Eagle design files for the circuit board
  2) A board reference for using the Arduino app with the circuit board.
  3) Arduino code to install on the circuit board

HARDWARE:

You can power the board from your USB port or use a 5V battery. 6V batteries seem to work OK too. We’ve tested 4AA batteries in series (which give ~6V). You can power the reader from the USB port or the push-in connectors. Check the + and - labels on the board before connecting the battery leads. The large switch on the circuit board alternates between power sources. If you have both USB and a battery connected, you will not really be able to turn the device off (but you can always use the reset button).

Antenna coils should have inductance of approximately 1.3 milliHenries. You can make the coils yourself by wrapping magnet wire around a form and measuring the inductance until you get it right. Don’t make them bigger than 15cm in diameter or they won’t work very well. You can also buy 1.25mH antennas that work pretty well here:

https://store.qkits.com/rfid/125khz-antenna-for-rfid-controllers-51-5mm.html


Tags need to work at 125kHz and follow the EM4100 communication protocol. Biomark tags generally follow the ISO11784/5 communication protocol will not work. Our favorite tag suppliers are www.cyntag.com (for PITtag style tags) and ibtechnology.co.uk (for RFID leg bands - look for “avian products”)


OTHER RESOURCES:

YouTube video demonstration: https://www.youtube.com/watch?v=-Nv7rSirzx8