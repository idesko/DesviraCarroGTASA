Tiny Peds

Goal:
- Make pedestrian models appear tiny while keeping animations stable.

What was done:
- Added a small ASI that scans the ped pool in-game.
- For every active ped except the player, it gets the ped's `RpClump` root frame and applies a uniform scale once.
- The mod now tracks already-processed peds so it does not keep rescaling them every frame.
- This is visual-only and avoids editing collisions or textures.

Active files:
- modloader/Tiny Peds/TinyPeds.asi

Notes:
- Delete this folder to remove the effect.
- If the scale is too small or animation looks off, tweak `kTinyScale` in `TinyPeds.cpp` and rebuild.
- The current scale is intentionally tiny (`0.12f`) to make the effect obvious.
