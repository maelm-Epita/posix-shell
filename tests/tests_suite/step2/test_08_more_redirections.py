import subprocess as sp
import pytest
import time
import os
import signal
import stat

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
# STEP 2 - more redirections / fd tricks
# ==============================

# > /dev/null
def test_redir_stdout_to_devnull():
    proc = spawn_42sh(["-c", "echo hello > /dev/null"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == "" 


# 2>/dev/null
def test_redir_stderr_to_devnull():
    proc = spawn_42sh(["-c", "ls /nope 2>/dev/null; echo ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# 1>&2 and 2>&1 basics
def test_redir_merge_stdout_stderr():
    proc = spawn_42sh(["-c", "echo out; echo err 1>&2; echo both 2>&1"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0


# < nonexistent file should fail
def test_redir_input_missing_file_fails():
    proc = spawn_42sh(["-c", "cat < /this/file/does/not/exist"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


# stdout and stderr separate
def test_redir_multiple_on_command(tmp_path):
    outf = tmp_path / "o.txt"
    errf = tmp_path / "e.txt"
    proc = spawn_42sh(["-c", f"echo ok > {outf} 2> {errf}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert outf.read_text().strip() == "ok"
    assert errf.read_text() == "" 

