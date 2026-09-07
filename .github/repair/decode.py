from pathlib import Path
import base64

parts = [Path(f".github/repair/pid.part{i}") for i in range(5)]
allowed = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=")
decoded_parts = []

for part in parts:
    clean = "".join(part.read_text().split())
    bad = sorted(set(clean) - allowed)
    if bad:
        raise SystemExit(f"{part}: invalid Base64 characters: {bad!r}")
    clean = clean.rstrip("=")
    clean += "=" * ((4 - (len(clean) % 4)) % 4)
    try:
        decoded_parts.append(base64.b64decode(clean, validate=True))
    except Exception as exc:
        raise SystemExit(f"{part}: invalid Base64 after padding restoration: {exc}") from exc

decoded = b"".join(decoded_parts)
try:
    source = decoded.decode("utf-8")
except UnicodeDecodeError as exc:
    offset = 0
    for index, chunk in enumerate(decoded_parts):
        if offset <= exc.start < offset + len(chunk):
            local = exc.start - offset
            lo = max(0, local - 120)
            hi = min(len(chunk), local + 120)
            window = chunk[lo:hi]
            print(f"bad UTF-8 in part{index}: global={exc.start} local={local} chunk_len={len(chunk)}")
            print(f"window_hex={window.hex()}")
            print("window_text=" + window.decode("utf-8", errors="replace"))
            break
        offset += len(chunk)
    raise SystemExit(f"reconstructed repair is not UTF-8 Python: {exc}") from exc

compile(source, "/tmp/repair.py", "exec")
Path("/tmp/repair.py").write_text(source)
print(f"Recovered PID repair script: {len(decoded)} bytes")
