# Supported romsets

Romsets are identified by both a family and version. The family corresponds to
a specific line of hardware, and the version identifies a specific revision of
that hardware.

The emulator accepts names in either of these two formats: `family-version` or
`family`. If only `family` is present and there are one or more complete
romsets for that family in the rom directory, one of those romsets will be
randomly selected. If you want to load a specific romset version, you must
provide both fields.

Supported family names and the hardware they correspond to:

- `mk2`: SC-55mk2
- `st`: SC-55st
- `mk1`: SC-55mk1
- `cm300`: CM-300/SCC-1
- `jv880`: JV-880
- `scb55`: SCB-55
- `rlp3237`: RLP-3237
- `sc155`: SC-155
- `sc155mk2`: SC-155mk2

Supported romset names and the SHA256 hashes of each rom in the set:

```
mk2-v1.01

8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042
a4c9fd821059054c7e7681d61f49ce6f42ed2fe407a7ec1ba0dfdc9722582ce0
b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8
c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

sc155mk2-v1.01

8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042
a4c9fd821059054c7e7681d61f49ce6f42ed2fe407a7ec1ba0dfdc9722582ce0
b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8
c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

st-v1.01

8a1eb33c7599b746c0c50283e4349a1bb1773b5c0ec0e9661219bf6c067d2042
03517ac0a3b1ad8b69a1a4ee045e0c21da0170027bd1ba1bd3cf72cd017bbe6a
b0b5f865a403f7308b4be8d0ed3ba2ed1c22db881b8a8326769dea222f6431d8
c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

mk1-v1.00

b4ecf44bc0520322b0d114d397951d3bf92ca6fa51d0d27b2407df58a6be2efe
014e2e21ea30de7a1e4f1cdea14dd9a719960535e257a9e40e98dbb1a5870226
5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007
c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1
334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2

mk1-v1.10

2fe88ec39f3ef4b1de8cdf74527419467975c47f7aacfcd07605e01d54bd89b5
ec064d6c4fc70ec990911089d966043cb819fba0e26e6f6afdd0a05e5301b91b
5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007
c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1
334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2

mk1-v1.20

7e1bacd1d7c62ed66e465ba05597dcd60dfc13fc23de0287fdbce6cf906c6544
22ce6ca59e6332143b335525e81fab501ea6fccce4b7e2f3bfc2cc8bf6612ff6
5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007
c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1
334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2

mk1-v1.21

7e1bacd1d7c62ed66e465ba05597dcd60dfc13fc23de0287fdbce6cf906c6544
effc6132d68f7e300aaef915ccdd08aba93606c22d23e580daf9ea6617913af1
5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007
c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1
334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2

mk1-v2.00

24a65c97cdbaa847d6f59193523ce63c73394b4b693a6517ee79441f2fb8a3ee
f5dac35d450ab986570a209dff3816eec75cee669e161f54b51224b467dd0bcc
5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007
c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1
334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2

cm300-v1.10

72ed35481efbf25b3c492b83183655d17a3b266ecb30ffbc6dc977e6a8d261b2
0283d32e6993a0265710c4206463deb937b0c3a4819b69f471a0eca5865719f9
40c093cbfb4441a5c884e623f882a80b96b2527f9fd431e074398d206c0f073d
9bbbcac747bd6f7a2693f4ef10633db8ab626f17d3d9c47c83c3839d4dd2f613
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

cm300-v1.20

72ed35481efbf25b3c492b83183655d17a3b266ecb30ffbc6dc977e6a8d261b2
fef1acb1969525d66238be5e7811108919b07a4df5fbab656ad084966373483f
40c093cbfb4441a5c884e623f882a80b96b2527f9fd431e074398d206c0f073d
9bbbcac747bd6f7a2693f4ef10633db8ab626f17d3d9c47c83c3839d4dd2f613
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

cm300-v1.30

9ec66abb5231b6c6f46f48b33d5412703041037d69a6803626ac402f25552af2
f89442734fdebacae87c7707c01b2d7fdbf5940abae738987aee912d34b5882e
40c093cbfb4441a5c884e623f882a80b96b2527f9fd431e074398d206c0f073d
9bbbcac747bd6f7a2693f4ef10633db8ab626f17d3d9c47c83c3839d4dd2f613
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

jv880-v1.0.0

aabfcf883b29060198566440205f2fae1ce689043ea0fc7074842aaa4fd4823e
11852e60ff597633c754c5441c1e3e06793bcd951fcea2c4969ac3041d130fce
aa3101a76d57992246efeda282a2cb0c0f8fdb441c2eed2aa0b0fad4d81f3ad4
a7b50bb47734ee9117fa16df1f257990a9a1a0b5ed420337ae4310eb80df75c8

jv880-v1.0.1

aabfcf883b29060198566440205f2fae1ce689043ea0fc7074842aaa4fd4823e
ed437f1bc75cc558f174707bcfeb45d5e03483efd9bfd0a382ca57c0edb2a40c
aa3101a76d57992246efeda282a2cb0c0f8fdb441c2eed2aa0b0fad4d81f3ad4
a7b50bb47734ee9117fa16df1f257990a9a1a0b5ed420337ae4310eb80df75c8

scb55-v2.00

00df835d3f97fc8b0059db63f36d608eec2bfd1f51ad54eb5af52c868c1111b1
541be4d0b1ef0d07bb042ba67ffd099c8a5d746aac4cd24ce8842c034379f213
c6429e21b9b3a02fbd68ef0b2053668433bee0bccd537a71841bc70b8874243b
5b753f6cef4cfc7fcafe1430fecbb94a739b874e55356246a46abe24097ee491

rlp3237-v2.01

00df835d3f97fc8b0059db63f36d608eec2bfd1f51ad54eb5af52c868c1111b1
e0a3d6d9b05e82374a0d289901273ce560ce1ead86459c75f844158b32d204a9
dae2a8bc0fd3bcaf3f5e3ab6c4c6fd30e2663bf26ca17afe52924874c0afc4e2

sc155-rev1

24a65c97cdbaa847d6f59193523ce63c73394b4b693a6517ee79441f2fb8a3ee
ceb7b9d3d9d264efe5dc3ba992b94f3be35eb6d0451abc574b6f6b5dc3db237b
5655509a531804f97ea2d7ef05b8fec20ebf46216b389a84c44169257a4d2007
c655b159792d999b90df9e4fa782cf56411ba1eaa0bb3ac2bdaf09e1391006b1
334b2d16be3c2362210fdbec1c866ad58badeb0f84fd9bf5d0ac599baf077cc2
```

