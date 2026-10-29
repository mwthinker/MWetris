---
name: Analyze MWetris Input Log
description: Find the MWetris runtime log and interpret InputDelay and InputToMoveDelay measurements
compatibility: Windows only for now
---

# Analyze MWetris Input Log

This workflow is Windows-specific for now. Never hard-code an absolute path.

## Find the log

1. Read `App/src/logger.cpp` and its call site in `App/src/main.cpp` to confirm the current filename and folder behavior.
2. Remember that a relative log path is resolved from the application's working directory, not necessarily from the executable directory. Visual Studio commonly launches the application from a build directory.
3. Search the workspace recursively for `logs/mwetris.log`. Prefer dedicated file-search tools.
4. If it is not in the workspace, use PowerShell to search the user's home directory:

   ```powershell
   Get-ChildItem -Path $HOME -Filter mwetris.log -File -Recurse -ErrorAction SilentlyContinue |
       Sort-Object LastWriteTime -Descending |
       Select-Object FullName, LastWriteTime, Length
   ```

5. When several logs exist, start with the most recently modified one. Verify it using its final `[main]` startup record, timestamp, Git hash, and subsequent shutdown record. Do not silently combine different application runs.

## Select the latest run

- A run begins at `[main] MWetris Version`.
- Analyze metric records after the last applicable startup line.
- Use surrounding records such as `Starting local game`, `Game started`, and `Player slot event` only as context.
- Do not infer the exact human/AI player mix unless the log states it or the user supplies it.
- If no metrics exist, check whether the binary predates the instrumentation, the run lasted less than five seconds, or no relevant input was produced.

## Interpret metrics

- `[InputDelay] event-to-handler`: SDL event timestamp to `MainWindow::processEvent`. This primarily measures main-loop/event-queue delay. It can include non-gameplay key presses.
- `[InputToMoveDelay] event-to-board-move`: SDL event timestamp to the initial human board action. It excludes inputs that produce no board move and rotations deferred during ARE.
- `[FrameTiming]`: outer-loop frame interval statistics and effective FPS. Compare its average with `1000 / monitor_hz`; the fixed gameplay timestep does not guarantee the same outer-loop rate.
- Different sample counts are expected. Do not directly subtract aggregate averages when their samples differ.
- For multiple summary records, compute the weighted average:

  ```text
  weighted average = sum(samples * average) / sum(samples)
  overall maximum = maximum of each reported maximum
  ```

- Never average the interval averages without weighting by their sample counts.
- Read `[TetrisWindow] Window ...Hz` and convert latency to frames with `latency_ms * refresh_hz / 1000`.
- If event-to-handler and event-to-board-move are nearly equal, board/controller processing adds little delay. Investigate event polling, frame pacing, rendering, and presentation next.
- These metrics do not measure photon-to-screen latency or GPU/display presentation time.

## Report

State:

1. Which relative log location and run were analyzed.
2. Sample count, weighted average, and maximum for each available latency metric, plus effective FPS when frame timing is available.
3. Approximate latency in frames when refresh rate is known.
4. A short conclusion that distinguishes measured evidence from likely causes.
