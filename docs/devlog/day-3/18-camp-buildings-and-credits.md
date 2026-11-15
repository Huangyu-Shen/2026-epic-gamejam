# Day 3 (15 Nov, night) · Real buildings for the camp, and the credits in the game

[Dev log index](../README.md) · [← previous](17-wood-stone-and-the-camp.md) · [next →](19-status-so-far.md)

The camp's buildings were stand-ins (a rock, a gate, a ridge) because the forest pack has no
houses. The author searched Fab for building packs, sent screenshots of five, and then
imported every one that was compatible. I was asked to use what fits.

## The five packs

| Pack | What I decided |
|---|---|
| Cozy Low Poly House (Demolition Studios) | **Used** for the clinic. The nearest style to the bright low-poly world. |
| WW2 RTS & SLG Production Buildings (Winyunq) | **Used** for the smithy (a factory with a chimney and a water tower) and the warehouse (the barracks). It is military, which fits the fantasy theme badly, but its buildings are one finished mesh each and the colours (olive, cream) sit well on the green meadow. |
| Stylized Medieval City (Baturinets) | Not imported. The author reported that only the compatible packs went in; this one did not. I had thought it the best source of a tower for the smithy. |
| ZenUV Modular Sci-Fi Kit, 5 Free Stylized Sci-Fi Skyscrapers | Imported, not used: science fiction. |

## How the house was put together

The house is not one mesh. It is a set of parts: foundation, main body, roof, porch, a brick
chimney tube, plus doors, windows, a car and more. I logged the parts' sizes and saw that their
bounding boxes all sit around one shared pivot (the roof's centre is above the main body's, and so
on), so spawning the five big parts at one point assembles the house without offsets. The screenshot
confirmed it: a pale blue house with a white roof. Doors, windows and the porch's decorations are
not placed, so the house is plain.

The clinic is therefore a small group of parts under one invisible root, and the code creates them
at the start of the level. If any of the five parts is missing the house is not built, because a
house with a missing roof would look broken. The smithy and the warehouse are one static mesh each
from the WW2 pack (it ships static meshes, rigged ones and level-of-detail variants; I used the
static one). They are scaled to about 420 cm across (the pack's footprint is 12 by 12 m).

The invisible cube under each building stays as the click target. The buildings grow 8% taller per
level (the cubes grew 50%), because a stretched house looks wrong. When a building's models are
missing, the old cube shows.

**A trap with the first test:** the first screenshot of the WW2 buildings showed them plain grey. The
game was still compiling the shaders. I added a longer wait and they came out olive and cream with
textures. I mention it because I nearly judged the pack on the grey picture.

## Credits, in the game

Two packs need credit: the music (CC BY, Kounine) and the WW2 buildings (CC BY 4.0, Winyunq). Until
now the credit was only in this log. The licence asks for credit in a way that suits the medium, so
the Esc menu now ends with a small credits text naming those two and the other packs. The WW2 pack's
licence also asks to indicate changes: only two of the buildings are used, scaled. The README's
credit section says the same.

## Checks

- Build compiles. A camp screenshot shows the house at the top, the barracks on the left and the
  factory at the bottom, with the camp's wood, stone and the new costs. Another shows the Esc menu
  with the credits. Evidence files 17 and 18 in `production/qa/evidence/story-011-art-audio-feel/`.
- A packaged build loads the camp and the run map, and the 150-enemy test averaged 126 FPS (worst
  57) on the warm run and 94 on the cold one.

## Not done and not checked

- The house is plain: no door, windows or garden. Its roof is large and white, and from the camp
  camera it takes a big part of the screen.
- The houses' front faces a random way: I did not rotate it toward the camera.
- Clicking the buildings with a real mouse. The invisible cubes are 400 by 400 cm, which is wider
  than before.
- The licence texts were not read in full, and the CC BY credit wording is mine.
