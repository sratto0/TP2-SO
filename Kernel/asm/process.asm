GLOBAL set_stack_frame
EXTERN pushState

section .text

set_stack_frame:
	; Entrada (sysv): rdi = arg1, rsi = arg2, rdx = stack_top, rcx = arg4
	; Guardar stack actual
	mov r8, rsp
	mov r9, rbp
	; Cambiar al nuevo stack
	mov rsp, rdx
	mov rbp, rdx
	; Valores mínimos para un contexto: RIP(0) | rdx (saved) | RFLAGS | CS | saved rdi
	push 0x0
	push rdx
	push 0x202
	push 0x8
	push rdi
	; preparar argumentos para la función que ejecutará el proceso: rdi = antiguo rsi, rsi = antiguo rcx
	mov rdi, rsi
	; mov rsi, rcx
	; Guardar registros generales
	pushState ; macro definida en interrupts.asm (push rax .. push r15)
	; rax = puntero al frame creado
	mov rax, rsp
	; Restaurar stack original
	mov rsp, r8
	mov rbp, r9
	ret
