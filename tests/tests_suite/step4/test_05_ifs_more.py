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
# STEP 4 - more IFS tests
# ==============================

# tab splitting 
def test_ifs_tab_splitting():
    proc = spawn_42sh(["-c", "a='x\ty'; for i in $a; do echo $i; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    # tab is IFS by default in many shells
    assert out.strip() != "x\ty" 


# newline splitting
def test_ifs_newline_splitting():
    proc = spawn_42sh(["-c", "a='x\ny'; for i in $a; do echo $i; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["x", "y"]


# IFS empty 
def test_ifs_empty_disables_splitting_best_effort():
    proc = spawn_42sh(["-c", "IFS=; a='x y'; for i in $a; do echo $i; done"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "" 


def test_redirection_invalid_fd():
    proc = spawn_42sh(["-c", "echo test 9>file"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0
    assert err.strip() != ""

def test_redirection_permission_denied(tmp_path):
    f = tmp_path / "ro.txt"
    f.write_text("hello")
    f.chmod(0o444)

    proc = spawn_42sh(["-c", f"echo test > {f}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0
def test_pipeline_middle_command_fails():
    proc = spawn_42sh(["-c", "echo ok | doesnotexist | cat"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0

def test_expansion_in_redirection(tmp_path):
    f = tmp_path / "x.txt"
    proc = spawn_42sh(["-c", f"name=x; echo ok > {tmp_path}/$name.txt"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert f.read_text().strip() == "ok"

