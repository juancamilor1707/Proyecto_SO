// ============================================================================
// scheduler_test.c  -  Prueba del planificador con syscalls ORIGINALES de xv6
// ----------------------------------------------------------------------------
// No usa setpriority() ni ninguna syscall nueva. Solo fork(), wait(), exit()
// y uptime(), que ya existen en el xv6 original.
//
// Idea: si modificaste el planificador (por ejemplo para que ya no sea
// round-robin puro, o para que trate distinto a procesos con distinta
// cantidad de trabajo, o que tenga alguna heuristica interna), esta prueba
// sirve para OBSERVAR el orden y los tiempos de finalizacion de varios
// procesos hijos con cargas de trabajo distintas, sin necesitar ninguna
// syscall adicional para "pedirle" una prioridad al kernel.
//
// Se crean 4 hijos con distinta cantidad de trabajo de CPU (deliberadamente
// en orden mezclado). Se imprime el tick de inicio, el tick en que cada uno
// termina, y el orden real de finalizacion. Eso ya es evidencia observable
// de como se comporta tu planificador modificado frente al original.
//
// IMPORTANTE: ejecutar con  make qemu CPUS=1
// Uso dentro de xv6:  scheduler_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NHIJOS 4

int
main(int argc, char *argv[])
{
  // Cargas de trabajo distintas para cada hijo (en iteraciones de CPU),
  // puestas en orden mezclado a proposito.
  long cargas[NHIJOS] = {300000000L, 900000000L, 500000000L, 700000000L};

  printf("== Prueba del planificador (solo syscalls originales) ==\n");
  printf("Recuerde ejecutar con: make qemu CPUS=1\n\n");

  int tick_inicio = uptime();
  printf("Tick de inicio: %d\n\n", tick_inicio);

  for (int i = 0; i < NHIJOS; i++) {
    int pid = fork();

    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }

    if (pid == 0) {
      volatile long x = 0;
      for (long j = 0; j < cargas[i]; j++)
        x += j;

      int tick_fin = uptime();
      printf("HIJO #%d pid=%d (carga=%ld) TERMINO en tick=%d (duro %d ticks)\n",
             i, getpid(), cargas[i], tick_fin, tick_fin - tick_inicio);
      exit(0);
    }
    // El padre sigue creando hijos sin esperar (todos quedan corriendo
    // "al mismo tiempo" segun el scheduler los vaya atendiendo).
  }

  for (int i = 0; i < NHIJOS; i++)
    wait(0);

  int tick_total = uptime();
  printf("\nTiempo total: %d ticks\n", tick_total - tick_inicio);
  printf("Compare el ORDEN de finalizacion contra el orden de las cargas\n");
  printf("(indice #0..#3) para ver como tu planificador los intercalo.\n");
  printf("== Fin de la prueba ==\n");
  exit(0);
}
