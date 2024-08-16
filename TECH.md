Build
-----

This project was developed in Qt 6.6 and tested on a single target: Win64.\
By the time I wrote this document (_2024-08-15_), a newer stable version was\
already available: 6.7.

I do, in fact, plan to upgrade in the near future.

Take this into account if you plan to run the demo as a standalone application\
or create an installer: the deployment tool won't detect that the project uses\
QML (minimal, but still), so we need to assist it.

_Without our help, the deployment will succeed, but the executable will crash as\
soon as you enter the Search settings, which, guess what...use QML_.

Just place this small file in the same directory as the executable:

**rangeslider.qml**
```qml
import QtQuick.Controls
RangeSlider {
    stepSize: 1
    snapMode: RangeSlider.SnapAlways
}
```

After that, it's safe to execute `windeployqt BadWrAPI.exe`


CAPTCHAs
--------

After implementing saving the session in a database, and considering that Badoo\
sessions last ONE YEAR (haha), CAPTCHAs stopped being an annoyance.

HOWEVER, is there someone interested in developing an autoCAPTCHASolver() to be\
incorporated to this project?

My findings so far:

1. The fonts used by the CAPTCHAs are these ones: [bearz](https://fontsgeek.com/bearz-font) and [fruitforears](https://fontsgeek.com/fruitforears-font).
2. No numbers or special characters are used. Only letters.
3. I have never seen any image including the letters G, I, L, or T.
4. I have never seen any image with more than eight letters.
5. Amazingly, all these words are pronounceable. You hardly find two consonants\
   in a row. -I'm not sure what this could be useful for, but it's interesting."

I know that a model can be trained to reach 100% success here.

The interface is this:
`typedef bool BadooCAPTCHASolver(QByteArray,QString &,QString &);`

* 1st parameter, the raw image file bytes.
* 2nd parameter, the solved text. The function returns true.
* 3rd parameter, an error text. The function returns false.

See the definition of `BadooAPI::manualCAPTCHASolver()` for reference.

And of course, drop me a line [here](mailto:quark1482@protonmail.com?subject=[GitHub]%20BadWrAPI) if you're interested.


Geocoding
---------

Changing locations requires that you send the desired coordinates (latitude and\
longitude) to the server, and the server does not have a way (at least exposed)\
to translate your location name back into coordinates.

This operation (AKA geocoding) is not provided by their API, so I had to use an\
external service for this.

Every geocoding service I've found either costs money, requires a registration,\
and/or is rate-limited. So to keep things simple, I'm using TripAdvisor because\
its autocomplete search also returns the requested place's coordinates.

HOWEVER, _have in mind these two things about that_:

1. It's a free website feature, but it’s not intended to be used by an external\
   application. Hence, I've designed this class to minimize data exchanges with\
   the server. -I am fine in case you want to use the TAGeoCoder elsewhere, but\
   please avoid abusing the service.
2. This approach is highly dependent on the website and may become wrong in the\
   long run. No guarantees can be made. Even with a fair use, and behaving like\
   normal web users, they could still implement protections or even ban us.