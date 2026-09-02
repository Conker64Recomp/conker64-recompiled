#include "recomp.h"
#include "funcs.h"

RECOMP_FUNC void recomp_entrypoint(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80001000: lui         $t0, 0x8003
    ctx->r8 = S32(0X8003 << 16);
    // 0x80001004: lui         $t1, 0x1
    ctx->r9 = S32(0X1 << 16);
    // 0x80001008: addiu       $t0, $t0, -0x2B50
    ctx->r8 = ADD32(ctx->r8, -0X2B50);
    // 0x8000100C: ori         $t1, $t1, 0x6690
    ctx->r9 = ctx->r9 | 0X6690;
L_80001010:
    // 0x80001010: addi        $t1, $t1, -0x8
    ctx->r9 = ADD32(ctx->r9, -0X8);
    // 0x80001014: sw          $zero, 0x0($t0)
    MEM_W(0X0, ctx->r8) = 0;
    // 0x80001018: sw          $zero, 0x4($t0)
    MEM_W(0X4, ctx->r8) = 0;
    // 0x8000101C: bne         $t1, $zero, L_80001010
    if (ctx->r9 != 0) {
        // 0x80001020: addi        $t0, $t0, 0x8
        ctx->r8 = ADD32(ctx->r8, 0X8);
            goto L_80001010;
    }
    // 0x80001020: addi        $t0, $t0, 0x8
    ctx->r8 = ADD32(ctx->r8, 0X8);
    // 0x80001024: lui         $t2, 0x8000
    ctx->r10 = S32(0X8000 << 16);
    // 0x80001028: lui         $sp, 0x8003
    ctx->r29 = S32(0X8003 << 16);
    // 0x8000102C: addiu       $t2, $t2, 0x5AB0
    ctx->r10 = ADD32(ctx->r10, 0X5AB0);
    // 0x80001030: jr          $t2
    // 0x80001034: addiu       $sp, $sp, 0x14B0
    ctx->r29 = ADD32(ctx->r29, 0X14B0);
    LOOKUP_FUNC(ctx->r10)(rdram, ctx);
    return;
    // 0x80001034: addiu       $sp, $sp, 0x14B0
    ctx->r29 = ADD32(ctx->r29, 0X14B0);
    // 0x80001038: nop

    // 0x8000103C: nop

    // 0x80001040: nop

    // 0x80001044: nop

    // 0x80001048: nop

    // 0x8000104C: nop

    // 0x80001050: addiu       $sp, $sp, -0x20
    ctx->r29 = ADD32(ctx->r29, -0X20);
    // 0x80001054: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x80001058: addiu       $a0, $a0, -0x2B50
    ctx->r4 = ADD32(ctx->r4, -0X2B50);
    // 0x8000105C: sw          $ra, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r31;
;}
RECOMP_FUNC void bootproc(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
;}
RECOMP_FUNC void main_switch_handler(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x8000277C: jr          $t7
    // 0x80002780: nop

    LOOKUP_FUNC(ctx->r15)(rdram, ctx);
    return;
    // 0x80002780: nop

    // 0x80002784: addiu       $at, $zero, 0x25
    ctx->r1 = ADD32(0, 0X25);
    // 0x80002788: beql        $v1, $at, L_80002D60
    if (ctx->r3 == ctx->r1) {
        // 0x8000278C: lw          $t6, 0xC($s1)
        ctx->r14 = MEM_W(ctx->r17, 0XC);
            goto L_80002D60;
    }
    goto skip_0;
    // 0x8000278C: lw          $t6, 0xC($s1)
    ctx->r14 = MEM_W(ctx->r17, 0XC);
    skip_0:
    // 0x80002790: b           L_80002D80
    // 0x80002794: lw          $t2, 0xC($s1)
    ctx->r10 = MEM_W(ctx->r17, 0XC);
        goto L_80002D80;
    // 0x80002794: lw          $t2, 0xC($s1)
    ctx->r10 = MEM_W(ctx->r17, 0XC);
    // 0x80002798: lw          $t8, 0x0($s0)
    ctx->r24 = MEM_W(ctx->r16, 0X0);
    // 0x8000279C: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x800027A0: addiu       $t9, $t8, 0x3
    ctx->r25 = ADD32(ctx->r24, 0X3);
    // 0x800027A4: and         $t0, $t9, $at
    ctx->r8 = ctx->r25 & ctx->r1;
    // 0x800027A8: addiu       $t1, $t0, 0x4
    ctx->r9 = ADD32(ctx->r8, 0X4);
    // 0x800027AC: sw          $t1, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r9;
    // 0x800027B0: lw          $t4, 0xC($s1)
    ctx->r12 = MEM_W(ctx->r17, 0XC);
    // 0x800027B4: lw          $t3, -0x4($t1)
    ctx->r11 = MEM_W(ctx->r9, -0X4);
    // 0x800027B8: addu        $t5, $a3, $t4
    ctx->r13 = ADD32(ctx->r7, ctx->r12);
    // 0x800027BC: sb          $t3, 0x0($t5)
    MEM_B(0X0, ctx->r13) = ctx->r11;
    // 0x800027C0: lw          $t6, 0xC($s1)
    ctx->r14 = MEM_W(ctx->r17, 0XC);
    // 0x800027C4: addiu       $t7, $t6, 0x1
    ctx->r15 = ADD32(ctx->r14, 0X1);
    // 0x800027C8: b           L_80002D94
    // 0x800027CC: sw          $t7, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r15;
        goto L_80002D94;
    // 0x800027CC: sw          $t7, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r15;
    // 0x800027D0: lbu         $v0, 0x34($s1)
    ctx->r2 = MEM_BU(ctx->r17, 0X34);
    // 0x800027D4: addiu       $at, $zero, 0x6C
    ctx->r1 = ADD32(0, 0X6C);
    // 0x800027D8: bnel        $v0, $at, L_80002810
    if (ctx->r2 != ctx->r1) {
        // 0x800027DC: addiu       $at, $zero, 0x4C
        ctx->r1 = ADD32(0, 0X4C);
            goto L_80002810;
    }
    goto skip_1;
    // 0x800027DC: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
    skip_1:
    // 0x800027E0: lw          $t8, 0x0($s0)
    ctx->r24 = MEM_W(ctx->r16, 0X0);
    // 0x800027E4: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x800027E8: addiu       $t9, $t8, 0x3
    ctx->r25 = ADD32(ctx->r24, 0X3);
    // 0x800027EC: and         $t0, $t9, $at
    ctx->r8 = ctx->r25 & ctx->r1;
    // 0x800027F0: addiu       $t1, $t0, 0x4
    ctx->r9 = ADD32(ctx->r8, 0X4);
    // 0x800027F4: sw          $t1, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r9;
    // 0x800027F8: lw          $t4, -0x4($t1)
    ctx->r12 = MEM_W(ctx->r9, -0X4);
    // 0x800027FC: sra         $t6, $t4, 31
    ctx->r14 = S32(SIGNED(ctx->r12) >> 31);
    // 0x80002800: sw          $t6, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r14;
    // 0x80002804: b           L_8000286C
    // 0x80002808: sw          $t4, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r12;
        goto L_8000286C;
    // 0x80002808: sw          $t4, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r12;
    // 0x8000280C: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
L_80002810:
    // 0x80002810: bnel        $v0, $at, L_80002848
    if (ctx->r2 != ctx->r1) {
        // 0x80002814: lw          $t1, 0x0($s0)
        ctx->r9 = MEM_W(ctx->r16, 0X0);
            goto L_80002848;
    }
    goto skip_2;
    // 0x80002814: lw          $t1, 0x0($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X0);
    skip_2:
    // 0x80002818: lw          $t3, 0x0($s0)
    ctx->r11 = MEM_W(ctx->r16, 0X0);
    // 0x8000281C: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80002820: addiu       $t5, $t3, 0x7
    ctx->r13 = ADD32(ctx->r11, 0X7);
    // 0x80002824: and         $t8, $t5, $at
    ctx->r24 = ctx->r13 & ctx->r1;
    // 0x80002828: addiu       $t9, $t8, 0x8
    ctx->r25 = ADD32(ctx->r24, 0X8);
    // 0x8000282C: sw          $t9, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r25;
    // 0x80002830: lw          $t3, -0x4($t9)
    ctx->r11 = MEM_W(ctx->r25, -0X4);
    // 0x80002834: lw          $t2, -0x8($t9)
    ctx->r10 = MEM_W(ctx->r25, -0X8);
    // 0x80002838: sw          $t3, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r11;
    // 0x8000283C: b           L_8000286C
    // 0x80002840: sw          $t2, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r10;
        goto L_8000286C;
    // 0x80002840: sw          $t2, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r10;
    // 0x80002844: lw          $t1, 0x0($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X0);
L_80002848:
    // 0x80002848: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x8000284C: addiu       $t4, $t1, 0x3
    ctx->r12 = ADD32(ctx->r9, 0X3);
    // 0x80002850: and         $t6, $t4, $at
    ctx->r14 = ctx->r12 & ctx->r1;
    // 0x80002854: addiu       $t7, $t6, 0x4
    ctx->r15 = ADD32(ctx->r14, 0X4);
    // 0x80002858: sw          $t7, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r15;
    // 0x8000285C: lw          $t8, -0x4($t7)
    ctx->r24 = MEM_W(ctx->r15, -0X4);
    // 0x80002860: sra         $t0, $t8, 31
    ctx->r8 = S32(SIGNED(ctx->r24) >> 31);
    // 0x80002864: sw          $t0, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r8;
    // 0x80002868: sw          $t8, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r24;
L_8000286C:
    // 0x8000286C: lbu         $t9, 0x34($s1)
    ctx->r25 = MEM_BU(ctx->r17, 0X34);
    // 0x80002870: addiu       $at, $zero, 0x68
    ctx->r1 = ADD32(0, 0X68);
    // 0x80002874: bnel        $t9, $at, L_80002898
    if (ctx->r25 != ctx->r1) {
        // 0x80002878: lw          $t0, 0x0($s1)
        ctx->r8 = MEM_W(ctx->r17, 0X0);
            goto L_80002898;
    }
    goto skip_3;
    // 0x80002878: lw          $t0, 0x0($s1)
    ctx->r8 = MEM_W(ctx->r17, 0X0);
    skip_3:
    // 0x8000287C: lw          $t3, 0x4($s1)
    ctx->r11 = MEM_W(ctx->r17, 0X4);
    // 0x80002880: sll         $t6, $t3, 16
    ctx->r14 = S32(ctx->r11 << 16);
    // 0x80002884: sra         $t7, $t6, 16
    ctx->r15 = S32(SIGNED(ctx->r14) >> 16);
    // 0x80002888: sra         $t8, $t7, 31
    ctx->r24 = S32(SIGNED(ctx->r15) >> 31);
    // 0x8000288C: sw          $t8, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r24;
    // 0x80002890: sw          $t7, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r15;
    // 0x80002894: lw          $t0, 0x0($s1)
    ctx->r8 = MEM_W(ctx->r17, 0X0);
L_80002898:
    // 0x80002898: bgtzl       $t0, L_800028D4
    if (SIGNED(ctx->r8) > 0) {
        // 0x8000289C: lw          $v0, 0x30($s1)
        ctx->r2 = MEM_W(ctx->r17, 0X30);
            goto L_800028D4;
    }
    goto skip_4;
    // 0x8000289C: lw          $v0, 0x30($s1)
    ctx->r2 = MEM_W(ctx->r17, 0X30);
    skip_4:
    // 0x800028A0: bltzl       $t0, L_800028B4
    if (SIGNED(ctx->r8) < 0) {
        // 0x800028A4: lw          $t2, 0xC($s1)
        ctx->r10 = MEM_W(ctx->r17, 0XC);
            goto L_800028B4;
    }
    goto skip_5;
    // 0x800028A4: lw          $t2, 0xC($s1)
    ctx->r10 = MEM_W(ctx->r17, 0XC);
    skip_5:
    // 0x800028A8: b           L_800028D4
    // 0x800028AC: lw          $v0, 0x30($s1)
    ctx->r2 = MEM_W(ctx->r17, 0X30);
        goto L_800028D4;
    // 0x800028AC: lw          $v0, 0x30($s1)
    ctx->r2 = MEM_W(ctx->r17, 0X30);
    // 0x800028B0: lw          $t2, 0xC($s1)
    ctx->r10 = MEM_W(ctx->r17, 0XC);
L_800028B4:
    // 0x800028B4: addiu       $t5, $zero, 0x2D
    ctx->r13 = ADD32(0, 0X2D);
    // 0x800028B8: addu        $t3, $a3, $t2
    ctx->r11 = ADD32(ctx->r7, ctx->r10);
    // 0x800028BC: sb          $t5, 0x0($t3)
    MEM_B(0X0, ctx->r11) = ctx->r13;
    // 0x800028C0: lw          $t4, 0xC($s1)
    ctx->r12 = MEM_W(ctx->r17, 0XC);
    // 0x800028C4: addiu       $t6, $t4, 0x1
    ctx->r14 = ADD32(ctx->r12, 0X1);
    // 0x800028C8: b           L_80002924
    // 0x800028CC: sw          $t6, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r14;
        goto L_80002924;
    // 0x800028CC: sw          $t6, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r14;
    // 0x800028D0: lw          $v0, 0x30($s1)
    ctx->r2 = MEM_W(ctx->r17, 0X30);
L_800028D4:
    // 0x800028D4: andi        $t7, $v0, 0x2
    ctx->r15 = ctx->r2 & 0X2;
    // 0x800028D8: beq         $t7, $zero, L_80002900
    if (ctx->r15 == 0) {
        // 0x800028DC: andi        $t5, $v0, 0x1
        ctx->r13 = ctx->r2 & 0X1;
            goto L_80002900;
    }
    // 0x800028DC: andi        $t5, $v0, 0x1
    ctx->r13 = ctx->r2 & 0X1;
    // 0x800028E0: lw          $t9, 0xC($s1)
    ctx->r25 = MEM_W(ctx->r17, 0XC);
    // 0x800028E4: addiu       $t8, $zero, 0x2B
    ctx->r24 = ADD32(0, 0X2B);
    // 0x800028E8: addu        $t0, $a3, $t9
    ctx->r8 = ADD32(ctx->r7, ctx->r25);
    // 0x800028EC: sb          $t8, 0x0($t0)
    MEM_B(0X0, ctx->r8) = ctx->r24;
    // 0x800028F0: lw          $t1, 0xC($s1)
    ctx->r9 = MEM_W(ctx->r17, 0XC);
    // 0x800028F4: addiu       $t2, $t1, 0x1
    ctx->r10 = ADD32(ctx->r9, 0X1);
    // 0x800028F8: b           L_80002924
    // 0x800028FC: sw          $t2, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r10;
        goto L_80002924;
    // 0x800028FC: sw          $t2, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r10;
L_80002900:
    // 0x80002900: beql        $t5, $zero, L_80002928
    if (ctx->r13 == 0) {
        // 0x80002904: lw          $t8, 0xC($s1)
        ctx->r24 = MEM_W(ctx->r17, 0XC);
            goto L_80002928;
    }
    goto skip_6;
    // 0x80002904: lw          $t8, 0xC($s1)
    ctx->r24 = MEM_W(ctx->r17, 0XC);
    skip_6:
    // 0x80002908: lw          $t4, 0xC($s1)
    ctx->r12 = MEM_W(ctx->r17, 0XC);
    // 0x8000290C: addiu       $t3, $zero, 0x20
    ctx->r11 = ADD32(0, 0X20);
    // 0x80002910: addu        $t6, $a3, $t4
    ctx->r14 = ADD32(ctx->r7, ctx->r12);
    // 0x80002914: sb          $t3, 0x0($t6)
    MEM_B(0X0, ctx->r14) = ctx->r11;
    // 0x80002918: lw          $t7, 0xC($s1)
    ctx->r15 = MEM_W(ctx->r17, 0XC);
    // 0x8000291C: addiu       $t9, $t7, 0x1
    ctx->r25 = ADD32(ctx->r15, 0X1);
    // 0x80002920: sw          $t9, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r25;
L_80002924:
    // 0x80002924: lw          $t8, 0xC($s1)
    ctx->r24 = MEM_W(ctx->r17, 0XC);
L_80002928:
    // 0x80002928: or          $a0, $s1, $zero
    ctx->r4 = ctx->r17 | 0;
    // 0x8000292C: andi        $a1, $a2, 0xFF
    ctx->r5 = ctx->r6 & 0XFF;
    // 0x80002930: addu        $t0, $t8, $a3
    ctx->r8 = ADD32(ctx->r24, ctx->r7);
    // 0x80002934: jal         0x800230F0
    // 0x80002938: sw          $t0, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r8;
    _Litob(rdram, ctx);
        goto after_0;
    // 0x80002938: sw          $t0, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r8;
    after_0:
    // 0x8000293C: b           L_80002D98
    // 0x80002940: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
        goto L_80002D98;
    // 0x80002940: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80002944: lbu         $v0, 0x34($s1)
    ctx->r2 = MEM_BU(ctx->r17, 0X34);
    // 0x80002948: addiu       $at, $zero, 0x6C
    ctx->r1 = ADD32(0, 0X6C);
    // 0x8000294C: bnel        $v0, $at, L_80002984
    if (ctx->r2 != ctx->r1) {
        // 0x80002950: addiu       $at, $zero, 0x4C
        ctx->r1 = ADD32(0, 0X4C);
            goto L_80002984;
    }
    goto skip_7;
    // 0x80002950: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
    skip_7:
    // 0x80002954: lw          $t1, 0x0($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X0);
    // 0x80002958: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x8000295C: addiu       $t2, $t1, 0x3
    ctx->r10 = ADD32(ctx->r9, 0X3);
    // 0x80002960: and         $t5, $t2, $at
    ctx->r13 = ctx->r10 & ctx->r1;
    // 0x80002964: addiu       $t4, $t5, 0x4
    ctx->r12 = ADD32(ctx->r13, 0X4);
    // 0x80002968: sw          $t4, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r12;
    // 0x8000296C: lw          $t6, -0x4($t4)
    ctx->r14 = MEM_W(ctx->r12, -0X4);
    // 0x80002970: sra         $t8, $t6, 31
    ctx->r24 = S32(SIGNED(ctx->r14) >> 31);
    // 0x80002974: sw          $t8, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r24;
    // 0x80002978: b           L_800029E0
    // 0x8000297C: sw          $t6, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r14;
        goto L_800029E0;
    // 0x8000297C: sw          $t6, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r14;
    // 0x80002980: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
L_80002984:
    // 0x80002984: bnel        $v0, $at, L_800029BC
    if (ctx->r2 != ctx->r1) {
        // 0x80002988: lw          $t3, 0x0($s0)
        ctx->r11 = MEM_W(ctx->r16, 0X0);
            goto L_800029BC;
    }
    goto skip_8;
    // 0x80002988: lw          $t3, 0x0($s0)
    ctx->r11 = MEM_W(ctx->r16, 0X0);
    skip_8:
    // 0x8000298C: lw          $t7, 0x0($s0)
    ctx->r15 = MEM_W(ctx->r16, 0X0);
    // 0x80002990: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80002994: addiu       $t0, $t7, 0x7
    ctx->r8 = ADD32(ctx->r15, 0X7);
    // 0x80002998: and         $t1, $t0, $at
    ctx->r9 = ctx->r8 & ctx->r1;
    // 0x8000299C: addiu       $t2, $t1, 0x8
    ctx->r10 = ADD32(ctx->r9, 0X8);
    // 0x800029A0: sw          $t2, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r10;
    // 0x800029A4: lw          $t5, -0x4($t2)
    ctx->r13 = MEM_W(ctx->r10, -0X4);
    // 0x800029A8: lw          $t4, -0x8($t2)
    ctx->r12 = MEM_W(ctx->r10, -0X8);
    // 0x800029AC: sw          $t5, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r13;
    // 0x800029B0: b           L_800029E0
    // 0x800029B4: sw          $t4, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r12;
        goto L_800029E0;
    // 0x800029B4: sw          $t4, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r12;
    // 0x800029B8: lw          $t3, 0x0($s0)
    ctx->r11 = MEM_W(ctx->r16, 0X0);
L_800029BC:
    // 0x800029BC: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x800029C0: addiu       $t6, $t3, 0x3
    ctx->r14 = ADD32(ctx->r11, 0X3);
    // 0x800029C4: and         $t8, $t6, $at
    ctx->r24 = ctx->r14 & ctx->r1;
    // 0x800029C8: addiu       $t9, $t8, 0x4
    ctx->r25 = ADD32(ctx->r24, 0X4);
    // 0x800029CC: sw          $t9, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r25;
    // 0x800029D0: lw          $t0, -0x4($t9)
    ctx->r8 = MEM_W(ctx->r25, -0X4);
    // 0x800029D4: sra         $t2, $t0, 31
    ctx->r10 = S32(SIGNED(ctx->r8) >> 31);
    // 0x800029D8: sw          $t2, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r10;
    // 0x800029DC: sw          $t0, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r8;
L_800029E0:
    // 0x800029E0: lbu         $v0, 0x34($s1)
    ctx->r2 = MEM_BU(ctx->r17, 0X34);
    // 0x800029E4: addiu       $at, $zero, 0x68
    ctx->r1 = ADD32(0, 0X68);
    // 0x800029E8: bne         $v0, $at, L_80002A08
    if (ctx->r2 != ctx->r1) {
        // 0x800029EC: nop
    
            goto L_80002A08;
    }
    // 0x800029EC: nop

    // 0x800029F0: lw          $t5, 0x4($s1)
    ctx->r13 = MEM_W(ctx->r17, 0X4);
    // 0x800029F4: addiu       $t8, $zero, 0x0
    ctx->r24 = ADD32(0, 0X0);
    // 0x800029F8: sw          $t8, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r24;
    // 0x800029FC: andi        $t6, $t5, 0xFFFF
    ctx->r14 = ctx->r13 & 0XFFFF;
    // 0x80002A00: b           L_80002A20
    // 0x80002A04: sw          $t6, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r14;
        goto L_80002A20;
    // 0x80002A04: sw          $t6, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r14;
L_80002A08:
    // 0x80002A08: bnel        $v0, $zero, L_80002A24
    if (ctx->r2 != 0) {
        // 0x80002A0C: lw          $t4, 0x30($s1)
        ctx->r12 = MEM_W(ctx->r17, 0X30);
            goto L_80002A24;
    }
    goto skip_9;
    // 0x80002A0C: lw          $t4, 0x30($s1)
    ctx->r12 = MEM_W(ctx->r17, 0X30);
    skip_9:
    // 0x80002A10: lw          $t1, 0x4($s1)
    ctx->r9 = MEM_W(ctx->r17, 0X4);
    // 0x80002A14: addiu       $t2, $zero, 0x0
    ctx->r10 = ADD32(0, 0X0);
    // 0x80002A18: sw          $t2, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r10;
    // 0x80002A1C: sw          $t1, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r9;
L_80002A20:
    // 0x80002A20: lw          $t4, 0x30($s1)
    ctx->r12 = MEM_W(ctx->r17, 0X30);
L_80002A24:
    // 0x80002A24: andi        $t5, $t4, 0x8
    ctx->r13 = ctx->r12 & 0X8;
    // 0x80002A28: beql        $t5, $zero, L_80002A7C
    if (ctx->r13 == 0) {
        // 0x80002A2C: lw          $t5, 0xC($s1)
        ctx->r13 = MEM_W(ctx->r17, 0XC);
            goto L_80002A7C;
    }
    goto skip_10;
    // 0x80002A2C: lw          $t5, 0xC($s1)
    ctx->r13 = MEM_W(ctx->r17, 0XC);
    skip_10:
    // 0x80002A30: lw          $t8, 0xC($s1)
    ctx->r24 = MEM_W(ctx->r17, 0XC);
    // 0x80002A34: addiu       $t6, $zero, 0x30
    ctx->r14 = ADD32(0, 0X30);
    // 0x80002A38: addiu       $at, $zero, 0x78
    ctx->r1 = ADD32(0, 0X78);
    // 0x80002A3C: addu        $t9, $a3, $t8
    ctx->r25 = ADD32(ctx->r7, ctx->r24);
    // 0x80002A40: sb          $t6, 0x0($t9)
    MEM_B(0X0, ctx->r25) = ctx->r14;
    // 0x80002A44: lw          $t0, 0xC($s1)
    ctx->r8 = MEM_W(ctx->r17, 0XC);
    // 0x80002A48: addiu       $t1, $t0, 0x1
    ctx->r9 = ADD32(ctx->r8, 0X1);
    // 0x80002A4C: beq         $v1, $at, L_80002A60
    if (ctx->r3 == ctx->r1) {
        // 0x80002A50: sw          $t1, 0xC($s1)
        MEM_W(0XC, ctx->r17) = ctx->r9;
            goto L_80002A60;
    }
    // 0x80002A50: sw          $t1, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r9;
    // 0x80002A54: addiu       $at, $zero, 0x58
    ctx->r1 = ADD32(0, 0X58);
    // 0x80002A58: bnel        $v1, $at, L_80002A7C
    if (ctx->r3 != ctx->r1) {
        // 0x80002A5C: lw          $t5, 0xC($s1)
        ctx->r13 = MEM_W(ctx->r17, 0XC);
            goto L_80002A7C;
    }
    goto skip_11;
    // 0x80002A5C: lw          $t5, 0xC($s1)
    ctx->r13 = MEM_W(ctx->r17, 0XC);
    skip_11:
L_80002A60:
    // 0x80002A60: lw          $t7, 0xC($s1)
    ctx->r15 = MEM_W(ctx->r17, 0XC);
    // 0x80002A64: addu        $t2, $a3, $t7
    ctx->r10 = ADD32(ctx->r7, ctx->r15);
    // 0x80002A68: sb          $a2, 0x0($t2)
    MEM_B(0X0, ctx->r10) = ctx->r6;
    // 0x80002A6C: lw          $t3, 0xC($s1)
    ctx->r11 = MEM_W(ctx->r17, 0XC);
    // 0x80002A70: addiu       $t4, $t3, 0x1
    ctx->r12 = ADD32(ctx->r11, 0X1);
    // 0x80002A74: sw          $t4, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r12;
    // 0x80002A78: lw          $t5, 0xC($s1)
    ctx->r13 = MEM_W(ctx->r17, 0XC);
L_80002A7C:
    // 0x80002A7C: or          $a0, $s1, $zero
    ctx->r4 = ctx->r17 | 0;
    // 0x80002A80: andi        $a1, $a2, 0xFF
    ctx->r5 = ctx->r6 & 0XFF;
    // 0x80002A84: addu        $t8, $t5, $a3
    ctx->r24 = ADD32(ctx->r13, ctx->r7);
    // 0x80002A88: jal         0x800230F0
    // 0x80002A8C: sw          $t8, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r24;
    _Litob(rdram, ctx);
        goto after_1;
    // 0x80002A8C: sw          $t8, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r24;
    after_1:
    // 0x80002A90: b           L_80002D98
    // 0x80002A94: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
        goto L_80002D98;
    // 0x80002A94: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80002A98: lbu         $t6, 0x34($s1)
    ctx->r14 = MEM_BU(ctx->r17, 0X34);
    // 0x80002A9C: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
    // 0x80002AA0: bnel        $t6, $at, L_80002B08
    if (ctx->r14 != ctx->r1) {
        // 0x80002AA4: lw          $v0, 0x0($s0)
        ctx->r2 = MEM_W(ctx->r16, 0X0);
            goto L_80002B08;
    }
    goto skip_12;
    // 0x80002AA4: lw          $v0, 0x0($s0)
    ctx->r2 = MEM_W(ctx->r16, 0X0);
    skip_12:
    // 0x80002AA8: lw          $v0, 0x0($s0)
    ctx->r2 = MEM_W(ctx->r16, 0X0);
    // 0x80002AAC: andi        $t9, $v0, 0x1
    ctx->r25 = ctx->r2 & 0X1;
    // 0x80002AB0: beq         $t9, $zero, L_80002AC8
    if (ctx->r25 == 0) {
        // 0x80002AB4: or          $v1, $v0, $zero
        ctx->r3 = ctx->r2 | 0;
            goto L_80002AC8;
    }
    // 0x80002AB4: or          $v1, $v0, $zero
    ctx->r3 = ctx->r2 | 0;
    // 0x80002AB8: addiu       $t0, $v1, 0x7
    ctx->r8 = ADD32(ctx->r3, 0X7);
    // 0x80002ABC: sw          $t0, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r8;
    // 0x80002AC0: b           L_80002AF8
    // 0x80002AC4: addiu       $v0, $t0, -0x16
    ctx->r2 = ADD32(ctx->r8, -0X16);
        goto L_80002AF8;
    // 0x80002AC4: addiu       $v0, $t0, -0x16
    ctx->r2 = ADD32(ctx->r8, -0X16);
L_80002AC8:
    // 0x80002AC8: andi        $t1, $v1, 0x2
    ctx->r9 = ctx->r3 & 0X2;
    // 0x80002ACC: beq         $t1, $zero, L_80002AE4
    if (ctx->r9 == 0) {
        // 0x80002AD0: addiu       $t2, $v0, 0x7
        ctx->r10 = ADD32(ctx->r2, 0X7);
            goto L_80002AE4;
    }
    // 0x80002AD0: addiu       $t2, $v0, 0x7
    ctx->r10 = ADD32(ctx->r2, 0X7);
    // 0x80002AD4: addiu       $t7, $v1, 0xA
    ctx->r15 = ADD32(ctx->r3, 0XA);
    // 0x80002AD8: sw          $t7, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r15;
    // 0x80002ADC: b           L_80002AF4
    // 0x80002AE0: addiu       $a0, $t7, -0x28
    ctx->r4 = ADD32(ctx->r15, -0X28);
        goto L_80002AF4;
    // 0x80002AE0: addiu       $a0, $t7, -0x28
    ctx->r4 = ADD32(ctx->r15, -0X28);
L_80002AE4:
    // 0x80002AE4: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80002AE8: and         $t3, $t2, $at
    ctx->r11 = ctx->r10 & ctx->r1;
    // 0x80002AEC: addiu       $a0, $t3, 0x8
    ctx->r4 = ADD32(ctx->r11, 0X8);
    // 0x80002AF0: sw          $a0, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r4;
L_80002AF4:
    // 0x80002AF4: or          $v0, $a0, $zero
    ctx->r2 = ctx->r4 | 0;
L_80002AF8:
    // 0x80002AF8: ldc1        $f4, -0x8($v0)
    CHECK_FR(ctx, 4);
    ctx->f4.u64 = LD(ctx->r2, -0X8);
    // 0x80002AFC: b           L_80002B5C
    // 0x80002B00: sdc1        $f4, 0x0($s1)
    CHECK_FR(ctx, 4);
    SD(ctx->f4.u64, 0X0, ctx->r17);
        goto L_80002B5C;
    // 0x80002B00: sdc1        $f4, 0x0($s1)
    CHECK_FR(ctx, 4);
    SD(ctx->f4.u64, 0X0, ctx->r17);
    // 0x80002B04: lw          $v0, 0x0($s0)
    ctx->r2 = MEM_W(ctx->r16, 0X0);
L_80002B08:
    // 0x80002B08: andi        $t5, $v0, 0x1
    ctx->r13 = ctx->r2 & 0X1;
    // 0x80002B0C: beq         $t5, $zero, L_80002B24
    if (ctx->r13 == 0) {
        // 0x80002B10: or          $v1, $v0, $zero
        ctx->r3 = ctx->r2 | 0;
            goto L_80002B24;
    }
    // 0x80002B10: or          $v1, $v0, $zero
    ctx->r3 = ctx->r2 | 0;
    // 0x80002B14: addiu       $t8, $v1, 0x7
    ctx->r24 = ADD32(ctx->r3, 0X7);
    // 0x80002B18: sw          $t8, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r24;
    // 0x80002B1C: b           L_80002B54
    // 0x80002B20: addiu       $v0, $t8, -0x16
    ctx->r2 = ADD32(ctx->r24, -0X16);
        goto L_80002B54;
    // 0x80002B20: addiu       $v0, $t8, -0x16
    ctx->r2 = ADD32(ctx->r24, -0X16);
L_80002B24:
    // 0x80002B24: andi        $t6, $v1, 0x2
    ctx->r14 = ctx->r3 & 0X2;
    // 0x80002B28: beq         $t6, $zero, L_80002B40
    if (ctx->r14 == 0) {
        // 0x80002B2C: addiu       $t0, $v0, 0x7
        ctx->r8 = ADD32(ctx->r2, 0X7);
            goto L_80002B40;
    }
    // 0x80002B2C: addiu       $t0, $v0, 0x7
    ctx->r8 = ADD32(ctx->r2, 0X7);
    // 0x80002B30: addiu       $t9, $v1, 0xA
    ctx->r25 = ADD32(ctx->r3, 0XA);
    // 0x80002B34: sw          $t9, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r25;
    // 0x80002B38: b           L_80002B50
    // 0x80002B3C: addiu       $a0, $t9, -0x28
    ctx->r4 = ADD32(ctx->r25, -0X28);
        goto L_80002B50;
    // 0x80002B3C: addiu       $a0, $t9, -0x28
    ctx->r4 = ADD32(ctx->r25, -0X28);
L_80002B40:
    // 0x80002B40: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80002B44: and         $t1, $t0, $at
    ctx->r9 = ctx->r8 & ctx->r1;
    // 0x80002B48: addiu       $a0, $t1, 0x8
    ctx->r4 = ADD32(ctx->r9, 0X8);
    // 0x80002B4C: sw          $a0, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r4;
L_80002B50:
    // 0x80002B50: or          $v0, $a0, $zero
    ctx->r2 = ctx->r4 | 0;
L_80002B54:
    // 0x80002B54: ldc1        $f6, -0x8($v0)
    CHECK_FR(ctx, 6);
    ctx->f6.u64 = LD(ctx->r2, -0X8);
    // 0x80002B58: sdc1        $f6, 0x0($s1)
    CHECK_FR(ctx, 6);
    SD(ctx->f6.u64, 0X0, ctx->r17);
L_80002B5C:
    // 0x80002B5C: lhu         $t2, 0x0($s1)
    ctx->r10 = MEM_HU(ctx->r17, 0X0);
    // 0x80002B60: andi        $t3, $t2, 0x8000
    ctx->r11 = ctx->r10 & 0X8000;
    // 0x80002B64: beql        $t3, $zero, L_80002B90
    if (ctx->r11 == 0) {
        // 0x80002B68: lw          $v0, 0x30($s1)
        ctx->r2 = MEM_W(ctx->r17, 0X30);
            goto L_80002B90;
    }
    goto skip_13;
    // 0x80002B68: lw          $v0, 0x30($s1)
    ctx->r2 = MEM_W(ctx->r17, 0X30);
    skip_13:
    // 0x80002B6C: lw          $t5, 0xC($s1)
    ctx->r13 = MEM_W(ctx->r17, 0XC);
    // 0x80002B70: addiu       $t4, $zero, 0x2D
    ctx->r12 = ADD32(0, 0X2D);
    // 0x80002B74: addu        $t8, $a3, $t5
    ctx->r24 = ADD32(ctx->r7, ctx->r13);
    // 0x80002B78: sb          $t4, 0x0($t8)
    MEM_B(0X0, ctx->r24) = ctx->r12;
    // 0x80002B7C: lw          $t6, 0xC($s1)
    ctx->r14 = MEM_W(ctx->r17, 0XC);
    // 0x80002B80: addiu       $t9, $t6, 0x1
    ctx->r25 = ADD32(ctx->r14, 0X1);
    // 0x80002B84: b           L_80002BE0
    // 0x80002B88: sw          $t9, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r25;
        goto L_80002BE0;
    // 0x80002B88: sw          $t9, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r25;
    // 0x80002B8C: lw          $v0, 0x30($s1)
    ctx->r2 = MEM_W(ctx->r17, 0X30);
L_80002B90:
    // 0x80002B90: andi        $t0, $v0, 0x2
    ctx->r8 = ctx->r2 & 0X2;
    // 0x80002B94: beq         $t0, $zero, L_80002BBC
    if (ctx->r8 == 0) {
        // 0x80002B98: andi        $t4, $v0, 0x1
        ctx->r12 = ctx->r2 & 0X1;
            goto L_80002BBC;
    }
    // 0x80002B98: andi        $t4, $v0, 0x1
    ctx->r12 = ctx->r2 & 0X1;
    // 0x80002B9C: lw          $t7, 0xC($s1)
    ctx->r15 = MEM_W(ctx->r17, 0XC);
    // 0x80002BA0: addiu       $t1, $zero, 0x2B
    ctx->r9 = ADD32(0, 0X2B);
    // 0x80002BA4: addu        $t2, $a3, $t7
    ctx->r10 = ADD32(ctx->r7, ctx->r15);
    // 0x80002BA8: sb          $t1, 0x0($t2)
    MEM_B(0X0, ctx->r10) = ctx->r9;
    // 0x80002BAC: lw          $t3, 0xC($s1)
    ctx->r11 = MEM_W(ctx->r17, 0XC);
    // 0x80002BB0: addiu       $t5, $t3, 0x1
    ctx->r13 = ADD32(ctx->r11, 0X1);
    // 0x80002BB4: b           L_80002BE0
    // 0x80002BB8: sw          $t5, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r13;
        goto L_80002BE0;
    // 0x80002BB8: sw          $t5, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r13;
L_80002BBC:
    // 0x80002BBC: beql        $t4, $zero, L_80002BE4
    if (ctx->r12 == 0) {
        // 0x80002BC0: lw          $t1, 0xC($s1)
        ctx->r9 = MEM_W(ctx->r17, 0XC);
            goto L_80002BE4;
    }
    goto skip_14;
    // 0x80002BC0: lw          $t1, 0xC($s1)
    ctx->r9 = MEM_W(ctx->r17, 0XC);
    skip_14:
    // 0x80002BC4: lw          $t6, 0xC($s1)
    ctx->r14 = MEM_W(ctx->r17, 0XC);
    // 0x80002BC8: addiu       $t8, $zero, 0x20
    ctx->r24 = ADD32(0, 0X20);
    // 0x80002BCC: addu        $t9, $a3, $t6
    ctx->r25 = ADD32(ctx->r7, ctx->r14);
    // 0x80002BD0: sb          $t8, 0x0($t9)
    MEM_B(0X0, ctx->r25) = ctx->r24;
    // 0x80002BD4: lw          $t0, 0xC($s1)
    ctx->r8 = MEM_W(ctx->r17, 0XC);
    // 0x80002BD8: addiu       $t7, $t0, 0x1
    ctx->r15 = ADD32(ctx->r8, 0X1);
    // 0x80002BDC: sw          $t7, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r15;
L_80002BE0:
    // 0x80002BE0: lw          $t1, 0xC($s1)
    ctx->r9 = MEM_W(ctx->r17, 0XC);
L_80002BE4:
    // 0x80002BE4: or          $a0, $s1, $zero
    ctx->r4 = ctx->r17 | 0;
    // 0x80002BE8: andi        $a1, $a2, 0xFF
    ctx->r5 = ctx->r6 & 0XFF;
    // 0x80002BEC: addu        $t2, $t1, $a3
    ctx->r10 = ADD32(ctx->r9, ctx->r7);
    // 0x80002BF0: jal         0x80001550
    // 0x80002BF4: sw          $t2, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r10;
    static_0_80001550(rdram, ctx);
        goto after_2;
    // 0x80002BF4: sw          $t2, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r10;
    after_2:
    // 0x80002BF8: b           L_80002D98
    // 0x80002BFC: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
        goto L_80002D98;
    // 0x80002BFC: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80002C00: lbu         $v0, 0x34($s1)
    ctx->r2 = MEM_BU(ctx->r17, 0X34);
    // 0x80002C04: addiu       $at, $zero, 0x68
    ctx->r1 = ADD32(0, 0X68);
    // 0x80002C08: bnel        $v0, $at, L_80002C3C
    if (ctx->r2 != ctx->r1) {
        // 0x80002C0C: addiu       $at, $zero, 0x6C
        ctx->r1 = ADD32(0, 0X6C);
            goto L_80002C3C;
    }
    goto skip_15;
    // 0x80002C0C: addiu       $at, $zero, 0x6C
    ctx->r1 = ADD32(0, 0X6C);
    skip_15:
    // 0x80002C10: lw          $t3, 0x0($s0)
    ctx->r11 = MEM_W(ctx->r16, 0X0);
    // 0x80002C14: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x80002C18: addiu       $t5, $t3, 0x3
    ctx->r13 = ADD32(ctx->r11, 0X3);
    // 0x80002C1C: and         $t4, $t5, $at
    ctx->r12 = ctx->r13 & ctx->r1;
    // 0x80002C20: addiu       $t6, $t4, 0x4
    ctx->r14 = ADD32(ctx->r12, 0X4);
    // 0x80002C24: sw          $t6, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r14;
    // 0x80002C28: lw          $t9, -0x4($t6)
    ctx->r25 = MEM_W(ctx->r14, -0X4);
    // 0x80002C2C: lw          $t8, 0x2C($s1)
    ctx->r24 = MEM_W(ctx->r17, 0X2C);
    // 0x80002C30: b           L_80002D94
    // 0x80002C34: sh          $t8, 0x0($t9)
    MEM_H(0X0, ctx->r25) = ctx->r24;
        goto L_80002D94;
    // 0x80002C34: sh          $t8, 0x0($t9)
    MEM_H(0X0, ctx->r25) = ctx->r24;
    // 0x80002C38: addiu       $at, $zero, 0x6C
    ctx->r1 = ADD32(0, 0X6C);
L_80002C3C:
    // 0x80002C3C: bnel        $v0, $at, L_80002C70
    if (ctx->r2 != ctx->r1) {
        // 0x80002C40: addiu       $at, $zero, 0x4C
        ctx->r1 = ADD32(0, 0X4C);
            goto L_80002C70;
    }
    goto skip_16;
    // 0x80002C40: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
    skip_16:
    // 0x80002C44: lw          $t0, 0x0($s0)
    ctx->r8 = MEM_W(ctx->r16, 0X0);
    // 0x80002C48: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x80002C4C: addiu       $t7, $t0, 0x3
    ctx->r15 = ADD32(ctx->r8, 0X3);
    // 0x80002C50: and         $t1, $t7, $at
    ctx->r9 = ctx->r15 & ctx->r1;
    // 0x80002C54: addiu       $t2, $t1, 0x4
    ctx->r10 = ADD32(ctx->r9, 0X4);
    // 0x80002C58: sw          $t2, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r10;
    // 0x80002C5C: lw          $t5, -0x4($t2)
    ctx->r13 = MEM_W(ctx->r10, -0X4);
    // 0x80002C60: lw          $t3, 0x2C($s1)
    ctx->r11 = MEM_W(ctx->r17, 0X2C);
    // 0x80002C64: b           L_80002D94
    // 0x80002C68: sw          $t3, 0x0($t5)
    MEM_W(0X0, ctx->r13) = ctx->r11;
        goto L_80002D94;
    // 0x80002C68: sw          $t3, 0x0($t5)
    MEM_W(0X0, ctx->r13) = ctx->r11;
    // 0x80002C6C: addiu       $at, $zero, 0x4C
    ctx->r1 = ADD32(0, 0X4C);
L_80002C70:
    // 0x80002C70: bnel        $v0, $at, L_80002CAC
    if (ctx->r2 != ctx->r1) {
        // 0x80002C74: lw          $t1, 0x0($s0)
        ctx->r9 = MEM_W(ctx->r16, 0X0);
            goto L_80002CAC;
    }
    goto skip_17;
    // 0x80002C74: lw          $t1, 0x0($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X0);
    skip_17:
    // 0x80002C78: lw          $t4, 0x0($s0)
    ctx->r12 = MEM_W(ctx->r16, 0X0);
    // 0x80002C7C: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x80002C80: addiu       $t2, $zero, 0x0
    ctx->r10 = ADD32(0, 0X0);
    // 0x80002C84: addiu       $t6, $t4, 0x3
    ctx->r14 = ADD32(ctx->r12, 0X3);
    // 0x80002C88: and         $t8, $t6, $at
    ctx->r24 = ctx->r14 & ctx->r1;
    // 0x80002C8C: addiu       $t9, $t8, 0x4
    ctx->r25 = ADD32(ctx->r24, 0X4);
    // 0x80002C90: sw          $t9, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r25;
    // 0x80002C94: lw          $t7, -0x4($t9)
    ctx->r15 = MEM_W(ctx->r25, -0X4);
    // 0x80002C98: lw          $t0, 0x2C($s1)
    ctx->r8 = MEM_W(ctx->r17, 0X2C);
    // 0x80002C9C: sw          $t2, 0x0($t7)
    MEM_W(0X0, ctx->r15) = ctx->r10;
    // 0x80002CA0: b           L_80002D94
    // 0x80002CA4: sw          $t0, 0x4($t7)
    MEM_W(0X4, ctx->r15) = ctx->r8;
        goto L_80002D94;
    // 0x80002CA4: sw          $t0, 0x4($t7)
    MEM_W(0X4, ctx->r15) = ctx->r8;
    // 0x80002CA8: lw          $t1, 0x0($s0)
    ctx->r9 = MEM_W(ctx->r16, 0X0);
L_80002CAC:
    // 0x80002CAC: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x80002CB0: addiu       $t5, $t1, 0x3
    ctx->r13 = ADD32(ctx->r9, 0X3);
    // 0x80002CB4: and         $t4, $t5, $at
    ctx->r12 = ctx->r13 & ctx->r1;
    // 0x80002CB8: addiu       $t6, $t4, 0x4
    ctx->r14 = ADD32(ctx->r12, 0X4);
    // 0x80002CBC: sw          $t6, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r14;
    // 0x80002CC0: lw          $t9, -0x4($t6)
    ctx->r25 = MEM_W(ctx->r14, -0X4);
    // 0x80002CC4: lw          $t8, 0x2C($s1)
    ctx->r24 = MEM_W(ctx->r17, 0X2C);
    // 0x80002CC8: b           L_80002D94
    // 0x80002CCC: sw          $t8, 0x0($t9)
    MEM_W(0X0, ctx->r25) = ctx->r24;
        goto L_80002D94;
    // 0x80002CCC: sw          $t8, 0x0($t9)
    MEM_W(0X0, ctx->r25) = ctx->r24;
    // 0x80002CD0: lw          $t0, 0x0($s0)
    ctx->r8 = MEM_W(ctx->r16, 0X0);
    // 0x80002CD4: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x80002CD8: or          $a0, $s1, $zero
    ctx->r4 = ctx->r17 | 0;
    // 0x80002CDC: addiu       $t2, $t0, 0x3
    ctx->r10 = ADD32(ctx->r8, 0X3);
    // 0x80002CE0: and         $t3, $t2, $at
    ctx->r11 = ctx->r10 & ctx->r1;
    // 0x80002CE4: addiu       $t7, $t3, 0x4
    ctx->r15 = ADD32(ctx->r11, 0X4);
    // 0x80002CE8: sw          $t7, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r15;
    // 0x80002CEC: lw          $t5, -0x4($t7)
    ctx->r13 = MEM_W(ctx->r15, -0X4);
    // 0x80002CF0: lw          $t6, 0xC($s1)
    ctx->r14 = MEM_W(ctx->r17, 0XC);
    // 0x80002CF4: addiu       $a1, $zero, 0x78
    ctx->r5 = ADD32(0, 0X78);
    // 0x80002CF8: sra         $t4, $t5, 31
    ctx->r12 = S32(SIGNED(ctx->r13) >> 31);
    // 0x80002CFC: addu        $t8, $t6, $a3
    ctx->r24 = ADD32(ctx->r14, ctx->r7);
    // 0x80002D00: sw          $t4, 0x0($s1)
    MEM_W(0X0, ctx->r17) = ctx->r12;
    // 0x80002D04: sw          $t8, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r24;
    // 0x80002D08: jal         0x800230F0
    // 0x80002D0C: sw          $t5, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r13;
    _Litob(rdram, ctx);
        goto after_3;
    // 0x80002D0C: sw          $t5, 0x4($s1)
    MEM_W(0X4, ctx->r17) = ctx->r13;
    after_3:
    // 0x80002D10: b           L_80002D98
    // 0x80002D14: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
        goto L_80002D98;
    // 0x80002D14: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80002D18: lw          $t9, 0x0($s0)
    ctx->r25 = MEM_W(ctx->r16, 0X0);
    // 0x80002D1C: addiu       $at, $zero, -0x4
    ctx->r1 = ADD32(0, -0X4);
    // 0x80002D20: addiu       $t0, $t9, 0x3
    ctx->r8 = ADD32(ctx->r25, 0X3);
    // 0x80002D24: and         $t2, $t0, $at
    ctx->r10 = ctx->r8 & ctx->r1;
    // 0x80002D28: addiu       $t3, $t2, 0x4
    ctx->r11 = ADD32(ctx->r10, 0X4);
    // 0x80002D2C: sw          $t3, 0x0($s0)
    MEM_W(0X0, ctx->r16) = ctx->r11;
    // 0x80002D30: lw          $a0, -0x4($t3)
    ctx->r4 = MEM_W(ctx->r11, -0X4);
    // 0x80002D34: jal         0x80022EEC
    // 0x80002D38: sw          $a0, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r4;
    strlen_recomp(rdram, ctx);
        goto after_4;
    // 0x80002D38: sw          $a0, 0x8($s1)
    MEM_W(0X8, ctx->r17) = ctx->r4;
    after_4:
    // 0x80002D3C: lw          $v1, 0x24($s1)
    ctx->r3 = MEM_W(ctx->r17, 0X24);
    // 0x80002D40: sw          $v0, 0x14($s1)
    MEM_W(0X14, ctx->r17) = ctx->r2;
    // 0x80002D44: bltz        $v1, L_80002D94
    if (SIGNED(ctx->r3) < 0) {
        // 0x80002D48: slt         $at, $v1, $v0
        ctx->r1 = SIGNED(ctx->r3) < SIGNED(ctx->r2) ? 1 : 0;
            goto L_80002D94;
    }
    // 0x80002D48: slt         $at, $v1, $v0
    ctx->r1 = SIGNED(ctx->r3) < SIGNED(ctx->r2) ? 1 : 0;
    // 0x80002D4C: beql        $at, $zero, L_80002D98
    if (ctx->r1 == 0) {
        // 0x80002D50: lw          $ra, 0x1C($sp)
        ctx->r31 = MEM_W(ctx->r29, 0X1C);
            goto L_80002D98;
    }
    goto skip_18;
    // 0x80002D50: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    skip_18:
    // 0x80002D54: b           L_80002D94
    // 0x80002D58: sw          $v1, 0x14($s1)
    MEM_W(0X14, ctx->r17) = ctx->r3;
        goto L_80002D94;
    // 0x80002D58: sw          $v1, 0x14($s1)
    MEM_W(0X14, ctx->r17) = ctx->r3;
    // 0x80002D5C: lw          $t6, 0xC($s1)
    ctx->r14 = MEM_W(ctx->r17, 0XC);
L_80002D60:
    // 0x80002D60: addiu       $t5, $zero, 0x25
    ctx->r13 = ADD32(0, 0X25);
    // 0x80002D64: addu        $t8, $a3, $t6
    ctx->r24 = ADD32(ctx->r7, ctx->r14);
    // 0x80002D68: sb          $t5, 0x0($t8)
    MEM_B(0X0, ctx->r24) = ctx->r13;
    // 0x80002D6C: lw          $t9, 0xC($s1)
    ctx->r25 = MEM_W(ctx->r17, 0XC);
    // 0x80002D70: addiu       $t0, $t9, 0x1
    ctx->r8 = ADD32(ctx->r25, 0X1);
    // 0x80002D74: b           L_80002D94
    // 0x80002D78: sw          $t0, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r8;
        goto L_80002D94;
    // 0x80002D78: sw          $t0, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r8;
    // 0x80002D7C: lw          $t2, 0xC($s1)
    ctx->r10 = MEM_W(ctx->r17, 0XC);
L_80002D80:
    // 0x80002D80: addu        $t3, $a3, $t2
    ctx->r11 = ADD32(ctx->r7, ctx->r10);
    // 0x80002D84: sb          $a2, 0x0($t3)
    MEM_B(0X0, ctx->r11) = ctx->r6;
    // 0x80002D88: lw          $t7, 0xC($s1)
    ctx->r15 = MEM_W(ctx->r17, 0XC);
    // 0x80002D8C: addiu       $t1, $t7, 0x1
    ctx->r9 = ADD32(ctx->r15, 0X1);
    // 0x80002D90: sw          $t1, 0xC($s1)
    MEM_W(0XC, ctx->r17) = ctx->r9;
L_80002D94:
    // 0x80002D94: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
L_80002D98:
    // 0x80002D98: lw          $s0, 0x14($sp)
    ctx->r16 = MEM_W(ctx->r29, 0X14);
    // 0x80002D9C: lw          $s1, 0x18($sp)
    ctx->r17 = MEM_W(ctx->r29, 0X18);
    // 0x80002DA0: jr          $ra
    // 0x80002DA4: addiu       $sp, $sp, 0x20
    ctx->r29 = ADD32(ctx->r29, 0X20);
    return;
    // 0x80002DA4: addiu       $sp, $sp, 0x20
    ctx->r29 = ADD32(ctx->r29, 0X20);
    // 0x80002DA8: nop

    // 0x80002DAC: nop

;}
RECOMP_FUNC void _VirtualToPhysicalTask(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80003220: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80003224: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80003228: lui         $a1, 0x8003
    ctx->r5 = S32(0X8003 << 16);
    // 0x8000322C: addiu       $a1, $a1, 0x6B60
    ctx->r5 = ADD32(ctx->r5, 0X6B60);
    // 0x80003230: jal         0x80023A10
    // 0x80003234: addiu       $a2, $zero, 0x40
    ctx->r6 = ADD32(0, 0X40);
    bcopy_recomp(rdram, ctx);
        goto after_0;
    // 0x80003234: addiu       $a2, $zero, 0x40
    ctx->r6 = ADD32(0, 0X40);
    after_0:
    // 0x80003238: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x8000323C: lw          $a0, 0x6B70($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B70);
    // 0x80003240: beq         $a0, $zero, L_80003258
    if (ctx->r4 == 0) {
        // 0x80003244: nop
    
            goto L_80003258;
    }
    // 0x80003244: nop

    // 0x80003248: jal         0x800233C0
    // 0x8000324C: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_1;
    // 0x8000324C: nop

    after_1:
    // 0x80003250: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x80003254: sw          $v0, 0x6B70($at)
    MEM_W(0X6B70, ctx->r1) = ctx->r2;
L_80003258:
    // 0x80003258: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x8000325C: lw          $a0, 0x6B78($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B78);
    // 0x80003260: beq         $a0, $zero, L_80003278
    if (ctx->r4 == 0) {
        // 0x80003264: nop
    
            goto L_80003278;
    }
    // 0x80003264: nop

    // 0x80003268: jal         0x800233C0
    // 0x8000326C: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_2;
    // 0x8000326C: nop

    after_2:
    // 0x80003270: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x80003274: sw          $v0, 0x6B78($at)
    MEM_W(0X6B78, ctx->r1) = ctx->r2;
L_80003278:
    // 0x80003278: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x8000327C: lw          $a0, 0x6B80($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B80);
    // 0x80003280: beq         $a0, $zero, L_80003298
    if (ctx->r4 == 0) {
        // 0x80003284: nop
    
            goto L_80003298;
    }
    // 0x80003284: nop

    // 0x80003288: jal         0x800233C0
    // 0x8000328C: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_3;
    // 0x8000328C: nop

    after_3:
    // 0x80003290: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x80003294: sw          $v0, 0x6B80($at)
    MEM_W(0X6B80, ctx->r1) = ctx->r2;
L_80003298:
    // 0x80003298: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x8000329C: lw          $a0, 0x6B88($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B88);
    // 0x800032A0: beq         $a0, $zero, L_800032B8
    if (ctx->r4 == 0) {
        // 0x800032A4: nop
    
            goto L_800032B8;
    }
    // 0x800032A4: nop

    // 0x800032A8: jal         0x800233C0
    // 0x800032AC: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_4;
    // 0x800032AC: nop

    after_4:
    // 0x800032B0: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x800032B4: sw          $v0, 0x6B88($at)
    MEM_W(0X6B88, ctx->r1) = ctx->r2;
L_800032B8:
    // 0x800032B8: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x800032BC: lw          $a0, 0x6B8C($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B8C);
    // 0x800032C0: beq         $a0, $zero, L_800032D8
    if (ctx->r4 == 0) {
        // 0x800032C4: nop
    
            goto L_800032D8;
    }
    // 0x800032C4: nop

    // 0x800032C8: jal         0x800233C0
    // 0x800032CC: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_5;
    // 0x800032CC: nop

    after_5:
    // 0x800032D0: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x800032D4: sw          $v0, 0x6B8C($at)
    MEM_W(0X6B8C, ctx->r1) = ctx->r2;
L_800032D8:
    // 0x800032D8: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x800032DC: lw          $a0, 0x6B90($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B90);
    // 0x800032E0: beq         $a0, $zero, L_800032F8
    if (ctx->r4 == 0) {
        // 0x800032E4: nop
    
            goto L_800032F8;
    }
    // 0x800032E4: nop

    // 0x800032E8: jal         0x800233C0
    // 0x800032EC: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_6;
    // 0x800032EC: nop

    after_6:
    // 0x800032F0: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x800032F4: sw          $v0, 0x6B90($at)
    MEM_W(0X6B90, ctx->r1) = ctx->r2;
L_800032F8:
    // 0x800032F8: lui         $a0, 0x8003
    ctx->r4 = S32(0X8003 << 16);
    // 0x800032FC: lw          $a0, 0x6B98($a0)
    ctx->r4 = MEM_W(ctx->r4, 0X6B98);
    // 0x80003300: beq         $a0, $zero, L_80003318
    if (ctx->r4 == 0) {
        // 0x80003304: nop
    
            goto L_80003318;
    }
    // 0x80003304: nop

    // 0x80003308: jal         0x800233C0
    // 0x8000330C: nop

    osVirtualToPhysical_recomp(rdram, ctx);
        goto after_7;
    // 0x8000330C: nop

    after_7:
    // 0x80003310: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x80003314: sw          $v0, 0x6B98($at)
    MEM_W(0X6B98, ctx->r1) = ctx->r2;
L_80003318:
    // 0x80003318: lui         $v0, 0x8003
    ctx->r2 = S32(0X8003 << 16);
    // 0x8000331C: addiu       $v0, $v0, 0x6B60
    ctx->r2 = ADD32(ctx->r2, 0X6B60);
    // 0x80003320: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80003324: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80003328: jr          $ra
    // 0x8000332C: nop

    return;
    // 0x8000332C: nop

;}
RECOMP_FUNC void allocate_memory(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
;}
RECOMP_FUNC void alHeapInit(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80012820: sw          $a1, 0x0($a0)
    MEM_W(0X0, ctx->r4) = ctx->r5;
    // 0x80012824: sw          $a2, 0x8($a0)
    MEM_W(0X8, ctx->r4) = ctx->r6;
    // 0x80012828: lw          $t6, 0x0($a0)
    ctx->r14 = MEM_W(ctx->r4, 0X0);
    // 0x8001282C: sw          $t6, 0x4($a0)
    MEM_W(0X4, ctx->r4) = ctx->r14;
    // 0x80012830: sw          $zero, 0xC($a0)
    MEM_W(0XC, ctx->r4) = 0;
    // 0x80012834: jr          $ra
    // 0x80012838: nop

    return;
    // 0x80012838: nop

    // 0x8001283C: jr          $ra
    // 0x80012840: nop

    return;
    // 0x80012840: nop

;}
RECOMP_FUNC void alHeapDBAlloc(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x80012844: addiu       $sp, $sp, -0x8
    ctx->r29 = ADD32(ctx->r29, -0X8);
    // 0x80012848: sw          $a0, 0x8($sp)
    MEM_W(0X8, ctx->r29) = ctx->r4;
    // 0x8001284C: sw          $a1, 0xC($sp)
    MEM_W(0XC, ctx->r29) = ctx->r5;
    // 0x80012850: sw          $zero, 0x0($sp)
    MEM_W(0X0, ctx->r29) = 0;
    // 0x80012854: lw          $t6, 0x18($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X18);
    // 0x80012858: addiu       $at, $zero, -0x10
    ctx->r1 = ADD32(0, -0X10);
    // 0x8001285C: multu       $a3, $t6
    result = U64(U32(ctx->r7)) * U64(U32(ctx->r14)); lo = S32(result >> 0); hi = S32(result >> 32);
    // 0x80012860: mflo        $t7
    ctx->r15 = lo;
    // 0x80012864: addiu       $t8, $t7, 0xF
    ctx->r24 = ADD32(ctx->r15, 0XF);
    // 0x80012868: and         $t9, $t8, $at
    ctx->r25 = ctx->r24 & ctx->r1;
    // 0x8001286C: sw          $t9, 0x4($sp)
    MEM_W(0X4, ctx->r29) = ctx->r25;
    // 0x80012870: lw          $t0, 0x4($a2)
    ctx->r8 = MEM_W(ctx->r6, 0X4);
    // 0x80012874: lw          $t1, 0x4($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X4);
    // 0x80012878: lw          $t3, 0x0($a2)
    ctx->r11 = MEM_W(ctx->r6, 0X0);
    // 0x8001287C: lw          $t4, 0x8($a2)
    ctx->r12 = MEM_W(ctx->r6, 0X8);
    // 0x80012880: addu        $t2, $t0, $t1
    ctx->r10 = ADD32(ctx->r8, ctx->r9);
    // 0x80012884: addu        $t5, $t3, $t4
    ctx->r13 = ADD32(ctx->r11, ctx->r12);
    // 0x80012888: sltu        $at, $t5, $t2
    ctx->r1 = ctx->r13 < ctx->r10 ? 1 : 0;
    // 0x8001288C: bne         $at, $zero, L_800128B0
    if (ctx->r1 != 0) {
        // 0x80012890: nop
    
            goto L_800128B0;
    }
    // 0x80012890: nop

    // 0x80012894: lw          $t6, 0x4($a2)
    ctx->r14 = MEM_W(ctx->r6, 0X4);
    // 0x80012898: sw          $t6, 0x0($sp)
    MEM_W(0X0, ctx->r29) = ctx->r14;
    // 0x8001289C: lw          $t7, 0x4($a2)
    ctx->r15 = MEM_W(ctx->r6, 0X4);
    // 0x800128A0: lw          $t8, 0x4($sp)
    ctx->r24 = MEM_W(ctx->r29, 0X4);
    // 0x800128A4: addu        $t9, $t7, $t8
    ctx->r25 = ADD32(ctx->r15, ctx->r24);
    // 0x800128A8: b           L_800128B0
    // 0x800128AC: sw          $t9, 0x4($a2)
    MEM_W(0X4, ctx->r6) = ctx->r25;
        goto L_800128B0;
    // 0x800128AC: sw          $t9, 0x4($a2)
    MEM_W(0X4, ctx->r6) = ctx->r25;
L_800128B0:
    // 0x800128B0: b           L_800128C0
    // 0x800128B4: lw          $v0, 0x0($sp)
    ctx->r2 = MEM_W(ctx->r29, 0X0);
        goto L_800128C0;
    // 0x800128B4: lw          $v0, 0x0($sp)
    ctx->r2 = MEM_W(ctx->r29, 0X0);
    // 0x800128B8: b           L_800128C0
    // 0x800128BC: nop

        goto L_800128C0;
    // 0x800128BC: nop

L_800128C0:
    // 0x800128C0: jr          $ra
    // 0x800128C4: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    return;
    // 0x800128C4: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    // 0x800128C8: nop

    // 0x800128CC: nop

;}
RECOMP_FUNC void alSeqFileNew(uint8_t* rdram, recomp_context* ctx) {
    uint64_t hi = 0, lo = 0, result = 0;
    int c1cs = 0;
    // 0x800128D0: addiu       $sp, $sp, -0x8
    ctx->r29 = ADD32(ctx->r29, -0X8);
    // 0x800128D4: sw          $a1, 0x4($sp)
    MEM_W(0X4, ctx->r29) = ctx->r5;
    // 0x800128D8: sw          $zero, 0x0($sp)
    MEM_W(0X0, ctx->r29) = 0;
    // 0x800128DC: lh          $t6, 0x2($a0)
    ctx->r14 = MEM_H(ctx->r4, 0X2);
    // 0x800128E0: blez        $t6, L_80012924
    if (SIGNED(ctx->r14) <= 0) {
        // 0x800128E4: nop
    
            goto L_80012924;
    }
    // 0x800128E4: nop

L_800128E8:
    // 0x800128E8: lw          $t7, 0x0($sp)
    ctx->r15 = MEM_W(ctx->r29, 0X0);
    // 0x800128EC: lw          $t1, 0x4($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X4);
    // 0x800128F0: sll         $t8, $t7, 3
    ctx->r24 = S32(ctx->r15 << 3);
    // 0x800128F4: addu        $t9, $a0, $t8
    ctx->r25 = ADD32(ctx->r4, ctx->r24);
    // 0x800128F8: lw          $t0, 0x4($t9)
    ctx->r8 = MEM_W(ctx->r25, 0X4);
    // 0x800128FC: addu        $t3, $a0, $t8
    ctx->r11 = ADD32(ctx->r4, ctx->r24);
    // 0x80012900: addu        $t2, $t0, $t1
    ctx->r10 = ADD32(ctx->r8, ctx->r9);
    // 0x80012904: sw          $t2, 0x4($t3)
    MEM_W(0X4, ctx->r11) = ctx->r10;
    // 0x80012908: lw          $t4, 0x0($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X0);
    // 0x8001290C: addiu       $t5, $t4, 0x1
    ctx->r13 = ADD32(ctx->r12, 0X1);
    // 0x80012910: sw          $t5, 0x0($sp)
    MEM_W(0X0, ctx->r29) = ctx->r13;
    // 0x80012914: lh          $t6, 0x2($a0)
    ctx->r14 = MEM_H(ctx->r4, 0X2);
    // 0x80012918: slt         $at, $t5, $t6
    ctx->r1 = SIGNED(ctx->r13) < SIGNED(ctx->r14) ? 1 : 0;
    // 0x8001291C: bne         $at, $zero, L_800128E8
    if (ctx->r1 != 0) {
        // 0x80012920: nop
    
            goto L_800128E8;
    }
    // 0x80012920: nop

L_80012924:
    // 0x80012924: b           L_8001292C
    // 0x80012928: nop

        goto L_8001292C;
    // 0x80012928: nop

L_8001292C:
    // 0x8001292C: jr          $ra
    // 0x80012930: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    return;
    // 0x80012930: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    // 0x80012934: addiu       $sp, $sp, -0x28
    ctx->r29 = ADD32(ctx->r29, -0X28);
    // 0x80012938: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x8001293C: sw          $a0, 0x28($sp)
    MEM_W(0X28, ctx->r29) = ctx->r4;
    // 0x80012940: sw          $a1, 0x2C($sp)
    MEM_W(0X2C, ctx->r29) = ctx->r5;
    // 0x80012944: sw          $a2, 0x30($sp)
    MEM_W(0X30, ctx->r29) = ctx->r6;
    // 0x80012948: lw          $t6, 0x28($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X28);
    // 0x8001294C: sw          $t6, 0x24($sp)
    MEM_W(0X24, ctx->r29) = ctx->r14;
    // 0x80012950: lw          $t7, 0x2C($sp)
    ctx->r15 = MEM_W(ctx->r29, 0X2C);
    // 0x80012954: sw          $t7, 0x20($sp)
    MEM_W(0X20, ctx->r29) = ctx->r15;
    // 0x80012958: lw          $t8, 0x30($sp)
    ctx->r24 = MEM_W(ctx->r29, 0X30);
    // 0x8001295C: sw          $t8, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r24;
    // 0x80012960: lw          $t9, 0x28($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X28);
    // 0x80012964: addiu       $at, $zero, 0x4231
    ctx->r1 = ADD32(0, 0X4231);
    // 0x80012968: lh          $t0, 0x0($t9)
    ctx->r8 = MEM_H(ctx->r25, 0X0);
    // 0x8001296C: beq         $t0, $at, L_8001297C
    if (ctx->r8 == ctx->r1) {
        // 0x80012970: nop
    
            goto L_8001297C;
    }
    // 0x80012970: nop

    // 0x80012974: b           L_80012A18
    // 0x80012978: nop

        goto L_80012A18;
    // 0x80012978: nop

L_8001297C:
    // 0x8001297C: lw          $t1, 0x28($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X28);
    // 0x80012980: sw          $zero, 0x18($sp)
    MEM_W(0X18, ctx->r29) = 0;
    // 0x80012984: lh          $t2, 0x2($t1)
    ctx->r10 = MEM_H(ctx->r9, 0X2);
    // 0x80012988: blez        $t2, L_80012A10
    if (SIGNED(ctx->r10) <= 0) {
        // 0x8001298C: nop
    
            goto L_80012A10;
    }
    // 0x8001298C: nop

L_80012990:
    // 0x80012990: lw          $t4, 0x18($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X18);
    // 0x80012994: lw          $t3, 0x28($sp)
    ctx->r11 = MEM_W(ctx->r29, 0X28);
    // 0x80012998: lw          $t8, 0x24($sp)
    ctx->r24 = MEM_W(ctx->r29, 0X24);
    // 0x8001299C: sll         $t5, $t4, 2
    ctx->r13 = S32(ctx->r12 << 2);
    // 0x800129A0: addu        $t6, $t3, $t5
    ctx->r14 = ADD32(ctx->r11, ctx->r13);
    // 0x800129A4: lw          $t7, 0x4($t6)
    ctx->r15 = MEM_W(ctx->r14, 0X4);
    // 0x800129A8: addu        $t9, $t7, $t8
    ctx->r25 = ADD32(ctx->r15, ctx->r24);
    // 0x800129AC: sw          $t9, 0x4($t6)
    MEM_W(0X4, ctx->r14) = ctx->r25;
    // 0x800129B0: lw          $t1, 0x18($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X18);
    // 0x800129B4: lw          $t0, 0x28($sp)
    ctx->r8 = MEM_W(ctx->r29, 0X28);
    // 0x800129B8: sll         $t2, $t1, 2
    ctx->r10 = S32(ctx->r9 << 2);
    // 0x800129BC: addu        $t4, $t0, $t2
    ctx->r12 = ADD32(ctx->r8, ctx->r10);
    // 0x800129C0: lw          $t3, 0x4($t4)
    ctx->r11 = MEM_W(ctx->r12, 0X4);
    // 0x800129C4: beq         $t3, $zero, L_800129F0
    if (ctx->r11 == 0) {
        // 0x800129C8: nop
    
            goto L_800129F0;
    }
    // 0x800129C8: nop

    // 0x800129CC: lw          $t7, 0x18($sp)
    ctx->r15 = MEM_W(ctx->r29, 0X18);
    // 0x800129D0: lw          $t5, 0x28($sp)
    ctx->r13 = MEM_W(ctx->r29, 0X28);
    // 0x800129D4: lw          $a1, 0x24($sp)
    ctx->r5 = MEM_W(ctx->r29, 0X24);
    // 0x800129D8: sll         $t8, $t7, 2
    ctx->r24 = S32(ctx->r15 << 2);
    // 0x800129DC: addu        $t9, $t5, $t8
    ctx->r25 = ADD32(ctx->r13, ctx->r24);
    // 0x800129E0: lw          $a0, 0x4($t9)
    ctx->r4 = MEM_W(ctx->r25, 0X4);
    // 0x800129E4: lw          $a2, 0x20($sp)
    ctx->r6 = MEM_W(ctx->r29, 0X20);
    // 0x800129E8: jal         0x80012A28
    // 0x800129EC: lw          $a3, 0x1C($sp)
    ctx->r7 = MEM_W(ctx->r29, 0X1C);
    static_0_80012A28(rdram, ctx);
        goto after_0;
    // 0x800129EC: lw          $a3, 0x1C($sp)
    ctx->r7 = MEM_W(ctx->r29, 0X1C);
    after_0:
L_800129F0:
    // 0x800129F0: lw          $t6, 0x18($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X18);
    // 0x800129F4: lw          $t0, 0x28($sp)
    ctx->r8 = MEM_W(ctx->r29, 0X28);
    // 0x800129F8: addiu       $t1, $t6, 0x1
    ctx->r9 = ADD32(ctx->r14, 0X1);
    // 0x800129FC: sw          $t1, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r9;
    // 0x80012A00: lh          $t2, 0x2($t0)
    ctx->r10 = MEM_H(ctx->r8, 0X2);
    // 0x80012A04: slt         $at, $t1, $t2
    ctx->r1 = SIGNED(ctx->r9) < SIGNED(ctx->r10) ? 1 : 0;
    // 0x80012A08: bne         $at, $zero, L_80012990
    if (ctx->r1 != 0) {
        // 0x80012A0C: nop
    
            goto L_80012990;
    }
    // 0x80012A0C: nop

L_80012A10:
    // 0x80012A10: b           L_80012A18
    // 0x80012A14: nop

        goto L_80012A18;
    // 0x80012A14: nop

L_80012A18:
    // 0x80012A18: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80012A1C: addiu       $sp, $sp, 0x28
    ctx->r29 = ADD32(ctx->r29, 0X28);
    // 0x80012A20: jr          $ra
    // 0x80012A24: nop

    return;
    // 0x80012A24: nop

    // 0x80012A28: addiu       $sp, $sp, -0x20
    ctx->r29 = ADD32(ctx->r29, -0X20);
    // 0x80012A2C: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80012A30: sw          $a0, 0x20($sp)
    MEM_W(0X20, ctx->r29) = ctx->r4;
    // 0x80012A34: sw          $a1, 0x24($sp)
    MEM_W(0X24, ctx->r29) = ctx->r5;
    // 0x80012A38: sw          $a2, 0x28($sp)
    MEM_W(0X28, ctx->r29) = ctx->r6;
    // 0x80012A3C: sw          $a3, 0x2C($sp)
    MEM_W(0X2C, ctx->r29) = ctx->r7;
    // 0x80012A40: lw          $t6, 0x20($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X20);
    // 0x80012A44: lbu         $t7, 0x2($t6)
    ctx->r15 = MEM_BU(ctx->r14, 0X2);
    // 0x80012A48: beq         $t7, $zero, L_80012A58
    if (ctx->r15 == 0) {
        // 0x80012A4C: nop
    
            goto L_80012A58;
    }
    // 0x80012A4C: nop

    // 0x80012A50: b           L_80012B74
    // 0x80012A54: nop

        goto L_80012B74;
    // 0x80012A54: nop

L_80012A58:
    // 0x80012A58: lw          $t9, 0x20($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X20);
    // 0x80012A5C: addiu       $t8, $zero, 0x1
    ctx->r24 = ADD32(0, 0X1);
    // 0x80012A60: sb          $t8, 0x2($t9)
    MEM_B(0X2, ctx->r25) = ctx->r24;
    // 0x80012A64: lw          $t0, 0x20($sp)
    ctx->r8 = MEM_W(ctx->r29, 0X20);
    // 0x80012A68: lw          $t1, 0x8($t0)
    ctx->r9 = MEM_W(ctx->r8, 0X8);
    // 0x80012A6C: beq         $t1, $zero, L_80012A98
    if (ctx->r9 == 0) {
        // 0x80012A70: nop
    
            goto L_80012A98;
    }
    // 0x80012A70: nop

    // 0x80012A74: lw          $t2, 0x20($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X20);
    // 0x80012A78: lw          $t4, 0x24($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X24);
    // 0x80012A7C: lw          $t3, 0x8($t2)
    ctx->r11 = MEM_W(ctx->r10, 0X8);
    // 0x80012A80: addu        $t5, $t3, $t4
    ctx->r13 = ADD32(ctx->r11, ctx->r12);
    // 0x80012A84: sw          $t5, 0x8($t2)
    MEM_W(0X8, ctx->r10) = ctx->r13;
    // 0x80012A88: lw          $t6, 0x20($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X20);
    // 0x80012A8C: lw          $a1, 0x2C($sp)
    ctx->r5 = MEM_W(ctx->r29, 0X2C);
    // 0x80012A90: jal         0x80012B84
    // 0x80012A94: lw          $a0, 0x8($t6)
    ctx->r4 = MEM_W(ctx->r14, 0X8);
    static_0_80012B84(rdram, ctx);
        goto after_1;
    // 0x80012A94: lw          $a0, 0x8($t6)
    ctx->r4 = MEM_W(ctx->r14, 0X8);
    after_1:
L_80012A98:
    // 0x80012A98: lw          $t7, 0x20($sp)
    ctx->r15 = MEM_W(ctx->r29, 0X20);
    // 0x80012A9C: sw          $zero, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = 0;
    // 0x80012AA0: lh          $t8, 0x0($t7)
    ctx->r24 = MEM_H(ctx->r15, 0X0);
    // 0x80012AA4: blez        $t8, L_80012B6C
    if (SIGNED(ctx->r24) <= 0) {
        // 0x80012AA8: nop
    
            goto L_80012B6C;
    }
    // 0x80012AA8: nop

L_80012AAC:
    // 0x80012AAC: lw          $t0, 0x1C($sp)
    ctx->r8 = MEM_W(ctx->r29, 0X1C);
    // 0x80012AB0: lw          $t9, 0x20($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X20);
    // 0x80012AB4: sll         $t1, $t0, 2
    ctx->r9 = S32(ctx->r8 << 2);
    // 0x80012AB8: addu        $t3, $t9, $t1
    ctx->r11 = ADD32(ctx->r25, ctx->r9);
    // 0x80012ABC: lw          $t4, 0xC($t3)
    ctx->r12 = MEM_W(ctx->r11, 0XC);
    // 0x80012AC0: beq         $t4, $zero, L_80012B4C
    if (ctx->r12 == 0) {
        // 0x80012AC4: nop
    
            goto L_80012B4C;
    }
    // 0x80012AC4: nop

    // 0x80012AC8: lw          $t5, 0x1C($sp)
    ctx->r13 = MEM_W(ctx->r29, 0X1C);
    // 0x80012ACC: bne         $t5, $zero, L_80012B18
    if (ctx->r13 != 0) {
        // 0x80012AD0: nop
    
            goto L_80012B18;
    }
    // 0x80012AD0: nop

    // 0x80012AD4: lw          $t6, 0x1C($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X1C);
    // 0x80012AD8: lw          $t2, 0x20($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X20);
    // 0x80012ADC: lw          $t9, 0x24($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X24);
    // 0x80012AE0: sll         $t7, $t6, 2
    ctx->r15 = S32(ctx->r14 << 2);
    // 0x80012AE4: addu        $t8, $t2, $t7
    ctx->r24 = ADD32(ctx->r10, ctx->r15);
    // 0x80012AE8: lw          $t0, 0xC($t8)
    ctx->r8 = MEM_W(ctx->r24, 0XC);
    // 0x80012AEC: addu        $t1, $t0, $t9
    ctx->r9 = ADD32(ctx->r8, ctx->r25);
    // 0x80012AF0: sw          $t1, 0xC($t8)
    MEM_W(0XC, ctx->r24) = ctx->r9;
    // 0x80012AF4: lw          $t4, 0x1C($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X1C);
    // 0x80012AF8: lw          $t3, 0x20($sp)
    ctx->r11 = MEM_W(ctx->r29, 0X20);
    // 0x80012AFC: lw          $a1, 0x2C($sp)
    ctx->r5 = MEM_W(ctx->r29, 0X2C);
    // 0x80012B00: sll         $t5, $t4, 2
    ctx->r13 = S32(ctx->r12 << 2);
    // 0x80012B04: addu        $t6, $t3, $t5
    ctx->r14 = ADD32(ctx->r11, ctx->r13);
    // 0x80012B08: jal         0x80012BD0
    // 0x80012B0C: lw          $a0, 0xC($t6)
    ctx->r4 = MEM_W(ctx->r14, 0XC);
    static_0_80012BD0(rdram, ctx);
        goto after_2;
    // 0x80012B0C: lw          $a0, 0xC($t6)
    ctx->r4 = MEM_W(ctx->r14, 0XC);
    after_2:
    // 0x80012B10: b           L_80012B4C
    // 0x80012B14: nop

        goto L_80012B4C;
    // 0x80012B14: nop

L_80012B18:
    // 0x80012B18: lw          $t2, 0x2C($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X2C);
    // 0x80012B1C: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80012B20: and         $t7, $t2, $at
    ctx->r15 = ctx->r10 & ctx->r1;
    // 0x80012B24: sll         $t0, $t7, 5
    ctx->r8 = S32(ctx->r15 << 5);
    // 0x80012B28: sw          $t0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r8;
    // 0x80012B2C: lw          $t1, 0x1C($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X1C);
    // 0x80012B30: lw          $t9, 0x20($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X20);
    // 0x80012B34: lw          $t5, 0x18($sp)
    ctx->r13 = MEM_W(ctx->r29, 0X18);
    // 0x80012B38: sll         $t8, $t1, 2
    ctx->r24 = S32(ctx->r9 << 2);
    // 0x80012B3C: addu        $t4, $t9, $t8
    ctx->r12 = ADD32(ctx->r25, ctx->r24);
    // 0x80012B40: lw          $t3, 0xC($t4)
    ctx->r11 = MEM_W(ctx->r12, 0XC);
    // 0x80012B44: addu        $t6, $t3, $t5
    ctx->r14 = ADD32(ctx->r11, ctx->r13);
    // 0x80012B48: sw          $t6, 0xC($t4)
    MEM_W(0XC, ctx->r12) = ctx->r14;
L_80012B4C:
    // 0x80012B4C: lw          $t2, 0x1C($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X1C);
    // 0x80012B50: lw          $t0, 0x20($sp)
    ctx->r8 = MEM_W(ctx->r29, 0X20);
    // 0x80012B54: addiu       $t7, $t2, 0x1
    ctx->r15 = ADD32(ctx->r10, 0X1);
    // 0x80012B58: sw          $t7, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r15;
    // 0x80012B5C: lh          $t1, 0x0($t0)
    ctx->r9 = MEM_H(ctx->r8, 0X0);
    // 0x80012B60: slt         $at, $t7, $t1
    ctx->r1 = SIGNED(ctx->r15) < SIGNED(ctx->r9) ? 1 : 0;
    // 0x80012B64: bne         $at, $zero, L_80012AAC
    if (ctx->r1 != 0) {
        // 0x80012B68: nop
    
            goto L_80012AAC;
    }
    // 0x80012B68: nop

L_80012B6C:
    // 0x80012B6C: b           L_80012B74
    // 0x80012B70: nop

        goto L_80012B74;
    // 0x80012B70: nop

L_80012B74:
    // 0x80012B74: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80012B78: addiu       $sp, $sp, 0x20
    ctx->r29 = ADD32(ctx->r29, 0X20);
    // 0x80012B7C: jr          $ra
    // 0x80012B80: nop

    return;
    // 0x80012B80: nop

    // 0x80012B84: lbu         $t6, 0x3($a0)
    ctx->r14 = MEM_BU(ctx->r4, 0X3);
    // 0x80012B88: beq         $t6, $zero, L_80012B98
    if (ctx->r14 == 0) {
        // 0x80012B8C: nop
    
            goto L_80012B98;
    }
    // 0x80012B8C: nop

    // 0x80012B90: jr          $ra
    // 0x80012B94: nop

    return;
    // 0x80012B94: nop

L_80012B98:
    // 0x80012B98: addiu       $t7, $zero, 0x1
    ctx->r15 = ADD32(0, 0X1);
    // 0x80012B9C: sb          $t7, 0x3($a0)
    MEM_B(0X3, ctx->r4) = ctx->r15;
    // 0x80012BA0: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80012BA4: and         $t8, $a1, $at
    ctx->r24 = ctx->r5 & ctx->r1;
    // 0x80012BA8: or          $a1, $t8, $zero
    ctx->r5 = ctx->r24 | 0;
    // 0x80012BAC: sll         $t9, $a1, 5
    ctx->r25 = S32(ctx->r5 << 5);
    // 0x80012BB0: or          $a1, $t9, $zero
    ctx->r5 = ctx->r25 | 0;
    // 0x80012BB4: lw          $t0, 0x10($a0)
    ctx->r8 = MEM_W(ctx->r4, 0X10);
    // 0x80012BB8: addu        $t1, $t0, $a1
    ctx->r9 = ADD32(ctx->r8, ctx->r5);
    // 0x80012BBC: sw          $t1, 0x10($a0)
    MEM_W(0X10, ctx->r4) = ctx->r9;
    // 0x80012BC0: jr          $ra
    // 0x80012BC4: nop

    return;
    // 0x80012BC4: nop

    // 0x80012BC8: jr          $ra
    // 0x80012BCC: nop

    return;
    // 0x80012BCC: nop

    // 0x80012BD0: addiu       $sp, $sp, -0x8
    ctx->r29 = ADD32(ctx->r29, -0X8);
    // 0x80012BD4: lbu         $t6, 0x3($a0)
    ctx->r14 = MEM_BU(ctx->r4, 0X3);
    // 0x80012BD8: beq         $t6, $zero, L_80012BE8
    if (ctx->r14 == 0) {
        // 0x80012BDC: nop
    
            goto L_80012BE8;
    }
    // 0x80012BDC: nop

    // 0x80012BE0: b           L_80012C54
    // 0x80012BE4: nop

        goto L_80012C54;
    // 0x80012BE4: nop

L_80012BE8:
    // 0x80012BE8: addiu       $t7, $zero, 0x1
    ctx->r15 = ADD32(0, 0X1);
    // 0x80012BEC: sb          $t7, 0x3($a0)
    MEM_B(0X3, ctx->r4) = ctx->r15;
    // 0x80012BF0: addiu       $at, $zero, -0x8
    ctx->r1 = ADD32(0, -0X8);
    // 0x80012BF4: and         $t8, $a1, $at
    ctx->r24 = ctx->r5 & ctx->r1;
    // 0x80012BF8: or          $a1, $t8, $zero
    ctx->r5 = ctx->r24 | 0;
    // 0x80012BFC: sll         $t9, $a1, 5
    ctx->r25 = S32(ctx->r5 << 5);
    // 0x80012C00: or          $a1, $t9, $zero
    ctx->r5 = ctx->r25 | 0;
    // 0x80012C04: sw          $zero, 0x4($sp)
    MEM_W(0X4, ctx->r29) = 0;
    // 0x80012C08: lh          $t0, 0xE($a0)
    ctx->r8 = MEM_H(ctx->r4, 0XE);
    // 0x80012C0C: blez        $t0, L_80012C4C
    if (SIGNED(ctx->r8) <= 0) {
        // 0x80012C10: nop
    
            goto L_80012C4C;
    }
    // 0x80012C10: nop

L_80012C14:
    // 0x80012C14: lw          $t1, 0x4($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X4);
    // 0x80012C18: sll         $t2, $t1, 2
    ctx->r10 = S32(ctx->r9 << 2);
    // 0x80012C1C: addu        $t3, $a0, $t2
    ctx->r11 = ADD32(ctx->r4, ctx->r10);
    // 0x80012C20: lw          $t4, 0x10($t3)
    ctx->r12 = MEM_W(ctx->r11, 0X10);
    // 0x80012C24: addu        $t6, $a0, $t2
    ctx->r14 = ADD32(ctx->r4, ctx->r10);
    // 0x80012C28: addu        $t5, $t4, $a1
    ctx->r13 = ADD32(ctx->r12, ctx->r5);
    // 0x80012C2C: sw          $t5, 0x10($t6)
    MEM_W(0X10, ctx->r14) = ctx->r13;
    // 0x80012C30: lw          $t7, 0x4($sp)
    ctx->r15 = MEM_W(ctx->r29, 0X4);
    // 0x80012C34: addiu       $t8, $t7, 0x1
    ctx->r24 = ADD32(ctx->r15, 0X1);
    // 0x80012C38: sw          $t8, 0x4($sp)
    MEM_W(0X4, ctx->r29) = ctx->r24;
    // 0x80012C3C: lh          $t9, 0xE($a0)
    ctx->r25 = MEM_H(ctx->r4, 0XE);
    // 0x80012C40: slt         $at, $t8, $t9
    ctx->r1 = SIGNED(ctx->r24) < SIGNED(ctx->r25) ? 1 : 0;
    // 0x80012C44: bne         $at, $zero, L_80012C14
    if (ctx->r1 != 0) {
        // 0x80012C48: nop
    
            goto L_80012C14;
    }
    // 0x80012C48: nop

L_80012C4C:
    // 0x80012C4C: b           L_80012C54
    // 0x80012C50: nop

        goto L_80012C54;
    // 0x80012C50: nop

L_80012C54:
    // 0x80012C54: jr          $ra
    // 0x80012C58: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    return;
    // 0x80012C58: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    // 0x80012C5C: addiu       $sp, $sp, -0x18
    ctx->r29 = ADD32(ctx->r29, -0X18);
    // 0x80012C60: sw          $ra, 0x14($sp)
    MEM_W(0X14, ctx->r29) = ctx->r31;
    // 0x80012C64: sw          $a0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r4;
    // 0x80012C68: sw          $a1, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r5;
    // 0x80012C6C: sw          $a2, 0x20($sp)
    MEM_W(0X20, ctx->r29) = ctx->r6;
    // 0x80012C70: lw          $t6, 0x18($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X18);
    // 0x80012C74: lbu         $t7, 0xE($t6)
    ctx->r15 = MEM_BU(ctx->r14, 0XE);
    // 0x80012C78: beq         $t7, $zero, L_80012C88
    if (ctx->r15 == 0) {
        // 0x80012C7C: nop
    
            goto L_80012C88;
    }
    // 0x80012C7C: nop

    // 0x80012C80: b           L_80012CEC
    // 0x80012C84: nop

        goto L_80012CEC;
    // 0x80012C84: nop

L_80012C88:
    // 0x80012C88: lw          $t9, 0x18($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X18);
    // 0x80012C8C: addiu       $t8, $zero, 0x1
    ctx->r24 = ADD32(0, 0X1);
    // 0x80012C90: sb          $t8, 0xE($t9)
    MEM_B(0XE, ctx->r25) = ctx->r24;
    // 0x80012C94: lw          $t0, 0x18($sp)
    ctx->r8 = MEM_W(ctx->r29, 0X18);
    // 0x80012C98: lw          $t2, 0x1C($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X1C);
    // 0x80012C9C: lw          $t1, 0x0($t0)
    ctx->r9 = MEM_W(ctx->r8, 0X0);
    // 0x80012CA0: addu        $t3, $t1, $t2
    ctx->r11 = ADD32(ctx->r9, ctx->r10);
    // 0x80012CA4: sw          $t3, 0x0($t0)
    MEM_W(0X0, ctx->r8) = ctx->r11;
    // 0x80012CA8: lw          $t4, 0x18($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X18);
    // 0x80012CAC: lw          $t6, 0x1C($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X1C);
    // 0x80012CB0: lw          $t5, 0x4($t4)
    ctx->r13 = MEM_W(ctx->r12, 0X4);
    // 0x80012CB4: addu        $t7, $t5, $t6
    ctx->r15 = ADD32(ctx->r13, ctx->r14);
    // 0x80012CB8: sw          $t7, 0x4($t4)
    MEM_W(0X4, ctx->r12) = ctx->r15;
    // 0x80012CBC: lw          $t8, 0x18($sp)
    ctx->r24 = MEM_W(ctx->r29, 0X18);
    // 0x80012CC0: lw          $t1, 0x1C($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X1C);
    // 0x80012CC4: lw          $t9, 0x8($t8)
    ctx->r25 = MEM_W(ctx->r24, 0X8);
    // 0x80012CC8: addu        $t2, $t9, $t1
    ctx->r10 = ADD32(ctx->r25, ctx->r9);
    // 0x80012CCC: sw          $t2, 0x8($t8)
    MEM_W(0X8, ctx->r24) = ctx->r10;
    // 0x80012CD0: lw          $t3, 0x18($sp)
    ctx->r11 = MEM_W(ctx->r29, 0X18);
    // 0x80012CD4: lw          $a1, 0x1C($sp)
    ctx->r5 = MEM_W(ctx->r29, 0X1C);
    // 0x80012CD8: lw          $a2, 0x20($sp)
    ctx->r6 = MEM_W(ctx->r29, 0X20);
    // 0x80012CDC: jal         0x80012CFC
    // 0x80012CE0: lw          $a0, 0x8($t3)
    ctx->r4 = MEM_W(ctx->r11, 0X8);
    static_0_80012CFC(rdram, ctx);
        goto after_3;
    // 0x80012CE0: lw          $a0, 0x8($t3)
    ctx->r4 = MEM_W(ctx->r11, 0X8);
    after_3:
    // 0x80012CE4: b           L_80012CEC
    // 0x80012CE8: nop

        goto L_80012CEC;
    // 0x80012CE8: nop

L_80012CEC:
    // 0x80012CEC: lw          $ra, 0x14($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X14);
    // 0x80012CF0: addiu       $sp, $sp, 0x18
    ctx->r29 = ADD32(ctx->r29, 0X18);
    // 0x80012CF4: jr          $ra
    // 0x80012CF8: nop

    return;
    // 0x80012CF8: nop

    // 0x80012CFC: addiu       $t6, $zero, 0x1
    ctx->r14 = ADD32(0, 0X1);
    // 0x80012D00: sb          $t6, 0x9($a0)
    MEM_B(0X9, ctx->r4) = ctx->r14;
    // 0x80012D04: lw          $t7, 0x0($a0)
    ctx->r15 = MEM_W(ctx->r4, 0X0);
    // 0x80012D08: addu        $t8, $t7, $a2
    ctx->r24 = ADD32(ctx->r15, ctx->r6);
    // 0x80012D0C: sw          $t8, 0x0($a0)
    MEM_W(0X0, ctx->r4) = ctx->r24;
    // 0x80012D10: lbu         $t9, 0x8($a0)
    ctx->r25 = MEM_BU(ctx->r4, 0X8);
    // 0x80012D14: bne         $t9, $zero, L_80012D48
    if (ctx->r25 != 0) {
        // 0x80012D18: nop
    
            goto L_80012D48;
    }
    // 0x80012D18: nop

    // 0x80012D1C: lw          $t0, 0x10($a0)
    ctx->r8 = MEM_W(ctx->r4, 0X10);
    // 0x80012D20: addu        $t1, $t0, $a1
    ctx->r9 = ADD32(ctx->r8, ctx->r5);
    // 0x80012D24: sw          $t1, 0x10($a0)
    MEM_W(0X10, ctx->r4) = ctx->r9;
    // 0x80012D28: lw          $t2, 0xC($a0)
    ctx->r10 = MEM_W(ctx->r4, 0XC);
    // 0x80012D2C: beq         $t2, $zero, L_80012D40
    if (ctx->r10 == 0) {
        // 0x80012D30: nop
    
            goto L_80012D40;
    }
    // 0x80012D30: nop

    // 0x80012D34: lw          $t3, 0xC($a0)
    ctx->r11 = MEM_W(ctx->r4, 0XC);
    // 0x80012D38: addu        $t4, $t3, $a1
    ctx->r12 = ADD32(ctx->r11, ctx->r5);
    // 0x80012D3C: sw          $t4, 0xC($a0)
    MEM_W(0XC, ctx->r4) = ctx->r12;
L_80012D40:
    // 0x80012D40: b           L_80012D70
    // 0x80012D44: sw          $zero, 0x14($a0)
    MEM_W(0X14, ctx->r4) = 0;
        goto L_80012D70;
    // 0x80012D44: sw          $zero, 0x14($a0)
    MEM_W(0X14, ctx->r4) = 0;
L_80012D48:
    // 0x80012D48: lbu         $t5, 0x8($a0)
    ctx->r13 = MEM_BU(ctx->r4, 0X8);
    // 0x80012D4C: addiu       $at, $zero, 0x1
    ctx->r1 = ADD32(0, 0X1);
    // 0x80012D50: bne         $t5, $at, L_80012D70
    if (ctx->r13 != ctx->r1) {
        // 0x80012D54: nop
    
            goto L_80012D70;
    }
    // 0x80012D54: nop

    // 0x80012D58: lw          $t6, 0xC($a0)
    ctx->r14 = MEM_W(ctx->r4, 0XC);
    // 0x80012D5C: beq         $t6, $zero, L_80012D70
    if (ctx->r14 == 0) {
        // 0x80012D60: nop
    
            goto L_80012D70;
    }
    // 0x80012D60: nop

    // 0x80012D64: lw          $t7, 0xC($a0)
    ctx->r15 = MEM_W(ctx->r4, 0XC);
    // 0x80012D68: addu        $t8, $t7, $a1
    ctx->r24 = ADD32(ctx->r15, ctx->r5);
    // 0x80012D6C: sw          $t8, 0xC($a0)
    MEM_W(0XC, ctx->r4) = ctx->r24;
L_80012D70:
    // 0x80012D70: jr          $ra
    // 0x80012D74: nop

    return;
    // 0x80012D74: nop

    // 0x80012D78: jr          $ra
    // 0x80012D7C: nop

    return;
    // 0x80012D7C: nop

    // 0x80012D80: addiu       $sp, $sp, -0x8
    ctx->r29 = ADD32(ctx->r29, -0X8);
    // 0x80012D84: andi        $a0, $a0, 0xFF
    ctx->r4 = ctx->r4 & 0XFF;
    // 0x80012D88: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x80012D8C: lwc1        $f4, -0x3BB0($at)
    ctx->f4.u32l = MEM_W(ctx->r1, -0X3BB0);
    // 0x80012D90: swc1        $f4, 0x4($sp)
    MEM_W(0X4, ctx->r29) = ctx->f4.u32l;
    // 0x80012D94: lui         $at, 0x3F80
    ctx->r1 = S32(0X3F80 << 16);
    // 0x80012D98: mtc1        $at, $f6
    ctx->f6.u32l = ctx->r1;
    // 0x80012D9C: nop

    // 0x80012DA0: swc1        $f6, 0x0($sp)
    MEM_W(0X0, ctx->r29) = ctx->f6.u32l;
    // 0x80012DA4: beq         $a0, $zero, L_80012DEC
    if (ctx->r4 == 0) {
        // 0x80012DA8: nop
    
            goto L_80012DEC;
    }
    // 0x80012DA8: nop

L_80012DAC:
    // 0x80012DAC: andi        $t6, $a0, 0x1
    ctx->r14 = ctx->r4 & 0X1;
    // 0x80012DB0: beq         $t6, $zero, L_80012DC8
    if (ctx->r14 == 0) {
        // 0x80012DB4: nop
    
            goto L_80012DC8;
    }
    // 0x80012DB4: nop

    // 0x80012DB8: lwc1        $f8, 0x0($sp)
    ctx->f8.u32l = MEM_W(ctx->r29, 0X0);
    // 0x80012DBC: lwc1        $f10, 0x4($sp)
    ctx->f10.u32l = MEM_W(ctx->r29, 0X4);
    // 0x80012DC0: mul.s       $f16, $f8, $f10
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f16.fl = MUL_S(ctx->f8.fl, ctx->f10.fl);
    // 0x80012DC4: swc1        $f16, 0x0($sp)
    MEM_W(0X0, ctx->r29) = ctx->f16.u32l;
L_80012DC8:
    // 0x80012DC8: lwc1        $f18, 0x4($sp)
    ctx->f18.u32l = MEM_W(ctx->r29, 0X4);
    // 0x80012DCC: mul.s       $f4, $f18, $f18
    CHECK_FR(ctx, 4);
    CHECK_FR(ctx, 18);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f18.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f4.fl = MUL_S(ctx->f18.fl, ctx->f18.fl);
    // 0x80012DD0: swc1        $f4, 0x4($sp)
    MEM_W(0X4, ctx->r29) = ctx->f4.u32l;
    // 0x80012DD4: srl         $t7, $a0, 1
    ctx->r15 = S32(U32(ctx->r4) >> 1);
    // 0x80012DD8: or          $a0, $t7, $zero
    ctx->r4 = ctx->r15 | 0;
    // 0x80012DDC: andi        $t8, $a0, 0xFF
    ctx->r24 = ctx->r4 & 0XFF;
    // 0x80012DE0: or          $a0, $t8, $zero
    ctx->r4 = ctx->r24 | 0;
    // 0x80012DE4: bne         $a0, $zero, L_80012DAC
    if (ctx->r4 != 0) {
        // 0x80012DE8: nop
    
            goto L_80012DAC;
    }
    // 0x80012DE8: nop

L_80012DEC:
    // 0x80012DEC: b           L_80012DFC
    // 0x80012DF0: lwc1        $f0, 0x0($sp)
    ctx->f0.u32l = MEM_W(ctx->r29, 0X0);
        goto L_80012DFC;
    // 0x80012DF0: lwc1        $f0, 0x0($sp)
    ctx->f0.u32l = MEM_W(ctx->r29, 0X0);
    // 0x80012DF4: b           L_80012DFC
    // 0x80012DF8: nop

        goto L_80012DFC;
    // 0x80012DF8: nop

L_80012DFC:
    // 0x80012DFC: jr          $ra
    // 0x80012E00: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    return;
    // 0x80012E00: addiu       $sp, $sp, 0x8
    ctx->r29 = ADD32(ctx->r29, 0X8);
    // 0x80012E04: addiu       $sp, $sp, -0x30
    ctx->r29 = ADD32(ctx->r29, -0X30);
    // 0x80012E08: sw          $ra, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r31;
    // 0x80012E0C: sw          $a0, 0x30($sp)
    MEM_W(0X30, ctx->r29) = ctx->r4;
    // 0x80012E10: sw          $a1, 0x34($sp)
    MEM_W(0X34, ctx->r29) = ctx->r5;
    // 0x80012E14: sw          $a2, 0x38($sp)
    MEM_W(0X38, ctx->r29) = ctx->r6;
    // 0x80012E18: sw          $a3, 0x3C($sp)
    MEM_W(0X3C, ctx->r29) = ctx->r7;
    // 0x80012E1C: sw          $s0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r16;
    // 0x80012E20: sw          $zero, 0x28($sp)
    MEM_W(0X28, ctx->r29) = 0;
    // 0x80012E24: lbu         $t6, 0x47($sp)
    ctx->r14 = MEM_BU(ctx->r29, 0X47);
    // 0x80012E28: bne         $t6, $zero, L_80012E38
    if (ctx->r14 != 0) {
        // 0x80012E2C: nop
    
            goto L_80012E38;
    }
    // 0x80012E2C: nop

    // 0x80012E30: b           L_80012F80
    // 0x80012E34: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
        goto L_80012F80;
    // 0x80012E34: or          $v0, $zero, $zero
    ctx->r2 = 0 | 0;
L_80012E38:
    // 0x80012E38: lui         $t7, 0x8004
    ctx->r15 = S32(0X8004 << 16);
    // 0x80012E3C: lw          $t7, 0x2800($t7)
    ctx->r15 = MEM_W(ctx->r15, 0X2800);
    // 0x80012E40: beq         $t7, $zero, L_80012F70
    if (ctx->r15 == 0) {
        // 0x80012E44: nop
    
            goto L_80012F70;
    }
    // 0x80012E44: nop

    // 0x80012E48: lui         $t8, 0x8004
    ctx->r24 = S32(0X8004 << 16);
    // 0x80012E4C: lw          $t8, 0x2800($t8)
    ctx->r24 = MEM_W(ctx->r24, 0X2800);
    // 0x80012E50: sw          $t8, 0x2C($sp)
    MEM_W(0X2C, ctx->r29) = ctx->r24;
    // 0x80012E54: lui         $t9, 0x8004
    ctx->r25 = S32(0X8004 << 16);
    // 0x80012E58: lw          $t9, 0x2800($t9)
    ctx->r25 = MEM_W(ctx->r25, 0X2800);
    // 0x80012E5C: lui         $at, 0x8004
    ctx->r1 = S32(0X8004 << 16);
    // 0x80012E60: lw          $t0, 0x0($t9)
    ctx->r8 = MEM_W(ctx->r25, 0X0);
    // 0x80012E64: sw          $t0, 0x2800($at)
    MEM_W(0X2800, ctx->r1) = ctx->r8;
    // 0x80012E68: lbu         $t1, 0x3B($sp)
    ctx->r9 = MEM_BU(ctx->r29, 0X3B);
    // 0x80012E6C: lw          $t2, 0x2C($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X2C);
    // 0x80012E70: sb          $t1, 0x4($t2)
    MEM_B(0X4, ctx->r10) = ctx->r9;
    // 0x80012E74: lw          $t3, 0x2C($sp)
    ctx->r11 = MEM_W(ctx->r29, 0X2C);
    // 0x80012E78: lw          $t4, 0x30($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X30);
    // 0x80012E7C: sw          $t3, 0x0($t4)
    MEM_W(0X0, ctx->r12) = ctx->r11;
    // 0x80012E80: lbu         $t5, 0x47($sp)
    ctx->r13 = MEM_BU(ctx->r29, 0X47);
    // 0x80012E84: sll         $t6, $t5, 14
    ctx->r14 = S32(ctx->r13 << 14);
    // 0x80012E88: sw          $t6, 0x28($sp)
    MEM_W(0X28, ctx->r29) = ctx->r14;
    // 0x80012E8C: lbu         $s0, 0x3B($sp)
    ctx->r16 = MEM_BU(ctx->r29, 0X3B);
    // 0x80012E90: addiu       $at, $zero, 0x1
    ctx->r1 = ADD32(0, 0X1);
    // 0x80012E94: beq         $s0, $at, L_80012EB0
    if (ctx->r16 == ctx->r1) {
        // 0x80012E98: nop
    
            goto L_80012EB0;
    }
    // 0x80012E98: nop

    // 0x80012E9C: addiu       $at, $zero, 0x80
    ctx->r1 = ADD32(0, 0X80);
    // 0x80012EA0: beq         $s0, $at, L_80012F24
    if (ctx->r16 == ctx->r1) {
        // 0x80012EA4: nop
    
            goto L_80012F24;
    }
    // 0x80012EA4: nop

    // 0x80012EA8: b           L_80012F68
    // 0x80012EAC: nop

        goto L_80012F68;
    // 0x80012EAC: nop

L_80012EB0:
    // 0x80012EB0: lw          $t7, 0x2C($sp)
    ctx->r15 = MEM_W(ctx->r29, 0X2C);
    // 0x80012EB4: sh          $zero, 0x24($t7)
    MEM_H(0X24, ctx->r15) = 0;
    // 0x80012EB8: lbu         $t8, 0x3F($sp)
    ctx->r24 = MEM_BU(ctx->r29, 0X3F);
    // 0x80012EBC: lw          $t1, 0x2C($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X2C);
    // 0x80012EC0: addiu       $t9, $zero, 0x103
    ctx->r25 = ADD32(0, 0X103);
    // 0x80012EC4: subu        $t0, $t9, $t8
    ctx->r8 = SUB32(ctx->r25, ctx->r24);
    // 0x80012EC8: sh          $t0, 0x22($t1)
    MEM_H(0X22, ctx->r9) = ctx->r8;
    // 0x80012ECC: lbu         $t2, 0x43($sp)
    ctx->r10 = MEM_BU(ctx->r29, 0X43);
    // 0x80012ED0: lw          $t4, 0x2C($sp)
    ctx->r12 = MEM_W(ctx->r29, 0X2C);
    // 0x80012ED4: sra         $t3, $t2, 1
    ctx->r11 = S32(SIGNED(ctx->r10) >> 1);
    // 0x80012ED8: sb          $t3, 0x28($t4)
    MEM_B(0X28, ctx->r12) = ctx->r11;
    // 0x80012EDC: lw          $t5, 0x2C($sp)
    ctx->r13 = MEM_W(ctx->r29, 0X2C);
    // 0x80012EE0: addiu       $t7, $zero, 0x7F
    ctx->r15 = ADD32(0, 0X7F);
    // 0x80012EE4: lbu         $t6, 0x28($t5)
    ctx->r14 = MEM_BU(ctx->r13, 0X28);
    // 0x80012EE8: subu        $t9, $t7, $t6
    ctx->r25 = SUB32(ctx->r15, ctx->r14);
    // 0x80012EEC: sb          $t9, 0x29($t5)
    MEM_B(0X29, ctx->r13) = ctx->r25;
    // 0x80012EF0: lw          $t8, 0x2C($sp)
    ctx->r24 = MEM_W(ctx->r29, 0X2C);
    // 0x80012EF4: lbu         $t0, 0x29($t8)
    ctx->r8 = MEM_BU(ctx->r24, 0X29);
    // 0x80012EF8: mtc1        $t0, $f4
    ctx->f4.u32l = ctx->r8;
    // 0x80012EFC: bgez        $t0, L_80012F14
    if (SIGNED(ctx->r8) >= 0) {
        // 0x80012F00: cvt.s.w     $f6, $f4
        CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 4);
    ctx->f6.fl = CVT_S_W(ctx->f4.u32l);
            goto L_80012F14;
    }
    // 0x80012F00: cvt.s.w     $f6, $f4
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 4);
    ctx->f6.fl = CVT_S_W(ctx->f4.u32l);
    // 0x80012F04: lui         $at, 0x4F80
    ctx->r1 = S32(0X4F80 << 16);
    // 0x80012F08: mtc1        $at, $f8
    ctx->f8.u32l = ctx->r1;
    // 0x80012F0C: nop

    // 0x80012F10: add.s       $f6, $f6, $f8
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 8);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f8.fl); 
    ctx->f6.fl = ctx->f6.fl + ctx->f8.fl;
