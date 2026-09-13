# Submitted baseline audit

Authoritative baseline: `Starter/submitted version`.

The active `Starter.ino` has the same executable code as
`submitted version/artemida2/artemida2.ino`. Three `IMPROVEMENT` comment lines
were added at its beginning. Some original Cyrillic comments display with a
different encoding; comments do not affect the compiled program.

## Retained improvement

`setup.ps1` installs `ArduboyTones@1.0.3`. The submitted code already includes
`ArduboyTones`, creates its `tones` object, and starts `interstellar`; the
dependency was missing from the local tool installation. The setup addition is
marked with a `SOUND UPDATE` comment.

## Preserved files

The following active files are byte-for-byte identical to their submitted
counterparts: `artemide.h`, `blocks.cpp`, `blocks.h`, `hero.cpp`, `hero.h`,
`level1.cpp`, `level1.h`, `level2.cpp`, `level2.h`, `music.cpp`, and `music.h`.

The submitted controls, movement, physics, collision handling, level logic,
transitions, graphics, scoring, lives, music, and sound definitions are
preserved. Earlier runner, cat-chase, rocket-transition, and extra sound changes
are excluded from the active build.

## Later level-alignment improvements

The submitted folder remains untouched. The active copies of `level1.h`,
`level1.cpp`, and `level2.cpp` now contain clearly marked `IMPROVEMENT` changes:

- Both levels have a 25-point target and display the score as `current/25`.
- Level 2 uses the same Right acceleration and Left slowdown range as level 1.
- Level 2 displays the same three two-part heart icons as level 1.
- Level 2 draws its ground with the same 8x8 map tiles as level 1 while retaining
  a flat lunar layout, scrolling stars, craters, bugs, clouds and broken rockets.

All other active source files still match the submitted versions. The project
compiles after these active-copy improvements.
