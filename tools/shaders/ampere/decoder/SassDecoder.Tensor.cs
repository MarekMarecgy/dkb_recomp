// SPDX-License-Identifier: MIT
// Warp-cooperative tensor ops of SM75+: HMMA 0x23c, MOVM 0x23a, LDSM 0x83b. Field positions: [NAK] sm70_encode.rs OpHmma / OpMovm / OpLdsm
// (MIT, Collabora): HMMA dst 16, A 24, B 32, C 64, size 75 (low) + 78 (high) = 0 m16n8k8, 1 m16n8k16, 2 m16n8k4, dst type 76 (F32), src type 82..84;
// LDSM address Ra 24 (+URb 32 when bit 91), offset 40..64, count 72..74 (0, 1, 2 = 1, 2, 4 matrices), 78..80 = 0 M8N8, 1 transposed (MT8N8);
// MOVM 78..80 = 0 (MT88). Mnemonic spelling follows nvdisasm's "HMMA.16816.F32", "LDSM.16.M88.4", "MOVM.16.MT88".
namespace Nx.Shaders.Ampere
{
    public static partial class SassDecoder
    {
        static void RegisterTensor()
        {
            Op12(0x23c, c =>
            {
                c.Op = "HMMA";
                int size = (c.B(75) ? 1 : 0) | (c.B(78) ? 2 : 0);
                c.Mod(size switch { 0 => "1688", 1 => "16816", 2 => "1684", _ => "?size" });
                c.Mod(c.B(76), "F32");
                if (c.I(82, 2) != 0) { c.Mod("?src" + c.I(82, 2)); c.Unknown = true; }
                c.Dst = 1; c.Add(c.R(16), c.R(24), c.R(32), c.R(64));
            });
            Op12(0x83b, c =>
            {
                c.Op = "LDSM"; c.Mod("16");
                int n = c.I(72, 2);
                int mode = c.I(78, 2);
                c.Mod(mode == 0 ? "M88" : mode == 1 ? "MT88" : "?mode" + mode);
                if (mode > 1 || n > 2) c.Unknown = true;
                c.Mod(n switch { 0 => "1", 1 => "2", 2 => "4", _ => "?n" });
                c.Dst = 1; c.Add(c.R(16), MemOp(c, c.I(24, 8), null, c.B(91) ? c.I(32, 6) : -1, c.S(40, 24)));
            });
            Op12(0x23a, c =>
            {
                c.Op = "MOVM"; c.Mod("16");
                c.Mod(c.I(78, 2) == 0 ? "MT88" : "?mode" + c.I(78, 2));
                if (c.I(78, 2) != 0) c.Unknown = true;
                c.Dst = 1; c.Add(c.R(16), c.R(24));
            });
        }
    }
}
