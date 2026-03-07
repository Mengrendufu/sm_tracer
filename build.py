#!/usr/bin/env python3

import os
import sys
import shutil
import subprocess
from pathlib import Path
from typing import List, Optional, Tuple

if sys.platform != "win32":
    import tty
    import termios

PROJECT_DIR = Path(__file__).parent.resolve()
BUILD_DIR = PROJECT_DIR / "build"
BIN_DIR = BUILD_DIR / "bin"

# ── ANSI ──────────────────────────────────────────────────────────────────────
RESET = "\033[0m"
BOLD = "\033[1m"
DIM = "\033[2m"

def fg(r: int, g: int, b: int) -> str:
    return f"\033[38;2;{r};{g};{b}m"

def bg(r: int, g: int, b: int) -> str:
    return f"\033[48;2;{r};{g};{b}m"

COL_BORDER = fg(60, 80, 110)
COL_TITLE = fg(130, 200, 255)
COL_SEL_FG = fg(15, 15, 25)
COL_SEL_BG = bg(100, 180, 255)
COL_NAME = fg(200, 215, 235)
COL_DESC = fg(90, 105, 120)
COL_HINT = fg(70, 85, 100)
COL_OK = fg(100, 220, 130)
COL_ERR = fg(255, 100, 90)
COL_CMD = fg(160, 130, 255)
COL_WARN = fg(255, 200, 80)

CURSOR_HIDE = "\033[?25l"
CURSOR_SHOW = "\033[?25h"
SAVE_CUR = "\033[s"
REST_CUR = "\033[u"

# ── Terminal helpers ──────────────────────────────────────────────────────────
def _term_size() -> Tuple[int, int]:
    try:
        sz = os.get_terminal_size()
        return sz.lines, sz.columns
    except OSError:
        return 24, 80

def _goto(row: int, col: int = 1) -> str:
    return f"\033[{row};{col}H"

def _erase_line() -> str:
    return "\033[2K"

# ── Keyboard ──────────────────────────────────────────────────────────────────
def _read_unix() -> bytes:
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        ch = sys.stdin.buffer.read(1)
        if ch == b"\x1b":
            nxt = sys.stdin.buffer.read(1)
            if nxt == b"[":
                fin = sys.stdin.buffer.read(1)
                return b"\x1b[" + fin
            return b"\x1b" + nxt
        return ch
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)

def _read_win() -> bytes:
    import msvcrt as _m
    _getch = getattr(_m, "getch")
    ch = _getch()
    if ch in (b"\x00", b"\xe0"):
        ch2 = _getch()
        return {b"H": b"\x1b[A", b"P": b"\x1b[B"}.get(ch2, b"")
    return ch

def getch() -> bytes:
    if sys.platform == "win32":
        return _read_win()
    return _read_unix()

# ── CMake helpers ─────────────────────────────────────────────────────────────
def _find(tool: str) -> Optional[str]:
    which = "where" if sys.platform == "win32" else "which"
    try:
        r = subprocess.run([which, tool], capture_output=True, text=True, check=False)
        if r.returncode == 0:
            return r.stdout.strip().splitlines()[0]
    except Exception:
        pass
    return None

CMAKE = _find("cmake")

# Hardcoded -G Ninja, no CMakePresets.json

def _cmake_configure_args() -> List[str]:
    args = [CMAKE, "-S", str(PROJECT_DIR), "-B", str(BUILD_DIR), "-G", "Ninja"]
    # Always set rpath for Linux
    if sys.platform != "win32":
        args += ["-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON"]
    return args

def _find_executable() -> Optional[Path]:
    if not BIN_DIR.exists():
        return None
    if sys.platform == "win32":
        exe = BIN_DIR / "sm_tracer.exe"
    else:
        exe = BIN_DIR / "sm_tracer"
    return exe if exe.exists() else None

# ── Subprocess runner ─────────────────────────────────────────────────────────
def _run(cmd: List[str], cwd: Path = PROJECT_DIR) -> int:
    print(f"{COL_CMD}  $ {' '.join(str(c) for c in cmd)}{RESET}", flush=True)
    try:
        return subprocess.run(cmd, cwd=str(cwd)).returncode
    except FileNotFoundError:
        print(f"{COL_ERR}  command not found: {cmd[0]}{RESET}")
        return 1

# ── Actions ───────────────────────────────────────────────────────────────────
def action_configure() -> bool:
    if not CMAKE:
        print(f"{COL_ERR}  cmake not found in PATH{RESET}")
        return False
    BUILD_DIR.mkdir(exist_ok=True)
    rc = _run(_cmake_configure_args())
    if rc == 0:
        print(f"{COL_OK}  configure OK  →  compile_commands.json generated{RESET}")
    else:
        print(f"{COL_ERR}  configure FAILED (exit {rc}){RESET}")
    return rc == 0

def action_build() -> bool:
    if not CMAKE:
        print(f"{COL_ERR}  cmake not found in PATH{RESET}")
        return False
    if not BUILD_DIR.exists() or not (BUILD_DIR / "build.ninja").exists():
        print(f"{COL_WARN}  not configured — running configure first…{RESET}")
        if not action_configure():
            return False
    rc = _run([CMAKE, "--build", str(BUILD_DIR)])
    if rc == 0:
        print(f"{COL_OK}  build OK{RESET}")
    else:
        print(f"{COL_ERR}  build FAILED (exit {rc}){RESET}")
    return rc == 0

