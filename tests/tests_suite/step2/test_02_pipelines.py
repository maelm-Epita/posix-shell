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
# STEP 2 - pipelines
# ==============================

# simple pipe
def test_pipe_uppercase():
    proc = spawn_42sh(["-c", "echo hello | tr a-z A-Z"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "HELLO" 


# 3 commands pipeline
def test_pipe_three_commands():
    proc = spawn_42sh(["-c", "printf 'a\nb\n' | grep b | tr b B"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "B" 

# pipeline exit status must be the exit status of the last command
def test_pipe_exit_status_last_command():
    proc = spawn_42sh(["-c", "true | false"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode != 0

# pipeline exit status must be the exit status of the last command
def test_pipe_exit_status_last_command_2():
    proc = spawn_42sh(["-c", "false | true"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 0


# pipe grammar with newline
def test_pipe_newline_after_bar():
    script = "echo hello |\ntr a-z A-Z"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "HELLO" 

