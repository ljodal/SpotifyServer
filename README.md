# SpotifyServer
This is a simple spotify player, controlled via a TCP connection.

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
