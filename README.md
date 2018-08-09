# iceflac

A FLAC audio streamer for Icecast servers.

## What? Why?
My friend wanted to create a webradio using lossless audio files. This task was very hard in the open-source-way, so I decided to create a FLAC streamer application for Icecast, because it's a working solution.
The targeted operating system was **Windows**, but the ```Icecast-libshout``` library was nearly impossible to compile, so I wrote a lightweight version of it.
This program only works with **FLAC** files.

## Compilation
The required libraries:
- libogg : https://github.com/gcp/libogg
- flac : https://github.com/xiph/flac
- mxml : https://github.com/michaelrsweet/mxml

Compile everything as *Release* into a **STATIC** library, so without the .DLL files. Copy the necessary files to the ```lib``` directory, so it will look like this:
```
.
+-- lib
    +-- grabbag_static.lib
    +-- libFLAC_static.lib
    +-- libogg_static.lib
    +-- mxml1.lib
    +-- utf8_static.lib
    +-- win_utf8_io_static.lib
```
Copy the include directories from ```flac/include```, ```libogg/include```, ```mxml/*.h``` to the  ```include ``` directory, so it will look like this:
```
.
+-- include
    +-- FLAC	(flac/include/FLAC)
    +-- mxml	(mxml/*.h, create this directory)
    +-- ogg     (libogg/include/ogg)
    +-- share	(include/share)
    +-- ...
```
Open the ```iceflac/iceflac.sln``` and compile the code in Win32 (x86) mode.
If everything was done correctly the ```iceflac.exe``` will be in the ```build/Release(Debug)``` directory.

## Usage
Copy the ```settings/iceflac.xml``` next to the ```iceflac.exe```.
Modify the content of the ```iceflac.xml```.
To use such a high bitrate (1411 kbps) modify the  ```<limits> / <queue-size>``` setting in the configuration file of the icecast server. My setting is ```<queue-size>111524288</queue-size>```.

## TODO
- More elegant package scheduling
- Support for UTF-8 filenames
- Implement verbose/silent features
- Linux port (rewrite the ice protocol)
- Clean the code

## Thanks to
- the developers of Icecast (https://icecast.org)
- the developers of ogg/flac (https://xiph.org)
- the developer of Mini-XML (https://www.msweet.org/mxml)
- Little Star Media for the base64 code (https://github.com/littlstar/b64.c)

## License
Use, modify as you wish, just mention me in your README ;)
