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
# STEP 3 - builtins: break / continue
# ==============================

# break stops loop
def test_break_exits_loop():
    proc = spawn_42sh(["-c", "for i in a b c; do echo $i; break; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "a" 


# continue skips 
def test_continue_skips_rest():
    proc = spawn_42sh(["-c", "for i in a b; do echo $i; continue; echo nope; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]


def test_break_outside_loop_error():
    proc = spawn_42sh(["-c", "break"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


def test_continue_outside_loop_error():
    proc = spawn_42sh(["-c", "continue"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


def test_break_with_too_many_args():
    proc = spawn_42sh(["-c", "break 1 2"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


def test_continue_with_too_many_args():
    proc = spawn_42sh(["-c", "continue 1 2"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


def test_break_with_invalid_argument():
    proc = spawn_42sh(["-c", "break foo"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


def test_continue_with_invalid_argument():
    proc = spawn_42sh(["-c", "continue bar"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0
