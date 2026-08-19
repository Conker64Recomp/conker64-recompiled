import hashlib
import os

filename = "Conker's Bad Fur Day (USA).n64"
if not os.path.exists(filename):
    print("File not found")
    exit(1)

with open(filename, "rb") as f:
    data = f.read()

magic = data[:4].hex()
print(f"Size: {len(data)} bytes")
print(f"Header magic: {magic}")
print(f"Current SHA1: {hashlib.sha1(data).hexdigest().upper()}")

# N64 formats:
# .z64 (big-endian): 80 37 12 40
# .v64 (byte-swapped): 37 80 40 12
# .n64 (little-endian): 40 12 37 80

converted = bytearray(data)
if magic == "80371240":
    print("ROM is already native Big-Endian (.z64)")
elif magic == "37804012":
    print("ROM is Byte-Swapped (.v64) -> Converting to .z64...")
    for i in range(0, len(converted), 2):
        converted[i], converted[i+1] = converted[i+1], converted[i]
elif magic == "40123780":
    print("ROM is Little-Endian (.n64) -> Converting to .z64...")
    for i in range(0, len(converted), 4):
        converted[i], converted[i+1], converted[i+2], converted[i+3] = converted[i+3], converted[i+2], converted[i+1], converted[i]

with open("baserom.us.z64", "wb") as f:
    f.write(converted)

z64_sha1 = hashlib.sha1(converted).hexdigest().upper()
print(f"Generated baserom.us.z64 with SHA1: {z64_sha1}")
expected = "4CBADD3C4E0729DEC46AF64AD018050EADA4F47A"
if z64_sha1 == expected:
    print(">> EXCELLENT: ROM matches the official Conker US release 100%!")
else:
    print(f">> WARNING: Expected {expected} but got {z64_sha1}")
