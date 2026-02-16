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
# STEP 2 - more && / || combos and precedence
# ==============================

# chain all true
def test_and_or_more_1():
    proc = spawn_42sh(["-c", 'true && true && echo ok'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == 'ok'


# stop on false in && chain
def test_and_or_more_2():
    proc = spawn_42sh(["-c", 'true && false && echo no'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == ''


# or chain ends with echo
def test_and_or_more_3():
    proc = spawn_42sh(["-c", 'false || false || echo ok'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == 'ok'


# a if left true
def test_and_or_more_5():
    proc = spawn_42sh(["-c", 'true && echo a || echo b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == 'a'


# b if left false
def test_and_or_more_6():
    proc = spawn_42sh(["-c", 'false && echo a || echo b'])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert out.strip() == 'b'


# $? after false||true is 0
def test_and_or_sets_dollar_question():
    proc = spawn_42sh(["-c", "false || true; echo $?"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "0" 


# $? after true&&false is non-zero
def test_and_or_sets_dollar_question_2():
    proc = spawn_42sh(["-c", "true && false; echo $?"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() != "0" 


def test_redir_input_basic(tmp_path):
    f = tmp_path / "in.txt"
    f.write_text("hello\n")
    p = spawn_42sh(["-c", f"cat < {f}"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert out == "hello\n"

# > truncate
def test_redir_output_truncate(tmp_path):
    f = tmp_path / "out.txt"
    p = spawn_42sh(["-c", f"echo first > {f}; echo second > {f}; cat {f}"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert out.strip() == "second"

# >> append
def test_redir_output_append(tmp_path):
    f = tmp_path / "out.txt"
    p = spawn_42sh(["-c", f"echo first > {f}; echo second >> {f}; cat {f}"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert out.strip().splitlines() == ["first", "second"]

# 2> stderr to file
def test_redir_stderr_to_file(tmp_path):
    f = tmp_path / "err.txt"
    p = spawn_42sh(["-c", f"ls does_not_exist 2> {f}; cat {f}"])
    out, err = p.communicate(timeout=TIMEOUT)
    # cat est le dernier => rc 0 en général
    assert p.returncode == 0
    assert out.strip() != ""

# 2>&1 stderr to stdout
def test_redir_stderr_to_stdout():
    p = spawn_42sh(["-c", "ls does_not_exist 2>&1"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert out.strip() != ""
    assert err.strip() == ""

# 1>&2 stdout to stderr (sur builtin)
def test_redir_stdout_to_stderr_builtin():
    p = spawn_42sh(["-c", "echo hello 1>&2"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert out.strip() == ""
    assert err.strip() == "hello"

# >&2 form (sans IONUMBER)
def test_redir_and_fd_sugar():
    p = spawn_42sh(["-c", "echo hi >&2"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert out.strip() == ""
    assert err.strip() == "hi"

# bad fd duplication (déclenche branche erreur)
def test_redir_bad_fd_dup_fails():
    p = spawn_42sh(["-c", "echo hi 1>&9"])
    p.communicate(timeout=TIMEOUT)
    assert p.returncode != 0

# redir inside pipeline end
def test_pipeline_with_output_redirection(tmp_path):
    f = tmp_path / "p.txt"
    p = spawn_42sh(["-c", f"echo pipe | cat > {f}; cat {f}"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert out.strip() == "pipe"

# create unused fd (ouvre/ferme un fd -> couvre du code)
def test_redir_create_unused_fd(tmp_path):
    f = tmp_path / "fd3.txt"
    p = spawn_42sh(["-c", f"echo ok 3> {f}; echo done"])
    out, err = p.communicate(timeout=TIMEOUT)
    assert p.returncode == 0
    assert (tmp_path / "fd3.txt").exists()
