.text
.global _hook_objc_msgSend

_hook_objc_msgSend:

stp q6, q7, [sp, #-32]!
stp q4, q5, [sp, #-32]!
stp q2, q3, [sp, #-32]!
stp q0, q1, [sp, #-32]!

stp x8, lr, [sp, #-16]!
stp x6, x7, [sp, #-16]!
stp x4, x5, [sp, #-16]!
stp x2, x3, [sp, #-16]!
stp x0, x1, [sp, #-16]!

mov x2, x1
mov x1, lr
bl _before_objc_msgSend
mov x9, x0

ldp x0, x1, [sp], #16
ldp x2, x3, [sp], #16
ldp x4, x5, [sp], #16
ldp x6, x7, [sp], #16
ldp x8, lr, [sp], #16

ldp q0, q1, [sp], #32
ldp q2, q3, [sp], #32
ldp q4, q5, [sp], #32
ldp q6, q7, [sp], #32

blr x9
stp x0, x1, [sp, #-16]!
stp q0, q1, [sp, #-32]!
bl _get_lr
mov lr, x0
ldp q0, q1, [sp], #32
ldp x0, x1, [sp], #16
ret

ret
