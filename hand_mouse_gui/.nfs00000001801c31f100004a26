# Hand Mouse Control Panel

A terminal-based control panel for the Hand Mouse system that allows you to:
- Start/Stop the hand tracking program
- Edit configuration parameters in real-time
- Save/reload configuration

## Features

### Main Menu
The control panel provides an interactive menu with the following options:

```
[1] Start/Stop Program    - Launch or terminate the hand mouse tracking
[2] Screen Width          - Set your screen width (pixels)
[3] Screen Height         - Set your screen height (pixels)
[4] Pinch Threshold       - Distance threshold for pinch detection
[5] Smoothing            - Cursor movement smoothing factor (0.0 - 1.0)
[6] Click Max Duration   - Maximum duration for a click gesture (seconds)
[7] Drag Min Duration    - Minimum duration for a drag gesture (seconds)
[8] Save Configuration   - Write current settings to config file
[9] Reload Configuration - Load settings from config file
[0] Exit                 - Quit the control panel
```

## Usage

### Running the Control Panel

```bash
./hand_mouse_gui/hand_mouse_gui_bin
```

### Starting the Hand Mouse

1. Run the control panel
2. Select option `[1]` to start the program
3. The status will change to `● RUNNING` with the process ID
4. The Python vision system will begin tracking your hand

### Stopping the Hand Mouse

1. With the program running, select option `[1]` again
2. The program will be terminated gracefully
3. Status returns to `○ STOPPED`

### Editing Configuration

1. Select any option from `[2]` to `[7]`
2. Enter the new value when prompted
3. Values are updated in memory immediately
4. Select `[8]` to save changes to disk

### Configuration Parameters

- **screen_width** / **screen_height**: Your monitor resolution
- **pinch_threshold**: Smaller values = more sensitive pinch detection (default: 0.04)
- **smoothing**: Higher values = smoother but slower cursor (0.0-1.0, default: 0.5)
- **click_max_duration**: Maximum time to hold pinch for a click (default: 0.7s)
- **drag_min_duration**: Minimum time to hold pinch for dragging (default: 0.8s)

## Status Indicators

- `● RUNNING` (green) - Hand mouse is active
- `○ STOPPED` (red) - Hand mouse is not running

## Configuration File

Settings are stored in: `config/hand_mouse.conf`

Example:
```
# Hand Mouse Configuration
screen_width=1920
screen_height=1080
pinch_threshold=0.040
smoothing=0.50
click_max_duration=0.7
drag_min_duration=0.8
```

## Process Management

The control panel uses `fork()` and `exec()` to launch the Python vision system:
- Process ID is tracked for proper cleanup
- `SIGTERM` is sent on stop to ensure graceful shutdown
- Auto-cleanup on exit prevents orphaned processes

## Architecture

This is a simple C-based terminal UI that:
- Requires no external GUI libraries (GTK, Qt, etc.)
- Provides full configuration control
- Integrates with the existing hand_mouse_core socket server
- Manages the Python vision_python/socket_client.py process

## Compilation

Included in the main Makefile:
```bash
make              # Build all components
make clean        # Remove binaries
```

The GUI binary is built with:
```bash
gcc -Wall -Wextra -O2 hand_mouse_gui/gui.c -o hand_mouse_gui/hand_mouse_gui_bin
```

## Dependencies

- Standard C library
- POSIX signals (`signal.h`)
- Process management (`unistd.h`, `sys/types.h`)
- Python 3 with the vision system installed

## Notes

- Configuration changes require selecting `[8]` to persist to disk
- The program auto-stops when you exit the control panel
- Invalid inputs are handled gracefully
- The interface uses ANSI color codes for status display