L_80012F14:
    // 0x80012F14: lw          $t1, 0x34($sp)
    ctx->r9 = MEM_W(ctx->r29, 0X34);
    // 0x80012F18: swc1        $f6, 0x0($t1)
    MEM_W(0X0, ctx->r9) = ctx->f6.u32l;
    // 0x80012F1C: b           L_80012F70
    // 0x80012F20: nop

        goto L_80012F70;
    // 0x80012F20: nop

L_80012F24:
    // 0x80012F24: jal         0x80012D80
    // 0x80012F28: lbu         $a0, 0x43($sp)
    ctx->r4 = MEM_BU(ctx->r29, 0X43);
    static_0_80012D80(rdram, ctx);
        goto after_4;
    // 0x80012F28: lbu         $a0, 0x43($sp)
    ctx->r4 = MEM_BU(ctx->r29, 0X43);
    after_4:
    // 0x80012F2C: lw          $t2, 0x2C($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X2C);
    // 0x80012F30: swc1        $f0, 0x28($t2)
    MEM_W(0X28, ctx->r10) = ctx->f0.u32l;
    // 0x80012F34: lw          $t3, 0x2C($sp)
    ctx->r11 = MEM_W(ctx->r29, 0X2C);
    // 0x80012F38: sh          $zero, 0x24($t3)
    MEM_H(0X24, ctx->r11) = 0;
    // 0x80012F3C: lbu         $t4, 0x3F($sp)
    ctx->r12 = MEM_BU(ctx->r29, 0X3F);
    // 0x80012F40: lw          $t9, 0x2C($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X2C);
    // 0x80012F44: addiu       $t7, $zero, 0x103
    ctx->r15 = ADD32(0, 0X103);
    // 0x80012F48: subu        $t6, $t7, $t4
    ctx->r14 = SUB32(ctx->r15, ctx->r12);
    // 0x80012F4C: sh          $t6, 0x22($t9)
    MEM_H(0X22, ctx->r25) = ctx->r14;
    // 0x80012F50: lui         $at, 0x3F80
    ctx->r1 = S32(0X3F80 << 16);
    // 0x80012F54: mtc1        $at, $f10
    ctx->f10.u32l = ctx->r1;
    // 0x80012F58: lw          $t5, 0x34($sp)
    ctx->r13 = MEM_W(ctx->r29, 0X34);
    // 0x80012F5C: swc1        $f10, 0x0($t5)
    MEM_W(0X0, ctx->r13) = ctx->f10.u32l;
    // 0x80012F60: b           L_80012F70
    // 0x80012F64: nop

        goto L_80012F70;
    // 0x80012F64: nop

