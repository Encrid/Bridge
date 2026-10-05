;; A module with init and shutdown and no reload: one of the three exports every module has.
(module
  (memory 1)
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0)))
