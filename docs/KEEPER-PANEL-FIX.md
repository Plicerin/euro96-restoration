# Goalkeeper recovery and match panels — 0.57.1

Human goalkeeper selection bypassed two original action transitions. A keeper carrying the ball in PICKUP_ACT had not yet set ball_in_hands, so the scheduler treated him as a normal controlled ball holder and stopped decrementing his I_GET_UP countdown. After a save, the human delivery branch correctly waited for recovery but skipped kphold_action's switch to MC_STANDB, leaving the get-up capture looping.

The native scheduler now includes the pickup action in keeper recovery, preserves the original countdown and pickup completion, and selects the original standing hold animation when recovery ends. Distribution remains under human control; it does not run the CPU's bounce/wave/delivery decisions. Original timing constants and SOURCE are unchanged.

The regression fixture covers ground pickup, left/right save recovery and crouched recovery in both halves. Each must reach MC_STANDB, retain possession, stay standing while awaiting input and then release a requested punt. The old scheduler [fails this fixture](keeper-recovery-red.txt); the corrected scheduler [passes](keeper-recovery-green.txt). Existing natural-catch, human delivery, selection and head-to-head checks also remain in the affected suite.

The R3/F6 display was a native development panel, not an original DOS options menu. It now uses the recovered Euro96 title bar, rounded panel and font. The score, clock, current player, CPU match statistics and radar state remain available. The pitch viewport and aiming coordinates are unchanged. Invisible development-tab mouse targets are disabled in team matches. Keyboard and controller controls remain the same.

The layout and Windows control hints remain native adaptations; this does not claim an exact recovered DOS radar/options layout. Dedicated set-piece power/height displays and explicitly selected development laboratories remain separate.

The menu contract captures keyboard/controller panels, checks that the radar label changes and that drawing does not advance match state. The executable's existing smoke flow exercises F6/R3 and radar toggles. All [13 affected checks](keeper-panel-checks.txt) passed in 42.94 seconds. After the final title-bar refinement, the [three affected rendering/input checks](keeper-panel-art-checks.txt) passed again in 19.86 seconds. [Actual executable panel capture](keeper-panel-live.png) was visually reviewed. The immediately preceding 0.57.0 release passed the complete 95-test suite; this focused patch does not claim a second full run.
