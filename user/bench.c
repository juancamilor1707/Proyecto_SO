// ============================================================================
// bench.c  -  BENCHMARK PORTABLE (corre en xv6 ORIGINAL y MODIFICADO)
// ----------------------------------------------------------------------------
// Este programa NO usa las syscalls nuevas (setpriority/freemem). Solo usa
// syscalls estandar de xv6 (fork, wait, getpid, uptime, exit), por lo que
// compila y corre IGUAL en la version sin modificar y en la modificada.
//
// Objetivo: medir de forma cuantitativa el comportamiento del PLANIFICADOR.
// Lanza N procesos hijos que hacen la MISMA carga de CPU y registra:
//   - el ORDEN en que terminan
//   - el instante (en ticks) en que termina cada uno
//   - el tiempo TOTAL del lote
//
// Como comparar:
//   1. Corre 'bench' en xv6 SIN modificar   -> anota el orden y los tiempos.
//   2. Corre 'bench' en xv6 MODIFICADO       -> anota el orden y los tiempos.
//   3. Compara. En Round-Robin (original) los hijos terminan de forma
//      intercalada/pareja. En la version por prioridades, con prioridades
//      iguales el comportamiento es equivalente (justo), lo que DEMUESTRA que
//      la modificacion no rompe la equidad base del planificador.
//
// IMPORTANTE: corre siempre con un solo core:  make qemu CPUS=1
// ============================================================================
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NHIJOS   4          // cantidad de procesos hijos
#define VUELTAS  200000000  // carga de CPU por hijo (ajustable)

// Trabajo de CPU puro: un bucle que no se puede optimizar a la nada.
// 'volatile' evita que el compilador elimine el bucle.
static void
carga_cpu(void)
{
  volatile unsigned long x = 0;
  for (unsigned long i = 0; i < VUELTAS; i++)
    x += i;
}

int
main(void)
{
  int inicio = uptime();

  printf("=== BENCHMARK DE PLANIFICADOR ===\n");
  printf("Hijos: %d | Vueltas/hijo: %d | tick inicial: %d\n",
         NHIJOS, VUELTAS, inicio);

  // Creamos los hijos. Cada hijo hace la carga y sale con un codigo = su indice.
  for (int i = 0; i < NHIJOS; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }
    if (pid == 0) {
      // ---- codigo del hijo ----
      carga_cpu();
      int t = uptime() - inicio;
      printf("  [hijo %d] pid=%d termino en t=%d ticks\n", i, getpid(), t);
      exit(i);       // el codigo de salida nos da el orden real
    }
    // el padre sigue creando mas hijos
  }

  // El padre espera a TODOS y registra el orden en que terminan.
  printf("--- orden de finalizacion (segun scheduler) ---\n");
  for (int i = 0; i < NHIJOS; i++) {
    int estado = 0;
    int pid = wait(&estado);
    printf("  termino pid=%d (indice=%d) en t=%d ticks\n",
           pid, estado, uptime() - inicio);
  }

  int total = uptime() - inicio;
  printf("=== TIEMPO TOTAL DEL LOTE: %d ticks ===\n", total);
  exit(0);
}
