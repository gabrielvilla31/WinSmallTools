start "WinSmallTools" /min cmd.exe /c Release\WinSmallTools.exe configWallpaper.xml

@start SendMessageToApp.exe -window "WinSmallTools" -msg hide > NUL