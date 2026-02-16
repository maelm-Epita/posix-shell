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
# STEP 2 - negation 
# ==============================

# ! true -> non-zero
def test_negation_true_becomes_false():
    proc = spawn_42sh(["-c", "! true"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode != 0


# ! false -> 0
def test_negation_false_becomes_true():
    proc = spawn_42sh(["-c", "! false"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 0


# ! pipeline inverts status
def test_negation_pipeline():
    proc = spawn_42sh(["-c", "! true | false"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 0

def test_negation_double_nosep():
    proc = spawn_42sh(["-c", "!! echo test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 127
    assert ("command not found") in err.strip()

def test_negation_double_sep():
    proc = spawn_42sh(["-c", "! ! echo test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2 

def test_negation_only():
    proc = spawn_42sh(["-c", "!"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
