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

## Verification status — 2026-09-21

The build at commit `1cdf975a1ce2624f4e4382e7fb4cf4ded059b39a` passed
compilation and parser regression tests, but it is **not confirmed working on an
end-user system**. Microsoft Defender real-time protection subsequently detected
`Trojan:Win32/Wacatac.C!ml` in its `LuconiaInjector.exe` and quarantined it.
An earlier on-demand scan reported no threats; that result did not predict the
later real-time detection and must not be treated as clearance.

The recorded detection identifies the injector EXE, not a client DLL. The source
still intentionally uses OpenProcess, WriteProcessMemory and CreateRemoteThread
with LoadLibraryW. This establishes process-injection behavior, not the exact
reason for Defender's classification or proof of malicious intent.

Keep detected files quarantined. Do not disable protection or add exclusions.
The client DLL must be identified and reviewed separately before testing a complete
setup. A suspected incorrect detection can be submitted through
[Microsoft Security Intelligence](https://www.microsoft.com/en-us/wdsi/filesubmission).
No confirmed malware-free or end-to-end working release is currently available.