L_80012F68:
    // 0x80012F68: b           L_80012F70
    // 0x80012F6C: nop

        goto L_80012F70;
    // 0x80012F6C: nop

L_80012F70:
    // 0x80012F70: b           L_80012F80
    // 0x80012F74: lw          $v0, 0x28($sp)
    ctx->r2 = MEM_W(ctx->r29, 0X28);
        goto L_80012F80;
    // 0x80012F74: lw          $v0, 0x28($sp)
    ctx->r2 = MEM_W(ctx->r29, 0X28);
    // 0x80012F78: b           L_80012F80
    // 0x80012F7C: nop

        goto L_80012F80;
    // 0x80012F7C: nop

L_80012F80:
    // 0x80012F80: lw          $ra, 0x1C($sp)
    ctx->r31 = MEM_W(ctx->r29, 0X1C);
    // 0x80012F84: lw          $s0, 0x18($sp)
    ctx->r16 = MEM_W(ctx->r29, 0X18);
    // 0x80012F88: addiu       $sp, $sp, 0x30
    ctx->r29 = ADD32(ctx->r29, 0X30);
    // 0x80012F8C: jr          $ra
    // 0x80012F90: nop

    return;
    // 0x80012F90: nop

    // 0x80012F94: addiu       $sp, $sp, -0x30
    ctx->r29 = ADD32(ctx->r29, -0X30);
    // 0x80012F98: sw          $ra, 0x1C($sp)
    MEM_W(0X1C, ctx->r29) = ctx->r31;
    // 0x80012F9C: sw          $a0, 0x30($sp)
    MEM_W(0X30, ctx->r29) = ctx->r4;
    // 0x80012FA0: sw          $a1, 0x34($sp)
    MEM_W(0X34, ctx->r29) = ctx->r5;
    // 0x80012FA4: sw          $s0, 0x18($sp)
    MEM_W(0X18, ctx->r29) = ctx->r16;
    // 0x80012FA8: lw          $t6, 0x30($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X30);
    // 0x80012FAC: sw          $t6, 0x28($sp)
    MEM_W(0X28, ctx->r29) = ctx->r14;
    // 0x80012FB0: addiu       $t7, $zero, 0x3E80
    ctx->r15 = ADD32(0, 0X3E80);
    // 0x80012FB4: sw          $t7, 0x24($sp)
    MEM_W(0X24, ctx->r29) = ctx->r15;
    // 0x80012FB8: lw          $t8, 0x28($sp)
    ctx->r24 = MEM_W(ctx->r29, 0X28);
    // 0x80012FBC: addiu       $at, $zero, 0x1
    ctx->r1 = ADD32(0, 0X1);
    // 0x80012FC0: lbu         $s0, 0x4($t8)
    ctx->r16 = MEM_BU(ctx->r24, 0X4);
    // 0x80012FC4: beq         $s0, $at, L_80012FE0
    if (ctx->r16 == ctx->r1) {
        // 0x80012FC8: nop
    
            goto L_80012FE0;
    }
    // 0x80012FC8: nop

    // 0x80012FCC: addiu       $at, $zero, 0x80
    ctx->r1 = ADD32(0, 0X80);
    // 0x80012FD0: beq         $s0, $at, L_800130E4
    if (ctx->r16 == ctx->r1) {
        // 0x80012FD4: nop
    
            goto L_800130E4;
    }
    // 0x80012FD4: nop

    // 0x80012FD8: b           L_800131AC
    // 0x80012FDC: nop

        goto L_800131AC;
    // 0x80012FDC: nop

