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
# STEP 1 - more echo tests
# ==============================

# echo -n empty
def test_echo_more_1():
    proc = spawn_42sh(["-c", "echo -n ''"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == ''


# echo empty quoted
def test_echo_more_2():
    proc = spawn_42sh(["-c", "echo ''"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == '\n'


# echo with empty arg
def test_echo_more_3():
    proc = spawn_42sh(["-c", "echo a '' b"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'a  b\n'


# echo preserves spaces in quotes
def test_echo_more_4():
    proc = spawn_42sh(["-c", "echo '  a  '"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == '  a  \n'


# backslash space
def test_echo_more_5():
    proc = spawn_42sh(["-c", 'echo a\\ b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'a b\n'


# echo backslash
def test_echo_more_6():
    proc = spawn_42sh(["-c", 'echo \\\\ '])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == '\\\n'


# no newline then newline
def test_echo_more_7():
    proc = spawn_42sh(["-c", 'echo -n a; echo b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'ab\n'


# newline then no newline
def test_echo_more_8():
    proc = spawn_42sh(["-c", 'echo a; echo -n b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'a\nb'


# multiple -n
def test_echo_more_9():
    proc = spawn_42sh(["-c", 'echo -n a; echo -n b; echo c'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'abc\n'


# echo -n -n 
def test_echo_more_10():
    proc = spawn_42sh(["-c", 'echo -n -n hello'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'hello'


# echo 1 words
def test_echo_word_count_1():
    proc = spawn_42sh(["-c", "echo w1"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().split() == ['w1']


# echo 2 words
def test_echo_word_count_2():
    proc = spawn_42sh(["-c", "echo w1 w2"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().split() == ['w1', 'w2']


# echo must separate arguments with single spaces (no tabs)
def test_echo_args_spacing():
    proc = spawn_42sh(["-c", "echo 4 2 s h"])
    out, err = proc.communicate(timeout=TIMEOUT)

    assert proc.returncode == 0
    assert err.strip() == ""
    assert out.strip() == "4 2 s h"

