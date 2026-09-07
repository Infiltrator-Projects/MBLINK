from pathlib import Path
import base64

alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
parts = [Path(f".github/repair/pid.part{i}") for i in range(5)]
allowed = set(alphabet + "=")
raw_parts = []

for part in parts:
    clean = "".join(part.read_text().split())
    bad = sorted(set(clean) - allowed)
    if bad:
        raise SystemExit(f"{part}: invalid Base64 characters: {bad!r}")
    raw_parts.append(clean)

prefix_chunks = []
for index, clean in enumerate(raw_parts[:4]):
    if len(clean) % 4 != 0:
        raise SystemExit(f"pid.part{index}: unexpected Base64 length {len(clean)}")
    prefix_chunks.append(base64.b64decode(clean, validate=True))
prefix = b"".join(prefix_chunks)

tail = raw_parts[4]
if len(tail) % 4 != 3:
    raise SystemExit(f"pid.part4: expected one missing Base64 character, length={len(tail)}")
first_padding = tail.find("=")
insert_limit = len(tail) if first_padding < 0 else first_padding
candidates = []

required_tail = '''for path in [
    "platform/apple/MBLinkDiagnosticsController.h",
    "platform/apple/MBLinkDiagnosticsController.m",
    "app/ios/MBLINK/ConnectionViewModel.swift",
    "app/ios/MBLINK/MBLINKApp.swift",
]:
'''

for position in range(insert_limit + 1):
    for char in alphabet:
        candidate_b64 = tail[:position] + char + tail[position:]
        if len(candidate_b64) % 4 != 0:
            continue
        try:
            candidate_tail = base64.b64decode(candidate_b64, validate=True)
        except Exception:
            continue
        payload = prefix + candidate_tail
        try:
            source = payload.decode("utf-8")
            compile(source, "/tmp/repair.py", "exec")
        except (UnicodeDecodeError, SyntaxError):
            continue
        if required_tail not in source:
            continue
        if 'print("PID architecture repair applied")' not in source:
            continue
        candidates.append((position, char, source, len(payload)))

if len(candidates) != 1:
    summary = [(position, char, size) for position, char, _source, size in candidates[:20]]
    raise SystemExit(f"expected one structurally valid PID payload, found {len(candidates)} candidates: {summary}")

position, char, source, payload_size = candidates[0]
Path("/tmp/repair.py").write_text(source)
print(f"Recovered PID repair script: {payload_size} bytes; inserted {char!r} at part4 offset {position}")
