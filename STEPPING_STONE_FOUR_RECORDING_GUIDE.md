# Stepping Stone Four recording guide

Use this outline to record the required narrated walkthrough. Save the finished video as `SteppingStoneFour_Wilson.mp4`. Do not claim a result that is not visible in the recording.

## Walkthrough

1. **Repository and project.** Show the GitHub repository source page and commits page. Open `SurvivalGame.uproject` in Unreal Engine 5.8 and show `/Game/Maps/smalltestmap`.
2. **Player HUD code.** Show `PlayerStatsWorldSubsystem.h/.cpp` and `PlayerStatsWidget.h/.cpp`. Explain that the world subsystem creates the HUD for the local player, reads Health, Hunger, and Stamina from the pawn, and refreshes the labels and progress bars as values change. Point out the comments describing property lookup and widget layout.
3. **AI wander code.** Show `WanderAIController.h/.cpp` and explain how it requests a reachable navigation point, moves to it, waits a random 1–3 seconds, and retries. Then open `/Game/AI/BP_WanderAIController` and show its commented EventGraph: BeginPlay → Delay (0.1) → StartWandering. Show the wander radius and pause settings in the Blueprint defaults.
4. **Live demonstration.** Press Play in `smalltestmap`. Show the Health, Hunger, and Stamina bars, including a value changing while the game runs. Show the AI pawn moving across the navigation area. Keep the HUD and moving pawn visible long enough to confirm both behaviors.
5. **Wrap up.** Return to the repository source and commits pages and identify the main implementation files and latest commit.

## Before recording

- Close and reopen `BP_WanderAIController` in the Content Browser if its graph is not refreshed in the editor.
- If navigation is not green, choose **Build → Build Paths** and wait for it to finish.
- Save the level, confirm the Blueprint compiles, and stop Play before changing assets.
- Record narration while showing the relevant code and the full gameplay HUD.
