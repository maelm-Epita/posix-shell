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
# STEP 3 - builtins: cd / exit
# ==============================

# exit 0
def test_exit_status_0():
    proc = spawn_42sh(["-c", "exit 0"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 0


# exit with code
def test_exit_status_42():
    proc = spawn_42sh(["-c", "exit 42"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 42

def test_exit_list():
    proc = spawn_42sh(["-c", "exit ; echo test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert not "test" in out.strip()

def test_exit_and():
    proc = spawn_42sh(["-c", "exit && echo test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert not "test" in out.strip()

def test_exit_or():
    proc = spawn_42sh(["-c", "exit || echo test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert not "test" in out.strip()

def test_exit_pipeline():
    proc = spawn_42sh(["-c", "exit | echo test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert "test" in out.strip()

def test_exit_if():
    proc = spawn_42sh(["-c", "if exit 0; then echo test; fi"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert not "test" in out.strip()

def test_exit_else():
    proc = spawn_42sh(["-c", "if exit 1; then echo test; else echo testelse; fi"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 1
    assert not "test" in out.strip()

def test_exit_for():
    proc = spawn_42sh(["-c", "for el in a b c; do exit 0; echo $el; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert not "test" in out.strip()

def test_exit_while():
    proc = spawn_42sh(["-c", "while exit 0; do echo test; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert not "test" in out.strip()

def test_exit_until():
    proc = spawn_42sh(["-c", "until exit 1; do echo test; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 1
    assert not "test" in out.strip()

# cd updates PWD
def test_cd_to_tmp_and_pwd():
    proc = spawn_42sh(["-c", "cd /tmp; echo $PWD"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().endswith("/tmp")


# cd - uses OLDPWD
def test_cd_dash_oldpwd():
    proc = spawn_42sh(["-c", "cd /tmp; cd -; echo $PWD"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "" 

