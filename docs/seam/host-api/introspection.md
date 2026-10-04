# Introspection

`bridge_elements` exists for one reason, and it is not a console: **an element that loaded can be
stopped later, and nothing else says so.** Past its failure budget Bridge-ASM700 stops an element - its
systems and observers stop running and its components stay, `failure.md`, *When something fails*.
That happens at frame nine thousand, long after any return code the host saw.

**Not for load failures.** The host calls `bridge_load` once per element and gets a code back each
time, so what loaded is something the host already watched happen.

**And it has to be a pull.** A record saying "stopped" would do, except that the log is *"Drop and
count, never block"* - `log.md`, *The log*. State must never arrive by a route that may drop
one, or a host's picture is permanently wrong with nothing to correct it. Which is also why this
call exists rather than a host reading its own log output back: that is using diagnostics as state.

So two fields, and everything else is deliberately absent. Kind, the ABI version, the grants and the
settings are all things the host handed over. Memory used, systems registered and times failed are
diagnostics, and go where diagnostics go. **An element that is not there does not appear** - the
list is what exists, the host holds what it handed over, and the difference is the answer.
