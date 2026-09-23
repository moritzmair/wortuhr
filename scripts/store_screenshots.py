#!/usr/bin/env python3
"""Erzeugt Vorschaubilder für den Pebble-Store unter store/<plattform>/.

Für jede Variante wird eine Kopie des Projekts mit festen Einstellungen, fester
Temperatur und ohne Wetterabfrage gebaut, im Emulator gestartet und abfotografiert.

    python3 scripts/store_screenshots.py [plattform ...]
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLATFORMS = ["basalt", "diorite", "emery"]

# name, Uhrzeit, Einstellungen (C-Ausdrücke)
VARIANTS = [
    ("1_klassisch", "10:25:00", dict(
        background="GColorWhite",
        text="PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite)",
        highlight="GColorBlack",
        show_date="false", show_temp="false", show_es_ist="true", info_grid_style="true")),
    ("2_dunkel_datum", "18:45:00", dict(
        background="GColorBlack",
        text="PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)",
        highlight="GColorWhite",
        show_date="true", show_temp="true", show_es_ist="false", info_grid_style="true")),
    ("3_farbig", "07:15:00", dict(
        background="PBL_IF_COLOR_ELSE(GColorOxfordBlue, GColorWhite)",
        text="PBL_IF_COLOR_ELSE(GColorCobaltBlue, GColorWhite)",
        highlight="PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorBlack)",
        show_date="true", show_temp="true", show_es_ist="true", info_grid_style="true")),
    ("4_textzeile", "21:50:00", dict(
        background="PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)",
        text="PBL_IF_COLOR_ELSE(GColorMayGreen, GColorBlack)",
        highlight="GColorWhite",
        show_date="true", show_temp="true", show_es_ist="false", info_grid_style="false")),
]


def run(*cmd, cwd):
    subprocess.run(cmd, cwd=cwd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)


def patch_source(path, settings):
    src = open(path, encoding="utf-8").read()
    body = "".join(f"  s_settings.{k} = {v};\n" for k, v in settings.items())
    body += "  s_settings.fahrenheit = false;\n  s_temp = 18;\n  s_has_temp = true;\n"
    src, n = re.subn(r"(static void default_settings\(void\) \{\n).*?(\n\})",
                     lambda m: m.group(1) + body.rstrip("\n") + m.group(2), src, flags=re.S)
    assert n == 1, "default_settings nicht gefunden"
    # Gespeicherte Einstellungen und echte Wetterdaten ignorieren
    src = src.replace("  persist_read_data(SETTINGS_KEY", "  if (0) persist_read_data(SETTINGS_KEY")
    src = src.replace("  if (persist_exists(TEMPERATURE_KEY)) {", "  if (0) {")
    src = src.replace("static void request_weather(void) {\n", "static void request_weather(void) {\n  return;\n")
    open(path, "w", encoding="utf-8").write(src)


def main():
    platforms = sys.argv[1:] or PLATFORMS
    with tempfile.TemporaryDirectory() as tmp:
        builds = {}
        for name, _, settings in VARIANTS:
            proj = os.path.join(tmp, name)
            shutil.copytree(ROOT, proj, ignore=shutil.ignore_patterns("build", ".git", "store", "node_modules"))
            patch_source(os.path.join(proj, "src/c/wortuhr.c"), settings)
            run("pebble", "build", cwd=proj)
            builds[name] = proj

        for platform in platforms:
            out_dir = os.path.join(ROOT, "store", platform)
            os.makedirs(out_dir, exist_ok=True)
            for name, clock, _ in VARIANTS:
                proj = builds[name]
                run("pebble", "install", "--emulator", platform, cwd=proj)
                # Kurz vor die Zielminute stellen und auf den Minuten-Tick warten, der neu
                # zeichnet (eine Neuinstallation würde die Uhr wieder auf Systemzeit setzen).
                h, m, _ = map(int, clock.split(":"))
                before = (h * 60 + m - 1) % (24 * 60)
                run("pebble", "emu-set-time", "--emulator", platform,
                    f"{before // 60:02d}:{before % 60:02d}:57", cwd=proj)
                time.sleep(5)
                out = os.path.join(out_dir, name + ".png")
                run("pebble", "screenshot", "--emulator", platform, "--no-open", out, cwd=proj)
                print(os.path.relpath(out, ROOT))
            run("pebble", "kill", cwd=ROOT)


if __name__ == "__main__":
    main()
