      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      nop
      lw $2, 0($0)
      lw $3, 4($0)
      add $4, $2, $3
      sub $5, $2, $3
      and $6, $2, $3
      or $7, $2, $3
      xor $8, $2, $3
      nor $9, $2, $3
      nand $10, $2, $3
      slt $11, $3, $2
      lw $2, 8($0)
      srl $12, $3, 12
      srl $13, $2, 12
      sra $14, $3, 12
      sra $15, $2, 12
      beq $12, $14, L0
  L2: addi $0, $4, 0
      j L1
  L0: bne $13, $15, L2
  L1: addi $4, $4, -323
      sh $9, 12($16)
      lw $9, 12($0)
      sw $4, 20($0)
      lh $16, 20($0)
      lhu $17, 20($0)
      lbu $18, 5($0)
      lb $19, 5($0)
      lui $2, 65535
      andi $12, $12, 61115
      ori $13, $13, 30566
      nori $14, $14, 56593
      slti $15, $11, 65534
      sub $0, $6, $7
      add $0, $6, $7
      and $0, $6, $7
      or $0, $6, $7
      xor $0, $6, $7
      nor $0, $6, $7
      nand $0, $6, $7
      slt $0, $6, $7
      sll $0, $6, 3
      srl $0, $6, 3
      sra $0, $6, 3
      lw $9, 1027($0)
      halt
