import subprocess as sp
import pytest
import time
import os
import signal

# configuration
EXECUTABLE = os.environ.get("BIN_PATH")
TIMEOUT = float(os.environ.get("TEST_TIMEOUT", "1.0"))

# helpers
def require_binary():
    if not EXECUTABLE or not os.path.exists(EXECUTABLE):
        pytest.skip("binary not found")

def spawn_42sh(args=None, stdin=None):
    require_binary()
    if args is None:
        args = []
    return sp.Popen(
        [EXECUTABLE] + args,
        stdin=sp.PIPE if stdin is None else stdin,
        stdout=sp.PIPE,
        stderr=sp.PIPE,
        text=True
    )

def run_42sh(args=None, input_data=None):
    proc = spawn_42sh(args=args)
    out, err = proc.communicate(input_data, timeout=TIMEOUT)
    return proc.returncode, out, err

def kill_42sh(proc):
    try:
        proc.kill()
    except Exception:
        pass

def is_bash_like():
    try:
        rc, out, err = run_42sh(["-c", "echo ${BASH_VERSION-}"])
        return out.strip() != ""
    except Exception:
        return False


# ==============================
# STEP 1 - input modes
# ==============================

# -c mode basic echo
def test_mode_c_echo():
    proc = spawn_42sh(["-c", "echo hello"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello" 


# stdin mode with input
def test_mode_stdin_pipe():
    proc = spawn_42sh()
    out, err = proc.communicate("echo stdin\n", timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "stdin" 


# stdin mode multi lines
def test_mode_stdin_multiple_lines():
    proc = spawn_42sh()
    out, err = proc.communicate("echo a\necho b\n", timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"] 


# script file mode
def test_mode_script_file_basic(tmp_path):
    script = tmp_path / "script.sh"
    script.write_text("echo from_script\n")
    proc = spawn_42sh([str(script)])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "from_script" 


# script file + positional args
def test_mode_script_file_with_args(tmp_path):
    script = tmp_path / "script.sh"
    script.write_text("echo $1 $2\n")
    proc = spawn_42sh([str(script), "x", "y"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "x y" 


# empty stdin should not hang
def test_mode_empty_stdin():
    proc = spawn_42sh()
    out, err = proc.communicate("", timeout=TIMEOUT)
    assert proc.returncode == 0 or proc.returncode is not None

