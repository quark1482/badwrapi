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
soon as you enter the Search settings, which, guess what...uses QML_.

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
   in a row. -I'm not sure what this could be useful for, but it's interesting.

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


Chats
-----

Although I already have methods for sending individual messages and for reading\
whole conversations (or their last messages), and I could quickly code a working\
ping-pong chat window, I’m not going to do it. I will do it the right way:\
_by receiving website events_. And for this approach, I need to figure out some\
things first.

The last time I checked, the website event system was somehow based on the Comet\
web application model, which is now considered obsolete by today’s standards. Who\
knows if they will change this someday.

That said, if you are still determined to code a chat window by polling the server\
for updates, you may use the following available methods of the BadooWrapper class:\
`getChatMessages()`, `getChatTotalMessages()` and `sendChatMessage()`.

Use the following snippets as a base to start from.\
_bwTest is an authenticated BadooWrapper instance._

**Total messages in a conversation**
```c++
int     iTotal;     // (out) The resulting total goes here.
QString sProfileId; // (in)  The chat partner's user id.
if(bwTest.getChatTotalMessages(sProfileId,iTotal)) {
    qDebug() << "total" << iTotal;
}
else
    qDebug() << bwTest.getLastError();
```

**Whole chat history with someone**
```c++
QString              sProfileId; // (in)  The chat partner's user id.
BadooChatMessageList bcmlChat;   // (out) The whole conversation.
if(bwTest.getChatMessages(sProfileId,bcmlChat)) {
    int iK=0;
    for(const auto &c:bcmlChat) {
        qDebug() << QStringLiteral("#%1").arg(++iK);
        qDebug() << "from" << c.sFromUserId;
        qDebug() << "to" << c.sToUserId;
        qDebug() << "type" << c.bcmtMessageType;
        qDebug() << "when" << c.dtmCreationTime;
        qDebug() << "text" << c.sMessageText;
        qDebug();
    }
    qDebug() << "total" << bcmlChat.count();
}
else
    qDebug() << bwTest.getLastError();
````

**Most recent N messages in a conversation**
```c++
int                  iHowMany;   // (in)  The number of messages to get.
QString              sProfileId; // (in)  The chat partner's user id.
BadooChatMessageList bcmlChat;   // (out) The list of messages (newest goes first).
if(bwTest.getChatMessages(sProfileId,bcmlChat,iHowMany)) {
    int iK=0;
    for(const auto &c:bcmlChat) {
        qDebug() << QStringLiteral("#%1").arg(++iK);
        qDebug() << "from" << c.sFromUserId;
        qDebug() << "to" << c.sToUserId;
        qDebug() << "type" << c.bcmtMessageType;
        qDebug() << "when" << c.dtmCreationTime;
        qDebug() << "text" << c.sMessageText;
        qDebug();
    }
    qDebug() << "total" << bcmlChat.count();
}
else
    qDebug() << bwTest.getLastError();
```

**Guess what**
```c++
QString sProfileId; // (in) The chat partner's user id.
QString sMessage;   // (in) The message contents.
if(bwTest.sendChatMessage(sProfileId,sMessage)) {
    qDebug() << "message sent";
}
else
    qDebug() << bwTest.getLastError();
```