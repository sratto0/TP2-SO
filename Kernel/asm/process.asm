GLOBAL set_stack_frame
EXTERN pushState

section .text

set_stack_frame:
	mov r8, rsp 	; Preservar rsp
	mov r9, rbp		; Preservar rbp
	mov rsp, rdx 	; Carga sp del proceso
	mov rbp, rdx
	push 0x0
	push rdx
	push 0x202
	push 0x8
	push rdi
	mov rdi, rsi 		; Primer argumento de wrapper, RIP
	mov rsi, rcx		; Segundo argumento, args
	pushState
	mov rax, rsp
	mov rsp, r8
	mov rbp, r9
	ret
