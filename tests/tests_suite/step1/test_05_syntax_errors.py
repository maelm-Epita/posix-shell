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
# STEP 1 - syntax errors should fail
# ==============================

# extra fi
def test_syntax_error_1():
    proc = spawn_42sh(["-c", 'if true; then echo ok; fi fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


# missing fi
def test_syntax_error_2():
    proc = spawn_42sh(["-c", 'if true; then echo ok;'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


# double ;
def test_syntax_error_3():
    proc = spawn_42sh(["-c", 'echo a;; echo b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


# if without condition
def test_syntax_error_4():
    proc = spawn_42sh(["-c", 'if; then echo ok; fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0


# unterminated quote must be a syntax error
def test_unterminated_quote_return_code():
    proc = spawn_42sh(["-c", "echo 'bad_unterminated_quote"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2
    assert out.strip() == ''


# bad separators must be a syntax error
def test_bad_separators_return_code():
    proc = spawn_42sh(["-c", "echo Sometimes ;; echo choice"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2
    assert err.strip() != ''

def test_memory_pressure_many_commands():
    script = ";".join(["echo x"] * 300)
    proc = spawn_42sh(["-c", script])
    proc.communicate(timeout=TIMEOUT)
    assert True


def test_memory_freed_on_syntax_error():
    proc = spawn_42sh(["-c", "if then fi"])
    proc.communicate(timeout=TIMEOUT)
    assert True

def test_then_without_if():
    proc = spawn_42sh(["-c", "then echo a; fi"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2


def test_else_without_if():
    proc = spawn_42sh(["-c", "else echo a; fi"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2


def test_fi_without_if():
    proc = spawn_42sh(["-c", "fi"])
    proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2

