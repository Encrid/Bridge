;; init asks for more memory than its limit allows, and says so by its answer.
(module
  (memory 1)
  (func (export "init") (result i32)
    (if (result i32) (i32.eq (memory.grow (i32.const 64)) (i32.const -1))
      (then (i32.const 7))
      (else (i32.const 0)))))
