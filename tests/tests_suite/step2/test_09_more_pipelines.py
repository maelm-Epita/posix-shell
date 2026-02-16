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
# STEP 2 - more pipelines
# ==============================

# pipe into grep
def test_pipe_with_grep():
    proc = spawn_42sh(["-c", "printf 'aa\nbb\n' | grep bb"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "bb" 


# pipe into wc
def test_pipe_with_wc():
    proc = spawn_42sh(["-c", "printf 'a\nb\n' | wc -l"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().isdigit()


# true | cat
def test_pipe_empty_left():
    proc = spawn_42sh(["-c", "true | cat"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0


# echo ok | false => status false
def test_pipe_empty_right_fails():
    proc = spawn_42sh(["-c", "echo ok | false"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode != 0

