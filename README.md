# LuconiaInjector

Simple native Minecraft Bedrock Edition Injector

## Features
- Discord rich presence
- Custom dll
- Manual DLL selection only; automatic downloads are disabled because no independently verifiable release signature or digest is provided.
- Doesn't require admin rights

![image](https://github.com/CuteNyami/Luconia-Injector/assets/96192624/ba2745f3-e1f1-4ed1-a087-44f78099c257)

The injector design is inspired of the [Fate injector](https://github.com/fligger/FateInjector) :)

## Security changes

The application no longer contacts the former update host or links the bundled
precompiled curl library. Select a compatible DLL from a source you trust; no
cached DLL is automatically selected. The bundled Discord parser now validates
frame sizes and JSON objects and owns the parsed message storage.

These changes address source-level risks, not an antivirus false-positive ruling.
The injector still uses Windows process-injection APIs. Do not disable antivirus
or add exclusions to run it. Neither a successful build nor these changes certify
the executable or a separately downloaded DLL as malware-free.
