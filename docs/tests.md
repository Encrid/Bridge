# The tests

What Bridge-ASM700 exposes, and the cases that show it. A capability is a thing somebody outside
the library can interact with, on one of its three seams: a function a host calls and a key it
states, an entry a module calls and a layout it reads, an entry a backend answers. The headers
are where each is, so the headers are what this list is held against.

**Every exposed name has a heading here, and every heading has its cases.** A case is named for
its heading, a hyphen, and what it shows: `bridge_reload-smaller-share-refused` is under
`bridge_reload`. `python build.py test -k bridge_reload` runs what there is for one.
`tools/cover.py` reads the headers of the three seams and of the two SDKs, this document, and
the cases of `build.py`, and fails on an exposed name with no heading, a heading with no case, a
heading the headers do not expose, a case under no heading, and a case listed that is not run.
It is itself a case, `cover`, so the build fails with it.

**A capability is added with its case.** A function, an entry, a key, a macro or a layout that a
header gains has no heading the day it is written, and the build says so until it has one and a
case under it. That is the rule `plan.md`, stage 13, states for every stage after this one.

**What is read as exposed.** A function, an entry of a table, a macro that takes arguments, a
constant, and a struct. A struct a host fills is exposed key by key, as `config.threads` or
`module_config.trusted`, since each key is stated and refused in its own words; a key that
counts an array is said with the array, and a member named `pad` is padding. Any other struct is
a layout, and is one heading. A name that ends in an underscore is the SDK's own.

**Several cases may be one run.** A run that loads two modules and reads a sum shows a grant, a
query, an accessor and a log at once, and each of them has its case: the same program and
argument, judged by the words that are that capability's. A case that only this run could not
show has a run of its own.

**Which document states each is not said here.** A heading is a name of a header, and the
documents state what the library does in their own words and under their own headings: which of
them states which name is the documents' to say, and stage 14 reads them against this list.

**A message is not a heading.** Every message the source can write is held another way:
`tools/said.py` reads each from the source and fails on one no case made appear. It is the case
`said`.


## The host seam

`include/bridge.h`: what a host calls, and each key of what it hands over.

### bridge_value_t

    bridge_value_t-a-value-of-each-kind

### setting.key

    setting.key-no-such-field-refused

### setting.kind

    setting.kind-text-for-a-float-refused

### setting.v

    setting.v-seeds-the-field-its-key-names

### backend.name

    backend.name-none-refused
    backend.name-twice-refused

### backend.library

    backend.library-cannot-open-refused
    backend.library-none-refused

### backend.settings

    backend.settings-handed-to-the-backend
    backend.settings-each-kind-read-by-the-backend

### budget.entities

    budget.entities-beyond-32-bits-refused
    budget.entities-unset-refused

### budget.tables

    budget.tables-unset-refused
    budget.tables-used-up-both-told

### budget.types

    budget.types-unset-refused
    budget.types-reached-refused

### budget.systems

    budget.systems-unset-refused
    budget.systems-reached-refused

### budget.terms

    budget.terms-unset-refused
    budget.terms-exceeded-refused

### budget.modules

    budget.modules-reached-refused
    budget.modules-unset-refused

### budget.persistent

    budget.persistent-unset-refused

### budget.columns

    budget.columns-no-room-for-direct-indexed-refused
    budget.columns-used-up-changes-not-applied
    budget.columns-unset-refused
    budget.columns-no-room-for-a-singleton-refused

### budget.sparse_sets

    budget.sparse_sets-none-refused
    budget.sparse_sets-unset-refused
    budget.sparse_sets-negative-refused
    budget.sparse_sets-beyond-32-bits-refused

### budget.direct_indexed

    budget.direct_indexed-none-refused
    budget.direct_indexed-unset-refused
    budget.direct_indexed-no-limit-refused
    budget.direct_indexed-beyond-32-bits-refused

### order.before

    order.before-none-refused

### order.after

    order.after-none-refused

### config.abi

    config.abi-another-refused
    config.abi-unset-refused

### config.backends

    config.backends-two-in-one-phase

