# SpotifyServer
This is a simple Spotify player. It is controlled with JSON commands via a TCP socket.

## API
### Queue
#### Add an uri to the queue
```json
{
    "command": "queue_add",
    "type": "uri",
    "uri": "<spotify uri>"
}
```
#### Delete the track at index `index` from the queue
```json
{
    "command": "queue_delete",
    "index": 0
}
```
#### Move a track in the queue from index `from` to index `to`
```json
{
    "command": "queue_move",
    "from": 0,
    "to": 1
}
```
#### Get the current queue
```json
{
    "command": "queue"
}
```

### Playback
* `play_next` **_not implemented_**
* `play_prev` **_not implemented_**
* `play` **_not implemented_**
* `stop` **_not implemented_**

### Metadata
#### Search for `query`
```json
{
    "command": "search",
    "type": "<all|artist|album|track>",
    "query": "<query>",
    "user_data": "<optinal data that will be returned in the response>"
}
```
* `metadata <spotify uri>` Get metadata for the given URI. **_not implemented_**
* `image <spotify  uri>` Get a base64 encoded image for the given URI. **_not implemented_**

### Responses
Responses are given as utf-8 encoded json objects, terminated with <LR><LF>. The messages will always contain a `type` field. Example response from queueing a song:
```json
{
    "type": "queue_uri",
    "success": true,
    "message": "Track added to the queue."
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

### Raspberry Pi

1. Get the debian image
2. Install required libraries and tools
    1. git: `sudo apt-get install git`
    2. alsa-utils: `sudo apt-get install alsa-utils`
        * To make sure that the audio-driver is loaded run `sudo modprobe snd_bcm2835`
    3. libao: `sudo apt-get install libao-dev`
    4. libspotify: Get the latest version [here](http://developer.spotify.com/technologies/libspotify/#download) (eabi-armv6hf architecture, tested with release 12.1.103 beta) and follow the install instructions.
    5. libevent: Get the latest 2.1 alpha [here](https://github.com/downloads/libevent/libevent/libevent-2.1.2-alpha.tar.gz) and follow the instructions.
    6. jansson: Get latest version [here](http://www.digip.org/jansson/) (tested with release 2.4) and follow the instructions.
3. Get your API key from spotify [(link)](http://developer.spotify.com/login/) and save it in the src directory as key.h
4. Go into the src directory and run `make`
5. You're good to go. Run `./server -u <username> -p <password>` to start the server. Default port is 3579

__Possible problems:__

If you run into problems with an error message like `error while loading shared libraries: libevent-2.1.so.1: cannot open shared object file: No such file or directory` your library path must be updated. You can do that by adding `/usr/local/lib` to the `/etc/ld.so.conf` file and then run `sudo ldconfig`

If you have problems with stuttering playback or other similar audio problems, try installing PulseAudio; `sudo apt-get install libpulse-dev`

## Notes
As this is not finished in any way, there's guaranteed to be bugs/crashes/memory leaks and other problems, so be aware of that.

## License
Copyright (C) 2013 Sigurd Ljødal

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