### Compatibility with upstream (`--legacy-romset-detection`)

When passing `--legacy-romset-detection` the loader will not use file hashing
to detect romsets. Instead, it will load roms using specific filenames. This
behavior matches upstream nukeykt/Nuked-SC55, but it is more difficult to set
up and offers no advantage over the new rom loader.

With this option, you must rename roms in the rom directory as follows:

```
SC-55mk2/SC-155mk2 (v1.01):
R15199858 (H8/532 mcu) -> rom1.bin
R00233567 (H8/532 extra code) -> rom2.bin
R15199880 (M37450M2 mcu) -> rom_sm.bin
R15209359 (WAVE 16M) -> waverom1.bin
R15279813 (WAVE 8M) -> waverom2.bin

SC-55st (v1.01):
R15199858 (H8/532 mcu) -> rom1.bin
R00561413 (H8/532 extra code) -> rom2_st.bin
R15199880 (M37450M2 mcu) -> rom_sm.bin
R15209359 (WAVE 16M) -> waverom1.bin
R15279813 (WAVE 8M) -> waverom2.bin

SC-55 (v1.00):
R15199748 (H8/532 mcu) -> sc55_rom1.bin
R1544925800 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v1.10):
R15199736 (H8/532 mcu) -> sc55_rom1.bin
R15209275 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v1.20):
R15199778 (H8/532 mcu) -> sc55_rom1.bin
R15209337 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v1.21):
R15199778 (H8/532 mcu) -> sc55_rom1.bin
R15209363 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

SC-55 (v2.0):
R15199799 (H8/532 mcu) -> sc55_rom1.bin
R15209387 (H8/532 extra code) -> sc55_rom2.bin
R15209276 (WAVE A) -> sc55_waverom1.bin
R15209277 (WAVE B) -> sc55_waverom2.bin
R15209281 (WAVE C) -> sc55_waverom3.bin

CM-300/SCC-1 (v1.10):
R15199774 (H8/532 mcu) -> cm300_rom1.bin
R15279809 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

CM-300/SCC-1 (v1.20):
R15199774 (H8/532 mcu) -> cm300_rom1.bin
R15279812 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

SCC-1A:
R00128523 (H8/532 mcu) -> cm300_rom1.bin
R00128567 (H8/532 extra code) -> cm300_rom2.bin
R15279806 (WAVE A) -> cm300_waverom1.bin
R15279807 (WAVE B) -> cm300_waverom2.bin
R15279808 (WAVE C) -> cm300_waverom3.bin

JV-880 (v1.0.0):
R15199810 (H8/532 mcu) -> jv880_rom1.bin
R15209386 (H8/532 extra code) -> jv880_rom2.bin
R15209312 (WAVE A) -> jv880_waverom1.bin
R15209313 (WAVE B) -> jv880_waverom2.bin
PCM Cards -> jv880_waverom_pcmcard.bin (optional)
Expansion PCBs -> jv880_waverom_expansion.bin (optional)

SCB-55/RLP-3194:
R15199827 (H8/532 mcu) -> scb55_rom1.bin
R15279828 (H8/532 extra code) -> scb55_rom2.bin
R15209359 (WAVE 16M) -> scb55_waverom1.bin
R15279813 (WAVE 8M) -> scb55_waverom2.bin

RLP-3237:
R15199827 (H8/532 mcu) -> rlp3237_rom1.bin
R15209486 (H8/532 extra code) -> rlp3237_rom2.bin
R15279824 (WAVE 16M) -> rlp3237_waverom1.bin

SC-155 (rev 1):
R15199799 (H8/532 mcu) -> sc155_rom1.bin
R15209361 (H8/532 extra code) -> sc155_rom2.bin
R15209276 (WAVE A) -> sc155_waverom1.bin
R15209277 (WAVE B) -> sc155_waverom2.bin
R15209281 (WAVE C) -> sc155_waverom3.bin

SC-155 (rev 2):
R15199799 (H8/532 mcu) -> sc155_rom1.bin
R15209400 (H8/532 extra code) -> sc155_rom2.bin
R15209276 (WAVE A) -> sc155_waverom1.bin
R15209277 (WAVE B) -> sc155_waverom2.bin
R15209281 (WAVE C) -> sc155_waverom3.bin
```
