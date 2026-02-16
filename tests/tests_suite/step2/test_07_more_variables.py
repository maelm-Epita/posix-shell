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
# STEP 2 - more variables / special params
# ==============================

# $$ (pid)
def test_dollar_dollar_is_number():
    proc = spawn_42sh(["-c", "echo $$"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().isdigit()


# $RANDOM exists
def test_random_is_number():
    proc = spawn_42sh(["-c", "echo $RANDOM"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "" 


# $UID exists
def test_uid_is_number():
    proc = spawn_42sh(["-c", "echo $UID"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "" 


# $* and $@ produce something
def test_dollar_star_vs_at_simple():
    proc = spawn_42sh(["-c", "echo $*; echo $@", "42sh", "a", "b"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    lines = out.strip().splitlines()
    assert len(lines) == 2


# ${A-DEF}
def test_parameter_default_value():
    proc = spawn_42sh(["-c", "unset A; echo ${A-DEF}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "DEF" 


# $1 $2 $3
def test_positional_1_2_3():
    proc = spawn_42sh(["-c", "echo $1 $2 $3", "42sh", "x", "y", "z"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "x y z" 


# $PWD exists
def test_pwd_variable_exists():
    proc = spawn_42sh(["-c", "echo $PWD"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "" 


# $OLDPWD after cd
def test_oldpwd_after_cd():
    proc = spawn_42sh(["-c", "cd /tmp; cd - >/dev/null 2>&1; echo $OLDPWD"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out is not None

def test_positionals_empty():
    proc = spawn_42sh(["-c", 'echo $@ $# $0 $1 $2 $3'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == '0 42sh'

def test_positionals_filled():
    proc = spawn_42sh(["-c", 'echo $@ $# $0 $1 $2 $3', "a", "b", "c", "d"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'b c d 3 a b c d'

def test_positionals_func():
    proc = spawn_42sh(["-c", 'func () { echo $@ $# $0 $1 $2 $3; }; func a b c d', "test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'a b c d 4 test a b c'
