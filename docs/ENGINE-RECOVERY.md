# Ball engine recovery, checkpoint 0.2

## Evidence recovered

`tools/omf.py` reads bounded OMF records: names, segments, publics, externs, enumerated data,
line references, and the fixups needed for this snapshot. It is a restoration tool, not a general linker.
Unsupported fixup data forms are rejected. The format reference is the
[TIS OMF 1.1 specification](https://openwatcom.org/ftp/devel/docs/omf.pdf), especially PUBDEF, SEGDEF, LEDATA and FIXUPP.

`tools/restore_ball.py` verifies the SHA-256 of BALL.OBJ and FOOTBALL.OBJ before interpreting
known instructions. It extracts complete original function bodies into generated C++ and records
source-body hashes, scalar types, values, storage segments, and offsets in `engine-recovery.json`.
The inventory contains 440 scalar globals. BSS zeroes are explicitly distinguished from initialized data;
runtime initialization still has to be restored. Arrays and missing structure layouts are not reconstructed by this inventory.

Selected evidence:

| Item | Recovered value | Evidence |
| --- | --- | --- |
| Pitch | 1280 x 800 | FOOTBALL.OBJ public initialized scalars |
| Ball diameter | 4 | FOOTBALL.OBJ `ball_diam` |
| Bounce loss | approximately 2.2 | FOOTBALL.OBJ `bounce_dis` float |
| Gravity | stored 0.5; match uses 0.6 | FOOTBALL.OBJ and FOOTBALL.CPP line 3454 |
| Air friction | approximately 0.986 | BALL.OBJ instruction immediate at _TEXT+0xa07 |
| Ground friction | approximately 0.965 | BALL.OBJ instruction immediate at _TEXT+0xa10 |
| Wind clamp | 1 | BALL.OBJ _TEXT+0xa54 |
| Spin hold factor | 4.0 | BALL.OBJ CONST+0x44 |
| In-air visual spin | -0.2 | BALL.OBJ CONST+0x84 |
| Pi | 3.1415926536 | BALL.OBJ CONST+0x8 |
| Zone spacing | 182 x 266 | FOOTBALL.OBJ startup code divides pitch_len by 7 and pitch_wid by 3 as integers |
| `game_action` | short | BALL.OBJ and FOOTBALL.OBJ symbol types; the surviving EXTERNS.H incorrectly declares char |
| User input slots | 20 | DEFINES.H MAX_USERS and user_vectors in EXTERNS.H |
| Simulation rate | 20 Hz | DEFINES.H REAL_SPEED |

The native adapter explicitly applies runtime centre-ball initialization and the match gravity override.
It leaves every extracted original routine body unchanged, adapting only function scope to `BallEngine`.
Each engine instance owns its state, enabling independent deterministic scenarios.

## Code comparison

`tools/compare_original_ball.py` maps the preserved BALL.OBJ, MATHS.OBJ and FOOTBALL.OBJ data
into Unicorn's x86 interpreter and resolves their original OMF references. It executes the original
ball trajectory code with original global types and test inputs. Watcom sin/cos/sqrt imports use
x87 fsin/fcos/fsqrt shims; any unexpected executed import fails. This is not a full DOS game execution
or a comparison against the original Watcom math-library implementation.

The comparison uses prediction mode to exclude audio and display orientation. Five 80-step scenarios
cover ground friction, a loft with bounces, wind, swerve, and aftertouch. Native state agrees within
0.002 original units; maximum measured error is about 0.000123. The JSON report includes input and executable hashes.

A sixth, horizontal-only aftertouch case exposes a precision boundary. At step 47 the native ball
is just below the floor while the emulated old code is about 0.0000024 units above it. The strict
original collision comparison therefore bounces one step earlier natively, causing larger later
differences (up to about 8.6 in the compared fields). This case remains recorded as a known difference.
No epsilon patch is inserted into the preserved source, and no bit-exact compatibility is claimed.

## Verification and limits

- Native x64 Release compilation of 18 original physics/math routine bodies.
- 72,185 assertions over physical invariants, exact launch/bounce cases, zone boundaries, independent
  engine instances, pause/reset, fixed-rate stepping, and bounded diagnostic shot combinations.
- Six restoration-tool unit tests with malformed and synthetic object records.
- Six CTest suites including the existing asset tests and SDL input/render smoke checks.
- A genuine native Windows render captured in `physics.png` and visually inspected.
- The direct launch/power/spin presets are new diagnostics, not original player kick behavior.
- Original sound calls are surfaced as counted events, not silent claims of restored audio.
- Boundaries stop the sandbox. Goals, posts/net collisions, players, actions, AI, ball interaction,
  refereeing, restarts, match timing, and the original 3D renderer remain to be connected.
- Physical controller behavior remains unverified.

## Next restoration target

Recover the newer match_player and supporting state layouts, then action/intent constants and the
missing ball animation data. ANDYDEFS.H contains a newer player layout than EXTERNS.H; using the old
structure blindly would be incorrect. Compile original player movement and ball interaction against
those recovered definitions before connecting the complete team AI and match rules.
