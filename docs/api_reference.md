[C_NamePlates](#c_nameplate) - [Unit](#unit) - [Inventory](#inventory) - [Misc](#misc)

# C_NamePlate
Backported C-Lua interfaces from retail and added custom distance-related functions

## C_NamePlate.GetNamePlateForUnit`API`
Arguments: **unitId** `string`

Returns: **namePlate** `frame`

Get nameplates by unitId
```lua
local namePlate = C_NamePlate.GetNamePlateForUnit("target")
```

## C_NamePlate.GetNamePlates`API`
Arguments: `none`

Returns: **namePlateList** `table`

Get all visible nameplates
```lua
for i, nameplate in pairs(C_NamePlate.GetNamePlates()) do
  -- something with nameplate
end
```

## C_NamePlate.GetNamePlatesDistance`API`
Arguments: `none`

Returns: **namePlateDistanceHash** `table`

Get a hash table mapping all visible nameplates to their distances relative to the player.
```lua
local distanceHash = C_NamePlate.GetNamePlatesDistance()
local namePlate = C_NamePlate.GetNamePlateForUnit("target")
local distance = distanceHash[namePlate]
-- Iterate over all nameplates
for nameplate, distance in pairs(distanceHash) do
  -- something with nameplate and distance
end
```

## C_NamePlate.GetNamePlatesDistanceList`API`
Arguments: `none`

Returns: **namePlateDistanceList** `table`

Get an indexed list of all visible nameplates and their distances relative to the player
```lua
for i, entry in ipairs(C_NamePlate.GetNamePlatesDistanceList()) do
  -- something with entry.nameplate and entry.distance
end
```

## C_NamePlate.GetDistanceForUnit`API`
Arguments: **unitId** `string`

Returns: **distance** `number`

Get distance by unitId
```lua
local distance = C_NamePlate.GetDistanceForUnit("target")
```

## C_NamePlate.GetDistanceForGUID`API`
Arguments: **GUID** `string`

Returns: **distance** `number`

Get distance by GUID
```lua
local distance = C_NamePlate.GetDistanceForGUID(UnitGUID("target"))
```

## C_NamePlate.GetDistanceForNamePlate`API`
Arguments: **namePlate** `frame`

Returns: **distance** `number`

Get distance by namePlate
```lua
local namePlate = C_NamePlate.GetNamePlateForUnit("target")
local distance = C_NamePlate.GetDistanceForNamePlate(namePlate)
```

## NAME_PLATE_CREATED`Event`
Parameters: **namePlateBase**`frame`

Fires when nameplate was created

## NAME_PLATE_UNIT_ADDED`Event`
Parameters: **unitId**`string`

Notifies that a new nameplate appeared

## NAME_PLATE_UNIT_REMOVED`Event`
Parameters: **unitId**`string`

Notifies that a nameplate will be hidden

## nameplateDistance`CVar`
Arguments: **distance**`number`

Default: **41**

Sets the display distance of nameplates in yards

# Unit

## UnitIsControlled`API`
Arguments: **unitId**`string`

Returns: **isControlled**`bool`

Returns true if unit being hard control

## UnitIsDisarmed`API`
Arguments: **unitId**`string`

Returns: **isDisarmed**`bool`

Returns true if unit is disarmed


## UnitIsSilenced`API`
Arguments: **unitId**`string`

Returns: **isSilenced**`bool`

Returns true if unit is silenced

# Inventory

## GetInventoryItemTransmog`API`
Arguments: **unitId**`string`, **slot**`number`

Returns: **itemId**`number`, **enchantId**`number`

Returns info about item transmogrification

# Misc

## FlashWindow`API`
Arguments: `none`

Returns: `none`

Starts flashing of game window icon in taskbar

## IsWindowFocused`API`
Arguments: `none`

Returns: `bool`

Returns 1 if game window is focused, overtwice nil

## FocusWindow`API`
Arguments: `none`

Returns: `none`

Raise game window

## CopyToClipboard`API`
Arguments: **text**`string`

Returns: `none`

Copies text to clipboard

## cameraFov`CVar`
Parameters: **value**`number`

Default: **100**

Сhanges the camera view area (fisheye effect), in range **1**-**200**
