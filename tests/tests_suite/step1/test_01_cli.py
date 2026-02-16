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
# STEP 1 - CLI / arguments
# ==============================

# invalid option: --not-exist
def test_cli_invalid_option_1():
    proc = spawn_42sh(["--not-exist"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert ("usage" in err.lower()) or ("invalid" in err.lower()) or ("option" in err.lower())


# invalid option: --bad
def test_cli_invalid_option_2():
    proc = spawn_42sh(["--bad"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert ("usage" in err.lower()) or ("invalid" in err.lower()) or ("option" in err.lower())


# invalid option: -Z
def test_cli_invalid_option_3():
    proc = spawn_42sh(["-Z"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert ("usage" in err.lower()) or ("invalid" in err.lower()) or ("option" in err.lower())


# invalid option: --
def test_cli_invalid_option_4():
    proc = spawn_42sh(["--"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert ("usage" in err.lower()) or ("invalid" in err.lower()) or ("option" in err.lower())


# invalid option: --unknown-option
def test_cli_invalid_option_5():
    proc = spawn_42sh(["--unknown-option"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert ("usage" in err.lower()) or ("invalid" in err.lower()) or ("option" in err.lower())


# -c without script should fail
def test_cli_c_missing_arg():
    proc = spawn_42sh(["-c"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert "c" in err.lower() or "usage" in err.lower()


# nonexistent script file
def test_cli_nonexistent_script_1():
    proc = spawn_42sh(["not_exist_0.sh"])
    out, err = proc.communicate(timeout=TIMEOUT)
    
    assert proc.returncode != 0
    assert ("no such" in err.lower()) or ("open" in err.lower()) or ("not exist" in err.lower())



# positional args after -c
def test_cli_positional_args_basic():
    rc, out, err = run_42sh(["-c", "echo $1 $2"], input_data=None)
    proc = spawn_42sh(["-c", "echo $1 $2", "42sh", "a", "b"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "a b" 


def test_cli_dollar0_present():
    proc = spawn_42sh(["-c", "echo $0", "myname"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "" 

