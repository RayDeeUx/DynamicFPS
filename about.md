# DynamicFPS(AndVolume)
Lowers FPS when game is unfocused. Optionally lowers game volume as well.

Methodology and notes:
- Since everyone has different amounts of FPS and audio volume under different circumstances, it makes no sense to set those values to exact amounts when the window is unfocused. As such, both FPS and audio volume reduction are done as a percentage of whatever those values were before the game window is unfocused.
- VSync already breaks FPS limiters by design, so for those who play with VSync enabled, only game audio is affected.
- macOS requires the <cl>SDL Mod by qimiko</c> for full effect. Incidentally, Windows users may install <cl>SDL Mod by qimiko</c> by accident (depending on the Geode SDK version whenever you're reading this). As such, <cl>SDL Mod by qimiko</c> is <co>automatically uninstalled</c> for convenience to avoid confusion (this is allowed since <cl>SDL Mod by qimiko</c> is never coming to Windows in the first place).

Inspirations (based on GitHub username):
- juliand665 + LostLuma
- Prevter
- PolloVerde2
- ~~[REDACTED]~~

<c-ff0000>***__NOTE__***: FOR BEST RESULTS, ***__DO NOT USE VSYNC.__***</c>

<c-ff0000>***__NOTE FOR MACOS USERS__***:</c> <cl>SDL Mod by qimiko</c> <c-ff0000>is ***__REQUIRED__*** TO USE THIS MOD. NO EXCEPTONS.</c>

<mod:zmx.sdl>

<co>***__NOTE FOR WINDOWS USERS__***:</c> <cl>SDL Mod by qimiko</c> <co>gets auto-installed when downloading this mod. This is a bug with the Geode index, and starting from v1.0.1 of this mod, the current fix is to ***__AUTOMATICALLY UNINSTALL__***</c> <cl>SDL Mod by qimiko</c> <co>. NO FURTHER ACTION IS NEEDED FROM YOU ASIDE FROM RESTARTING THE GAME. Sorry for the inconvenience, guys!</c>

Mod logo and idea based on the original mod logo of the Dynamic FPS mod for Minecraft Fabric.

This [Geode mod](https://geode-sdk.org) is licensed under LGPLv2. To report bugs or get help with using my mods, press the Discord button in the lower left corner.