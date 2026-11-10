"""Run native integration tests and retain plain-text + machine-readable evidence."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import unittest

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--exe', type=Path, default=root / 'build/Release/MouseOptions.exe')
parser.add_argument('--output', type=Path, default=root / 'docs/evidence')
args = parser.parse_args()
os.environ['MOUSE_OPTIONS_EXE'] = str(args.exe.resolve())
args.output.mkdir(parents=True, exist_ok=True)

class Tee:
    def __init__(self, log):
        self.log = log
    def write(self, value):
        sys.stdout.write(value)
        self.log.write(value)
    def flush(self):
        sys.stdout.flush()
        self.log.flush()

suite = unittest.defaultTestLoader.discover(str(root / 'tests'), pattern='test_*.py')
with (args.output / 'integration-tests.txt').open('w', encoding='utf-8') as log:
    result = unittest.TextTestRunner(stream=Tee(log), verbosity=2).run(suite)
report = dict(executable=str(args.exe.resolve()), executable_sha256=hashlib.sha256(args.exe.read_bytes()).hexdigest(),
              tests_run=result.testsRun, successful=result.wasSuccessful(),
              failures=[str(test) for test, _ in result.failures], errors=[str(test) for test, _ in result.errors],
              skipped=[(str(test), reason) for test, reason in result.skipped],
              note='See testing.md for system-boundary replay versus physical-hardware coverage.')
(args.output / 'integration-summary.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
sys.exit(0 if result.wasSuccessful() else 1)
