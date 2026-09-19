# Gecko revision used by EXTART

EXTART tracks a specific Mozilla Firefox source revision for its native Gecko
embedding work.

Current revision:

`b16f852ba66d190cacd42d0d43ac4d38bd07c4bd`

Source tree:

`https://github.com/mozilla-firefox/firefox/tree/b16f852ba66d190cacd42d0d43ac4d38bd07c4bd`

This is a source revision, not a Firefox executable dependency. EXTART does not
launch Firefox as its browser engine.

The native embedding work is intentionally gated until EXTART has the Gecko
headers/libraries produced from this revision. The integration must create and
own Gecko browsing contexts and their docshells rather than wrapping an
external Firefox process.
