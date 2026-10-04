# An engine callback answers from storage

This is where the design pays. An engine's callbacks mostly ask for **state**, and state is in
storage, so a plugin answers from it with no call and no crossing:

    _process              the module's logic is a system; its output is a component the
                          plugin flushes
    _get, _set            a module's properties are components; the plugin reads them
    _get_property_list    the descriptors are a component array
    _has_point            the shape is a component; the plugin does the test
    a signal firing       an entity carrying an event component; whoever subscribed runs
                          at the barrier - `data.md`, *Observers*

Godot's own documented example needs none of it: a position computed from elapsed time is a system
writing a component. It also fixes a Bridge defect, where a module could not set its own property
because `Object.set` came back as a re-entry.

What is left over is the callback whose return value the engine consumes **inside its own loop**,
with the engine on the stack waiting - `Array.sort_custom` asking a comparator for a bool N log N
times. No component write answers a question that has to be answered now, so **a deployment cannot
express that one**, and `costs.md`, *What we give up*, is where it is written down.
