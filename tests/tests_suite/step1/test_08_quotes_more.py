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
# STEP 1 - more quotes / escapes
# ==============================

# double quotes keep spaces
def test_quotes_more_1():
    proc = spawn_42sh(["-c", 'echo "a b"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'a b'


# double quotes expand
def test_quotes_more_2():
    proc = spawn_42sh(["-c", 'a=hi; echo "$a"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'hi'


# variable expansion inside double quotes
def test_quotes_more_3():
    proc = spawn_42sh(["-c", 'a=hi; echo "${a}there"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'hithere'


# hash not comment in quotes
def test_quotes_more_4():
    proc = spawn_42sh(["-c", 'echo "#not_comment"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == '#not_comment'


# escaped quote in double quotes
def test_quotes_more_5():
    proc = spawn_42sh(["-c", 'echo "a\\"b"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'a"b'


# escaped backslash
def test_quotes_more_6():
    proc = spawn_42sh(["-c", 'echo "a\\\\b"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'a\\b'


# tab escape
def test_quotes_more_7():
    proc = spawn_42sh(["-c", 'echo "a\\tb"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() in ['a\\tb','a\tb','atb']

def test_quotes_more_escape_if_keyword():
    proc = spawn_42sh(["-c", 'i\\\nf true; then echo test; fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test"

def test_line_continuation_basic():
    proc = spawn_42sh(["-c", "echo hello \\\nworld"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello world"


def test_line_continuation_inside_quotes():
    proc = spawn_42sh(["-c", 'echo "hello \\\nworld"'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello world"

def test_mixed_quotes_and_escapes():
    proc = spawn_42sh(["-c", "echo \"a\\\"b\"'c'"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'a"bc'


def test_backslash_at_end_of_input():
    proc = spawn_42sh(["-c", "echo a\\"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2

