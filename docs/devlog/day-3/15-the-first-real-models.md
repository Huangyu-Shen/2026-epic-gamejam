# Day 3 (14 to 15 Nov, night) · The first real models

[Dev log index](../README.md) · [← previous](14-esc-menu-and-leaving-a-run.md) · [next →](16-the-forest-floor.md)

Until now every character was a grey, red or blue primitive. This entry covers swapping the
hero, the enemies and the boss for models from Fab.

## Choosing, and one dead end

I looked at Fab pages with the author, who took screenshots. What I could and could not tell
from a page mattered more than how the models looked:

- **A page does not say everything.** It gave triangle counts only for one pack (1273 to
  7523 per monster). For the others I could not tell the engine version, the size or the
  licence text, and I said so each time.
- **Two packs turned out unusable.** I had recommended Creative Characters FREE and Animals
  FREE (ithappy). The author found they only support Unreal 4. I had written that "UE is in
  the format list" and had not checked the version, which was my mistake. They were dropped.
- **Two packs from one author were picked:** RPG Hero Squad (the hero) and Monster for
  Survival Game (the enemies), both Dungeon Mason, Polyart style. The same author was the
  reason I expected the style to match, and in the screenshots it does.
- **Performance was the first worry.** About 150 enemies on screen, each now an animated
  skeletal mesh instead of a cube. That is a much heavier thing to draw than a cube, so I
  measured before swapping everything.

## What was built

- **The hero** uses the pack's RPG Hero model with its sword and shield attached to the
  sockets the model already has (named `Sword` and `Shield`, found by reading the asset's
  file, not by guessing). It plays a Run animation while moving and a frozen Walk pose when
  standing still, because the pack has no idle animation. It is scaled up 1.6 times to read
  from the camera.
- **The enemies:** slime, mushroom, cactus, turtle shell and beholder for the grunts, picked
  at random for each spawn, and the chest monster for the boss. The old cube stays as an
  invisible root so scale, hit punch and the crowd's spacing work as before. Each model
  loops its Run animation from a random point so a crowd does not step in unison.
  Grunt models are scaled 1.8 times: at their natural size they looked tiny next to the hero.
- **Without the packs the game still runs**, with the old cubes and the cylinder. The packs
  are not in the repository (see [entry 13](13-story-012-boss-and-the-void.md)), so a clone
  looks like the old version. They are referenced directly from the code, so the cook
  includes them without a setting.

## Checks

- A packaged build played back the 150-enemy test with the weapons off: the first, cold run
  averaged 98 FPS with a worst frame of 21.5. Two more runs on the same build averaged 132 and
  128 FPS with worst frames of 54 and 64. The earlier cube build averaged 130, so I read the
  first run as the first-time shader and asset load and the others as the real result. I did
  not prove that; I only repeated the run. The test still does not fire any weapon, so the
  cost of weapons plus animated enemies together is not measured.
- Screenshots, kept in `production/qa/evidence/story-011-art-audio-feel/` (files 10 and 11):
  a knight with sword and shield, a chest-with-teeth boss, and a horde of the five grunt
  types.

## Not done and not checked

- The floor, the loot crates, the extraction point and the camp are still plain shapes.
- The enemies' facing: I did not check from the models' own forward direction that they look
  where they walk. The hero's sword points out ahead in the screenshots, which looks right.
- Hit flash on the new models: the old white flash worked by changing a material colour on the
  cube. The new models only get the size punch.
- Boss attacks are still orange balls and a red ring, not drawn to match the new art.
- Licence texts were not read in full.
