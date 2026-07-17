// ============================================================================
// mem_concurrente_test.c  -  CONCURRENCIA en el asignador de memoria fisica
// ----------------------------------------------------------------------------
// Varios procesos hijos, EN PARALELO, hacen ciclos repetidos de reservar y
// liberar memoria (sbrk +/-). Esto somete al spinlock de kmem (kalloc.c) a
// presion real de varios nucleos/procesos accediendo al freelist al mismo
// tiempo. Al final comparamos freemem() antes y despues: si el asignador es
// correcto bajo concurrencia, la memoria libre debe volver EXACTAMENTE al
// mismo valor inicial (ninguna pagina se pierde ni se cuenta doble).
//
// IMPORTANTE: se puede correr con CPUS=1 (concurrencia via interleaving de
// procesos) o CPUS>1 (concurrencia real entre nucleos, prueba mas exigente
// para el spinlock).
//
// Uso dentro de xv6:  mem_concurrente_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE            4096
#define NHIJOS            6
#define CICLOS            20
#define PAGINAS_POR_CICLO 5

int
main(void)
{
  int libre_inicial = freemem();
  printf("== Prueba de CONCURRENCIA en el asignador de memoria ==\n");
  printf("Se recomienda probar primero con CPUS=1 y luego con CPUS=3\n\n");
  printf("Memoria libre inicial: %d bytes (%d paginas)\n\n",
         libre_inicial, libre_inicial / PGSIZE);

  for (int i = 0; i < NHIJOS; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }
    if (pid == 0) {
      // Cada hijo martilla el asignador: reserva N paginas, las toca
      // (para forzar la asignacion fisica real via vmfault) y las libera,
      // repitiendo CICLOS veces.
      for (int c = 0; c < CICLOS; c++) {
        int n = PAGINAS_POR_CICLO * PGSIZE;
        char *p = sbrk(n);
        if (p == (char *)-1) {
          printf("  [hijo %d] sbrk fallo en ciclo %d\n", getpid(), c);
          break;
        }
        for (int k = 0; k < n; k += PGSIZE)
          p[k] = (char)(getpid() + c); // tocar cada pagina
        sbrk(-n);                      // liberar de inmediato
      }
      printf("  [hijo %d] termino sus %d ciclos de alloc/free\n", getpid(), CICLOS);
      exit(0);
    }
  }

  for (int i = 0; i < NHIJOS; i++)
    wait(0);

  int libre_final = freemem();
  printf("\nMemoria libre final   : %d bytes (%d paginas)\n",
         libre_final, libre_final / PGSIZE);
  printf("Diferencia vs inicial  : %d bytes\n", libre_inicial - libre_final);

  if (libre_inicial == libre_final)
    printf("RESULTADO: el asignador es consistente bajo concurrencia (sin fugas, sin corrupcion).\n");
  else
    printf("RESULTADO: hay una diferencia -> posible fuga o condicion de carrera en kalloc/kfree.\n");

  printf("\n== Fin de la prueba ==\n");
  exit(0);
}
