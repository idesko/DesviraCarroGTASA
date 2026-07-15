Flipped Car Rescue

Source:
- FlippedCarRescue.txt

Behavior:
- prevents the flipped-car burn/explosion patch from the original mod
- when the player's current car is flipped, pressing W/A/S/D asks the game to try restoring the same car
- the trigger waits for the car to stay flipped for a few seconds before accepting input
- while flipped, the script gives the car a small handling boost and a light torque nudge
- the original vehicle is kept; no replacement entity is created
- the rescue is torque-only, so mission entities stay intact

Notes:
- this is intentionally conservative and CLEO-only
- it does not try to rewrite vehicle physics
- if you want, this can be moved directly into modloader/CLEO after compile
