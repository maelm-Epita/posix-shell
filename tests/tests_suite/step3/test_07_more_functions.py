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
# STEP 3 - more functions
# ==============================

# functions defined inside another function become globally available 
def test_function_can_define_function_inside():
    script = "foo() { bar() { echo inner; }; }; foo; bar"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "inner" 


# redir on function call
def test_function_redirection(tmp_path):
    outfile = tmp_path / "f.txt"
    script = f"f() {{ echo ok; }}; f > {outfile}"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert outfile.read_text().strip() == "ok" 


# function returns last status
def test_function_return_status_from_last_command():
    script = "f() { true; }; f"
    proc = spawn_42sh(["-c", script])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 0


# vars inside function
def test_function_can_use_local_var_like_normal():
    script = "f() { a=42; echo $a; }; f; echo ${a-EMPTY}"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    # global a may or may not leak; just check f printed 42
    assert out.strip().splitlines()[0] == "42" 

def test_function_arguments_and_restore():
    script = "func () { echo $@; }; echo $@haha; func a b c d; echo $@"
    proc = spawn_42sh(["-c", script, "wow", "arg2"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    lines = out.strip().splitlines()
    assert lines[0] == "arg2haha"
    assert lines[1] == "a b c d"
    assert lines[2] == "arg2"

def test_function_unset():
    script = "foo () { echo test; }; foo; unset foo; foo; unset -f foo; foo"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 127
    lines = out.strip().splitlines()
    assert lines[0] == "test"
    assert lines[1] == "test"
    assert "command not found" in err

def test_function_var_evaluation_time_and_function_var_namespace():
    script = "foo=bar; foo () { echo $foo; }; foo; unset foo; foo"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "bar"

def test_functions_double_inner_def():
    script = "foo () { bar () { echo test; }; }; foo; foo; bar"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test"
