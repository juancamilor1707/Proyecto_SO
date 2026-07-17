// ============================================================================
// many_priorities_test.c  -  Planificacion por prioridades a mayor ESCALA
// ----------------------------------------------------------------------------
// Extiende la idea de prioridad.c pero con 8 procesos y prioridades
// distribuidas por todo el rango valido (1..20), en orden mezclado
// deliberadamente. Sirve para confirmar que el algoritmo "elegir siempre el
// RUNNABLE con mayor prioridad" sostiene el ORDEN TOTAL correcto incluso con
// mas procesos compitiendo (no solo con 4, como en la prueba original).
//
// IMPORTANTE: ejecutar con  make qemu CPUS=1
// Uso dentro de xv6:  many_priorities_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NHIJOS  8
#define TRABAJO 400000000L

int
main(void)
{
  // Prioridades en orden mezclado a proposito.
  int prioridades[NHIJOS] = {2, 20, 7, 14, 1, 18, 9, 12};

  printf("== Prueba de planificacion por prioridades a escala (%d procesos) ==\n", NHIJOS);
  printf("Recuerde ejecutar con: make qemu CPUS=1\n\n");

  for (int i = 0; i < NHIJOS; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }
    if (pid == 0) {
      setpriority(getpid(), prioridades[i]);
      volatile long x = 0;
      for (long j = 0; j < TRABAJO; j++)
        x += j;
      printf("HIJO pid=%d prioridad=%d TERMINO\n", getpid(), prioridades[i]);
      exit(0);
    }
  }

  for (int i = 0; i < NHIJOS; i++)
    wait(0);

  printf("\nOrden esperado (mayor a menor prioridad): 20,18,14,12,9,7,2,1\n");
  printf("== Fin de la prueba ==\n");
  exit(0);
}
