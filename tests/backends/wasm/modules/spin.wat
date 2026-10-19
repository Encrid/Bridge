;; init never returns: only its timeout ends it.
(module
  (func (export "init") (result i32)
    (loop $again (br $again))
    (unreachable)))
