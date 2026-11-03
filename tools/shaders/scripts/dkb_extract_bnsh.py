#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Extracts the NVN2 shader programs of Donkey Kong Bananza's BNSH containers into the harness layout
# (manifest.tsv, code_<hash>.bin, ctrl_<hash>.bin; see tools/shaders/README.md "Use").
#
# BNSH layout as used by nxnative's dm/gfx/tools/shader_split.py (MIT-compatible notes, read, not copied):
#   BNSH+0x7c u32 variation count, +0x80 u64 variation table; variation = 0x40 bytes, u64 program pointer at +0x10;
#   program: six u64 stage descriptor pointers from +0x08 (vertex, hull, domain, geometry, pixel, compute), 0 = none;
#   descriptor: u64 control pointer +0x08, u64 code pointer +0x10, u32 code size +0x18, u32 control size +0x1c.
#   All pointers are relative to the BNSH start. NVN2 control sections start with 0x98761234 (tools/shaders/docs/AMPERE.md section 3).
# BAHS (agl sharcb, version 11 here): u32 +0x18 code region length, +0x1c code region start; 16-byte table entries
#   {u32 code offset in the region, 0, u32 control length, i32 offset from the entry to the control section}; the code area length is
#   control +0x44 and the stage control u16 +0x56 (0 vertex, 1 fragment, 2 geometry, 3 tess control, 4 tess eval, 5 compute).
# The hash is the first 16 hex digits of the SHA-1 of the whole code area, the runtime's file name for a translated shader.
# Usage: python -I dkb_extract_bnsh.py <out dir> <container or RomFS folder> [...]   (Python 3.14: zstd files are read through compression.zstd)
import hashlib, os, struct, sys

ZSTD_MAGIC = bytes.fromhex("28b52ffd")
BAHS_V11 = b"BAHS" + struct.pack("<I", 11)
STAGES = ["vertex", "hull", "domain", "geometry", "fragment", "compute"]


def bnsh_offsets(d):
    pos = 0
    while True:
        pos = d.find(b"BNSH\0\0\0\0", pos)
        if pos < 0:
            return
        yield pos
        pos += 8


def extract(d, found, stats, source):
    for b in bnsh_offsets(d):
        try:
            count = struct.unpack_from("<I", d, b + 0x7C)[0]
            table = struct.unpack_from("<Q", d, b + 0x80)[0]
        except struct.error:
            continue
        if count == 0 or count > 1 << 20 or b + table + count * 0x40 > len(d):
            stats["bad_bnsh"] += 1
            continue
        stats["bnsh"] += 1
        for i in range(count):
            prog = struct.unpack_from("<Q", d, b + table + i * 0x40 + 0x10)[0]
            if prog == 0 or b + prog + 0x38 > len(d):
                stats["no_program"] += 1
                continue
            stats["variations"] += 1
            for s in range(6):
                desc = struct.unpack_from("<Q", d, b + prog + 8 + 8 * s)[0]
                if desc == 0:
                    continue
                if b + desc + 0x20 > len(d):
                    stats["bad_desc"] += 1
                    continue
                cptr, kptr = struct.unpack_from("<QQ", d, b + desc + 8)
                ksize, csize = struct.unpack_from("<II", d, b + desc + 0x18)
                ok = (ksize and csize and b + cptr + csize <= len(d) and b + kptr + ksize <= len(d)
                      and struct.unpack_from("<I", d, b + cptr)[0] == 0x98761234)
                if not ok:
                    stats["bad_stage"] += 1
                    continue
                code = d[b + kptr:b + kptr + ksize]
                ctrl = d[b + cptr:b + cptr + csize]
                h = hashlib.sha1(code).hexdigest()[:16]
                stats["stages"] += 1
                if h in found:
                    continue
                found[h] = (STAGES[s], code, ctrl, source)


