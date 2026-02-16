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


# ===========================================
# STEP 1 - more command lists / if / comments
# ===========================================

# no spaces around ;
def test_list_variant_1():
    proc = spawn_42sh(["-c", 'echo a;echo b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ['a', 'b']

# if false should return 0 and execute nothing
def test_if_false_return_code():
    proc = spawn_42sh(["-c", 'if false; then echo OK; fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == ''


# spaces around ;
def test_list_variant_2():
    proc = spawn_42sh(["-c", 'echo a ; echo b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ['a', 'b']


# newline after ;
def test_list_variant_3():
    proc = spawn_42sh(["-c", 'echo a;\necho b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ['a', 'b']


# blank line
def test_list_variant_4():
    proc = spawn_42sh(["-c", 'echo a\n\necho b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ['a', 'b']


# comments between
def test_list_variant_5():
    proc = spawn_42sh(["-c", '#c1\necho a\n#c2\necho b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ['a', 'b']


# elif taken
def test_if_elif_1():
    proc = spawn_42sh(["-c", 'if false; then echo a; elif true; then echo b; else echo c; fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'b'


# else taken
def test_if_elif_2():
    proc = spawn_42sh(["-c", 'if false; then echo a; elif false; then echo b; else echo c; fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'c'


# only the first true branch is executed
def test_if_elif_3():
    proc = spawn_42sh(["-c", 'if true; then echo a; elif true; then echo b; else echo c; fi'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == 'a'


# compound list contain commands before then
def test_if_compound_list_stops_on_then():
    script = "if true\necho a\nthen\necho b\nfi"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]

def test_empty_command_only_newlines():
    proc = spawn_42sh(["-c", "\n\n\n"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == ""


def test_only_semicolons():
    proc = spawn_42sh(["-c", ";;;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2


def test_semicolon_and_newlines():
    proc = spawn_42sh(["-c", ";\n;\n"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2

