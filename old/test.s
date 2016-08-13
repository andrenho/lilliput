        section .text

        mov     C, 10   ; initialize C
next:   dec     C
        cmp     C, 0    ; TODO - this is not needed
        bnz     next

halt:   jmp     halt