L_80012FE0:
    // 0x80012FE0: lw          $t9, 0x28($sp)
    ctx->r25 = MEM_W(ctx->r29, 0X28);
    // 0x80012FE4: lhu         $t0, 0x24($t9)
    ctx->r8 = MEM_HU(ctx->r25, 0X24);
    // 0x80012FE8: addiu       $t1, $t0, 0x1
    ctx->r9 = ADD32(ctx->r8, 0X1);
    // 0x80012FEC: sh          $t1, 0x24($t9)
    MEM_H(0X24, ctx->r25) = ctx->r9;
    // 0x80012FF0: lw          $t2, 0x28($sp)
    ctx->r10 = MEM_W(ctx->r29, 0X28);
    // 0x80012FF4: lhu         $t3, 0x24($t2)
    ctx->r11 = MEM_HU(ctx->r10, 0X24);
    // 0x80012FF8: lhu         $t4, 0x22($t2)
    ctx->r12 = MEM_HU(ctx->r10, 0X22);
    // 0x80012FFC: slt         $at, $t3, $t4
    ctx->r1 = SIGNED(ctx->r11) < SIGNED(ctx->r12) ? 1 : 0;
    // 0x80013000: bne         $at, $zero, L_80013010
    if (ctx->r1 != 0) {
        // 0x80013004: nop
    
            goto L_80013010;
    }
    // 0x80013004: nop

    // 0x80013008: lw          $t5, 0x28($sp)
    ctx->r13 = MEM_W(ctx->r29, 0X28);
    // 0x8001300C: sh          $zero, 0x24($t5)
    MEM_H(0X24, ctx->r13) = 0;
