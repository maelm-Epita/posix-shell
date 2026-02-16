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
# STEP 4 - aliases (skip on bash)
# ==============================
def require_not_bash_for_alias():
    if is_bash_like():
        pytest.skip('bash non-interactive alias behavior differs')


# alias same line
def test_alias_same_line():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias ll='echo ok'; ll"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0

# alias basic
def test_alias_basic():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias ll='echo ok'\n ll"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 

# alias not first
def test_alias_not_first():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias ll='echo ok'\n echo ll"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ll" 

def test_alias_nested():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias b='echo test'; alias a=b\n a"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test" 

# alias nested not first
def test_alias_nested_not_first():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a=test; alias b='echo a'\n b"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "a" 

def test_alias_keyword():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias if=b; alias if\n if;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2

def test_alias_def():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a=\"a=b\"; alias;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "a='a=b'"

def test_alias_redir():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a=\"1>test.txt\"; alias;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "a='1>test.txt'"

def test_alias_negation():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a='echo test'\n ! a;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 1
    assert out.strip() == "test"

def test_alias_command_block_pipeline_and_or():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a='echo test'\n { ! a | a && a; };"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 1
    assert out.strip() == "test"

# alias should be ignored because this is funcdec not simple command name
def test_alias_funcdec():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a=b\n a () { echo test; }; unalias a\n a"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "test"

# unalias removes alias
def test_unalias_basic():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias ll='echo ok'; unalias ll\n ll"])
    out, err = proc.communicate(timeout=TIMEOUT)
    # calling ll should now fail
    assert proc.returncode != 0

# unalias all
def test_unalias_basic():
    require_not_bash_for_alias()
    proc = spawn_42sh(["-c", "alias a=b; alias c=d; unalias -a; alias"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == ""


# token-level alias (from subject)
def test_alias_affects_lexer_token_level_best_effort():
    require_not_bash_for_alias()
    script = "alias funcdec='foo('\nfuncdec) { echo ok; }\nfoo"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "ok" 

