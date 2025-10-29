
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler
GLOBAL _syscallHandler

GLOBAL _ex00Handler
GLOBAL _ex06Handler
GLOBAL _ex0DHandler
GLOBAL _ex0EHandler

GLOBAL set_stack_frame

EXTERN irqDispatcher
EXTERN syscallDispatcher
EXTERN exceptionDispatcher
EXTERN load_main

SECTION .text

%macro pushState 1
	%if %1
		push rax
	%endif
	; push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 1
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	; pop rax
	%if %1
		pop rax
	%endif

%endmacro

; %macro irqHandlerMaster 1
; 	pushState 1

; 	; pasar (irq, rsp) a irqDispatcher
; 	mov rdi, %1
; 	mov rsi, rsp
; 	call irqDispatcher

; 	; usar el rsp devuelto por irqDispatcher/schedule
; 	mov rsp, rax

; 	mov al, 20h
; 	out 20h, al

; 	popState 1
; 	iretq
; %endmacro

%macro irqHandlerMaster 1
	pushState 1

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState 1
	iretq
%endmacro


%macro exceptionHandler 1
	pushState 1; Se cargan 15 registros en stack

	mov rsi, rsp
	add rsi, 15*8 ; RIP 
	mov rdx, rsp
	add rdx, 18*8 ; RSP 

	mov rdi, %1 ; pasaje de parametro
	call exceptionDispatcher

	popState 1
	; El iretq necesita que le manden los valores de RIP|CS|RFLAGS|SP|SS de userland por stack
	add rsp, 8
	push load_main
	iretq
%endmacro

_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0

;Keyboard
_irq01Handler:
	irqHandlerMaster 1

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5

;Syscall
_syscallHandler:
	pushState 1
	mov rbp, rsp

	push r9
	mov r9, r8
	mov r8, r10
	mov rcx, rdx
	mov rdx, rsi
	mov rsi, rdi
	mov rdi, rax
	call syscallDispatcher
	mov [aux], rax

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	mov rsp, rbp
	popState 0
	mov rax, [aux]
	iretq

;Zero Division Exception
_ex00Handler:
	exceptionHandler 0

;Invalid Op Code Exception
_ex06Handler:
	exceptionHandler 6

; General Protection
_ex0DHandler:
	exceptionHandler 13

; Page Fault
_ex0EHandler:
	exceptionHandler 14

haltcpu:
	cli
	hlt
	ret

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
	mov rsi, rcx
	; Guardar registros generales
	pushState 1; macro definida en interrupts.asm (push rax .. push r15)
	; rax = puntero al frame creado
	mov rax, rsp
	; Restaurar stack original
	mov rsp, r8
	mov rbp, r9
	ret


SECTION .bss
	aux resq 1