def extract_bahs(d, found, stats, source):
    region_len, region_off = struct.unpack_from("<II", d, 0x18)
    names = ["vertex", "fragment", "geometry", "hull", "domain", "compute"]
    for e in range(0x20, min(region_off, len(d)) - 16, 4):
        code_off, zero, clen, rel = struct.unpack_from("<IIIi", d, e)
        c = e + rel
        if zero or clen < 0xD8 or clen > 0x1000 or c < 0 or c + clen > len(d) or struct.unpack_from("<I", d, c)[0] != 0x98761234:
            continue
        ksize = struct.unpack_from("<I", d, c + 0x44)[0]
        k = region_off + code_off
        if ksize == 0 or code_off + ksize > region_len or k + ksize > len(d):
            stats["bad_bahs_entry"] = stats.get("bad_bahs_entry", 0) + 1
            continue
        stage = struct.unpack_from("<H", d, c + 0x56)[0]
        code, ctrl = d[k:k + ksize], d[c:c + clen]
        h = hashlib.sha1(code).hexdigest()[:16]
        stats["bahs_stages"] = stats.get("bahs_stages", 0) + 1
        if h not in found:
            found[h] = (names[stage] if stage < 6 else "stage%d" % stage, code, ctrl, source)


def scan_container(d, found, stats, source):
    """BNSH programs plus any BAHS (agl sharcb) blocks embedded in a decompressed file (SARC members are stored uncompressed)."""
    extract(d, found, stats, source)
    pos = 0
    while True:
        pos = d.find(BAHS_V11, pos)
        if pos < 0:
            return
        size = struct.unpack_from("<I", d, pos + 8)[0]
        if 0x40 < size <= len(d) - pos:
            extract_bahs(d[pos:pos + size], found, stats, source)
        pos += 8


def walk_romfs(root, found, stats):
    """Every file under a RomFS folder (zstd files decompressed first) that holds an NVN2 control section."""
    from compression import zstd
    magic = struct.pack("<I", 0x98761234)
    for dirpath, _, names in os.walk(root):
        for n in names:
            path = os.path.join(dirpath, n)
            with open(path, "rb") as f:
                data = f.read()
            if data[:4] == ZSTD_MAGIC:
                data = zstd.decompress(data)
            if magic in data:
                stats["files"] = stats.get("files", 0) + 1
                scan_container(data, found, stats, os.path.relpath(path, root))


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    found, stats = {}, {k: 0 for k in ("bnsh", "bad_bnsh", "variations", "no_program", "bad_desc", "bad_stage", "stages")}
    for path in sys.argv[2:]:
        if os.path.isdir(path):
            walk_romfs(path, found, stats)
            continue
        with open(path, "rb") as f:
            data = f.read()
        if data[:4] == b"BAHS":
            extract_bahs(data, found, stats, os.path.basename(path))
        else:
            scan_container(data, found, stats, os.path.basename(path))
    rows = ["id\tstage\tcode_off\tcode_size\tctrl_off\tctrl_size\tprogram_start\tprogram_bytes\tconst_off\tconst_size\tcode_sha1_16\tcode_file\tctrl_file"]
    for h, (stage, code, ctrl, src) in sorted(found.items()):
        ctrl_prog = struct.unpack_from("<I", ctrl, 0x38)[0]
        const_size = struct.unpack_from("<I", ctrl, 0x3C)[0]
        const_off = struct.unpack_from("<I", ctrl, 0x40)[0]
        with open(os.path.join(out, f"code_{h}.bin"), "wb") as f:
            f.write(code)
        with open(os.path.join(out, f"ctrl_{h}.bin"), "wb") as f:
            f.write(ctrl)
        rows.append("\t".join([f"bnsh_{h}", stage, "0x0", hex(len(code)), "0x0", hex(len(ctrl)), "0x0", hex(ctrl_prog),
                               hex(const_off), hex(const_size), h, f"code_{h}.bin", f"ctrl_{h}.bin"]))
    with open(os.path.join(out, "manifest.tsv"), "w") as f:
        f.write("\n".join(rows) + "\n")
    print(stats, "distinct stage programs:", len(found))


if __name__ == "__main__":
    main()
