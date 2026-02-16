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
# STEP 3 - command blocks and functions
# ==============================

# { ...; } block
def test_command_block_basic():
    proc = spawn_42sh(["-c", "{ echo a; echo b; }"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]


# block + redirection
def test_block_with_redirection(tmp_path):
    outfile = tmp_path / "x.txt"
    proc = spawn_42sh(["-c", f"{{ echo a; echo b; }} > {outfile}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert outfile.read_text().strip().splitlines() == ["a", "b"]


# function define + call
def test_function_definition_and_call():
    proc = spawn_42sh(["-c", "foo() { echo ok; }; foo"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# function receives args
def test_function_args():
    proc = spawn_42sh(["-c", "f() { echo $1 $2; }; f a b"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "a b" 

