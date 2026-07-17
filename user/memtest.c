// ============================================================================
// memtest.c  -  Programa de prueba de la INSTRUMENTACION DE MEMORIA
// ----------------------------------------------------------------------------
// Usa la nueva syscall freemem(), que devuelve los BYTES de memoria fisica
// libre en el sistema. Medimos la memoria libre en tres momentos:
//   1) al inicio,
//   2) despues de reservar memoria con sbrk(),
//   3) despues de liberarla.
// Asi se comprueba de forma CUANTITATIVA como el asignador de paginas del
// kernel entrega y recupera memoria.
//
// Uso dentro de xv6:  memtest
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE   4096
#define NPAGINAS 100                 // paginas a reservar (100 * 4KB = 400KB)

int
main(int argc, char *argv[])
{
  int libre_inicial = freemem();
  printf("== Prueba de gestion de memoria (freemem) ==\n\n");
  printf("1) Memoria libre inicial : %d bytes  (%d KB, %d paginas)\n",
         libre_inicial, libre_inicial / 1024, libre_inicial / PGSIZE);

  // Reservamos NPAGINAS paginas de memoria. sbrk() en esta version reserva
  // de forma inmediata (eager), por lo que las paginas se toman al momento.
  int n = NPAGINAS * PGSIZE;
  char *p = sbrk(n);
  if (p == (char *)-1) {
    printf("sbrk fallo\n");
    exit(1);
  }

  // Escribimos en cada pagina para asegurarnos de que quedan realmente en uso.
  for (int i = 0; i < n; i += PGSIZE)
    p[i] = 1;

  int libre_despues = freemem();
  printf("2) Tras reservar %d paginas: %d bytes  (%d KB)\n",
         NPAGINAS, libre_despues, libre_despues / 1024);
  printf("   -> Memoria consumida     : %d bytes  (~%d paginas)\n",
         libre_inicial - libre_despues,
         (libre_inicial - libre_despues) / PGSIZE);

  // Liberamos la memoria reservada (sbrk con argumento negativo).
  sbrk(-n);

  int libre_final = freemem();
  printf("3) Tras liberar la memoria : %d bytes  (%d KB)\n",
         libre_final, libre_final / 1024);
  printf("   -> Memoria recuperada    : %d bytes  (~%d paginas)\n",
         libre_final - libre_despues,
         (libre_final - libre_despues) / PGSIZE);

  printf("\n== Fin de la prueba ==\n");
  exit(0);
}
