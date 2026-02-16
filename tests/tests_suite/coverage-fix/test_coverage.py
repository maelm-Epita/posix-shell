import os
import subprocess as sp
import pytest
import tempfile

EXECUTABLE = os.environ.get("BIN_PATH")
TIMEOUT = float(os.environ.get("TEST_TIMEOUT", "1.0"))


def run(cmd, env=None):
    if not EXECUTABLE or not os.path.exists(EXECUTABLE):
        pytest.skip("binary not found")
    p = sp.Popen(
        [EXECUTABLE, "-c", cmd],
        stdout=sp.PIPE,
        stderr=sp.PIPE,
        text=True,
        env=env,
    )
    out, err = p.communicate(timeout=TIMEOUT)
    return p.returncode, out, err


# =========================
# SIMPLE COMMANDS
# =========================

def test_simple_command():
    run("a=1 b=2 echo hello")


def test_pipeline_and_or():
    run("true && echo ok || echo no")


def test_pipeline_negated():
    run("! true | false &")


# =========================
# STRUCTURES
# =========================

def test_if_command():
    run("if true; then echo ok; fi")


def test_while_command():
    run("while false; do echo no; done")


def test_for_command():
    run("for i in a b; do echo $i; done")


def test_subshell_command():
    run("(echo ok)")


def test_list_command():
    run("{ echo a; echo b; }")


# =========================
# CASE
# =========================

def test_case_simple():
    run("case a in a) echo ok ;; esac")


def test_case_multi_pattern():
    run("case a in a|b|c) echo ok ;; esac")


def test_case_with_newlines():
    run("case a in a)\n\necho ok\n;;\nesac")


def test_case_with_pipeline(tmp_path):
    f = tmp_path / "x"
    run(f"case a in a) echo ok | cat > {f} ;; esac")


# =========================
# REDIRECTIONS
# =========================

def test_redirections(tmp_path):
    f = tmp_path / "f"
    run(f"echo hi > {f}")
    run(f"echo hi >> {f}")
    run(f"cat < {f}")
    run(f"cat <> {f}")
    run("echo hi 1>&2")
    run("echo hi 1>&-")


def test_heredoc():
    run("cat <<EOF\nhello\nEOF")


# =========================
# VARIABLES / EXPORT
# =========================

def test_export_and_env():
    run("export A=1 B=2; env | grep A")


def test_export_invalid():
    run("export 1A=42")
    run("export A-=42")


def test_unset():
    run("export A=1; unset A")


def test_env_to_vars():
    env = {"A": "1"}
    run("echo $A", env)


# =========================
# EXPANSION
# =========================

def test_expansion_basic():
    run("A=1 B=2; echo $A$B")


def test_expansion_ifs():
    run("IFS=,; A='1,2,3'; echo $A")


def test_command_substitution():
    run("echo $(echo ok)")


def test_param_expansion():
    run("unset A; echo ${A:-42}")
    run("A=ok; echo ${A:+yes}")


# =========================
# CD
# =========================

def test_cd_basic(tmp_path):
    run(f"cd {tmp_path}")


def test_cd_home():
    env = os.environ.copy()
    with tempfile.TemporaryDirectory() as d:
        env["HOME"] = d
        run("cd", env)

def test_redir_right_fd_invalid():
    run("echo hi 1>&abc")


def test_redir_left_fd_invalid():
    run("cat 0<&abc")


def test_redir_right_fd_close():
    run("echo hi 1>&-")


def test_redir_left_fd_close():
    run("cat 0<&-")

def test_redir_right_left(tmp_path):
    f = tmp_path / "rw"
    run(f"cat <> {f}")

def test_redir_left_open_fail():
    run("cat < /this/path/does/not/exist")

def test_multiple_redirections_same_command(tmp_path):
    f1 = tmp_path / "a"
    f2 = tmp_path / "b"
    run(f"echo hi > {f1} 1>&2 >> {f2}")


def test_heredoc_basic():
    run("cat <<EOF\nhello\nEOF")


def test_heredoc_minus():
    run("cat <<-EOF\n\thello\nEOF")


def test_redir_with_expansion(tmp_path):
    f = tmp_path / "out"
    run(f"X={f}; echo hi > $X")

def test_redir_revert_stdout(tmp_path):
    f = tmp_path / "out"
    run(f"echo hi > {f}")
    rc, out, err = run("echo ok")
    assert "ok" in out


def test_redir_right_overwrite(tmp_path):
    f = tmp_path / "f"
    f.write_text("old")
    run(f"echo hi >| {f}")

def test_redir_right_append(tmp_path):
    f = tmp_path / "f"
    run(f"echo first > {f}")
    run(f"echo second >> {f}")

def test_redir_left_fd_valid():
    run("cat 0<&0")

def test_redir_right_fd_valid():
    run("echo hi 1>&1")

def test_redir_empty_expansion(tmp_path):
    f = tmp_path / "out"
    run(f"X=''; echo hi > {f}")

def test_redir_revert_multiple(tmp_path):
    f = tmp_path / "out"
    run(f"echo hi > {f}")
    rc, out, err = run("echo ok")
    assert out.strip() == "ok"
