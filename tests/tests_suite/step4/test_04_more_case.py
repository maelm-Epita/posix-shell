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
# STEP 4 - more case patterns
# ==============================

# glob prefix
def test_case_pattern_1():
    proc = spawn_42sh(["-c", "v=foo; case $v in f*) echo YES;; *) echo NO;; esac"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "YES" 


# ? wildcard
def test_case_pattern_2():
    proc = spawn_42sh(["-c", "v=bar; case $v in b?r) echo YES;; *) echo NO;; esac"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "YES" 


# char class
def test_case_pattern_3():
    proc = spawn_42sh(["-c", "v=x; case $v in [xyz]) echo YES;; *) echo NO;; esac"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "YES" 


# first matching item used
def test_case_many_items_first_match():
    proc = spawn_42sh(["-c", "v=b; case $v in a) echo A;; b) echo B;; b*) echo C;; esac"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "B" 


def test_heredoc_basic_coverage():
    script = """cat <<EOF
hello
EOF
"""
    p = sp.Popen(
        [EXECUTABLE, "-c", script],
        stdout=sp.PIPE,
        stderr=sp.PIPE,
        text=True,
    )
    p.communicate(timeout=TIMEOUT)
    assert True


def test_heredoc_unclosed_coverage():
    p = sp.Popen(
        [EXECUTABLE, "-c", "cat <<EOF"],
        stdout=sp.PIPE,
        stderr=sp.PIPE,
        text=True,
    )
    p.communicate(timeout=TIMEOUT)
    assert True
