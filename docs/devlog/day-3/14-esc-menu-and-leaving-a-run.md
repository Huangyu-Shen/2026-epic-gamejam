# Day 3 (14 Nov, night) · Leaving a run: a button, then an Esc menu

[Dev log index](../README.md) · [← previous](13-story-012-boss-and-the-void.md) · [next →](15-the-first-real-models.md)

## The problem

After the boss and the floor edge were done, the author pointed out that a run could
only end by extracting or dying (or quitting the whole game). There was no way to go
back to the camp by hand.

## First answer: a button, kept for about an hour

I added a red "Abandon run (lose bag)" button under the bag panel. The first click
changed its text to "Click again to abandon" (it lapsed after 3 seconds); the second
ended the run as a loss and showed a "RUN ABANDONED" screen with the existing Return to
Camp button. A test flag pressed it twice and the screenshots showed all three states.
I packaged it.

**A design choice I made and said so:** abandoning counts as dying, so the bag is lost.
Without a cost, leaving would be a free "save and quit" that makes extraction pointless.
The author was asked to say if they wanted a partial refund; they did not.

## What the author actually wanted

The author then described the intended shape in a few lines: pressing Esc, in the camp
or in a run, should open a menu with options that depend on the screen:

- Camp: Settings, Quit, and some other things.
- In a run: Settings, Return to camp, Quit, and some other things.

So the corner button was a narrower answer to a bigger question, and I removed it.
This is a scope change. The PRD lists a pause menu and settings as Should/not now, and
the user asked for them directly, so they are in. The PRD and the design doc have not
been updated for it yet.

## What was built

- One menu widget for both screens, opened by Esc. In the camp the corner button became
  "MENU (Esc)" and opens the same menu.
- **Camp:** Resume, Settings, Quit game.
- **Run:** Resume, Settings, Return to camp (lose bag), Quit game. Opening it pauses the
  game; closing it resumes. It does not open while a level-up offer is showing.
- **Settings:** three volume levels (master, music, effects) in steps of 10%. They last
  for the session and are not saved. A new audio function reads the player's value first
  and falls back to the value in Director Tuning.
- **Return to camp** keeps the two-click confirmation and the loss. A run that already
  ended goes straight back. A message about the lost bag follows the player to the camp.
- The only "other thing" I added is a short list of the controls. I did not know what
  else was meant and said so.

## Two things that went wrong while building it

- **A paused world runs no timers.** My first test script used the game's timers for the
  steps after the menu opened, so they never fired. The steps now run on the engine's
  own ticker, which keeps going while the game is paused. This also matters for any
  future probe that opens the menu.
- **Esc can arrive at the wrong place.** After a click, the keyboard focus is on a button,
  so Esc goes to the menu and not to the player's input binding. The menu now handles Esc
  itself, and the player binding is set to work while paused. I have not tested this
  focus case by hand: the test script called the menu's functions directly.

## What was checked

- Build compiles. A test flag opened the menu in a run: enemies stopped (the pause
  works), Music went from 35% to 15% after two presses of the minus button, the Return
  button showed its confirmation text, and the second press logged the death and loaded
  the camp map. Screenshots are in `production/qa/evidence/pause-menu/`.
- **Not checked:** the camp's menu (same widget, no screenshot), a real mouse click, Esc
  after clicking a button, and the volume rows' alignment, which is slightly off.
- The old screenshots in `production/qa/evidence/abandon-button/` show a button that no
  longer exists. I left the folder because deleting needs the author's approval.