### config.phases

    config.phases-none-refused
    config.phases-nameless-refused
    config.phases-twice-refused

### config.order

    config.order-mover-before-halver
    config.order-halver-before-mover
    config.order-cycle-refused
    config.order-observers-watcher-first
    config.order-observers-fixer-first

### config.threads

    config.threads-two-run-at-once
    config.threads-one-runs-in-turn
    config.threads-unset-refused
    config.threads-four-wasm-instances-hop

### phase_name.from

    phase_name.from-mapped-loads
    phase_name.from-unset-refused

### phase_name.to

    phase_name.to-undeclared-refused

### module_config.identity

    module_config.identity-none-refused

### module_config.abi

    module_config.abi-another-refused

### module_config.trusted

    module_config.trusted-unset-refused

### module_config.reloadable

    module_config.reloadable-false-with-hot-reload-refused
    module_config.reloadable-unset-refused

### module_config.hot_reload

    module_config.hot_reload-false-refused
    module_config.hot_reload-unset-refused

### module_config.pinned

    module_config.pinned-runs-on-the-loading-thread

### module_config.on_error

    module_config.on_error-fail-stops-and-skip-does-not
    module_config.on_error-unset-refused

### module_config.max_system_failures

    module_config.max_system_failures-reached-stops-the-module
    module_config.max_system_failures-one-in-three-forgiven
    module_config.max_system_failures-two-in-three-reached
    module_config.max_system_failures-clean-system-forgives-no-other
    module_config.max_system_failures-no-limit-never-stops
    module_config.max_system_failures-unset-refused

### module_config.work_limit

    module_config.work_limit-spinning-system-ended
    module_config.work_limit-unset-refused

### module_config.memory_limit

    module_config.memory_limit-growth-refused
    module_config.memory_limit-memory-larger-at-start
    module_config.memory_limit-unset-refused
    module_config.memory_limit-negative-refused

### module_config.init_timeout

    module_config.init_timeout-init-interrupted
    module_config.init_timeout-unset-refused

### module_config.reload_timeout

    module_config.reload_timeout-unset-refused

### module_config.shutdown_timeout

    module_config.shutdown_timeout-shutdown-interrupted
    module_config.shutdown_timeout-unset-refused

### module_config.log_level

    module_config.log_level-unset-refused
    module_config.log_level-no-severity-refused
    module_config.log_level-warnings-drop-info

### module_config.scratch

    module_config.scratch-private-beyond-32-bits-refused
    module_config.scratch-unset-refused

### module_config.log_ring

    module_config.log_ring-full-lines-dropped
    module_config.log_ring-unset-refused
    module_config.log_ring-smaller-than-a-record-refused

### module_config.spans

    module_config.spans-too-small-refused
    module_config.spans-unset-refused
    module_config.spans-outgrown-at-a-barrier

### module_config.spawns

    module_config.spawns-used-up
    module_config.spawns-unset-refused
    module_config.spawns-more-than-entities-refused
    module_config.spawns-sum-beyond-entities-refused

### module_config.persistent

    module_config.persistent-more-than-the-arena-refused
    module_config.persistent-unset-refused

### module_config.changes

    module_config.changes-used-up
    module_config.changes-stated-as-none-refused
    module_config.changes-unset-refused
    module_config.changes-beyond-32-bits-refused

### module_config.read

    module_config.read-another-namespace-read

### module_config.write

    module_config.write-destroy-goes

### module_config.config

    module_config.config-unset-refused
    module_config.config-undeclared-refused
    module_config.config-taken-refused
    module_config.config-not-a-singleton-refused

### module_config.settings

    module_config.settings-of-every-kind
    module_config.settings-text-beyond-the-share-refused
    module_config.settings-reseeded-in-place

### module_config.phases

    module_config.phases-unmapped-refused
    module_config.phases-mapped-twice-refused

### module_config.losses

    module_config.losses-count-unset-refused
    module_config.losses-counted-and-none-refused

### artifact.backend

    artifact.backend-not-configured-refused

### artifact.bytes

    artifact.bytes-none-for-wasm

### artifact.len

    artifact.len-as-many-bytes-compiled

