**Note**: This extension depends on the [Qt D-Bus](http://doc.qt.io/qt-5/qtdbus-index.html) module.

This extension is for linux only. It lets you control running MPRIS-capable media players like Rhythmbox or VLC.
MPRIS (Media Player Remote Interfacing Specification) is a protocol that enables media players to be controlled via DBus.
Currently this extension supports the following commands:
- Play
- Pause
- Next
- Previous
- Stop

Just enter the action you want to perform (like "play" to start playback) and there will be an entry for every player that is running and the action is applicable.
I.e. when you have only one player running and it's already playing, the Play-entry will not occure.
Or; when you have two players running and one is playing, "next" will yield two entries. One for each player.

If the media player supports the Raise method it can also be brought to front with albert.
This may or may not work as well depending on the implementation of the Raise method within the media player.
**But** there is no seperate entry to raise the player, this is an alternative action attached to all MPRIS-entries.
