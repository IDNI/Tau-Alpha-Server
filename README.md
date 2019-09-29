# Tau-Alpha-Server

## Usage for messagingClient is

```
./messagingClient SERVERIP SERVERPORT UID
```
example
```
./messagingClient 127.0.0.1 10000 Ohad
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

