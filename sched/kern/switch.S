/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */



/* Explicacion minuto 2:54:00 a minuto 3:03:00 --> clase 12 de Mayo (https://drive.google.com/file/d/1IOl-Updh_ckXmNK1rXS1mcJO5UXK9Oou/view) */
.globl context_switch;
context_switch:
	movl 4(%esp), %esp	 /* recuperar el puntero a Traframe */
	/* Cargar desde el Trapframe (en el stack) los valores a los registros de la cpu correspondientes */
	popal /* restaurar los pushregs (de proposito general) */
	pop %es 
	pop	%ds	
	add	$4,%esp	 /* me deshago del tf_trapno (trap number) */
	add $4,%esp  /* me deshago del tf_err (error code) */
	/* ya estamos apuntando a tf_eip, se puede llamar a iret */
	iret	/* Restaurar los valores de eip, eflags, cs, y esp con los que encontró en el stack */


spin:
	jmp spin
