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
# STEP 2 - more redirection edge cases
# ==============================

# > clobbers existing
def test_redir_clobber_existing_file(tmp_path):
    outfile = tmp_path / "o.txt"
    outfile.write_text("OLD\n")
    proc = spawn_42sh(["-c", f"echo NEW > {outfile}"])
    proc.wait(timeout=TIMEOUT)
    assert outfile.read_text().strip() == "NEW" 


# >> appends
def test_redir_append_existing_file(tmp_path):
    outfile = tmp_path / "o.txt"
    outfile.write_text("A\n")
    proc = spawn_42sh(["-c", f"echo B >> {outfile}"])
    proc.wait(timeout=TIMEOUT)
    assert outfile.read_text().strip().splitlines() == ["A", "B"]


# cat < empty file
def test_redir_input_from_empty_file(tmp_path):
    infile = tmp_path / "in.txt"
    infile.write_text("")
    proc = spawn_42sh(["-c", f"cat < {infile}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out == "" 


# spaces around redirection
def test_redir_with_spaces_around(tmp_path):
    outfile = tmp_path / "o.txt"
    proc = spawn_42sh(["-c", f"echo hello  >   {outfile}"])
    proc.wait(timeout=TIMEOUT)
    assert outfile.read_text().strip() == "hello" 

# redir bad 
def test_redir_bad(tmp_path):
    proc = spawn_42sh(["-c", "2>;"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 2
    assert "Syntax error" in err


def test_pipe_redir2():
    outfile_pipe_redir2 = "out_pipe_redir2"

    proc = spawn_42sh([
        "-c",
        f"echo Sometimes you have to make a choice | cat > {outfile_pipe_redir2}"
    ])
    out, err = proc.communicate(timeout=TIMEOUT)

    assert proc.returncode == 0
    assert out.strip() == ""
    assert err.strip() == ""

    with open(outfile_pipe_redir2, "r") as f:
        content = f.read().strip()
    assert content == "Sometimes you have to make a choice"

    os.remove(outfile_pipe_redir2)

