;; Two pages of a block Bridge owns, imported as the shared memory.
;; Each function is one thing tests/adopt.c needs a module to do to it.
(module
  (import "bridge" "shared" (memory 2 2))

  (func (export "write") (result i32)
    (i32.store (i32.const 16) (i32.const 0x600DF00D))
    (i32.const 0))

  ;; byte by byte, so that two stores doing this at once really interleave
  (func $fill (param $from i32) (param $value i32)
    (local $i i32)
    (loop $next
      (i32.store8 (i32.add (local.get $from) (local.get $i)) (local.get $value))
      (local.set $i (i32.add (local.get $i) (i32.const 1)))
      (br_if $next (i32.lt_u (local.get $i) (i32.const 65536)))))

  (func (export "fill_low") (result i32)
    (call $fill (i32.const 0) (i32.const 0xAA))
    (i32.const 0))

  (func (export "fill_high") (result i32)
    (call $fill (i32.const 65536) (i32.const 0xBB))
    (i32.const 0))

  ;; what this store sees in the page the other store filled
  (func (export "sum_high") (result i32)
    (local $i i32) (local $sum i32)
    (loop $next
      (local.set $sum (i32.add (local.get $sum)
        (i32.load8_u (i32.add (i32.const 65536) (local.get $i)))))
      (local.set $i (i32.add (local.get $i) (i32.const 1)))
      (br_if $next (i32.lt_u (local.get $i) (i32.const 65536))))
    (local.get $sum))

  ;; one byte past the block: nothing is mapped there
  (func (export "stray") (result i32)
    (i32.load8_u (i32.const 131072))))
