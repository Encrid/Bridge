;; The accessors a compiled wasm module reaches Bridge's two memories through.
;; docs/intern/backends/wasm/sdk/bridge_wasm.wat.md.
;;
;; No C compiler can emit a load from a memory other than the module's own, so these are
;; written where a load names its memory. The module build merges this file into the compiled
;; module as "acc", inlines every function here into its callers, and strips the exports. A
;; module therefore reaches the shared memory only through $resolve, which bounds every access
;; to a column the module declared; it never holds a shared address.
;; bridge_wasm_acc.h declares them.
;;
;; What is merged with this file is a work based on BB-Wasm and is under the same licence,
;; the GNU Affero General Public License, version 3 only: see license. Other terms:
;; docs/licence.md.
(module
  (import "bridge" "shared" (memory $shared 1))
  (import "bridge" "private" (memory $private 1))
  (import "main" "memory" (memory $own 0))

  ;; $resolve maps a capability coordinate to a shared offset, or traps. It reads the per-module
  ;; capability table the host publishes each barrier into the private memory (header words at
  ;; fixed offsets: types@40, tables@44, entity_index@48, entities@52, column_base@36, kept@80,
  ;; rows@84, blocks@88, stride@92, write@96, kind@100). The module names a column by its declared
  ;; slot; ab packs (table << 32 | row) for a table or toggle column, or (entity id << 32) for a
  ;; sparse or direct-indexed component; field is the offset inside the cell.
  (func $resolve (param $slot i32) (param $ab i64) (param $field i32)
        (param $width i32) (param $store i32) (result i32)
    (local $a i32) (local $b i32) (local $types i32) (local $tables i32)
    (local $kind i32) (local $stride i32) (local $base i32) (local $cell i32)
    (local.set $a (i32.wrap_i64 (i64.shr_u (local.get $ab) (i64.const 32))))
    (local.set $b (i32.wrap_i64 (local.get $ab)))
    (local.set $types (i32.load $private (i32.const 40)))
    (if (i32.ge_u (local.get $slot) (local.get $types)) (then (unreachable)))
    (local.set $tables (i32.load $private (i32.const 44)))
    (local.set $stride (i32.load $private
      (i32.add (i32.load $private (i32.const 92)) (i32.mul (local.get $slot) (i32.const 4)))))
    (if (local.get $store)
      (then (if (i32.eqz (i32.load $private
              (i32.add (i32.load $private (i32.const 96)) (i32.mul (local.get $slot) (i32.const 4)))))
              (then (unreachable)))))
    (if (i32.gt_u (i32.add (local.get $field) (local.get $width)) (local.get $stride))
      (then (unreachable)))
    (local.set $kind (i32.load $private
      (i32.add (i32.load $private (i32.const 100)) (i32.mul (local.get $slot) (i32.const 4)))))
    (if (i32.or (i32.eq (local.get $kind) (i32.const 1)) (i32.eq (local.get $kind) (i32.const 3)))
      (then
        (if (i32.ge_u (local.get $a) (local.get $tables)) (then (unreachable)))
        (local.set $base (i32.load $private
          (i32.add (i32.load $private (i32.const 36))
            (i32.mul (i32.const 4)
              (i32.add (i32.mul (local.get $a) (local.get $types)) (local.get $slot))))))
        (if (i32.eqz (local.get $base)) (then (unreachable)))
        (if (i32.ge_u (local.get $b) (i32.load $private
              (i32.add (i32.load $private (i32.const 84)) (i32.mul (local.get $a) (i32.const 4)))))
          (then (unreachable)))
        (return (i32.add (local.get $base)
          (i32.add (i32.mul (local.get $b) (local.get $stride)) (local.get $field))))))
    (if (i32.eq (local.get $kind) (i32.const 5))
      (then
        (local.set $base (i32.load $private
          (i32.add (i32.load $private (i32.const 80)) (i32.mul (local.get $slot) (i32.const 4)))))
        (if (i32.eqz (local.get $base)) (then (unreachable)))
        (return (i32.add (local.get $base) (local.get $field)))))
    (if (i32.ge_u (local.get $a) (i32.load $private (i32.const 52))) (then (unreachable)))
    (local.set $base (i32.load $private
      (i32.add (i32.load $private (i32.const 80)) (i32.mul (local.get $slot) (i32.const 4)))))
    (if (i32.eq (local.get $kind) (i32.const 2))
      (then
        (local.set $cell (i32.load $shared
          (i32.add (local.get $base) (i32.mul (local.get $a) (i32.const 4)))))
        (if (i32.eqz (local.get $cell)) (then (unreachable)))
        (return (i32.add (local.get $cell) (local.get $field)))))
    (if (i32.eq (local.get $kind) (i32.const 4))
      (then
        (if (i32.eqz (i32.and (i32.wrap_i64 (i64.shr_u
              (i64.load $shared (i32.add (i32.load $shared (i32.sub (local.get $base) (i32.const 16)))
                (i32.mul (i32.shr_u (local.get $a) (i32.const 6)) (i32.const 8))))
              (i64.extend_i32_u (i32.and (local.get $a) (i32.const 63))))) (i32.const 1)))
          (then (unreachable)))
        (return (i32.add (local.get $base)
          (i32.add (i32.mul (local.get $a) (local.get $stride)) (local.get $field))))))
    (unreachable))

  ;; the shared memory, by width
  (func (export "acc_ld_i8")  (param $slot i32)(param $ab i64)(param $field i32)(result i32)
    (i32.load8_s  $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 1)(i32.const 0))))
  (func (export "acc_ld_u8")  (param $slot i32)(param $ab i64)(param $field i32)(result i32)
    (i32.load8_u  $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 1)(i32.const 0))))
  (func (export "acc_ld_i16") (param $slot i32)(param $ab i64)(param $field i32)(result i32)
    (i32.load16_s $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 2)(i32.const 0))))
  (func (export "acc_ld_u16") (param $slot i32)(param $ab i64)(param $field i32)(result i32)
    (i32.load16_u $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 2)(i32.const 0))))
  (func (export "acc_ld_32")  (param $slot i32)(param $ab i64)(param $field i32)(result i32)
    (i32.load     $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 4)(i32.const 0))))
  (func (export "acc_ld_64")  (param $slot i32)(param $ab i64)(param $field i32)(result i64)
    (i64.load     $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 8)(i32.const 0))))
  (func (export "acc_ld_f32") (param $slot i32)(param $ab i64)(param $field i32)(result f32)
    (f32.load     $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 4)(i32.const 0))))
  (func (export "acc_ld_f64") (param $slot i32)(param $ab i64)(param $field i32)(result f64)
    (f64.load     $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 8)(i32.const 0))))
  (func (export "acc_st_8")   (param $slot i32)(param $ab i64)(param $field i32)(param $v i32)
    (i32.store8  $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 1)(i32.const 1)) (local.get $v)))
  (func (export "acc_st_16")  (param $slot i32)(param $ab i64)(param $field i32)(param $v i32)
    (i32.store16 $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 2)(i32.const 1)) (local.get $v)))
  (func (export "acc_st_32")  (param $slot i32)(param $ab i64)(param $field i32)(param $v i32)
    (i32.store   $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 4)(i32.const 1)) (local.get $v)))
  (func (export "acc_st_64")  (param $slot i32)(param $ab i64)(param $field i32)(param $v i64)
    (i64.store   $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 8)(i32.const 1)) (local.get $v)))
  (func (export "acc_st_f32") (param $slot i32)(param $ab i64)(param $field i32)(param $v f32)
    (f32.store   $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 4)(i32.const 1)) (local.get $v)))
  (func (export "acc_st_f64") (param $slot i32)(param $ab i64)(param $field i32)(param $v f64)
    (f64.store   $shared (call $resolve (local.get $slot)(local.get $ab)(local.get $field)(i32.const 8)(i32.const 1)) (local.get $v)))

  ;; an entity id -> (table << 32 | row), or -1 if it is not alive
  (func (export "acc_entity") (param $id i64) (result i64)
    (local $index i32) (local $rec i32)
    (local.set $index (i32.wrap_i64 (local.get $id)))
    (if (i32.ge_u (local.get $index) (i32.load $private (i32.const 52)))
      (then (return (i64.const -1))))
    (local.set $rec (i32.add (i32.load $private (i32.const 48))
      (i32.mul (local.get $index) (i32.const 12))))
    (if (i32.ne (i32.load $shared (i32.add (local.get $rec) (i32.const 8)))
                (i32.wrap_i64 (i64.shr_u (local.get $id) (i64.const 32))))
      (then (return (i64.const -1))))
    (i64.or (i64.shl (i64.extend_i32_u (i32.load $shared (local.get $rec))) (i64.const 32))
            (i64.extend_i32_u (i32.load $shared (i32.add (local.get $rec) (i32.const 4))))))

  ;; the entity id at (table, row)
  (func (export "acc_who") (param $table i32) (param $row i32) (result i64)
    (if (i32.ge_u (local.get $table) (i32.load $private (i32.const 44))) (then (unreachable)))
    (if (i32.ge_u (local.get $row) (i32.load $private
          (i32.add (i32.load $private (i32.const 84)) (i32.mul (local.get $table) (i32.const 4)))))
      (then (unreachable)))
    (i64.load $shared (i32.add (i32.load $private
          (i32.add (i32.load $private (i32.const 88)) (i32.mul (local.get $table) (i32.const 4))))
        (i32.mul (local.get $row) (i32.const 8)))))

  ;; whether a sparse-set, direct-indexed, or singleton component is present (by entity id; a
  ;; singleton ignores the id). A table column's presence is column_base != 0, which a module reads
  ;; directly from its private memory (bridge_base).
  (func (export "acc_has") (param $slot i32) (param $id i32) (result i32)
    (local $kind i32) (local $base i32)
    (if (i32.ge_u (local.get $slot) (i32.load $private (i32.const 40))) (then (return (i32.const 0))))
    (local.set $kind (i32.load $private
      (i32.add (i32.load $private (i32.const 100)) (i32.mul (local.get $slot) (i32.const 4)))))
    (if (i32.eq (local.get $kind) (i32.const 5))
      (then (return (i32.ne (i32.load $private
        (i32.add (i32.load $private (i32.const 80)) (i32.mul (local.get $slot) (i32.const 4))))
        (i32.const 0)))))
    (if (i32.ge_u (local.get $id) (i32.load $private (i32.const 52))) (then (return (i32.const 0))))
    (local.set $base (i32.load $private
      (i32.add (i32.load $private (i32.const 80)) (i32.mul (local.get $slot) (i32.const 4)))))
    (if (i32.eq (local.get $kind) (i32.const 2))
      (then (return (i32.ne (i32.load $shared
        (i32.add (local.get $base) (i32.mul (local.get $id) (i32.const 4)))) (i32.const 0)))))
    (if (i32.eq (local.get $kind) (i32.const 4))
      (then (return (i32.and (i32.wrap_i64 (i64.shr_u
        (i64.load $shared (i32.add (i32.load $shared (i32.sub (local.get $base) (i32.const 16)))
          (i32.mul (i32.shr_u (local.get $id) (i32.const 6)) (i32.const 8))))
        (i64.extend_i32_u (i32.and (local.get $id) (i32.const 63))))) (i32.const 1)))))
    (i32.const 0))

  ;; a toggle component's presence bit at (table, row): the bits live below its column base, whose
  ;; offset the host stored at base-16.
  (func $toggle_bits (param $slot i32) (param $table i32) (param $row i32) (result i32)
    (local $types i32) (local $base i32)
    (local.set $types (i32.load $private (i32.const 40)))
    (if (i32.ge_u (local.get $slot) (local.get $types)) (then (unreachable)))
    (if (i32.ne (i32.load $private
          (i32.add (i32.load $private (i32.const 100)) (i32.mul (local.get $slot) (i32.const 4))))
          (i32.const 3))
      (then (unreachable)))
    (if (i32.ge_u (local.get $table) (i32.load $private (i32.const 44))) (then (unreachable)))
    (local.set $base (i32.load $private
      (i32.add (i32.load $private (i32.const 36))
        (i32.mul (i32.const 4)
          (i32.add (i32.mul (local.get $table) (local.get $types)) (local.get $slot))))))
    (if (i32.eqz (local.get $base)) (then (unreachable)))
    (if (i32.ge_u (local.get $row) (i32.load $private
          (i32.add (i32.load $private (i32.const 84)) (i32.mul (local.get $table) (i32.const 4)))))
      (then (unreachable)))
    (i32.add (i32.load $shared (i32.sub (local.get $base) (i32.const 16)))
      (i32.mul (i32.shr_u (local.get $row) (i32.const 6)) (i32.const 8))))
  (func (export "acc_toggle") (param $slot i32) (param $table i32) (param $row i32) (result i32)
    (i32.and (i32.wrap_i64 (i64.shr_u
      (i64.load $shared (call $toggle_bits (local.get $slot) (local.get $table) (local.get $row)))
      (i64.extend_i32_u (i32.and (local.get $row) (i32.const 63))))) (i32.const 1)))
  (func (export "acc_toggle_set") (param $slot i32) (param $table i32) (param $row i32) (param $on i32)
    (local $at i32) (local $word i64) (local $bit i64)
    (if (i32.eqz (i32.load $private
          (i32.add (i32.load $private (i32.const 96)) (i32.mul (local.get $slot) (i32.const 4)))))
      (then (unreachable)))
    (local.set $at (call $toggle_bits (local.get $slot) (local.get $table) (local.get $row)))
    (local.set $bit (i64.shl (i64.const 1) (i64.extend_i32_u (i32.and (local.get $row) (i32.const 63)))))
    (local.set $word (i64.load $shared (local.get $at)))
    (i64.store $shared (local.get $at)
      (if (result i64) (local.get $on)
        (then (i64.or (local.get $word) (local.get $bit)))
        (else (i64.and (local.get $word) (i64.xor (local.get $bit) (i64.const -1)))))))

  ;; the private memory: the header, the spans, the ring. Reads are free; a write may not touch
  ;; the capability table (offset >= queries@32, which is the start of the published region) nor a
  ;; header word other than scratch_used@12 and ring_head@24.
  (func (export "acc_own_32") (param i32) (result i32) (i32.load $private (local.get 0)))
  (func (export "acc_own_64") (param i32) (result i64) (i64.load $private (local.get 0)))
  (func (export "acc_own_st") (param $at i32) (param $v i32)
    (if (i32.or
          (i32.ge_u (local.get $at) (i32.load $private (i32.const 32)))
          (i32.and (i32.lt_u (local.get $at) (i32.const 128))
            (i32.and (i32.ne (local.get $at) (i32.const 12)) (i32.ne (local.get $at) (i32.const 24)))))
      (then (unreachable)))
    (i32.store $private (local.get $at) (local.get $v)))
  (func (export "acc_to_private") (param $to i32) (param $from i32) (param $len i32)
    (if (i32.or (i32.lt_u (local.get $to) (i32.const 128))
          (i32.gt_u (i32.add (local.get $to) (local.get $len)) (i32.load $private (i32.const 32))))
      (then (unreachable)))
    (memory.copy $private $own (local.get $to) (local.get $from) (local.get $len)))

  ;; bytes between the module's own memory and the shared memory. A write stays inside one of the
  ;; module's owned arena ranges; a read stays inside the arena region [arena_lo@108, arena_hi@112).
  (func (export "acc_to_shared") (param $to i32) (param $from i32) (param $len i32)
    (local $o i32) (local $n i32) (local $i i32) (local $at i32) (local $end i32)
    (local.set $o (i32.load $private (i32.const 104)))
    (local.set $n (i32.load $private (local.get $o)))
    (local.set $end (i32.add (local.get $to) (local.get $len)))
    (block $ok
      (loop $next
        (if (i32.ge_u (local.get $i) (local.get $n)) (then (unreachable)))
        (local.set $at (i32.load $private
          (i32.add (local.get $o) (i32.add (i32.const 4) (i32.mul (local.get $i) (i32.const 8))))))
        (if (i32.and (i32.le_u (local.get $at) (local.get $to))
              (i32.le_u (local.get $end) (i32.add (local.get $at) (i32.load $private
                (i32.add (local.get $o) (i32.add (i32.const 8) (i32.mul (local.get $i) (i32.const 8))))))))
          (then (br $ok)))
        (local.set $i (i32.add (local.get $i) (i32.const 1)))
        (br $next)))
    (memory.copy $shared $own (local.get $to) (local.get $from) (local.get $len)))
  (func (export "acc_from_shared") (param $to i32) (param $from i32) (param $len i32)
    (if (i32.or (i32.lt_u (local.get $from) (i32.load $private (i32.const 108)))
          (i32.gt_u (i32.add (local.get $from) (local.get $len)) (i32.load $private (i32.const 112))))
      (then (unreachable)))
    (memory.copy $own $shared (local.get $to) (local.get $from) (local.get $len))))