### artifact.path

    artifact.path-cannot-open
    artifact.path-none-for-native

### record.identity

    record.identity-whose-it-is

### record.system

    record.system-which-system-said-it

### record.text

    record.text-what-was-said

### record.len

    record.len-as-long-as-the-text

### record.when

    record.when-the-barrier-it-was-said-after

### record.arg

    record.arg-the-two-numbers

### record.level

    record.level-info-with-its-two-numbers

### module_info.identity

    module_info.identity-of-each-that-is-loaded

### module_info.state

    module_info.state-running-and-stopped

### BRIDGE_BOOL

    BRIDGE_BOOL-seeds-a-bool

### BRIDGE_INT

    BRIDGE_INT-beyond-the-field-refused
    BRIDGE_INT-beyond-the-enum-refused

### BRIDGE_FLOAT

    BRIDGE_FLOAT-seeds-a-float

### BRIDGE_STRING

    BRIDGE_STRING-longer-than-the-field-refused
    BRIDGE_STRING-no-such-constant-refused

### BRIDGE_FAIL

    BRIDGE_FAIL-stopped-module-stops-the-bridge
    BRIDGE_FAIL-failed-reload-stops-the-bridge

### BRIDGE_SKIP

    BRIDGE_SKIP-failed-init-skipped

### BRIDGE_UNSET

    BRIDGE_UNSET-flag-refused

### BRIDGE_TRUE

    BRIDGE_TRUE-a-module-pinned

### BRIDGE_FALSE

    BRIDGE_FALSE-a-reload-forbidden

### BRIDGE_ERR

    BRIDGE_ERR-nothing-above-an-error-written

### BRIDGE_WARN

    BRIDGE_WARN-a-warning-and-what-is-worse

### BRIDGE_INFO

    BRIDGE_INFO-information-and-what-is-worse

### BRIDGE_DEBUG

    BRIDGE_DEBUG-every-line-written

### BRIDGE_RUNNING

    BRIDGE_RUNNING-a-module-that-still-runs

### BRIDGE_STOPPED

    BRIDGE_STOPPED-a-module-at-its-failures

### BRIDGE_HOST_ABI

    BRIDGE_HOST_ABI-stated-and-created

### BRIDGE_NO_LIMIT

    BRIDGE_NO_LIMIT-stated-for-every-limit

### BRIDGE_NONE

    BRIDGE_NONE-for-a-limit-refused

### BRIDGE_NO_CONFIG

    BRIDGE_NO_CONFIG-with-settings-refused

### BRIDGE_LEVEL_MOST

    BRIDGE_LEVEL_MOST-every-line-written

### bridge_create

    bridge_create-no-sink-refused
    bridge_create-budget-sum-beyond-32-bits-refused
    bridge_create-two-bridges-at-once

### bridge_destroy

    bridge_destroy-shuts-down-what-is-loaded

### bridge_load

    bridge_load-loaded
    bridge_load-memory-with-a-data-segment
    bridge_load-init-trapped
    bridge_load-no-init-export
    bridge_load-failed-init-undone
    bridge_load-refused-leaves-nothing
    bridge_load-lifts-a-stopped-bridge
    bridge_load-no-reload-export
    bridge_load-loaded-already-refused
    bridge_load-import-nobody-answers

### bridge_unload

    bridge_unload-of-what-stopped-the-bridge
    bridge_unload-declarer-goes-user-holds-the-type
    bridge_unload-not-loaded-refused
    bridge_unload-after-a-failed-native-reload
    bridge_unload-two-hundred-rounds
    bridge_unload-share-given-back
    bridge_unload-share-kept-while-pointed-into

### bridge_reload

    bridge_reload-reloaded
    bridge_reload-room-for-one-system
    bridge_reload-failed-with-room-for-one-system
    bridge_reload-more-spawns-stated
    bridge_reload-wider-layout
    bridge_reload-reload-answered-nonzero
    bridge_reload-larger-share
    bridge_reload-larger-share-past-a-neighbour
    bridge_reload-same-share-exhausted
    bridge_reload-smaller-share-refused
    bridge_reload-share-more-than-the-arena-refused
    bridge_reload-lifts-a-stopped-bridge
    bridge_reload-to-another-backend-refused
    bridge_reload-dropped-declaration-frees-its-name
    bridge_reload-stopped-module-revived
    bridge_reload-not-loaded-refused
    bridge_reload-native
    bridge_reload-settings-row-kept
    bridge_reload-config-dropped

