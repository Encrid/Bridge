;; init reads past its own memory: a trap, and the process goes on.
(module
  (memory 1)
  (func (export "init") (result i32) (i32.load (i32.const 70000))))
