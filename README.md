# DynamicFPS(AndVolume)
Lowers FPS when game is unfocused. Optionally mutes the game as well.

Methodology and notes:
- Since everyone has different amounts of FPS and audio volume under different circumstances, it makes no sense to set those values to exact amounts when the window is unfocused. As such, both FPS and audio volume reduction are done as a percentage of whatever those values were before the game window is unfocused.
- VSync already breaks FPS limiters by design, so for those who play with VSync enabled, only game audio is affected.
- macOS requires the <cl>SDL Mod by qimiko</c> for full effect. Incidentally, Windows users may install <cl>SDL Mod by qimiko</c> by accident (depending on the Geode SDK version whenever you're reading this). As such, <cl>SDL Mod by qimiko</c> is <co>automatically uninstalled</c> for convenience to avoid confusion (this is allowed since <cl>SDL Mod by qimiko</c> is never coming to Windows in the first place).

Inspirations (based on GitHub usernames):
- juliand665 + LostLuma
- Prevter
- PolloVerde2
- ~~[REDACTED]~~

Mod logo and idea based on the original mod logo of the Dynamic FPS mod for Minecraft Fabric.

This [Geode mod](https://geode-sdk.org) is licensed under LGPLv2.