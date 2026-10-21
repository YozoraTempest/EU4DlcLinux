"""Exercise the standalone plugin in a game path containing spaces."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

plugin, original, fixture, ini = (Path(arg).resolve() for arg in sys.argv[1:])
with tempfile.TemporaryDirectory(prefix="eu4-plugin-") as temporary:
    game = Path(temporary) / "game with spaces"
    data = game / "plugin/creamlinux"
    data.mkdir(parents=True)
    shutil.copyfile(plugin, game / "plugin/lib64Creamlinux.so")
    shutil.copyfile(original, game / "libsteam_api.so")
    shutil.copyfile(fixture, game / "eu4")
    (game / "eu4").chmod(0o755)
    shutil.copyfile(ini, data / "cream_api.ini")
    environment = os.environ.copy()
    for key in ("CREAM_CONFIG_PATH", "CREAM_STEAM_API", "LD_PRELOAD"):
        environment.pop(key, None)
    # A space-free relative preload matches Augustus-Launcher's game entry.
    environment["LD_PRELOAD"] = "./plugin/lib64Creamlinux.so"
    def run(*arguments, **overrides):
        result = subprocess.run(["./eu4", *arguments], cwd=game,
            env=environment | overrides, capture_output=True, text=True, timeout=15)
        assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    run()
    override = game / "custom settings.ini"
    override.write_text("[steam]\nappid=236850\n[dlc]\n241360=Custom DLC\n")
    run("1", CREAM_CONFIG_PATH=str(override))
    run("--failure", CREAM_CONFIG_PATH=str(game / "missing.ini"))
    run("--failure", CREAM_STEAM_API=str(game / "missing API.so"),
        LD_PRELOAD="./plugin/lib64Creamlinux.so:./libsteam_api.so")
    alternate = game / "original API.so"
    (game / "libsteam_api.so").rename(alternate)
    run(CREAM_STEAM_API=str(alternate))
    run("--failure")
    alternate.rename(game / "libsteam_api.so")
    (data / "cream_api.ini").rename(game / "cream_api.ini")
    run()
print("Standalone plugin package, explicit overrides, legacy config and missing API checks passed")
