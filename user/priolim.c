// ============================================================================
// priority_bounds_test.c  -  Validacion de LIMITES de la syscall setpriority
// ----------------------------------------------------------------------------
// Prueba de caja negra sobre la validacion de argumentos de setpriority():
//   - prioridad negativa            -> debe rechazarse (-1)
//   - prioridad > MAXPRIO (20)      -> debe rechazarse (-1)
//   - prioridad = 0  (limite inf.)  -> debe aceptarse
//   - prioridad = 20 (limite sup.)  -> debe aceptarse
//   - pid inexistente               -> debe rechazarse (-1)
//
// Uso dentro de xv6:  priority_bounds_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int mypid = getpid();

  printf("== Prueba de LIMITES de setpriority ==\n\n");

  printf("1) Prioridad invalida negativa (-5):\n");
  int r1 = setpriority(mypid, -5);
  printf("   setpriority(%d, -5)  -> %d   (se esperaba -1, RECHAZO)\n\n", mypid, r1);

  printf("2) Prioridad invalida excesiva (21, MAXPRIO+1):\n");
  int r2 = setpriority(mypid, 21);
  printf("   setpriority(%d, 21)  -> %d   (se esperaba -1, RECHAZO)\n\n", mypid, r2);

  printf("3) Limite inferior valido (0):\n");
  int r3 = setpriority(mypid, 0);
  printf("   setpriority(%d, 0)   -> %d   (prioridad anterior, se esperaba >= 0)\n\n", mypid, r3);

  printf("4) Limite superior valido (20 = MAXPRIO):\n");
  int r4 = setpriority(mypid, 20);
  printf("   setpriority(%d, 20)  -> %d   (prioridad anterior, se esperaba 0)\n\n", mypid, r4);

  printf("5) PID inexistente (99999):\n");
  int r5 = setpriority(99999, 10);
  printf("   setpriority(99999,10) -> %d   (se esperaba -1, RECHAZO)\n\n", r5);

  printf("== Resumen ==\n");
  printf("Negativa rechazada   : %s\n", r1 == -1 ? "OK" : "FALLO");
  printf("Excesiva rechazada   : %s\n", r2 == -1 ? "OK" : "FALLO");
  printf("Limite 0 aceptado    : %s\n", r3 != -1 ? "OK" : "FALLO");
  printf("Limite 20 aceptado   : %s\n", r4 != -1 ? "OK" : "FALLO");
  printf("PID invalido rechazado: %s\n", r5 == -1 ? "OK" : "FALLO");

  printf("\n== Fin de la prueba ==\n");
  exit(0);
}
