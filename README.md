# Metin2_Packet_Editor_Test


This is a packet editor for metin2. It should work for almost every metin2 pvp servers. This program can log and re-send the packets.

I used visual studio 2022 for this project. If you get any errors then use 2022 version.

If you want to use this program first you need to change the HOOK_ADDRESS variable in dllmain.cpp

"56 57 8B F1 E8 ?? ?? ?? ?? 8B F8 85 FF ?? ?? 8B CE E8 ?? ?? ?? ?? 84 C0"  make an aob scan on the game and copy the address of the first call right after the function start.

If aob scan isn't working then put a breakpoint on "ws2_32.send" function and step outside of the function and copy the first call right after the function start.


Then change the HOOK_ADDRESS variable with the address you copied. Compile and use it

Finally you must inject the dll with thread injection method otherwise the window won't spawn.
