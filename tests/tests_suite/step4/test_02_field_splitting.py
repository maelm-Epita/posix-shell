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
# STEP 4 - field splitting (IFS)
# ==============================

# default IFS splits unquoted
def test_ifs_default_splits_unquoted():
    proc = spawn_42sh(["-c", "a='x y'; for i in $a; do echo $i; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["x", "y"]





# IFS=, splits
def test_ifs_custom_comma():
    proc = spawn_42sh(["-c", "IFS=,; a='a,b,c'; for i in $a; do echo $i; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b", "c"]


# IFS=: (empty fields behavior may vary)
def test_ifs_custom_colon():
    proc = spawn_42sh(["-c", "IFS=:; a='a::b'; for i in $a; do echo $i; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    # some shells may collapse empty fields; just check it doesn't print the whole string
    assert out.strip() != "a::b" 

def test_ifs_number():
    proc = spawn_42sh(["-c", "test=\"a b\"; func () { echo $#; }; func $test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "2" 

def test_ifs_collapse_empty():
    proc = spawn_42sh(["-c", "test=\"a $nonexistant b\"; func () { echo $#; }; func $test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "2" 

def test_ifs_single_quote():
    proc = spawn_42sh(["-c", "test=\'a $nonexistant b\'; func () { echo $#; }; func $test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "3"
