# SPDX-License-Identifier: GPL-3.0-or-later
"""Check the selected DKB modules before generating build-specific code."""
import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from nxrecomp.nso import nso_info
from config import MODULE_BUILD_IDS


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('exefs', type=Path)
    args = parser.parse_args()
    for name, expected in MODULE_BUILD_IDS.items():
        info = nso_info((args.exefs / name).read_bytes())
        if info['build_id'] != expected.ljust(64, '0'):
            raise SystemExit(f'{name}: expected DKB update 458752 build ID')
        if not all(segment['sha_ok'] for segment in info['segments']):
            raise SystemExit(f'{name}: NSO segment hash mismatch')
        print(f'{name}: build ID and segment hashes verified')


if __name__ == '__main__':
    main()