L_80013010:
    // 0x80013010: lw          $t6, 0x28($sp)
    ctx->r14 = MEM_W(ctx->r29, 0X28);
    // 0x80013014: lhu         $t7, 0x24($t6)
    ctx->r15 = MEM_HU(ctx->r14, 0X24);
    // 0x80013018: mtc1        $t7, $f4
    ctx->f4.u32l = ctx->r15;
    // 0x8001301C: bgez        $t7, L_80013034
    if (SIGNED(ctx->r15) >= 0) {
        // 0x80013020: cvt.s.w     $f6, $f4
        CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 4);
    ctx->f6.fl = CVT_S_W(ctx->f4.u32l);
            goto L_80013034;
    }
    // 0x80013020: cvt.s.w     $f6, $f4
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 4);
    ctx->f6.fl = CVT_S_W(ctx->f4.u32l);
    // 0x80013024: lui         $at, 0x4F80
    ctx->r1 = S32(0X4F80 << 16);
    // 0x80013028: mtc1        $at, $f8
    ctx->f8.u32l = ctx->r1;
    // 0x8001302C: nop

    // 0x80013030: add.s       $f6, $f6, $f8
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 8);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f8.fl); 
    ctx->f6.fl = ctx->f6.fl + ctx->f8.fl;
L_80013034:
    // 0x80013034: lhu         $t8, 0x22($t6)
    ctx->r24 = MEM_HU(ctx->r14, 0X22);
    // 0x80013038: mtc1        $t8, $f10
    ctx->f10.u32l = ctx->r24;
    // 0x8001303C: bgez        $t8, L_80013054
    if (SIGNED(ctx->r24) >= 0) {
        // 0x80013040: cvt.s.w     $f16, $f10
        CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 10);
    ctx->f16.fl = CVT_S_W(ctx->f10.u32l);
            goto L_80013054;
    }
    // 0x80013040: cvt.s.w     $f16, $f10
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 10);
    ctx->f16.fl = CVT_S_W(ctx->f10.u32l);
    // 0x80013044: lui         $at, 0x4F80
    ctx->r1 = S32(0X4F80 << 16);
    // 0x80013048: mtc1        $at, $f18
    ctx->f18.u32l = ctx->r1;
    // 0x8001304C: nop

    // 0x80013050: add.s       $f16, $f16, $f18
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 16);
    CHECK_FR(ctx, 18);
    NAN_CHECK(ctx->f16.fl); NAN_CHECK(ctx->f18.fl); 
    ctx->f16.fl = ctx->f16.fl + ctx->f18.fl;
