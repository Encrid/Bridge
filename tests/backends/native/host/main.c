/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const host_case_t TEST_CASES[] = {
    { "BRIDGE_NATIVE-system-reads-and-writes", test_run_mover, "", test_set },
    { "test:cover-the-sdk-guards", test_run_cover, "", test_set },
    { "BRIDGE_NATIVE_COMPONENT-declared", test_run_mover, "", test_set },
    { "BRIDGE_NATIVE_ADD_SYSTEM-registered-and-run",
        test_run_mover, "", test_set },
    { "bridge_native_get_query-the-query-of-a-system",
        test_run_mover, "", test_set },
    { "bridge_native_get_span-two-tables-walked",
        test_run_mover, "", test_set },
    { "bridge_native_log-a-line-with-two-numbers",
        test_run_mover, "", test_set },
    { "BRIDGE_NATIVE_ATTACH-seen-by-an-observer", test_observe, "", test_set },
    { "bridge_native_lend-a-value-lent-to-attach",
        test_observe, "", test_set },
    { "BRIDGE_NATIVE_PUT_CHANGES-changes-stated-and-made",
        test_order_changes, "", test_set },
    { "bridge_api_t-entry-from-a-foreign-thread-refused",
        test_enter_from_foreign_thread, "", test_set },
    { "call-an-entry-refused-fails-the-invocation",
        test_refuse_entry, "", test_set },
    { "backend_get_capabilities-untrusted-refused",
        test_state_untrusted, "", test_set },
    { "backend_get_capabilities-work-limit-refused",
        test_state_limit, "w", test_set },
    { "backend_get_capabilities-memory-limit-refused",
        test_state_limit, "m", test_set },
    { "backend_get_capabilities-timeout-refused",
        test_state_limit, "t", test_set },
    { "module_compile-no-path-refused", test_state_no_path, "", test_set },
    { "module_compile-library-cannot-open",
        test_name_no_library, "", test_set },
    { "backend_create-a-setting-refused", test_hand_setting, "", test_set },
    { "bridge_native_get_single-the-one-row",
        test_read_kept, "single", test_set },
    { "bridge_native_get_bits-read-and-turned",
        test_read_kept, "toggle", test_set },
    { "BRIDGE_NATIVE_DECLARE_TAG-carried-with-no-bytes",
        test_read_kept, "tag", test_set },
    { "bridge_native_get_cells-reached-by-an-id",
        test_read_kept, "direct", test_set },
    { "bridge_native_get_sparse_index-reached-by-an-id",
        test_read_kept, "sparse", test_set },
    { "BRIDGE_NATIVE_DECLARE-an-enum-a-struct-names",
        test_read_kept, "layout", test_set },
    { "bridge_native_get_list-triggers-of-an-observer",
        test_observe, "", test_set },
    { "BRIDGE_NATIVE_OBSERVER-registered-and-told",
        test_observe, "", test_set },
    { "bridge_native_get_header-read-through-a-pointer",
        test_observe, "", test_set },
    { "BRIDGE_NATIVE_STATE-a-statement-put-in-scratch",
        test_run_mover, "", test_set },
    { "bridge_native_put-bytes-copied-into-scratch",
        test_run_mover, "", test_set },
    { "bridge_line_t-a-line-with-its-two-numbers",
        test_run_mover, "", test_set },
    { "declare-two-components-declared", test_run_mover, "", test_set },
    { "add_system-registered-and-run", test_run_mover, "", test_set },
    { "spawn-a-thousand-made-at-init", test_run_mover, "", test_set },
    { "observe-told-of-an-attach", test_observe, "", test_set },
    { "attach-a-value-given-again", test_read_kept, "changed", test_set },
    { "detach-a-component-taken", test_read_kept, "changed", test_set },
    { "destroy-two-entities-gone", test_read_kept, "changed", test_set },
    { "alloc-bytes-kept-from-init", test_keep_bytes, "", test_set },
    { "instance_get_func-no-such-export-refused",
        test_load_no_init, "", test_set },
};

int main(int argc, char **argv)
{
    host_t h;

    host_init(&h, argc, argv);
    return (host_run(&h, TEST_CASES, sizeof(TEST_CASES) / sizeof(*TEST_CASES)));
}
