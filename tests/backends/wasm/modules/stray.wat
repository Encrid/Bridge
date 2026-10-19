;; A module that imports a function nobody offers: it cannot be instantiated.
(module
  (import "elsewhere" "nothing" (func $nothing))
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