### bridge_list_modules

    bridge_list_modules-fills-no-more-than-asked
    bridge_list_modules-what-is-loaded-and-whether-it-runs

### bridge_run_phase

    bridge_run_phase-undeclared-phase-refused
    bridge_run_phase-wasm-beside-native
    bridge_run_phase-pinned-from-another-thread-refused

## The module seam

`include/bridge_abi.h`, `include/bridge_schema.h`, and the binding of each backend: `backends/wasm/sdk` and
`backends/native/sdk`. The eight entries are named as the ABI names them.

### bridge_term_t

    bridge_term_t-access-unset-refused
    bridge_term_t-access-neither-refused
    bridge_term_t-operator-unset-refused
    bridge_term_t-slot-undeclared-refused

### bridge_trigger_t

    bridge_trigger_t-who-triggered-and-how

### bridge_declaration_t

    bridge_declaration_t-name-outside-scratch-refused

### bridge_system_statement_t

    bridge_system_statement_t-phase-outside-scratch-refused
    bridge_system_statement_t-name-outside-scratch-refused

### bridge_observer_statement_t

    bridge_observer_statement_t-events-unset-refused

### bridge_header_t

    bridge_header_t-read-by-a-native-module

### bridge_query_t

    bridge_query_t-found-by-a-native-system

### bridge_text_t

    bridge_text_t-a-string-a-component-keeps

### bridge_list_t

    bridge_list_t-triggers-of-a-native-observer

### bridge_entity_t

    bridge_entity_t-index-read-by-a-native-module

### bridge_log_record_t

    bridge_log_record_t-written-by-a-module-and-drained

### BRIDGE_STRUCT

    BRIDGE_STRUCT-a-struct-a-field-names

### BRIDGE_ENUM

    BRIDGE_ENUM-an-enum-a-field-names

### BRIDGE_BITMASK

    BRIDGE_BITMASK-a-bitmask-a-field-names

### BRIDGE_TAG

    BRIDGE_TAG-carried-with-no-bytes
    BRIDGE_TAG-with-bytes-refused

### BRIDGE_STRIP

    BRIDGE_STRIP-unload-strips

### BRIDGE_SWEEP

    BRIDGE_SWEEP-unload-sweeps

### BRIDGE_DELETE

    BRIDGE_DELETE-unload-deletes

### BRIDGE_TABLE

    BRIDGE_TABLE-walked-by-its-rows

### BRIDGE_SPARSE_SET

    BRIDGE_SPARSE_SET-as-second-term-refused
    BRIDGE_SPARSE_SET-walked-and-reached

### BRIDGE_TOGGLE

    BRIDGE_TOGGLE-counted-on-and-off

### BRIDGE_DIRECT_INDEXED

    BRIDGE_DIRECT_INDEXED-as-second-term-refused
    BRIDGE_DIRECT_INDEXED-reached-from-rows

### BRIDGE_SINGLETON

    BRIDGE_SINGLETON-given-and-read
    BRIDGE_SINGLETON-walked-refused

### BRIDGE_READ

    BRIDGE_READ-a-term-that-reads

### BRIDGE_WRITE

    BRIDGE_WRITE-ungranted-refused

### BRIDGE_AND

    BRIDGE_AND-only-the-entities-that-carry-both

### BRIDGE_NOT

    BRIDGE_NOT-of-a-sparse-set-refused

### BRIDGE_OPTIONAL

    BRIDGE_OPTIONAL-a-term-an-entity-may-lack

### BRIDGE_ON_ENTER

    BRIDGE_ON_ENTER-told-with-the-attach-that-made-it

### BRIDGE_ON_ATTACH

    BRIDGE_ON_ATTACH-told-alone-of-an-attach-again

