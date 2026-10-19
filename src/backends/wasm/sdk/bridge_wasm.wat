;; The accessors a compiled wasm module reaches Bridge's two memories through.
;; docs/backends/wasm/wasm.md, *A compiled module reaches them through accessors*.
;;
;; No C compiler can emit a load from a memory other than the module's own, so these are
;; written where a load names its memory. The module build merges this file into the compiled
;; module as "acc", inlines every function here into its callers, and strips the exports.
;; bridge_wasm.h declares them.
;;
;; What is merged with this file is a work based on BB-Wasm and is under the same licence,
;; the GNU Affero General Public License, version 3 only: see license. Other terms:
;; docs/licence.md.
(module
  (import "bridge" "shared" (memory $shared 1))
  (import "bridge" "private" (memory $private 1))
  (import "main" "memory" (memory $own 0))

  ;; Where a row is, twice: carrying the test that it is one of the span's rows, and not. A
  ;; load past them is still inside the shared memory, which wasm permits, so the test is the
  ;; only thing that catches one. Which of the two a module is built with is its author's to
  ;; state: bridge_wasm.h, BRIDGE_ROWS_CHECKED and BRIDGE_ROWS_UNCHECKED.
  (func (export "acc_row") (param $base i32) (param $row i32) (param $count i32) (param $stride i32)
        (result i32)
    (if (i32.ge_u (local.get $row) (local.get $count)) (then (unreachable)))
    (i32.add (local.get $base) (i32.mul (local.get $row) (local.get $stride))))
  (func (export "acc_row_unchecked") (param $base i32) (param $row i32) (param $count i32)
        (param $stride i32) (result i32)
    (i32.add (local.get $base) (i32.mul (local.get $row) (local.get $stride))))

  ;; the shared memory, by width
  (func (export "acc_ld_i8")  (param i32) (result i32) (i32.load8_s  $shared (local.get 0)))
  (func (export "acc_ld_u8")  (param i32) (result i32) (i32.load8_u  $shared (local.get 0)))
  (func (export "acc_ld_i16") (param i32) (result i32) (i32.load16_s $shared (local.get 0)))
  (func (export "acc_ld_u16") (param i32) (result i32) (i32.load16_u $shared (local.get 0)))
  (func (export "acc_ld_32")  (param i32) (result i32) (i32.load     $shared (local.get 0)))
  (func (export "acc_ld_64")  (param i32) (result i64) (i64.load     $shared (local.get 0)))
  (func (export "acc_ld_f32") (param i32) (result f32) (f32.load     $shared (local.get 0)))
  (func (export "acc_ld_f64") (param i32) (result f64) (f64.load     $shared (local.get 0)))
  (func (export "acc_st_8")   (param i32 i32) (i32.store8  $shared (local.get 0) (local.get 1)))
  (func (export "acc_st_16")  (param i32 i32) (i32.store16 $shared (local.get 0) (local.get 1)))
  (func (export "acc_st_32")  (param i32 i32) (i32.store   $shared (local.get 0) (local.get 1)))
  (func (export "acc_st_64")  (param i32 i64) (i64.store   $shared (local.get 0) (local.get 1)))
  (func (export "acc_st_f32") (param i32 f32) (f32.store   $shared (local.get 0) (local.get 1)))
  (func (export "acc_st_f64") (param i32 f64) (f64.store   $shared (local.get 0) (local.get 1)))

  ;; the private memory: the header, the spans, the ring
  (func (export "acc_own_32") (param i32) (result i32) (i32.load $private (local.get 0)))
  (func (export "acc_own_64") (param i32) (result i64) (i64.load $private (local.get 0)))
  (func (export "acc_own_st") (param i32 i32) (i32.store $private (local.get 0) (local.get 1)))

  ;; bytes between the module's own memory and the other two
  (func (export "acc_to_private") (param $to i32) (param $from i32) (param $len i32)
    (memory.copy $private $own (local.get $to) (local.get $from) (local.get $len)))
  (func (export "acc_to_shared") (param $to i32) (param $from i32) (param $len i32)
    (memory.copy $shared $own (local.get $to) (local.get $from) (local.get $len)))
  (func (export "acc_from_shared") (param $to i32) (param $from i32) (param $len i32)
    (memory.copy $own $shared (local.get $to) (local.get $from) (local.get $len))))
