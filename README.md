# BadWrAPI - a C++ wrapper for the undocumented Badoo API
Demo application and wrapper class made in Qt/C++ as a proof of concept
on how to connect to the Badoo network from a desktop program.

Based on reverse-engineering works on the website, and the mobile apps\
Badoo, Bumble and the now-defunct Blendr.


Updates
-------
Look [here](UPDATES.md) for details.


Features
--------

* Allows you to quickly toggle your online status between hidden and visible.
* Makes you able to move to any supported location in the world.
* Automatically imports the user "search settings" and allows modifications.
* Plays the "Encounters" game, displaying the -complete- profiles.
* Allows you to "skip" profiles, for later.
* Shows every other section, "People nearby", "Favorites", "Visitors", etc.
* Reveals people in the "Liked you" section.
* Shows a special profile badge for people that have already voted you.
* Makes you anonymous, no matter if the visited profiles have super-powers.
* Downloads profiles in self-contained single HTML files.


Pre-requisites
--------------

A valid/verified Badoo account (Email / Phone number and Password).

Other sign-in options are not suported -yet-.


ToDo's
------

- [x] Save credentials/session.
- [x] Show "People nearby".
- [x] Show "Likes".
- [x] Show "Matches".
- [x] Show "Favorites".
- [x] Show "Visitors".
- [x] Show "Blocked people".
- [ ] Implement chat.


Technical notes
---------------
Look [here](TECH.md) for details.


Known issues
------------

_2024-08-15_: The People nearby search Distance setting is broken at this time.\
The API returns multiple choices, as usual. But no matter what you choose, It\
always results in a selected radius of 50km.\
Both the mobile and the web app removed this from the options, so I think the\
API cannot be tricked into accepting other values.\
Not sure if this will be needed again. I'll keep it, for potential future use.


Getting involved
----------------

I'm open to criticism and suggestions. Feel free to [send me](mailto:quark1482@protonmail.com?subject=[GitHub]%20BadWrAPI) your comments. :slightly_smiling_face:

To code purists, I advocate for standards and code conventions. But I also love\
structured code **with all my heart**.


Acknowledgements
----------------

This program uses a set of nice icons created by ['bqlqn'](https://www.flaticon.com/authors/bqlqn).


_Notice_
--------

This is a continuous WIP. The ToDo section isn't even complete ATM.


<br><br>
_This README file is under construction._