### BRIDGE_ON_LEAVE

    BRIDGE_ON_LEAVE-told-of-what-was-destroyed

### BRIDGE_LOG_ERR

    BRIDGE_LOG_ERR-written-as-severity-3

### BRIDGE_LOG_WARN

    BRIDGE_LOG_WARN-written-as-severity-4

### BRIDGE_LOG_INFO

    BRIDGE_LOG_INFO-written-as-severity-6

### BRIDGE_LOG_DEBUG

    BRIDGE_LOG_DEBUG-written-as-severity-7

### BRIDGE_ABI

    BRIDGE_ABI-stated-and-loaded

### BRIDGE_NAMESPACE

    BRIDGE_NAMESPACE-where-every-import-is

### BRIDGE_SHARED

    BRIDGE_SHARED-the-memory-two-modules-read

### BRIDGE_PRIVATE

    BRIDGE_PRIVATE-the-memory-of-one-module

### BRIDGE_CARRIED

    BRIDGE_CARRIED-where-a-tag-would-have-a-value

### BRIDGE_ZEROED

    BRIDGE_ZEROED-attached-with-no-value

### BRIDGE_SPAN_BASE

    BRIDGE_SPAN_BASE-where-a-span-keeps-its-columns

### BRIDGE_SPAN_SIZE

    BRIDGE_SPAN_SIZE-what-a-span-takes-of-the-spans

### BRIDGE_SCHEMA_TEXT

    BRIDGE_SCHEMA_TEXT-a-descriptor-of-the-type-s-own-text

### BRIDGE_SCHEMA_STRUCT_TYPE

    BRIDGE_SCHEMA_STRUCT_TYPE-laid-out-as-its-descriptor

### BRIDGE_SCHEMA_STRUCT_DESC

    BRIDGE_SCHEMA_STRUCT_DESC-read-and-agreed-with

### BRIDGE_SCHEMA_ENUM_TYPE

    BRIDGE_SCHEMA_ENUM_TYPE-of-the-integer-stated

### BRIDGE_SCHEMA_ENUM_DESC

    BRIDGE_SCHEMA_ENUM_DESC-read-and-agreed-with

### BRIDGE_SCHEMA_BITMASK_TYPE

    BRIDGE_SCHEMA_BITMASK_TYPE-of-the-integer-stated

### BRIDGE_SCHEMA_BITMASK_DESC

    BRIDGE_SCHEMA_BITMASK_DESC-read-and-agreed-with

### BRIDGE_DECLARE

    BRIDGE_DECLARE-an-enum-and-a-bitmask

### BRIDGE_COMPONENT

    BRIDGE_COMPONENT-declared-by-two-modules

### BRIDGE_DECLARE_TAG

    BRIDGE_DECLARE_TAG-carried-by-two-of-three

### BRIDGE_ADD_SYSTEM

    BRIDGE_ADD_SYSTEM-registered-and-run

### BRIDGE_OBSERVER

    BRIDGE_OBSERVER-registered-and-told

### BRIDGE_PUT_CHANGES

    BRIDGE_PUT_CHANGES-an-observer-that-attaches

### BRIDGE_ATTACH

    BRIDGE_ATTACH-the-value-travels-with-it

### BRIDGE_ROW

    BRIDGE_ROW-no-call-in-the-loop
    BRIDGE_ROW-checked-and-unchecked-timed

### BRIDGE_GET

    BRIDGE_GET-a-field-read

### BRIDGE_SET

    BRIDGE_SET-a-field-written-each-frame

### BRIDGE_IMPORT

    BRIDGE_IMPORT-the-entries-imported

### BRIDGE_ACC

    BRIDGE_ACC-the-accessors-merged-in

### BRIDGE_EXPORT

    BRIDGE_EXPORT-a-system-found-by-its-name

### BRIDGE_HEADER

    BRIDGE_HEADER-the-scratch-arena-found

### bridge_get_entity

    bridge_get_entity-the-id-of-a-row

### bridge_get_ref

    bridge_get_ref-own-entities-followed

### bridge_is_alive

    bridge_is_alive-ids-a-sweep-left

