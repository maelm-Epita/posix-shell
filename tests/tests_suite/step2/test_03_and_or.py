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
# STEP 2 - && and || operators
# ==============================

# && true
def test_and_runs_second_on_success():
    proc = spawn_42sh(["-c", "true && echo ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# && second command must not run on failure
def test_and_short_circuit_on_failure():
    proc = spawn_42sh(["-c", "false && echo should_not_print"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0
    assert out.strip() == "" 


# || executes right when left fails
def test_or_runs_second_on_failure():
    proc = spawn_42sh(["-c", "false || echo ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# || second command must not run on success
def test_or_short_circuit_on_success():
    proc = spawn_42sh(["-c", "true || echo should_not_print"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "" 

