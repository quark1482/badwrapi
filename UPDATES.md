_2024-08-15_ "YES" 💕
------------

* Login by user/pass had to be refactored because of serious API modifications.
* Login by pin verification is now possible.\
  CAVEAT: the server is not sending SMS messages. Only e-mails are accepted.
* The ability to reveal or hide the online presence has been implemented.\
  CAVEAT: being visible does not mean to be seen online forever. If enough idle\
  time passes, you may even appear as offline.
* The folder type MATCHES was having problems, returning zero profiles inside.\
  It's fixed now, but see the comment in `BadooWrapper::getFolderPage()`.
* BadooAPI now uses a global, configurable User-Agent to appear more "human".
* The contents inside the People nearby folder were not reflecting the search\
  settings' Location. This has been fixed.
* You can now change your profile's current location in the Settings menu.\
  **CAVEAT #1**: it's possible to move between locations in a very short time span.\
  But sometimes, the server just does not allow it for some reason. Or in other\
  words, the request succeeds, but the actual profile's location never changes.\
  The problem typically resolves itself after about an hour. However, if you're\
  in a hurry, logging out and back in may help.\
  **CAVEAT #2**: if you move to another country, the votes cast there, stay there.\
  Changing countries again will make those votes appear like never registered.\
  But if you return to the previous country, the votes will magically reappear.\
  By contrast, voting in a country different from the one you have moved to is\
  possible, but definitely won't be registered. You need to go there and vote.\
  **CAVEAT #3**: as for the People nearby search, it is possible to set a location\
  different from the current logged-in profile's location. However, after some\
  time (I've not checked how long, though), the People nearby location will be\
  reset to match the profile's location. But you could force this operation by\
  Logging back in or connecting from the website or mobile app.


_2024-05-07_ "DEN" 🥳
------------

* New folder type supported: Blocked people.
* A "danger" toolbar (Block/Unblock and Unmatch) appears in each profile.
* Profiles with no albums show the profile picture instead (when available).
* The logout operation now requires confirmation.