### bridge_get_sparse_index

    bridge_get_sparse_index-where-the-index-is

### bridge_get_sparse_value

    bridge_get_sparse_value-the-value-of-an-id

### bridge_get_single

    bridge_get_single-where-the-one-row-is

### bridge_get_cells

    bridge_get_cells-where-the-cells-are

### bridge_get_cell

    bridge_get_cell-the-cell-of-an-id

### bridge_line_t

    bridge_line_t-a-line-of-text-found-at-run-time

### BRIDGE_LOG

    BRIDGE_LOG-a-line-with-two-numbers

### bridge_log

    bridge_log-a-line-of-text-found-at-run-time

### bridge_count_triggers

    bridge_count_triggers-how-many-this-barrier

### bridge_get_trigger

    bridge_get_trigger-each-in-turn

### bridge_get_trigger_id

    bridge_get_trigger_id-who-it-was

### bridge_get_trigger_events

    bridge_get_trigger_events-enter-attach-leave

### BRIDGE_STATE

    BRIDGE_STATE-a-statement-put-in-scratch

### bridge_take_scratch

    bridge_take_scratch-more-than-given-stops-the-module

### bridge_put

    bridge_put-bytes-copied-into-scratch

### bridge_lend

    bridge_lend-scratch-given-back

### bridge_count_spans

    bridge_count_spans-one-table-matched

### bridge_get_span

    bridge_get_span-one-table-walked

### bridge_count_rows

    bridge_count_rows-rows-of-a-span

### bridge_get_entities

    bridge_get_entities-where-a-span-keeps-its-ids

### bridge_get_base

    bridge_get_base-where-a-column-begins

### bridge_get_bits

    bridge_get_bits-where-a-toggle-keeps-them

### bridge_is_on

    bridge_is_on-counted-by-its-bit

### bridge_turn

    bridge_turn-four-turned-off

### BRIDGE_NATIVE_ATTACH

    BRIDGE_NATIVE_ATTACH-seen-by-a-wasm-observer

### BRIDGE_NATIVE_DECLARE

    BRIDGE_NATIVE_DECLARE-an-enum-a-struct-names

### BRIDGE_NATIVE_COMPONENT

    BRIDGE_NATIVE_COMPONENT-declared

### BRIDGE_NATIVE_DECLARE_TAG

    BRIDGE_NATIVE_DECLARE_TAG-carried-with-no-bytes

### BRIDGE_NATIVE_ADD_SYSTEM

    BRIDGE_NATIVE_ADD_SYSTEM-registered-and-run

### BRIDGE_NATIVE_OBSERVER

    BRIDGE_NATIVE_OBSERVER-registered-and-told

### BRIDGE_NATIVE_PUT_CHANGES

    BRIDGE_NATIVE_PUT_CHANGES-changes-stated-and-made

### bridge_native_get_query

    bridge_native_get_query-the-query-of-a-system

### bridge_native_get_span

    bridge_native_get_span-two-tables-walked

### declare

    declare-storage-unset-refused
    declare-another-layout-refused
    declare-ungranted-namespace-refused
    declare-nested-types
    declare-undeclared-field-type-refused
    declare-statement-outside-scratch-refused
    declare-name-with-no-namespace-refused
    declare-kind-unset-refused
    declare-on-unload-unset-refused
    declare-size-other-than-descriptor-refused
    declare-outside-init-refused

### add_system

    add_system-unknown-operator-refused
    add_system-own-conflict-in-registration-order
    add_system-statement-outside-scratch-refused
    add_system-more-changes-than-types-refused
    add_system-name-twice-refused
    add_system-change-of-an-undeclared-slot-refused
    add_system-change-ungranted-refused
    add_system-outside-init-refused

### observe

    observe-native-attach-wasm-observer
    observe-chain-one-hop-a-barrier
    observe-reserved-name-refused
    observe-no-term-refused
    observe-statement-outside-scratch-refused
    observe-outside-init-refused

### spawn

    spawn-refused-fails-a-native-invocation
    spawn-stated-as-none-refused
    spawn-same-ids-on-one-thread-and-four

