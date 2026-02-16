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
# STEP 2 - redirections
# ==============================

# > overwrite
def test_redir_output_overwrite(tmp_path):
    outfile = tmp_path / "out.txt"
    proc = spawn_42sh(["-c", f"echo hello > {outfile}"])
    proc.wait(timeout=TIMEOUT)
    assert outfile.read_text().strip() == "hello" 


# >> append
def test_redir_output_append(tmp_path):
    outfile = tmp_path / "out.txt"
    spawn_42sh(["-c", f"echo a > {outfile}"]).wait(timeout=TIMEOUT)
    spawn_42sh(["-c", f"echo b >> {outfile}"]).wait(timeout=TIMEOUT)
    assert outfile.read_text().splitlines() == ["a", "b"]


# < input redirection
def test_redir_input_file(tmp_path):
    infile = tmp_path / "in.txt"
    infile.write_text("hello\n")
    proc = spawn_42sh(["-c", f"cat < {infile}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello" 


# 2> redirect stderr
def test_redir_stderr_to_file(tmp_path):
    errfile = tmp_path / "err.txt"
    proc = spawn_42sh(["-c", f"ls /this/path/does/not/exist 2> {errfile}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0
    assert errfile.read_text() != "" 


# order of redirections 
def test_redir_stdout_and_stderr_same_file(tmp_path):
    outf = tmp_path / "all.txt"
    proc = spawn_42sh(["-c", f"echo out; ls /nope 2>&1 > {outf}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode != 0
    assert outf.read_text().strip() == "out" 


# > file then 2>&1
def test_redir_both_to_file_other_order(tmp_path):
    outf = tmp_path / "all.txt"
    proc = spawn_42sh(["-c", f"echo out; ls /nope > {outf} 2>&1"])
    out, err = proc.communicate(timeout=TIMEOUT)
    txt = outf.read_text()
    assert "out" in txt
    assert txt != "" 


# builtin redirection
def test_redir_builtin_restore_stdout(tmp_path):
    outf = tmp_path / "x.txt"
    script = f"echo one > {outf}; echo two"
    proc = spawn_42sh(["-c", script])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert outf.read_text().strip() == "one"
    assert out.strip() == "two" 