def action_run() -> bool:
    if not action_build():
        return False
    exe = _find_executable()
    if not exe:
        print(f"{COL_ERR}  no executable found in {BIN_DIR}{RESET}")
        return False
    print(f"{COL_OK}  running  {exe.name}{RESET}")
    rc = _run([str(exe)])
    if rc != 0:
        print(f"{COL_ERR}  exited with code {rc}{RESET}")
    return rc == 0

def action_clean() -> bool:
    if not BUILD_DIR.exists():
        print(f"{COL_WARN}  build dir does not exist — nothing to clean{RESET}")
        return True
    try:
        shutil.rmtree(BUILD_DIR)
        print(f"{COL_OK}  clean OK  →  {BUILD_DIR} removed{RESET}")
        return True
    except Exception as e:
        print(f"{COL_ERR}  clean FAILED: {e}{RESET}")
        return False

def action_setup() -> bool:
    print(f"{COL_CMD}  $ git submodule update --init --recursive{RESET}")
    try:
        rc = subprocess.run([
            "git", "submodule", "update", "--init", "--recursive"
        ], cwd=str(PROJECT_DIR)).returncode
        if rc == 0:
            print(f"{COL_OK}  submodules initialized{RESET}")
            return True
        else:
            print(f"{COL_ERR}  git submodule update failed (exit {rc}){RESET}")
            return False
    except Exception as e:
        print(f"{COL_ERR}  git submodule update failed: {e}{RESET}")
        return False

# ── TUI ───────────────────────────────────────────────────────────────────────
MENU = [
    (b"run",         "run",         "build & run",                  action_run,        True),
    (b"build-only",  "build-only",  "compile without running",      action_build,      False),
    (b"configure",   "configure",   "generate compile_commands",    action_configure,  True),
    (b"clean",       "clean",       "remove build directory",       action_clean,      False),
    (b"setup",       "setup",       "init git submodules",          action_setup,      False),
]
MENU_LINES = len(MENU) + 4

def _paint_overlay(selected: int) -> None:
    rows, cols = _term_size()
    out: List[str] = []
    top = rows - MENU_LINES + 1
    sep = f"{COL_BORDER}{'\u2500' * min(cols, 60)}{RESET}"
    out.append(_goto(top, 1) + _erase_line() + sep)
    title_row = top + 1
    out.append(_goto(title_row, 1) + _erase_line()
               + f"  {BOLD}{COL_TITLE}build{RESET}{COL_DESC}  ↑/↓ j/k  Enter  q=quit{RESET}")
    for i, (_, name, desc, _action, _quit) in enumerate(MENU):
        row = top + 2 + i
        out.append(_goto(row, 1) + _erase_line())
        if i == selected:
            line = (f"  {COL_SEL_BG}{COL_SEL_FG}{BOLD}"
                    f"  ▶  {name:<13}{RESET}"
                    f"{COL_SEL_BG}{COL_SEL_FG}  {desc:<34}{RESET}")
        else:
            line = (f"  {COL_NAME}     {name:<13}{RESET}"
                    f"{COL_DESC}  {desc}{RESET}")
        out.append(line)
    hint_row = top + 2 + len(MENU)
    out.append(_goto(hint_row, 1) + _erase_line()
               + f"  {COL_HINT}{'\u2500' * min(cols - 4, 56)}{RESET}")
    sys.stdout.write("".join(out))
    sys.stdout.flush()

def _erase_overlay() -> None:
    rows, _ = _term_size()
    top = rows - MENU_LINES + 1
    out: List[str] = []
    for r in range(top, rows + 1):
        out.append(_goto(r, 1) + _erase_line())
    sys.stdout.write("".join(out))
    sys.stdout.flush()

def tui() -> None:
    selected = 0
    n = len(MENU)
    sys.stdout.write(CURSOR_HIDE)
    sys.stdout.flush()
    try:
        while True:
            _paint_overlay(selected)
            key = getch()
            if key in (b"q", b"Q", b"\x03", b"\x1b"):
                break
            elif key in (b"\x1b[A", b"k"):
                selected = (selected - 1) % n
            elif key in (b"\x1b[B", b"j"):
                selected = (selected + 1) % n
            elif key in (b"\r", b"\n"):
                _erase_overlay()
                rows, _ = _term_size()
                sys.stdout.write(_goto(rows - MENU_LINES, 1))
                sys.stdout.write(CURSOR_SHOW)
                sys.stdout.flush()
                print()
                MENU[selected][3]()
                if MENU[selected][4]:
                    break
                print(f"\n{COL_HINT}  press any key…{RESET}", end="", flush=True)
                sys.stdout.write(CURSOR_HIDE)
                sys.stdout.flush()
                getch()
    finally:
        _erase_overlay()
        sys.stdout.write(CURSOR_SHOW)
        sys.stdout.flush()

# ── CLI entry ─────────────────────────────────────────────────────────────────
ACTIONS = {
    "configure": action_configure,
    "build": action_build,
    "run": action_run,
    "clean": action_clean,
    "setup": action_setup,
    "build-only": action_build,
}

def main():
    if len(sys.argv) > 1:
        cmd = sys.argv[1].lower()
        action = ACTIONS.get(cmd)
        if action:
            ok = action()
            sys.exit(0 if ok else 1)
        else:
            print(f"{COL_ERR}  unknown command: {cmd}{RESET}")
            print("Available: configure, build, run, clean, setup")
            sys.exit(1)
    else:
        try:
            tui()
        except KeyboardInterrupt:
            sys.stdout.write(CURSOR_SHOW)
            sys.stdout.flush()

if __name__ == "__main__":
    main()
