import os

def pytest_sessionfinish(session, exitstatus):
    session.exitstatus = 0

    output = os.environ.get("OUTPUT_FILE")
    if not output:
        return

    total = session.testscollected
    failed = session.testsfailed
    passed = total - failed

    percent = int((passed * 100) / total) if total else 0

    with open(output, "w") as f:
        f.write(str(percent))
