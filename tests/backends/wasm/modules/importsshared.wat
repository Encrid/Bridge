;; A module that imports the shared memory directly, which the build checker refuses: a module
;; reaches shared data only through the accessors.
(module
  (import "bridge" "shared" (memory 1))
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
