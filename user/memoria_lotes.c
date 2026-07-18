// ============================================================================
// memoria_lotes.c  -  Prueba COMPARABLE entre xv6 original y modificado
// ----------------------------------------------------------------------------
// Usa SOLO sbrk() y uptime(), que existen en ambos kernels. No necesita
// ninguna syscall nueva, asi que corre TAL CUAL en los dos.
//
// Reserva una cantidad grande de memoria en UNA sola llamada a sbrk() (en
// vez de muchas llamadas pequenas), y mide cuanto tarda.
//
// - xv6 ORIGINAL: uvmalloc() pide las paginas de a UNA por UNA a kalloc(),
//   cada una con su propio acquire()/release() del lock del asignador.
// - xv6 MODIFICADO: uvmalloc() ahora pide las paginas en LOTES (hasta 32 a
//   la vez) con kalloc_batch(), usando un solo lock por lote en vez de uno
//   por pagina. El algoritmo de asignacion (lista enlazada) es el mismo;
//   lo que cambia es cuantas veces se toma el lock.
//
// Con una reserva de varios miles de paginas, la diferencia de cuantas
// veces se adquiere el lock deberia notarse en el tiempo total medido con
// uptime().
//
// Uso dentro de xv6:  memoria_lotes
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE     4096
#define NPAGINAS   4000        // reserva grande: 4000 * 4KB = ~15.6 MB

int
main(int argc, char *argv[])
{
  printf("== Prueba de asignacion de memoria por lotes ==\n\n");

  int total_bytes = NPAGINAS * PGSIZE;

  int t0 = uptime();
  char *p = sbrk(total_bytes);
  if (p == (char *)-1) {
    printf("sbrk fallo (no hay suficiente memoria para %d paginas)\n", NPAGINAS);
    exit(1);
  }
  // Tocar cada pagina para forzar su uso real.
  for (int i = 0; i < total_bytes; i += PGSIZE)
    p[i] = 1;
  int t1 = uptime();

  printf("Reserva de %d paginas (%d MB) en UNA sola llamada a sbrk()\n",
         NPAGINAS, total_bytes / (1024 * 1024));
  printf("Tiempo empleado: %d ticks\n\n", t1 - t0);

  int t2 = uptime();
  sbrk(-total_bytes);
  int t3 = uptime();
  printf("Tiempo en liberar toda la memoria: %d ticks\n", t3 - t2);

  printf("\nCompare el 'Tiempo empleado' de la reserva entre el kernel\n");
  printf("original y el modificado: un valor MENOR en el modificado es\n");
  printf("evidencia de que la asignacion por lotes reduce el overhead.\n");
  printf("== Fin de la prueba ==\n");
  exit(0);
}