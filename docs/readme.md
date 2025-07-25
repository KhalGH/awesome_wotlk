# Awesome WotLK
## World of Warcraft 3.3.5a 12340 improvements library
___
## Details
> - Auto Login (Through cmdline/shortcuts, Usage: `Wow.exe -login "LOGIN" -password "PASSWORD" -realmlist "REALMLIST" -realmname "REALMNAME" `)
> - Changing cameras FOV
> - Improved nameplates sorting
> - New API:<br>
    - C_NamePlate.GetNamePlates<br>
    - C_NamePlate.GetNamePlateForUnit<br>
    - C_NamePlate.GetNamePlateForGUID<br>
    - C_NamePlate.GetGUIDForNamePlate<br>
    - C_NamePlate.GetNamePlatesDistance<br>
    - C_NamePlate.GetNamePlatesDistanceList<br>
    - C_NamePlate.GetDistanceForUnit<br>
    - C_NamePlate.GetDistanceForGUID<br>
    - C_NamePlate.GetDistanceForNamePlate<br>
    - UnitIsControlled<br>
    - UnitIsDisarmed<br>
    - UnitIsSilenced<br>
    - GetInventoryItemTransmog<br>
    - FlashWindow<br>
    - IsWindowFocused<br>
    - FocusWindow<br>
    - CopyToClipboard
> - New events:<br>
    - NAME_PLATE_CREATED<br>
    - NAME_PLATE_UNIT_ADDED<br>
    - NAME_PLATE_UNIT_REMOVED
> - New CVars:<br>
    - nameplateDistance<br>
    - cameraFov<br>

## Documentation
See [Docs](https://github.com/KhalGH/awesome_wotlk/blob/main/docs/api_reference.md) for details

## Installation
1) Download latest [release](https://github.com/KhalGH/awesome_wotlk/releases)
2) Unpack files to root game folder
3) Launch `AwesomeWotlkPatch.exe`, you should get a message
4) To update just download and replace dll

## 3rd party libraries
- [microsoft-Detours](https://github.com/microsoft/Detours) - [license](https://github.com/microsoft/Detours/blob/6782fe6e6ab11ae34ae66182aa5a73b5fdbcd839/LICENSE.md)
