"""Fast checks suitable for hosted CI without an Unreal installation."""
import json
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
tracked = subprocess.check_output(["git", "ls-files", "-z"], cwd=root).decode().split("\0")
errors = []
generated = {"Binaries", "Intermediate", "Saved", "DerivedDataCache", ".vs"}
for name in filter(None, tracked):
    path = pathlib.PurePosixPath(name)
    if generated.intersection(path.parts):
        errors.append(f"Generated file tracked: {name}")
    if path.suffix.lower() in {".ini", ".json", ".uproject"}:
        text = (root / name).read_text(encoding="utf-8-sig")
        if re.search(r"(?im)^[ \t]*(SecurityToken|Password|ClientSecret|ApiKey)[ \t]*=[ \t]*[^\s]", text):
            errors.append(f"Nonempty credential setting: {name}")

project_dir = root / "PLAYSEED"
project = json.loads((project_dir / "PLAYSEED.uproject").read_text(encoding="utf-8-sig"))
if not any(m["Name"] == "PLAYSEED" and m["Type"] == "Runtime" for m in project["Modules"]):
    errors.append("PLAYSEED runtime module is missing")
engine_config = (project_dir / "Config/DefaultEngine.ini").read_text(encoding="utf-8-sig")
match = re.search(r"(?m)^GameDefaultMap=(/Game/[^\r\n]+)", engine_config)
if not match or not (project_dir / "Content" / (match[1][6:] + ".umap")).is_file():
    errors.append("Configured game map does not exist")
for material in ("M_Palette", "M_Sky"):
    if not (project_dir / f"Content/Windseed/{material}.uasset").is_file():
        errors.append(f"Original material is missing: {material}")
for name in ("create_shared_assets.py", "check_repository.py"):
    compile((root / "Tools" / name).read_text(encoding="utf-8"), name, "exec")
if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print("PASS: project descriptor, map, palette, Python syntax, generated-file and credential checks")
