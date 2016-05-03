      lw $1, 0($0)
      lw $2, 4($0)
      add $3, $1, $2
      and $4, $1, $2
      lui $30, -1
      add $1, $30, $30
      sub $5, $1, $2
      and $6, $5, $5
      addi $1, $0, 5
      andi $2, $6, 0
  LA: nop
      nop
      addi $2, $2, 1
      bne $1, $2, LA
      xor $8, $5, $6
      slt $11, $5, $6
      add $12, $11, $11
      srl $4, $3, 3
      sra $4, $3, 12
      sra $5, $4, 20
      srl $0, $0, 2
      sll $0, $0, 1
      add $1, $0, $0
      j L1
  L0: jr $31
  L1: jal L0
      jal L2
      nop
      addi $1, $0, 22
      sh $5, 0($1)
      add $20, $5, $5
      addi $21, $0, -1
      add $22, $21, $21
      add $23, $21, $21
      sw $0, 20($0)
      addi $2, $0, 3
      sw $2, 0($0)
      lh $3, 3($2)
      sw $3, 0($0)
      lw $5, 20($0)
      sw $0, 0($5)
      addi $5, $5, -1
      sw $0, 0($5)
      halt
      halt
      halt
      halt
      halt
  L2: nop
      jr $31
