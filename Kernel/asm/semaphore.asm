GLOBAL acquire_lock
GLOBAL release_lock

section .text

acquire_lock:
    mov al, 0
.retry:
    xchg [rdi], al
    test al, al
    jz .retry
    ret

release_lock:
    mov byte [rdi], 1
    ret