L_80013054:
    // 0x80013054: div.s       $f4, $f6, $f16
    CHECK_FR(ctx, 4);
    CHECK_FR(ctx, 6);
    CHECK_FR(ctx, 16);
    NAN_CHECK(ctx->f6.fl); NAN_CHECK(ctx->f16.fl); 
    ctx->f4.fl = DIV_S(ctx->f6.fl, ctx->f16.fl);
    // 0x80013058: swc1        $f4, 0x2C($sp)
    MEM_W(0X2C, ctx->r29) = ctx->f4.u32l;
    // 0x8001305C: lui         $at, 0x8003
    ctx->r1 = S32(0X8003 << 16);
    // 0x80013060: lwc1        $f10, -0x3BAC($at)
    ctx->f10.u32l = MEM_W(ctx->r1, -0X3BAC);
    // 0x80013064: lwc1        $f8, 0x2C($sp)
    ctx->f8.u32l = MEM_W(ctx->r29, 0X2C);
    // 0x80013068: mul.s       $f12, $f8, $f10
    CHECK_FR(ctx, 12);
    CHECK_FR(ctx, 8);
    CHECK_FR(ctx, 10);
    NAN_CHECK(ctx->f8.fl); NAN_CHECK(ctx->f10.fl); 
    ctx->f12.fl = MUL_S(ctx->f8.fl, ctx->f10.fl);
    // 0x8001306C: jal         0x85047D60
