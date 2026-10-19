;; A module whose own memory starts with data: init answers 0 only if the first byte is the
;; "s" of the segment. wasmtime would lay such a segment down as a copy-on-write image, which
;; a memory it did not map itself cannot take: docs/backends/wasm/wasmtime.md, *Memory*.
(module
  (memory 1)
  (data (i32.const 0) "static")
  (func (export "init") (result i32)
    (i32.sub (i32.load8_u (i32.const 0)) (i32.const 115)))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
