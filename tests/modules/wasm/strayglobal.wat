;; A module that imports a global nobody offers: an import that is neither a function nor a
;; memory, and it cannot be instantiated.
(module
  (import "elsewhere" "number" (global i32))
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
