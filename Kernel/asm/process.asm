GLOBAL set_stack_frame

section .text

set_stack_frame:
	mov r8, rsp          ; Guardar rsp original del caller
	mov r9, rbp          ; Guardar rbp original del caller

	mov rsp, rdx         ; rsp y rbp pasan a apuntar al tope del stack del proceso
	mov rbp, rdx

	push 0               ; SS dummy (permite compatibilidad si se baja de ring)
	push rdx             ; RSP que queremos restaurar cuando entremos al proceso
	push 0x202           ; RFLAGS con IF=1
	push 0x8             ; CS de kernel
	push rdi             ; RIP = wrapper (process_caller)

	mov rdi, rsi         ; RDI = entry_point
	mov rsi, rcx         ; RSI = argv

	xor rax, rax         ; Usamos RAX como cero para inicializar los registros
	xor rbx, rbx
	xor rcx, rcx
	xor rdx, rdx

	push rbx             ; RBX
	push rcx             ; RCX
	push rdx             ; RDX
	push rax             ; RBP (inicial en cero)
	push rdi             ; RDI = entry_point
	push rsi             ; RSI = argv
	push rax             ; R8
	push rax             ; R9
	push rax             ; R10
	push rax             ; R11
	push rax             ; R12
	push rax             ; R13
	push rax             ; R14
	push rax             ; R15
	push rax             ; RAX

	mov rax, rsp         ; Devolvemos el puntero al contexto completo
	mov rsp, r8          ; Restaurar rsp original del caller
	mov rbp, r9          ; Restaurar rbp original del caller
	ret
