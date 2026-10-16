# Euro 96 media inspection - 2026-10-13

**Correction:** subsequent inspection followed the retail UV table to record 38 and recovered both complete 1-20 shirt-number sets. The initial 1-15 finding below applies only to the older record-107 atlas. [Discovery and decoded image](EURO96-NUMBER-DISCOVERY.md).

The supplied `UEFA_Euro_96_England_1996.zip` contains a PC/DOS Euro 96 installation and a CloneCD image. The image volume is `EURO96`; READ.ME specifies MS-DOS 5.0+, a 486DX2/66 and 8 MB RAM. The installed executable, graphics archives, frontend archives and English script match their counterparts on the image byte for byte. This establishes the correct game/platform, not identical revision compatibility with the preserved source.

The single raw MODE1/2352 track was converted to a separate 2048-byte-sector ISO after validating every sector's sync and mode. Files were read through 7-Zip. No installer, game executable or bundled batch file was executed. Staging is in `work/euro96-inspection` outside SOURCE and active game data. [Input hashes and findings](euro96-media-inspection.json).

## Useful additions

- The retail graphics archive has 287 records versus 229 in the preserved archive. One hundred preserved records occur byte for byte in the new archive. Layouts differ: the existing Actua Soccer kit importer does not match this executable's descriptors and must be adapted; a wholesale archive swap is unsuitable.
- English speech banks exist for all 16 countries, including `E_CRO.MAD` and `E_TRK.MAD`. The audio inspector validated 78 files and every original pitch-effect name. [Audio inventory and individual clip hashes](euro96-cd-audio-inspection.json).
- The compressed English script was decoded using the token operations in `SOURCE/UNMANGLE.C`, with input/output bounds and full input consumption checked. It describes 320 main-roster players, their appearance categories, animation references and speech identifiers, plus extra reserve data. There are 256 direct normalized-name matches to the current 320-player roster, each with all three referenced English voice recordings. Spelling changes and actual squad changes require explicit mapping rather than assigning names by slot. [Coverage and unresolved references](euro96-roster-coverage.json).
- Country-specific player/keeper preview animations and home/away-labelled GDV files include Croatia and Turkey. Subsequent sample decoding of the HOUTS/AOUTS videos showed trophy sequences; these filenames alone do not establish kit previews. Eight stadium-preview FAP files and Euro96 music are present. These are inventory findings, not claims that all previews have been decoded or that eight playable stadiums have been imported.

## Gaps this does not automatically close

Retail graphics record 107 is byte-identical to preserved record 109: the shirt-number atlas still contains 1-15 only. [Decoded atlas](euro96-retail-numbers.png). Do not claim 16-20 artwork was recovered.

This image contains English team speech, without French team banks. Seven main-roster speech references in its own script are placeholders or point to absent clips; 313 of 320 retail players have all three referenced recordings. These references should not be silently replaced with unrelated names.

The native release remains 0.53.0. SOURCE and active imported media were not changed, and no running game was restarted. The next implementation batch should add a revision-aware Euro96 importer, verify actual in-match kit/appearance mappings, and integrate the validated English speech alongside the existing media. Original source rosters and saved fixtures should not be changed implicitly.

Implementation follow-up: [0.54.0 outfield kits, appearances and names](EURO96-IMPORT.md). The unchanged-game statement above describes the initial inspection only.
