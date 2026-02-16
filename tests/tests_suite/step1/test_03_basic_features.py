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
# STEP 1 - basic builtins / lists / if / comments
# ==============================

# true builtin returns 0
def test_true_exit_code():
    proc = spawn_42sh(["-c", "true"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode == 0


# false builtin returns != 0
def test_false_exit_code():
    proc = spawn_42sh(["-c", "false"])
    proc.wait(timeout=TIMEOUT)
    assert proc.returncode != 0


# echo one word
def test_echo_variant_1():
    proc = spawn_42sh(["-c", 'echo hello'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'hello\n'


# echo two words
def test_echo_variant_2():
    proc = spawn_42sh(["-c", 'echo hello world'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'hello world\n'


# echo -n
def test_echo_variant_3():
    proc = spawn_42sh(["-c", 'echo -n hello'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'hello'


# echo -E
def test_echo_variant_4():
    proc = spawn_42sh(["-c", 'echo -E hello'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'hello\n'


# echo -e newline
def test_echo_variant_5():
    proc = spawn_42sh(["-c", "echo -e 'a\\nb'"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'a\nb\n'


# echo -e tab
def test_echo_variant_6():
    proc = spawn_42sh(["-c", "echo -e 'a\\tb'"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == 'a\tb\n'


# comment line ignored
def test_comment_full_line_ignored():
    proc = spawn_42sh(["-c", "# comment\necho ok"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# trailing comment
def test_comment_after_command_ignored():
    proc = spawn_42sh(["-c", "echo ok # trailing comment"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# # not first char => not a comment
def test_hash_not_comment_when_not_first_char():
    proc = spawn_42sh(["-c", "echo not#comment"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "not#comment" 


# command list with ;
def test_command_list_semicolon():
    proc = spawn_42sh(["-c", "echo a; echo b"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]


# list with trailing ;
def test_command_list_trailing_semicolon():
    proc = spawn_42sh(["-c", "echo a; echo b;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]


# list with newlines
def test_compound_list_newlines():
    proc = spawn_42sh(["-c", "echo a\necho b\n"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]


# if true
def test_if_then_else_true():
    proc = spawn_42sh(["-c", "if true; then echo ok; else echo ko; fi"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 


# if false
def test_if_then_else_false():
    proc = spawn_42sh(["-c", "if false; then echo ok; else echo ko; fi"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ko" 

# if false without else
def test_if_then_false():
    proc = spawn_42sh(["-c", "if false; then echo ok; fi"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0

# if false without else
def test_if_bad_sep():
    proc = spawn_42sh(["-c", "if false; ; then echo test; fi"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2

# if with newlines
def test_if_multiline_compound_list():
    script = "if true\nthen\n echo a\n echo b\nfi"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["a", "b"]

