# SpotifyServer
This is a simple spotify player, controlled via a TCP connection. It was created because I wanted to use my raspberry pi as a remote spotify player.

## API
### Queue
* `QUEUE <SPOTIFY URI>` Queue the given URL. The URL must be a valid album or track URI. **_partially implemented_**
* `QUEUE DELETE <index>` **_not implemented_**
* `QUEUE GET` **_not implemented_**

### Playback
* `NEXT` Play the next song in the queue
* `PREV` **_not implemented_**
* `PAUSE` **_not implemented_**
* `STOP` **_not implemented_**

### Responses
**_not implemented_**
Responses are given as bencoded hashes. The messages will always contain a `type` field. Example response from queueing a song:
```json
{
    "type": "queue",
    "status": "success",
    "message": "Suicide by Star successfully queued."
}
```

## Status updates.
**_not implemented_**
Once every second the server will send out status updates containing information about the current status of the player. Here's an example of a status update:
```json
{
    "type": "status",
    "status": "playing",
    "playtime": 123
}
```

## Install instructions

**NOT WORKING UNTILL I MAKE SOME SMALL CHANGES**

1. Get the debian image
2. Install required libraries and tools
    1. git: `sudo apt-get install git`
    2. libao: `sudo apt-get install libao`
    3. libspotify: Get the latest version [here](http://developer.spotify.com/technologies/libspotify/#download) (eabi-armv6hf architecture, tested with release 12.1.103 beta) and follow the install instructions.
    4. libevent: Get the latest 2.1 alpha [here](https://github.com/downloads/libevent/libevent/libevent-2.1.2-alpha.tar.gz) and follow the instructions.
    5. jansson: Get latest version [here](http://www.digip.org/jansson/) (tested with release 2.4) and follow the instructions.
3. Get your API key from spotify [(link)](http://developer.spotify.com/login/)
4. Go into the src directory and run `make`
5. You're good to go. Run `./server -u <username> -p <password>` to start the server. Default port is 3579

## Notes
As this is not finished in any way, there's guaranteed to be bugs/crashes/memory leaks and other problems, so be aware of that.

## License
Copyright (C) 2013 Sigurd Ljødal

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
