# GAM 312 — Module Three: Shelter Building

Open `SurvivalGame.uproject` in **Unreal Engine 5.8**, open `/Game/Maps/smalltestmap`, and press Play. This is a separate copy of the existing SurvivalGame project. The original project has not been edited.

## Controls

- WASD / mouse: move and look; existing sprint and harvesting controls are retained.
- Aim at a resource node and press **E**: harvest through the original Blueprint character.
- **F**: open or close the crafting widget.
- **CRAFT**: spend wood and stone to add an item to your building supplies.
- **PLACE**: spawn that item's preview and return to the game.
- Aim at a surface: a visibility trace positions the preview on a construction grid.
- **R**: rotate 90 degrees. **Page Up / Page Down**: adjust height by 20 cm.
- **Left click**: confirm a green preview. **Right click**: cancel.
- Green means placement is clear; red means no surface or blocked space.

| Item | Wood | Stone | Dimensions (cm) |
| --- | ---: | ---: | --- |
| Wall | 30 | 15 | 20 × 400 × 300 |
| Floor | 15 | 5 | 400 × 400 × 20 |
| Ceiling | 50 | 30 | 400 × 400 × 20 |

Inventory starts empty. Collect resources first. Crafting deducts materials immediately; placement consumes one crafted supply. Cancelling a preview keeps the crafted item. Floor and ceiling tiles snap to a 400 cm grid; walls use a 200 cm grid. A ceiling aimed at a floor is raised by 300 cm. Adjust the height if needed when building on irregular surfaces.

For a basic shelter, place a floor, then walls along its edges (rotate the side walls with R), then a ceiling. Leave an opening for entry. Collect from the additional trees and rocks in the test level as needed.

## Implementation

`Source/SurvivalGame/ShelterSystem.h` declares the building actors and UMG widget. `ShelterSystem.cpp` implements the inventory bridge, recipes, preview trace, overlap validation, placement, widget layout, and button events. Inline comments explain the non-obvious decisions.

`AShelterSystem` reads and writes the original `BP_SurvivalCharacter` integer properties `Wood` and `Stone`; harvesting and crafting share one inventory. `UShelterWidget` is a native UMG widget assembled in C++, rather than the unfinished legacy `CraftingWidget` asset. The retained legacy building assets are not used by this implementation. New placed objects use `AShelterPiece` and an explicit UBoxComponent for simple collision, separate from the visual cube mesh.

`/Game/ModuleThree/M_Shelter` supplies the brown placed-object material and green/red preview feedback. A saved `ShelterSystem` actor in `smalltestmap` starts the system. Extra resource nodes are also saved in that map.

This is a local, single-player assignment implementation. It does not add multiplayer replication or save/load of placed structures.

## Validation and remaining submission work

The editor target has been compiled using the installed Unreal Engine 5.8 toolchain. All 24 isolated Unreal checks passed, including camera tracing, valid placement, rejected overlaps, supply consumption, and ceiling height. See `VERIFICATION.txt` for the complete report. Live standalone-game checks also confirmed F opens/closes the menu, E harvesting updates the wood count, and clicking the crafting buttons updates materials and supplies. Temporary materials were seeded only in the running test session; new sessions still start with empty inventory.

No screencast was recorded, as requested. Use `SCREENCAST_GUIDE.md` to record your own walkthrough. The assignment filename is `SteppingStoneThree_Wilson.mp4`; show the repository source and commits pages.

The Git history includes the existing project and the Module Three implementation. The C++ source, Unreal assets, and verification report are available on the GitHub repository's main branch.
