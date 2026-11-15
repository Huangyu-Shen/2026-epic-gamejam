# Day 3 (15 Nov, night) · Wood and stone, a forest camp, and sizes

[Dev log index](../README.md) · [← previous](16-the-forest-floor.md) · [next →](18-status-so-far.md)

The author played the forest build and sent screenshots of the camp, a tree stump, a boulder
and the portal, with four requests.

## What the author asked

1. Since the containers are stumps and boulders, make the loot **wood and stone** instead of Iron.
2. The camp is still cubes on a grey floor: change it.
3. Make the portal bigger, and the hero and monster models smaller.
4. Leave the stumps and boulders as they are.

## 1. Wood and stone

The author did not say who uses which. I chose, and said so:

- A supply crate (stump) gives **wood**; a vault (boulder) gives **stone**. The amounts are the
  old Iron ranges (crate 3 to 6, vault 10 to 20).
- The **Smithy** costs **stone** and the **Clinic** costs **wood**. The costs are the old Iron
  costs. The editing-room cards multiply both, and their text now says "Loot earned".
- The bag shows "Wood n  Stone n" on a second line (the first try wrapped badly). Slots are
  counted per resource: wood and stone each round up to whole slots, a reel is one slot.
- Extraction adds both to the camp and death loses both; the result screens show both.

**What stayed as it was, on purpose:** the code and the data tables still call the wood
field "Iron" (`IronMin`, `IronCount`, the save field `Iron`). Renaming them would change the
saved data and the tables, which can only be edited through the editor, so I added the stone
fields next to them and noted the old name in comments. The effect on the save: an old save's
Iron now reads as wood, stone starts at 0. The author's real save had 104 Iron and the camp
now shows Wood 104, Stone 0. I did not bump the save version.

**A balance risk I did not fix:** stone only comes from vaults, which are few, while the Smithy
asks for 10 to 80 stone. Wood comes from the common crates. The upgrade paths may now be very
uneven. I kept the old numbers because I have no play data for the new split.

## 2. The camp

- The same forest floor and rim as the run map now covers the camp (the floor look is built
  for the camp as well; both maps have the same 8000 cm floor).
- **The pack has no houses**, so the buildings are stand-ins: a rock outcrop for the Smithy, a
  stone gate for the Clinic, a low mountain ridge for the warehouse. From the camp's camera the
  gate looks like a thin strip and the ridge like a white ring. They grow taller with each
  level, as the cubes did.
- The invisible cube stays under each model as the click target, so clicking a building still
  works in principle. I did not click one with a real mouse.
- If the author wants real houses, that needs another Fab pack ("low poly fantasy house"
  was the search I suggested).

## 3. Sizes

- Hero model scale 1.6 to 1.1. Grunt models 1.8 to 1.2. The chest-monster boss 1.0 to 0.85.
- The portal went from 0.5 to 1.0 times its size (about 830 cm across). I also **widened the
  extraction zone from 400 to 800 cm and the green disc to match**, because a portal twice the
  size of the zone would invite standing in it without extracting. This makes extraction a bit
  easier to enter and to stay inside; it is a gameplay change the author did not ask for.

## 4. Stumps and boulders: not touched.

## Checks

- Build compiles; the 12 automated tests pass (the settlement test did not need changes).
- A temporary probe (removed afterwards) searched every container: crates added wood (5, 10,
  16), a vault added 14 stone and a reel, and the bag counted 5 of 6 slots; the screenshot
  shows "Wood 16, Stone 14" and the refused-pickup message.
- Camp screenshot shows "Wood: 104  Stone: 0", the Smithy asking for stone and the Clinic for
  wood. Evidence files 14 to 16 in `production/qa/evidence/story-011-art-audio-feel/`.
- Packaged build: the 150-enemy test (weapons off) averaged 134 and 133 FPS on warm runs,
  worst frames 44 and 47; the cold first run was 97 FPS, worst 47. The worst frames are a bit
  lower than the last package's 58 and 62; I did not find out why and did not repeat enough
  to call it noise.

## Not done and not checked

- The design documents, the PRD and the wiki still say Iron.
- Clicking the new camp buildings, and whether the gate and the ridge read as buildings.
- Whether stone is too scarce for the Smithy.
- Whether the widened zone changes how extraction plays.
