;; A start that traps: the module compiles, but its first call cannot instantiate it.
(module
  (func $boom unreachable)
  (start $boom)
  (func (export "init") (result i32) (i32.const 0)))
