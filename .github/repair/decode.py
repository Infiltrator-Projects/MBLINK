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

# Parts 0-3 are complete Base64 blocks. The final block is exactly one
# character short of a valid quartet. Recover that one character by requiring
# the complete decoded payload to be both UTF-8 and syntactically valid Python.
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
        candidates.append((position, char, source, len(payload)))

if len(candidates) != 1:
    summary = [(position, char, size) for position, char, _source, size in candidates[:20]]
    raise SystemExit(f"expected one recoverable PID payload, found {len(candidates)} candidates: {summary}")

position, char, source, payload_size = candidates[0]
Path("/tmp/repair.py").write_text(source)
print(f"Recovered PID repair script: {payload_size} bytes; inserted {char!r} at part4 offset {position}")