### destroy

    destroy-with-a-read-grant-left

### attach

    attach-singleton-to-an-id-refused
    attach-to-id-zero-refused
    attach-unstated-change-fails
    attach-undeclared-slot-refused
    attach-ungranted-refused
    attach-value-outside-scratch-refused

### detach

    detach-undeclared-slot-refused
    detach-queued-and-applied-at-the-barrier

### alloc

    alloc-string-kept
    alloc-share-exhausted
    alloc-stated-as-none-refused

### bridge_api_t

    bridge_api_t-entry-from-a-foreign-thread-refused

### BRIDGE_NATIVE

    BRIDGE_NATIVE-system-reads-and-writes

### BRIDGE_NATIVE_STATE

    BRIDGE_NATIVE_STATE-a-statement-put-in-scratch

### bridge_native_get_header

    bridge_native_get_header-read-through-a-pointer

### bridge_native_put

    bridge_native_put-bytes-copied-into-scratch

### bridge_native_lend

    bridge_native_lend-a-value-lent-to-attach

### bridge_native_get_single

    bridge_native_get_single-the-one-row

### bridge_native_get_cells

    bridge_native_get_cells-reached-by-an-id

### bridge_native_get_sparse_index

    bridge_native_get_sparse_index-reached-by-an-id

### bridge_native_get_bits

    bridge_native_get_bits-read-and-turned

### bridge_native_get_list

    bridge_native_get_list-triggers-of-an-observer

### bridge_native_log

    bridge_native_log-a-line-with-two-numbers

## The backend seam

`include/bridge_backend.h`: the eighteen entries a backend answers, and what they are
handed.

### backend_err_t

    backend_err_t-why-an-entry-failed

### backend_value_t

    backend_value_t-handed-over-unread

### backend_setting_t

    backend_setting_t-a-key-the-backend-refuses

### backend_sig_t

    backend_sig_t-an-i32-and-an-i64-answered

### backend_import_t

    backend_import_t-the-two-memories-found

### backend_memory_import_t

    backend_memory_import_t-one-block-in-two-stores

### backend_source_t

    backend_source_t-no-bytes-refused

### backend_entry_t

    backend_entry_t-what-answers-an-import

### backend_get_abi

    backend_get_abi-another-refused

### backend_create

    backend_create-unknown-setting-refused

### backend_destroy

    backend_destroy-the-library-leaves-the-process

### backend_get_capabilities

    backend_get_capabilities-held-against-what-a-module-states

### memory_get_shape

    memory_get_shape-the-block-reserved-as-asked

### memory_adopt

    memory_adopt-refused-refuses-the-bridge

### memory_release

    memory_release-after-the-last-instance

### module_compile

    module_compile-no-path-refused

### module_list_imports

    module_list_imports-the-two-memories-lent

### module_destroy

    module_destroy-a-failed-load-leaves-nothing

### instance_create

    instance_create-one-block-in-two-stores

### instance_destroy

    instance_destroy-an-unload-gives-its-share-back

### instance_set_work_limit

    instance_set_work_limit-a-spinning-system-ended

### instance_set_timeout

    instance_set_timeout-a-spinning-init-interrupted

### instance_set_memory_limit

    instance_set_memory_limit-growth-refused

### define_func

    define_func-called-by-a-module

### instance_get_func

    instance_get_func-export-taking-an-argument-refused
    instance_get_func-export-answering-64-bits-refused

### call

    call-a-trap-answered-and-the-store-runs-on

### BACKEND_UNTRUSTED

    BACKEND_UNTRUSTED-lacking-untrusted-refused

### BACKEND_WORK_LIMIT

    BACKEND_WORK_LIMIT-lacking-work-limit-refused

### BACKEND_TIMEOUT

    BACKEND_TIMEOUT-lacking-timeout-refused

### BACKEND_MEMORY_LIMIT

    BACKEND_MEMORY_LIMIT-lacking-memory-limit-refused

### BACKEND_RELOAD

    BACKEND_RELOAD-a-module-replaced-while-the-bridge-runs

