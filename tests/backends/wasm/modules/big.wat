;; Its own memory starts larger than its limit: it cannot be instantiated at all.
(module
  (memory 64)
  (func (export "init") (result i32) (i32.const 0)))
