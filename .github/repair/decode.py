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
    raise SystemExit(f"reconstructed repair is not UTF-8 Python: {exc}") from exc

compile(source, "/tmp/repair.py", "exec")
Path("/tmp/repair.py").write_text(source)
print(f"Recovered PID repair script: {len(decoded)} bytes")
