// ============================================================================
// preempcion_test.c  -  Prueba de PREEMPCION del planificador por prioridades
// ----------------------------------------------------------------------------
// Objetivo: comprobar que un proceso de ALTA prioridad que llega DESPUES
// interrumpe (preempta) a un proceso de BAJA prioridad que ya esta corriendo,
// en vez de esperar a que termine.
//
// Estrategia:
//   1) Se lanza el proceso A con prioridad BAJA (5) y trabajo largo. A
//      imprime "checkpoints" periodicos con el tick actual y cuanto avanzo
//      desde el checkpoint anterior.
//   2) El padre se DUERME con pause() (no compite por CPU) mientras A corre
//      solo.
//   3) Pasado un rato, el padre lanza al proceso B con prioridad ALTA (20)
//      y trabajo mas corto.
//   4) Si el planificador es preemptivo por prioridad, en cuanto B esta
//      RUNNABLE debe tomar el CPU de inmediato, y eso se vera reflejado en
//      el checkpoint de A que coincide con la ejecucion de B: su "delta"
//      (ticks transcurridos desde el checkpoint anterior) sera MUCHO mayor
//      que el de los demas checkpoints, porque A estuvo detenido mientras
//      B corria.
//
// IMPORTANTE: ejecutar con  make qemu CPUS=1  (para forzar que compitan
// por el mismo nucleo; con varios nucleos podrian correr en paralelo y no
// se veria el efecto de preempcion).
//
// Uso dentro de xv6:  preempcion_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define TRABAJO_A   1500000000L // trabajo largo para A (prioridad baja)
#define TRABAJO_B   500000000L  // trabajo mas corto para B (prioridad alta)
#define CHECKPOINTS 10

int
main(void)
{
  printf("== Prueba de PREEMPCION por prioridad ==\n");
  printf("Recuerde ejecutar con: make qemu CPUS=1\n\n");

  int pid_a = fork();
  if (pid_a < 0) {
    printf("fork fallo\n");
    exit(1);
  }
  if (pid_a == 0) {
    // ---- Proceso A: prioridad BAJA ----
    setpriority(getpid(), 5);

    volatile long x = 0;
    long paso = TRABAJO_A / CHECKPOINTS;
    int t_prev = uptime();
    for (int c = 1; c <= CHECKPOINTS; c++) {
      for (long j = 0; j < paso; j++)
        x += j;
      int t_now = uptime();
      printf("  [A pid=%d] checkpoint %d/%d tick=%d (avanzo %d ticks desde el anterior)\n",
             getpid(), c, CHECKPOINTS, t_now, t_now - t_prev);
      t_prev = t_now;
    }
    printf("[A pid=%d] TERMINO\n", getpid());
    exit(0);
  }

  // ---- Padre: se duerme para dejar correr a A solo, sin competir ----
  pause(15);

  printf("\n>>> Lanzando proceso B de ALTA prioridad (20) en tick=%d <<<\n\n", uptime());

  int pid_b = fork();
  if (pid_b < 0) {
    printf("fork fallo\n");
    exit(1);
  }
  if (pid_b == 0) {
    // ---- Proceso B: prioridad ALTA ----
    setpriority(getpid(), 20);

    volatile long x = 0;
    int tb0 = uptime();
    for (long j = 0; j < TRABAJO_B; j++)
      x += j;
    printf("  [B pid=%d] TERMINO en tick=%d (duro %d ticks)\n",
           getpid(), uptime(), uptime() - tb0);
    exit(0);
  }

  wait(0);
  wait(0);

  printf("\n== Fin de la prueba ==\n");
  printf("Si el checkpoint de A que coincide con la ejecucion de B muestra un\n");
  printf("'delta' MUCHO mayor que los demas, eso demuestra que B PREEMPTO a A.\n");
  exit(0);
}
