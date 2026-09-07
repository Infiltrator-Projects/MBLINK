from pathlib import Path
import base64

parts = [Path(f".github/repair/pid.part{i}") for i in range(5)]
raw = "".join(part.read_text() for part in parts)
clean = "".join(raw.split())
allowed = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=")
bad = sorted(set(clean) - allowed)
if bad:
    raise SystemExit(f"repair payload contains invalid Base64 characters: {bad!r}")

# The payload was split as text, not as independently padded Base64 objects.
# Reconstruct the single original stream and restore only its final padding.
clean = clean.rstrip("=")
clean += "=" * ((4 - (len(clean) % 4)) % 4)
try:
    decoded = base64.b64decode(clean, validate=True)
except Exception as exc:
    raise SystemExit(f"repair payload is not valid Base64 after reconstruction: {exc}") from exc

try:
    source = decoded.decode("utf-8")
except UnicodeDecodeError as exc:
    raise SystemExit(f"repair payload is not UTF-8 Python: {exc}") from exc

compile(source, "/tmp/repair.py", "exec")
Path("/tmp/repair.py").write_text(source)
print(f"Recovered PID repair script: {len(decoded)} bytes")
