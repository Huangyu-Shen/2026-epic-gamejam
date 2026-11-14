# Day 2 (13 Nov) · The moment I stopped, and the research

[Dev log index](../README.md) · [← previous](01-finishing-the-first-loop.md) · [next →](03-seven-ideas-and-the-agent-review.md)

## The moment I stopped

On Day 2 I looked at the build as a stranger would. It worked. It was also
a standard survivors-style game with a camp attached. This is what I told
Claude, translated from my own message:

> What we are making has nothing to do with Director's Cut. It is a very
> traditional Vampire-Survivors-like with nothing new about it. It cannot stand
> out.

The design documents did mention "editing", but the build did not show it. The
director's decisions happened in one menu before the run. During the three to ten
minutes a judge actually spends inside a run, nothing they did was a director's
act. A theme that is only readable in the documents does not score.

## Research: could an open-source game be the base?

A video I came across suggests starting from an existing open-source game
(there is a large catalogue, `Trilarion/opensourcegames`), having the AI map its
structure, then changing theme, art and rules. I asked Claude to read the
catalogue properly instead of guessing.

It cloned the list: about 1,930 entries, 1,801 of them games. Searching it for
the genre's names and keywords found **no survivors-like game and no Unreal
Engine project**. Most entries are C or C++ on SDL or pygame, roughly 12% are
remakes of old games and 45% are inactive. The closest are Crimsonland clones and
Diablo-likes, mostly GPL-3.0.

I did not use it as a base: the jam requires gameplay made during the jam,
nothing in the list runs on my engine, and my loop already worked. What I kept
from the advice is the method: start from something proven and add one small
real innovation. The innovation was the weak part.
