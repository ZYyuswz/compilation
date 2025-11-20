.MODEL SMALL
.STACK 100h

.DATA

.CODE
MAIN PROC
    MOV AX, @DATA
    MOV DS, AX

MainLoop:

    ;-----------------------------------------
    ; 第一步：检查 Shift 状态（每一轮都检查）
    ;-----------------------------------------
    MOV AH, 02h
    INT 16h
    TEST AL, 03h       ; bit0 = 左 Shift，bit1 = 右 Shift
    JNZ ExitProgram    ; 任意 Shift 按下就退出


    ;-----------------------------------------
    ; 第二步：检查是否有按键（非阻塞）
    ;-----------------------------------------
    MOV AH, 01h
    INT 16h
    JZ MainLoop        ; ZF=1 → 无按键 → 回去继续循环


    ;-----------------------------------------
    ; 第三步：读取按键（此时一定有按键）
    ;-----------------------------------------
    MOV AH, 00h
    INT 16h            ; AL=ASCII, AH=扫描码

    ;-----------------------------------------
    ; 显示可打印字符
    ;-----------------------------------------
    CMP AL, 0
    JE MainLoop        ; 不可打印字符跳过

    MOV AH, 0Eh
    MOV BH, 0
    MOV BL, 07h
    INT 10h

    JMP MainLoop


ExitProgram:
    MOV AH, 4Ch
    INT 21h

MAIN ENDP
END MAIN
