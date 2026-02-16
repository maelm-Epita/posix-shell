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
# STEP 2 - variables
# ==============================

# a=42 then $a
def test_var_assignment_and_expand():
    proc = spawn_42sh(["-c", "a=42; echo $a"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "42" 


# ${a}
def test_var_brace_expand():
    proc = spawn_42sh(["-c", "a=hello; echo ${a}"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "hello" 


# $? after false
def test_special_dollar_question():
    proc = spawn_42sh(["-c", "false; echo $?"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    # should be non-zero, commonly '1'
    assert out.strip() != "0" 


# $# is a number
def test_special_dollar_hash():
    proc = spawn_42sh(["-c", "echo $# ", "x", "y", "z"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().isdigit()


# for i; do ... (args)
def test_special_dollar_at():
    proc = spawn_42sh(["-c", "for i; do echo $i; done", "42sh", "caca", "pipi", "prout"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip().splitlines() == ["caca", "pipi", "prout"]

def test_var_command_assignment():
    proc = spawn_42sh(["-c", "testvar=var env | grep testvar; echo $testvar"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "testvar=var"

def test_var_command_reassignment():
    proc = spawn_42sh(["-c", "testvar=a testvar=b env | grep testvar"])
    out, err = proc.communicate(timeout=TIMEOUT)
    assert proc.returncode == 0
    assert out.strip() == "testvar=b"


def test_mixing_arguments():
    proc = spawn_42sh([
        "-c",
        'echo "$@"',
        "_",
        "Using",
        "LLMs",
        "is",
        "prohibited,",
        "even",
        "if",
        "an",
        "LLM",
        "might",
        "be",
        "a",
        "human."
    ])
    out, err = proc.communicate(timeout=TIMEOUT)

    assert proc.returncode == 0
    assert err.strip() == ""
    assert out.strip() == "Using LLMs is prohibited, even if an LLM might be a human."


def test_counting_and_numbering():
    proc = spawn_42sh([
        "-c",
        'echo "There are $# arguments and the 3rd one is \'$3\'"',
        "_",
        "carotte",
        "salade",
        "pomme de terre",
        "oignon"
    ])
    out, err = proc.communicate(timeout=TIMEOUT)

    assert proc.returncode == 0
    assert err.strip() == ""
    assert out.strip() == "There are 4 arguments and the 3rd one is 'pomme de terre'"



def test_all_args2():
    proc = spawn_42sh([
        "-c",
        'echo "$@"',
        "_",
        "best",
        "workshop",
        "is",
        "Java"
    ])
    out, err = proc.communicate(timeout=TIMEOUT)

    assert proc.returncode == 0
    assert err.strip() == ""
    assert out.strip() == "best workshop is Java"

