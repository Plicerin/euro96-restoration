# Retail rebound-goal commentary — 0.68.0

Update in 0.69.0: the two live pass categories described below are now resolved through verified shared-bank and silent-branch routing. [Current pass-audio evidence](RETAIL-PASS-AUDIO.md). The recording imports and comparisons in this historical report remain unchanged.

Goals scored after a rebound now use all eight original Euro 96 POS recordings. The source event remains GL_POST (27); only its imported WAV bank changes from the Actua GLF bank to the verified Euro POS bank. A post impact without a goal continues to use its separate existing event. Gameplay, event priority, goal classification and random-number consumption are unchanged.

## Caller evidence

In the pinned retail EURO96.EXE, both scoring branches test the same rebound flag at file offsets `0xb7bf7` and `0xb8050`. Their nonzero paths select event 22 at `0xb7c12` and `0xb806b`, then jump to the commentary calls at `0xb7cac` and `0xb80e3`. Retail descriptor 22 at the table beginning at `0x1424b0` names POS and specifies eight takes. The preserved source selects GL_POST in the corresponding rebound-goal branches. This mapping is established by the callers and descriptor, not the name of the audio bank.

The importer checks the executable hash, both sets of instruction bytes, descriptor and independently inventoried MAD hash before importing the eight WAVs. It permits replacement of only the known previous overlay or identical new output; other existing overlays are protected. [Instruction and clip evidence](euro96-audio-import.json).

Comparing the previous published overlay with this one confirms that exactly eight records changed, all category 27. The other 919 records are byte-identical and the total remains 927. Other imported media and preserved SOURCE remain unchanged. [Import comparison](rebound-audio-import-delta.json).

## Corrected remaining-audio inventory

The live source-only pass categories are CP_SHORT and CP_SETPIECE. FP_SHORT and FP_LOOSE appear in a commented-out branch of `SOURCE/BALLINT.CPP::has_ball`; stripping comments and literals from the source and generated gameplay code finds no direct symbolic calls to either. Their compatibility recordings remain available through the audio API. This is a caller audit, not a claim that these categories were absent from every retail revision.

Missing/ambiguous half-time score recordings still need verified identities. Eight duplicated HLF filenames contain two distinct recordings of unresolved score identity. The supplied disc also lacks matching Euro French audio. Neither gap is filled by guessing.

## Validation

The independent provenance test compares every imported clip with the disc inventory. Native tests compare complete PCM (apart from the existing short edge fades) for all eight rebound recordings, exercise both actual goal-crossing branches, verify single dispatch and unchanged gameplay RNG, and retain coverage for score variants, old-media fallback and corrupt-pack rejection. The existing goal-collision and commentary tests cover post impacts and goal-classification precedence. These are dummy-device checks; physical audio-output testing remains separate.

Build, full-suite, package and launcher results are recorded in [release status](RELEASE-CANDIDATE.md).

The first full regression run encountered resource failures with only about 55 MB free on C:. Three disposable package-audit copies were removed (1,809,273,690 bytes); their logs, original release packages, published builds and saves were retained. The seven failed checks were rerun serially without code changes. Both the initial log and the recheck log are retained, rather than presenting the initial run as clean.
