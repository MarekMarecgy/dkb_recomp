# SPDX-License-Identifier: GPL-3.0-or-later
"""Donkey Kong Bananza, Switch 2 update 458752. No game-specific patches or hooks yet."""
from nxrecomp.config import GameConfig, ModuleSpec

BUILDS = {'E25C90C1C136638885659FD55BF8A03B3403489D': '458752'}
MODULE_BUILD_IDS = {
    'main': 'e25c90c1c136638885659fd55bf8a03b3403489d',
    'sdk': '18265daea2105b31f25346adfbbd09b5edf1207f',
    'rtld': 'cc38c02463e7dd1136ccf03f2a31c39dbe0758ed',
}


class Config(GameConfig):
    name = 'dkb'
    title_id = '040074A01BF12000'
    version = '458752'
    build_id = next(iter(BUILDS))
    modules = [
        ModuleSpec('main', 0x7100000000, 'main'),
        ModuleSpec('sdk', 0x7500000000, 'sdk'),
        ModuleSpec('rtld', 0x7700000000, 'rtld'),
    ]
    extra_entries = {'rtld': [0]}
