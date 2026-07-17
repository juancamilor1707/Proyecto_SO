// ============================================================================
// starvation.c  -  Prueba de INANICION (starvation) en el planificador
// ----------------------------------------------------------------------------
// Objetivo: comprobar si tu planificador por prioridades permite que un
// proceso de BAJA prioridad muera de hambre (starvation) cuando siempre hay
// procesos de mayor prioridad listos para ejecutar.
//
// Se crean:
//   - 3 procesos de ALTA prioridad que se reparten el CPU entre ellos de
//     forma continua (trabajo largo).
//   - 1 proceso de BAJA prioridad que hace un trabajo pequeno.
//
// Con un planificador ingenuo "estricto" (siempre el de mayor prioridad, sin
// mecanismo anti-starvation), el proceso de baja prioridad podria NUNCA
// llegar a ejecutar mientras los otros 3 sigan listos. Si tu planificador
// tiene envejecimiento (aging) u otra proteccion, el proceso de baja
// prioridad terminara en un tiempo razonable.
//
// Se usa uptime() para medir cuanto tarda en terminar el proceso de baja
// prioridad. Si tarda un tiempo desproporcionado (o el programa nunca
// termina y hay que matarlo con Ctrl+P o timeout), es evidencia de
// starvation.
//
// IMPORTANTE: ejecutar con  make qemu CPUS=1
// Uso dentro de xv6:  starvation
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define TRABAJO_ALTO  2000000000L   // trabajo largo (procesos de alta prioridad)
#define TRABAJO_BAJO  5000000L      // trabajo corto (proceso de baja prioridad)
#define NALTOS 3

int
main(int argc, char *argv[])
{
  printf("== Prueba de starvation en el planificador ==\n");
  printf("Recuerde ejecutar con: make qemu CPUS=1\n\n");

  int tick_inicio = uptime();

  // ---- Procesos de ALTA prioridad ----
  for (int i = 0; i < NALTOS; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }
    if (pid == 0) {
      volatile long x = 0;
      for (long j = 0; j < TRABAJO_ALTO; j++)
        x += j;
      printf("[ALTA prioridad] pid=%d TERMINO en tick=%d\n", getpid(), uptime());
      exit(0);
    } else {
      setpriority(pid, 20);   // prioridad alta para todos estos
    }
  }

  // ---- Proceso de BAJA prioridad (el que queremos observar) ----
  int pid_bajo = fork();
  if (pid_bajo < 0) {
    printf("fork fallo\n");
    exit(1);
  }
  if (pid_bajo == 0) {
    volatile long x = 0;
    for (long j = 0; j < TRABAJO_BAJO; j++)
      x += j;
    int tick_fin = uptime();
    printf("\n[BAJA prioridad] pid=%d TERMINO en tick=%d (tardo %d ticks)\n",
           getpid(), tick_fin, tick_fin - tick_inicio);
    exit(0);
  } else {
    setpriority(pid_bajo, 1);  // prioridad minima
  }

  // Esperamos a todos (3 altos + 1 bajo)
  for (int i = 0; i < NALTOS + 1; i++)
    wait(0);

  int tick_total = uptime();
  printf("\nTiempo total de la prueba: %d ticks\n", tick_total - tick_inicio);
  printf("Si el proceso de BAJA prioridad tardo desproporcionadamente mas\n");
  printf("que los de ALTA (o parecio no avanzar), hay indicios de starvation.\n");
  printf("== Fin de la prueba ==\n");
  exit(0);
}