# Original English commentary — 0.30.0

The supplied CD now provides 443 verified English clips from 52 banks (363 event phrases, 19 pre-match
remarks, 28 country-name intonations and 33 complete spoken-score recordings).
Original event calls feed a bounded native speech player. One batch covers phrase
mapping/opening introductions, period announcements, saves/misses, goals/restarts/fouls, and speech
priority/repetition/volume controls. Use the existing launcher on the next launch;
publishing does not restart the running game.

## Controls

F9 cycles commentary volume: 75 → 50 → 25 → off → 100 percent. F8 remains the master
sound mute. Menu and pause text show commentary volume. The setting lasts for the
current session. Crowd/pitch sounds and menu/result music are quieter while speech
plays. Pause, focus loss, setup, squad screens and replay discard speech requests;
resuming does not replay stale commentary. Half-time and result screens allow their
announcements to finish. The full-time line may overlap the original result music,
which is ducked until speech finishes.

## Verified original mappings

`tools/restore_commentary.py` pins AUDIO.OBJ to SHA-256
`3bd8d90d2cef292aeda77ef86a9aee490ddb9dfbc0f2adef1fd493f4a5da33f9`.
The 35 six-byte `CommentaryCategories` records contain three-letter bank names and
counts. Their order/counts match AUDIO.H's named event enumeration and MAX values.
LoadCommentarySamples walks 35 category records at six-byte stride and sample
records at sixteen-byte stride. This is data recovery; no DOS object code is linked.

Examples: SP_CORNER → SPA; SP_GOALKICK → SPB; GL_GENERIC → GLA; PM_CATCH → PMA;
PM_BLOCK → PMB; PM_WIDE → PMC; PM_CLOSE → PMD; PM_POST → PME; PM_CROSSBAR → PMF.
PlayCommentaryMessage's explicit branches select TIM10 for TI_HALFTIME (42) and
TIM03 for TI_FULLTIME (43). The pinned instructions and full map are recorded in
`commentary-recovery.json`.

`tools/import_cd_commentary.py` reads only the English phrase banks through 7-Zip,
without mounting the disc or executing its installer. Every WAV must be unsigned
8-bit mono PCM at 11,025 Hz. It accepts only numbered clips within the source
category's range and excludes both copies of ambiguous duplicate filenames.
Missing numbers are not synthesized. The report lists bank and clip hashes, sizes,
PCM format and duration. The runtime reads an immutable `COMMENTARY/ENGLISH3.BIN`
container holding unchanged original WAV payloads; its record keys are category
and clip number. Original SOURCE archives are unchanged.

## Original event boundaries

Recovered keeper contact and post/crossbar routines already call the named
commentary events. Rules and card excerpts now retain their original messages.
Corner/goal-kick prefixes and throw-in classification are restored. The native
restart adapter preserves `respot_ball`'s shot classification before resetting
the ball so goal kicks distinguish ordinary exits from wide/close shots.
The original `has_ball` passing-commentary body replaces the earlier event-only
stub, preserving its seed-bit gate and pass classification; a bounds guard avoids
reading an invalid previous player. It observes the existing seed without taking
additional random numbers. Its team-name callback remains silent.

The native goal-crossing latch now calls commentary-only excerpts from both original
`go_left_goal` and `go_right_goal` functions before resetting the shot. Defensive
last touches choose keeper-error or deflection commentary; attacking touches
choose header, rebound off the woodwork, curl, powerful shot or generic goal, in
that original priority order. A missing/invalid last-touch player safely uses the
generic call. Prediction and dead-ball crossings do not request speech; repeated
collision checks cannot repeat a goal call.

`GOOD_SWERVE` is 0.038, recovered as a double from the pinned BALL.OBJ CONST offsets
0x16c and 0x174. The source intentionally remains asymmetric: right-end powerful
shots require speed >18, left-end shots >16; right-end curls distinguish a wall,
while left-end curls use the ordinary curl category. These are preserved source
rules, not new gameplay tuning. Headers and rebounds take precedence over curl
and power. The original source archive and scoring/physics behavior are unchanged.

The five-part 0.30 batch covers headers, woodwork goals, keeper errors/deflections,
power/curl reactions, and spoken scores. Native regulation half-time and final-result transitions emit
42/43 once, replacing the presentation calls excluded from the original timing
excerpt. Extra-time intervals do not incorrectly announce full time; the final
result, including a shootout, does. Native diagnostics can request every verified
category independently for testing.

## Native scheduling and limits

- One speaking voice and up to four pending requests. Engine event storage holds
  128 IDs and counts overflow; prediction cannot emit events into the live queue.
- Period announcements outrank goals/scores, which outrank saves/misses, restarts and
  passing remarks. Goals/periods interrupt lower-priority speech and discard older
  low-priority requests. Other speech waits or expires rather than overlapping.
- Pending routine phrases expire after 1.5 seconds, shot reactions after three,
  and goal/period calls after eight. Normal speech has a one-second gap.
- Category cooldowns and a history of the last 64 spoken clips limit repetition.
  Fixed period lines are allowed again in subsequent matches. An independent
  speech PRNG chooses among validated candidates; gameplay RNG is untouched.
