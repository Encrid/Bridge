;; Calls a function the host defines: an i32 and an i64 in, an i32 out.
(module
  (import "bridge" "ping" (func $ping (param i32 i64) (result i32)))
  (func (export "init") (result i32)
    (call $ping (i32.const 7) (i64.const 9))))
