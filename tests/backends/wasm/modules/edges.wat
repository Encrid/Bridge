;; Two exports the backend must weigh: a global, and a function that is not () -> i32.
(module
  (global (export "glob") i32 (i32.const 0))
  (func (export "voidinit")))
