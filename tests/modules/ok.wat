;; A module that loads: its init answers 0.
(module
  (memory 1)
  (func (export "init") (result i32) (i32.const 0)))