### BACKEND_SIDE_BY_SIDE

    BACKEND_SIDE_BY_SIDE-lacking-failed-reload-stops-the-module

### BACKEND_UNLOAD

    BACKEND_UNLOAD-library-gone-or-kept

### BACKEND_BOOL

    BACKEND_BOOL-read-as-it-was-stated

### BACKEND_INT

    BACKEND_INT-read-as-it-was-stated

### BACKEND_FLOAT

    BACKEND_FLOAT-read-as-it-was-stated

### BACKEND_STRING

    BACKEND_STRING-read-as-it-was-stated

### BACKEND_I32

    BACKEND_I32-an-argument-of-32-bits

### BACKEND_I64

    BACKEND_I64-an-argument-of-64-bits

### BACKEND_IMPORT_FUNC

    BACKEND_IMPORT_FUNC-an-entry-imported-is-no-memory

### BACKEND_IMPORT_MEMORY

    BACKEND_IMPORT_MEMORY-the-two-memories-lent

### BACKEND_IMPORT_OTHER

    BACKEND_IMPORT_OTHER-a-global-nobody-answers

### BACKEND_ABI

    BACKEND_ABI-answered-and-the-library-opened

### BACKEND_ENTRY_COUNT

    BACKEND_ENTRY_COUNT-an-entry-missing-refused

## What several of them do together

### order

Two systems that conflict in one phase, or two observers at one barrier, run in an order the
deployment stated, and a pair nothing orders is refused.

    order-unordered-conflict-refused
    order-unordered-observers-refused
    order-unordered-changes-refused
    order-changes-ordered-loaded-first
    order-changes-ordered-loaded-last

### static

The library linked statically: what a host does first, again.

    static-bridge_load-loaded
    static-BRIDGE_NO_LIMIT-stated-for-every-limit
    static-bridge_load-memory-with-a-data-segment
    static-bridge_load-init-trapped
    static-module_config.init_timeout-init-interrupted
    static-module_config.memory_limit-growth-refused
    static-module_config.memory_limit-memory-larger-at-start
    static-instance_get_func-export-taking-an-argument-refused
    static-instance_get_func-export-answering-64-bits-refused
    static-bridge_load-no-init-export
    static-module_config.abi-another-refused
    static-artifact.backend-not-configured-refused
    static-module_config.log_level-unset-refused
    static-module_config.memory_limit-unset-refused
    static-BRIDGE_NONE-for-a-limit-refused
    static-config.abi-another-refused
    static-config.abi-unset-refused
    static-BRIDGE_UNSET-flag-refused
    static-module_config.memory_limit-negative-refused
    static-module_config.on_error-fail-stops-and-skip-does-not
    static-backend.library-cannot-open-refused
    static-bridge_create-no-sink-refused

## Inside the library

### storage

The storage, reached from inside and held to a transcript.

    storage-changes
    storage-shape
    storage-tables
    storage-stress
    storage-stress1
    storage-notables
    storage-nocolumns
    storage-scribble
    storage-strip
    storage-stripfull
    storage-weigh
    storage-toggle
    storage-togglefull
    storage-move
    storage-layout
    storage-direct
    storage-directstrip
    storage-directsweep
    storage-directdelete
    storage-sparse
    storage-sparsefull
    storage-sparsestrip
    storage-sparsesweep
    storage-sparsesweepa
    storage-sparsedelete
    storage-baseline
    storage-measure

### allocation

The library built with its calls to the allocator counted.

    allocation-changes
    allocation-tables
    allocation-strip
    allocation-each-refused-in-turn
    allocation-each-refused-in-turn-laying-out-again

### schema

The reader of descriptors: each that is read, and each that is refused.

    schema

### adopt

A backend, called as the library calls it.

    adopt

## The tree

### c23

What C23 states is used: `tools/c23.py`.

    c23

### style

The coding style holds: `tools/style.py`.

    style

### deps

What the built library imports is what `dependencies.md` lists: `tools/deps.py`.

    deps

### said

Every message of the source was written by a case: `tools/said.py`.

    said

### cover

This document holds against the headers and the cases: `tools/cover.py`.

    cover
