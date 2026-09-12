# POC A: QDockWidget hosts

This proof of concept keeps `BoardViewerWidget` as the main window central widget and hosts the three embeddable engine tools in native Qt docks.

## Manual evaluation matrix

- Add Principal Variations, Engine Logs, and Engine Options from Views > Embedded engine tools.
- Confirm adding a second instance of the same stable tool type is refused.
- Move, resize, float, close, and re-dock each tool.
- Confirm closing a dock removes it from the embedded configuration and that it can be added again.
- Open a standalone instance and confirm it does not appear in the embedded configuration.
- Restart with the same running-engine setup and verify dock selection, object names, floating state, dock placement, and main-window geometry restore.
- Use Reset embedded tool layout and confirm active tools return to the right dock area.
- Interact with the board and move list after dock movement and after floating a dock over the main window.
- Remove a Principal Variations dock while its engine is producing info messages; verify no callback targets the destroyed widget.

## POC observations to record

- Native docking provides movement, floating, closing, and re-docking without changing tool widget internals.
- Docked tools reduce the central board area; floating tools can visually overlap the board but remain separate top-level event targets.
- `QMainWindow::restoreState()` requires the stable `engineToolDock_<tool-id>` object names to exist before restoration.
- Engine associations are validated against the current running-engine service; invalid persisted associations are discarded.
