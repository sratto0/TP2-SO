GLOBAL write
GLOBAL read
GLOBAL getSeconds
GLOBAL clear
GLOBAL getInfoReg
GLOBAL setFontSize
GLOBAL getScreenResolution
GLOBAL drawRect
GLOBAL getTicks
GLOBAL getMemory
GLOBAL playSound
GLOBAL kaboom
GLOBAL setFontColor
GLOBAL getFontColor
GLOBAL sys_malloc
GLOBAL sys_free
GLOBAL sys_create_process
GLOBAL sys_exit_process
GLOBAL sys_yield
GLOBAL sys_getpid
GLOBAL sys_block_process
GLOBAL sys_unblock_process
GLOBAL sys_set_priority
GLOBAL sys_get_processes_info
GLOBAL sys_kill_process
GLOBAL sys_wait_pid
GLOBAL sys_total_cpu_ticks
GLOBAL sys_sem_open
GLOBAL sys_sem_wait
GLOBAL sys_sem_post
GLOBAL sys_sem_close
GLOBAL sys_memory_get_info
GLOBAL sys_sleep


read:
    mov rax, 0
    int 80h
    ret

write:
    mov rax, 1
    int 80h
    ret

clear:
    mov rax, 2
    int 80h
    ret

getSeconds:
    mov rax, 3
    int 80h
    ret

getInfoReg:
    mov rax, 4
    int 80h
    ret

setFontSize:
    mov rax, 5
    int 80h
    ret

getScreenResolution:
    mov rax, 6
    int 80h
    ret

drawRect:
    mov rax, 7
    mov r10, rcx
    int 80h
    ret

getTicks:
    mov rax, 8
    int 80h
    ret

getMemory:
    mov rax, 9
    int 80h
    ret

setFontColor:
    mov rax, 10
    int 80h
    ret

getFontColor:
    mov rax, 11
    int 80h
    ret

sys_malloc:
    mov rax, 12
    int 80h
    ret

sys_free:
    mov rax, 13
    int 80h
    ret

sys_create_process:
    mov rax, 14
    mov r10, rcx
    int 80h
    ret

sys_exit_process:
    mov rax, 15
    int 80h
    ret

sys_yield:
    mov rax, 16
    int 80h
    ret

sys_getpid:
    mov rax, 17
    int 80h
    ret

sys_block_process:  
    mov rax, 18
    int 80h
    ret

sys_unblock_process:
    mov rax, 19
    int 80h
    ret

sys_set_priority:
    mov rax, 20
    int 80h
    ret

sys_get_processes_info:
    mov rax, 21
    int 80h
    ret

sys_kill_process:
    mov rax, 22
    int 80h
    ret

sys_wait_pid:
    mov rax, 23
    int 80h
    ret

sys_total_cpu_ticks:
    mov rax, 24
    int 80h
    ret

sys_sem_open:
    mov rax, 25
    int 80h
    ret

sys_sem_wait:
    mov rax, 26
    int 80h
    ret

sys_sem_post:
    mov rax, 27
    int 80h
    ret

sys_sem_close:
    mov rax, 28
    int 80h
    ret

sys_memory_get_info:
    mov rax, 29
    int 80h
    ret

sys_sleep:
    mov rax, 30
    int 80h
    ret