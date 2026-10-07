# Introspection

`bridge_list_modules` exists for one reason, and it is not a console: **a module that loaded can be
stopped later, and nothing else says so.** Past its failure budget Bridge-ASM700 stops a module - its
systems and observers stop running and its components stay, `failure.md`, *When something fails*.
That happens at frame nine thousand, long after any return code the host saw.

**And it says why the bridge stopped**, when a module's stop was what stopped it - `failure.md`,
*When something fails*: the module shows as STOPPED like any other, and the record that named it may
have been dropped. A module whose reload failed with no old instance to keep shows as STOPPED too:
for a host the two are one state, neither runs, both keep their data, and a reload or an unload is
what comes next.

**Not for load failures.** The host calls `bridge_load` once per module and gets a code back each
time, so what loaded is something the host already watched happen.

**And it has to be a pull.** A record saying "stopped" would do, except that the log is *"Drop and
count, never block"* - `log.md`, *The log*. State must never arrive by a route that may drop
one, or a host's picture is permanently wrong with nothing to correct it. Which is also why this
call exists rather than a host reading its own log output back: that is using diagnostics as state.

So two fields, and everything else is deliberately absent. The backend, the ABI version, the grants
and the settings are all things the host handed over. Memory used, systems registered and times
failed are diagnostics, and go where diagnostics go: `profile.md`, *The profile*, which stage 17
builds, for how full each budget is. **A module that is not there does not appear** - the list is
what exists, the host holds what it handed over, and the difference is the answer.
