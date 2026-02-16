import subprocess as sp
import pytest
import time
import os
import signal

# configuration
EXECUTABLE = os.environ.get("BIN_PATH")
TIMEOUT = float(os.environ.get("TEST_TIMEOUT", "2.0"))

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
# STEP 3 - subshells / command substitution
# ==============================

# ( ) subshell keeps parent vars
def test_subshell_does_not_change_parent_vars():
    proc = spawn_42sh(["-c", "a=sh; (a=42; echo -n $a); echo $a"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert "42" in out and "sh" in out


# $(...) basic
def test_command_substitution_basic():
    proc = spawn_42sh(["-c", "echo $(echo hello) world"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello world" 

def test_command_substitution_basic_pipe():
    proc = spawn_42sh(["-c", "echo $(echo test | cat)"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test" 

def test_command_substitution_exit_code():
    proc = spawn_42sh(["-c", "$(rm test); echo $?"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "1" 

def test_command_substitution_unknown_command():
    proc = spawn_42sh(["-c", "\"$(rm test)\"; echo $?"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "127" 

def test_command_substitution_hard_context_restore():
    proc = spawn_42sh(["-c", "echo test > test; $(mkdir test_tmp && cd test_tmp); rm -rf test_tmp; cat test; rm test"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test" 

def test_command_substitution_with_redirection():
    proc = spawn_42sh(["-c", "echo $(echo a > a.txt)"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "" 

# $(...) in var assignment
def test_command_substitution_in_assignment():
    proc = spawn_42sh(["-c", "a=$(echo ok); echo $a"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 

def test_command_substitution_quoted_double():
    proc = spawn_42sh(["-c", "echo \"$(echo test)\""])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test"

def test_command_substitution_quoted_single():
    proc = spawn_42sh(["-c", "echo \'$(echo test)\'"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "$(echo test)"

def test_command_substitution_quoted_inner():
    proc = spawn_42sh(["-c", "echo \"$(\"echo test\")\""])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert "command not found" in err

def test_command_substitution_nested():
    proc = spawn_42sh(["-c", "echo $(echo $(echo test))"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test"

def test_command_substitution_nested_hard():
    proc = spawn_42sh(["-c", "$($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($($(echo test))))))))))))))))))))))))))))))))))))))))))))"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 1

def test_command_substitution_unterminated_paren():
    proc = spawn_42sh(["-c", "echo e("])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2


