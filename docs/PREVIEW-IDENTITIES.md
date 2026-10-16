# All 320 player previews — 0.62.1

All 23 previously unresolved player previews are now restored from the supplied Euro 96 disc. None required invented art or a substitute player. The earlier count reflected importer naming restrictions, not absent animations.

The importer now decodes the DOS scripts as CP850 and folds accented characters for name comparison. Where names still differ, it requires a unique agreement on country/player ID, birth date, normalized club, position and all eight gameplay attributes. It never chooses the closest spelling. Unknown metadata, conflicting assignments or mismatched identity fields are rejected.

Examples include Marcus BABBLE/BABBEL, Christo/Hristo STOICHKOV, Jurgen/Jürgen KLINSMANN, and the expanded Portuguese names. **NENO is Adelino Barros NENO**: both records are POR12, born 27/01/62, goalkeeper at Vitória Setúbal, with identical eight gameplay attributes. His authored preview is PORGO2 using FRAPAL. Osman and Cañizares also match their complete independent identity fields despite spelling/encoding differences.

The retail script lists Yuri Kovtun in both the main roster and reserves. Matching the player ID and complete identity fields selects the correct entry; the earlier, already-working animation remains identical.

The final pack contains 109 unique animation/palette combinations serving all 320 players and eight venues. Three additional animation files are needed; the other recovered players use animations already shared by retail roster entries. The [resolution inventory](preview-identity-resolution.json) records all 23 original and retail names and their corroborating fields. The [full import report](euro96-previews.json) includes all 320 assignments and source hashes.

Validation checks every previous mapping against its old palette and compressed animation bytes: **all 305 previously available player/venue slots remain identical**. Native tests decode every imported animation, compare 327 independently computed pixel hashes, and render Neno, Klinsmann and Osman through the actual statistics menu. Identity tests reject mismatched IDs, dates, clubs, positions, attributes, unknown metadata and conflicting assignments. The original roster, gameplay, saves and speech mappings are unchanged.

All five focused checks passed, including six identity unit cases. The package passed 53 hashes and nine isolated smoke checks. Version 0.62.1 is published and the usual launcher verifies. [Release evidence](preview-identities-release.json). Captures: [Neno](preview-neno.png), [Klinsmann](preview-klinsmann.png), [Osman](preview-osman.png).
