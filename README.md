# iceflac

A FLAC audio streamer for Icecast servers.

## What? Why?

My friend wanted to create a webradio using lossless audio files. This task was very hard in the open-source-way, so I decided to create a FLAC streamer application for Icecast, because it's a working solution.
The targeted operating system was **Windows**, but the ```Icecast-libshout``` library was nearly impossible to compile, so I wrote a lightweight version of it.

This program only works with **FLAC** files.

## Libraries

Required libraries:

- libogg : <https://github.com/gcp/libogg>
- flac : <https://github.com/xiph/flac>
- mxml : <https://github.com/michaelrsweet/mxml>

## Build - Windows

Compile everything as *Release* into a **STATIC** library, so without the .DLL files. Copy the necessary files to the ```lib``` directory, it should look like this:

```
.
+-- lib
    +-- libFLAC_static.lib
    +-- libogg_static.lib
    +-- mxmlstat.lib
```

Copy the include directories from ```flac/include```, ```libogg/include```, ```mxml/*.h``` to the  ```include``` directory, it should look like this:

```
.
+-- include
    +-- FLAC (flac/include/FLAC)
    +-- mxml (mxml/*.h, create this directory)
    +-- ogg  (libogg/include/ogg)
```

Open the ```iceflac/iceflac.sln``` and compile the code.
If everything was done correctly the ```iceflac.exe``` will be in the ```build/Release(Debug)``` directory.

## Build - Linux

Compile and install every library.

Ubuntu 20.04:

```
sudo apt-get install -y libogg-dev libflac-dev
cd lib
git clone https://github.com/michaelrsweet/mxml.git
cd mxml
./configure
make 
sudo make install
cd ../..
```

Then use the Makefile:

```make```

The ```iceflac``` binary will be in the ```build``` build directory.

## Usage

Copy the ```settings/iceflac.xml``` next to the ```iceflac``` executable.
Modify the content of the ```iceflac.xml```.
To use such a high bitrate a FLAC can provide modify the  ```<limits> / <queue-size>``` setting in the configuration file of the icecast server. My setting is ```<queue-size>111524288</queue-size>```.

```
iceflac.exe -p playlist.txt
```

where ```playlist.txt``` is a text file, where every line contains the path of a .FLAC file.

Use ```iceflac --help``` for additional informaton.

## TODO

- Support for UTF-8 filenames
- Fix warnings
- Clean the code

## Thanks to

- the developers of Icecast (<https://icecast.org>)
- the developers of ogg/flac (<https://xiph.org>)
- the developer of Mini-XML (<https://www.msweet.org/mxml>)
- Little Star Media for the base64 code (<https://github.com/littlstar/b64.c>)

## License

Use, modify as you wish, just mention me in your README ;)
