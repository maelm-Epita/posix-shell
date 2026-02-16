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
# STEP 3 - dot builtin (.)
# ==============================

# . file sets variables in current shell
def test_dot_sources_file_sets_var(tmp_path):
    script = tmp_path / "s.sh"
    script.write_text("A=hello\n")
    proc = spawn_42sh(["-c", f". {script}; echo $A"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello" 


# . file defines functions
def test_dot_can_call_functions(tmp_path):
    script = tmp_path / "s.sh"
    script.write_text("f() { echo ok; }\n")
    proc = spawn_42sh(["-c", f". {script}; f"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 

