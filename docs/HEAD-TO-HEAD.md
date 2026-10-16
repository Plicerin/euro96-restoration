# Local head-to-head - 0.49.0

Connect two controllers and select **H / West (Xbox X): 2 PLAYERS** on exhibition setup. Choose the countries, formations and match format, then confirm the lineups. On the first local launch, the controller used to confirm kickoff becomes P1/home; the other connected controller becomes P2/away. Rematches retain these assignments. With keyboard confirmation, the most recently used controller becomes P1. The game requires two controllers before kickoff.

Each person uses the existing controls, including automatic player selection, East/LB switching, passes, shots, tackles and aftertouch. RB also switches in local play; it cannot change the match to CPU control. Both controllers retain their countries across half-time and extra time. Unselected footballers continue to use the original AI. The shared camera follows the same match. Both selected players have original highlight artwork, and a top strip identifies them.

Start from either controller pauses. Disconnecting either player pauses; an unassigned controller cannot steal an occupied slot. Press Start on a replacement to claim the empty slot, release, then press Start again to resume. Focus return also leaves the match paused. Both players must release action buttons after presentation/menu transitions. Keyboard remains available for menus and display controls, but gameplay requires the two assigned controllers.

Both teams can take awarded set pieces, distribute from the goalkeeper, make substitutions, change half-time tactics and take shootout kicks/control shootout saves. Ordinary goalkeeper positioning and saves remain original AI except for the existing human distribution/shootout controls. Kickoffs retain the existing automatic centre-kick routine. This release is local exhibition head-to-head, not co-op, multiplayer tournaments or network play.

## Implementation boundaries

The native adapter now retains separate direction, held-button/counter, pending-kick, selected-player, selection cadence, restart and shootout input state for P1 and P2. Scoped current-user contexts supply the original routines with user IDs 1 and 2 while both teams share one engine, clock and ball. Original `auto_select_a/b`, user possession/defense decisions, animations, contacts, AI and rules remain the gameplay implementation. Player identity follows countries through physical end swaps.

Controller assignments use SDL device IDs and stay pinned until a device leaves the match. Rejoining explicitly claims a vacancy. Head-to-head uses both original selected-player color variants: `plhi1_f1` and `plhi1_f2`, corresponding to `3DENG.C`'s `plhilight[1][0/1]`. No new gameplay simulation or network transport is added. SOURCE remains unchanged.

## Verification

Engine contracts cover simultaneous independent movement, team-bound switching, shots and charges on both halves; ground/loft pass reception through the original incoming-ball control path; close steals; dismissed-player selection; substitutions; 24 awarded restart cases with opposing input rejected; both keeper distributions; half-time/rematch behavior; and a complete two-human-input shootout.

SDL tests cover pinned controllers, unrelated third-device input, loss of either assigned controller, replacement devices and minimum-device checks. The executable smoke enters via the actual frontend and checks two-pad movement, switching, P2 shooting, pause, disconnect/rejoin, half-time and focus behavior. These use virtual controllers; physical two-controller feel remains a combined human playtest.

Release evidence and delivery details appear in [release status](RELEASE-CANDIDATE.md).
