;
; Author: Ice.Marek
; 2023 IceNET Technology
;
; Order of input arguments
;
; 1st ---> RDI
; 2nd ---> RSI
; 3rh ---> RDX
; 4th ---> RCX
;

section .data

LOCKED:
	dd 0

section .text

global MEMORY_READ
global CALL_PROCEDURE
global SPIN_LOCK
global SPIN_UNLOCK


MEMORY_READ:
	mov rax, rdi
	mov eax, [eax]
	ret

CALL_PROCEDURE:
	endbr64  			; Intel 64-bit support ---> For Jump/call oriented coding
	push rbp  			; Push base pointer RBP into the stack
	mov  rbp,rsp  		; Move current stack RSP into the base 
	mov  rax, rdi 	 	; Move the pointer of the function into RAX
	call rax 			; Call function
	pop  rbp 			; Pop stored base pointer RBP from the stack
	ret     			; Return to C/C++

; SPIN_LOCK:
; 	mov eax, 1  		; Set EAX = 1
; 	xchg eax,[LOCKED] 	; Swap EAX with lock variable 
; 	test eax, eax 		; If EAX = 0 ---> ZF = 0
; 	jnz SPIN_LOCK 		; If ZF != 0 then jump to spin_lock
; 	ret 				; If the spin is locked then procedure will run in a loop until released

; SPIN_UNLOCK:
; 	xor eax, eax 		; Zero EAX register
; 	xchg rax, [LOCKED] 	; Swap EAX with lock variable
; 	ret 				; Lock has been released 