- SDL plays float stereo at the original 11,025 Hz rate, centred, with 5 ms sample
  edge ramps and a small output queue. Speech volume defaults to 75 percent;
  background effects drop to 45 percent and music to 25 percent of their current
  mix during speech. These are native mix/scheduling decisions, not a recovered
  DOS commentary scheduler.

The opening introduction is now verified for 14 of the 16 Euro teams. The CD's
SOCCER.EXE contains 45 team_info records at file offset 1,031,292, stride 872;
the country name begins each record and its two-letter speech code is at +136.
The importer pins the executable hash and matches normalized country names to
SCRIPT.96 (ignoring punctuation/case). It never uses CD player indices for Euro
players. The report records each matched row's offset, name and speech code.

AUDIO.OBJ's PlayWelcomeMessage loads a random MCA record plus the second team's
YTT and first team's XTT. UpdateCommentary consumes LinkStack from the last entry
backwards. The restored sequence therefore plays home XTT, away YTT, then MCA,
joined as one speech voice at kickoff. A goal/period announcement can interrupt
it; pausing discards it. Neither country name is guessed: if either country is
unmatched (Croatia or Turkey), the introduction remains silent. This does not
change either team's gameplay or ordinary event commentary.

Individual player names and substitutions are still unverified
and disabled. The original team database is used only as evidence for country
names, not to replace the Euro roster.

## Spoken scores

AUDIO.OBJ `PlayScoreSample` checks home <=7 at 0x25f2 and away <=5 at 0x25fb,
then compares the two at 0x2622 and passes the larger digit first.
`CreateScoreFileName` appends those digit strings at 0x2879 and 0x289d to
`E_SCO`, producing `E_SCOxy.WAV` in `DIGI/PHRASES/E/E_SCO.MAD`.
The importer asserts these instructions and the SCO constant strings against the
pinned object. All 33 available score recordings are imported unchanged, including
00; their hashes and PCM metadata appear in the recovery JSON. Category 44 is a
native pack key for scores; the original ID 44 denotes the link bank.

The native celebration adapter queues the exact score when the goal countdown reaches zero,
matching `process_ball`'s call after its `just_scored` countdown. It does not announce
scores during shootouts or invent missing high-score speech. It retains the original
asymmetric home/away guards; for example, 6:0 has a recording, but 0:6 is suppressed
by the original away-score guard. Negative scores are also rejected.

A score waits behind the goal reaction, while a half/full-time call can cancel it.
Pause, mute, loss of focus, replay and match reset flush pending speech. A golden
goal that ends the match goes directly to full-time commentary rather than a
restart score call. Score recordings may repeat when the same score occurs in a
new match; repetition protection never substitutes an incorrect score.
The original Windows mixer still uses native scheduling rather than the DOS
sound-card/link-bank implementation.

## Reproduction and validation

Run `python tools/import_cd_commentary.py PATH_TO_ISO` from the repository. Imported
media stays in ignored `local-data/`; no replacement speech is generated.
Publication packages and verifies the commentary container with the other media,
and includes its content hash in the immutable build identity.

`original_commentary_contracts` checks actual CD decoding, every category, bounded
PCM, event-buffer limits, prediction, priority, mute/pause, device output and a
complete original CPU match with speech. Country introductions are checked for
matched England/Scotland and suppressed for Croatia/Turkey. It verifies ball state/gameplay RNG are
unchanged by playback and records actual phrase timing and repetition.
The 0.30 contracts add 20 goal-classification/precedence cases, every one of the 48
permitted home/away score inputs, original PCM playback, silent unsupported scores,
priority/cancellation and a celebration-to-restart transition. The full CPU match
also checks that each goal receives a score announcement.

`native_commentary_smoke` exercises F9, F8 and front-end/match transitions through
the real SDL event loop. Both use dummy audio; actual listening is still the final
user check. Full-suite and match evidence is in `commentary-test-results.txt`.

0.29 verification: 57/57 regression checks passed; five follow-up checks passed
after country introductions were connected. The final CPU match finished 1:0,
7,328 ticks, eight spoken sequences and no repeated clips or lost speech events.
The immutable published snapshot also passed its hidden commentary smoke.

0.30 focused verification: all 20 goal-classification cases and 48 permitted
home/away score inputs passed, including PCM, unsupported-score, prediction,
priority, pause/reset, shootout and golden-goal checks. The original CPU match
again finished 1:0 in 7,328 ticks (366.4 simulation seconds). Its goal at 76.65s
selected GL_HEADER (25/15), and the 1:0 recording (44/10) played at 87.65s after
the celebration. Nine speech sequences, no repeats, no dropped events, and no
priority interruptions were recorded. The gameplay result and tick count match
the preceding 0.29 run.

Final 0.30 release verification: 57/57 checks passed (the focused commentary
contract plus the other 56 checks). The published immutable snapshot
`9646b54303d5a8ff-2b4b128d-0703f4d7-298c5bfa` passed the hidden SDL commentary smoke;
all seven media hashes were verified. SOURCE has no Git diff. The user's running
game was not restarted. Logs are saved in `commentary-test-results.txt`.
