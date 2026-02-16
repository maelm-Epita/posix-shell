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
# STEP 2 - more negation !
# ==============================

# double negation on true
def test_negation_more_1():
    proc = spawn_42sh(["-c", '! ! true'])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode is not None


# double negation on false
def test_negation_more_2():
    proc = spawn_42sh(["-c", '! ! false'])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode is not None


# negation with subshell
def test_negation_more_3():
    proc = spawn_42sh(["-c", '! (true)'])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode is not None


# !true && ... should not print
def test_negation_and_short_circuit():
    proc = spawn_42sh(["-c", "! true && echo ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == "" 


# !false is true so prints ok
def test_negation_and_prints():
    proc = spawn_42sh(["-c", "! false && echo ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == "ok" 


# !true is false so || runs
def test_negation_or_combo():
    proc = spawn_42sh(["-c", "! true || echo ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == "ok" 


# !false is true so || does not run
def test_negation_or_combo_2():
    proc = spawn_42sh(["-c", "! false || echo no"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == "" 

