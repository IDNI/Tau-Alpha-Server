# Tau-Alpha-Server

## To build
You need boost 1.68+ and cmake 3.14+
For ubuntu installation looks like:
Boost
```
sudo add-apt-repository ppa:mhier/libboost-latest
sudo apt update
sudo apt install libboost1.70
sudo apt install libboost1.70-dev

apt install gcc-8
apt install g++-8
```

CMake
```
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
apt install cmake

cmake -Bbuild -H.
cmake --build build --target all
```

Then you need to change directory to the service you want to build.
For example
```
cd sendMessage
```
and run
```
cmake -Bbuild -H.
cmake --build build --target all
```

Binaries will be located in ./build directory

## Usage for messagingClient is

```
./messagingClient SERVERIP SERVERPORT UID [messages directory]
```
example
```
./messagingClient 127.0.0.1 10000 Ohad /myMessages
```

## Usage for sendMessage is

```
./sendMessage serverIP serverPort myUID destinationUIDs(comma separated) fileToSend
```
example
```
./sendMessage 127.0.0.1 10000 Andrei Fola,Isar,Ohad,Tomas ./text.txt
```

## Usage for messagingServer is

```
./messagingServer portToListen directoryForMessages
```
default port is 10000
default directory is ```messages```

