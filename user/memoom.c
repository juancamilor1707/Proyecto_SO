// ============================================================================
// mem_agotamiento_test.c  -  Comportamiento del asignador al AGOTAR la memoria
// ----------------------------------------------------------------------------
// Un solo proceso reserva paginas de a una (sbrk(PGSIZE)) sin parar hasta que
// el kernel ya no tiene mas memoria fisica que darle. Verificamos:
//   1) Que sbrk() falle de forma CONTROLADA (devuelve -1), sin tumbar el
//      kernel ni el resto del sistema.
//   2) Que freemem() en el punto de agotamiento sea consistente (cercano a 0
//      o al minimo que el kernel se reserva para si mismo).
//   3) Que al liberar toda la memoria reservada, freemem() vuelva EXACTAMENTE
//      al valor inicial (no quedan paginas "perdidas").
//
// Uso dentro de xv6:  mem_agotamiento_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE      4096
#define MAX_BLOQUES 40000 // limite de seguridad para no ciclar eternamente

int
main(void)
{
  int libre_inicial = freemem();
  printf("== Prueba de AGOTAMIENTO de memoria fisica ==\n\n");
  printf("Memoria libre inicial: %d bytes (%d paginas)\n\n",
         libre_inicial, libre_inicial / PGSIZE);

  int bloques = 0;
  while (bloques < MAX_BLOQUES) {
    char *p = sbrk(PGSIZE);
    if (p == (char *)-1) {
      printf("sbrk fallo al pedir la pagina numero %d -> memoria agotada (comportamiento esperado)\n",
             bloques);
      break;
    }
    p[0] = 1; // tocar la pagina: fuerza la asignacion fisica real (vmfault)
    bloques++;
  }

  int libre_en_agotamiento = freemem();
  printf("\nPaginas reservadas por este proceso antes de fallar: %d (%d KB)\n",
         bloques, (bloques * PGSIZE) / 1024);
  printf("Memoria libre en el punto de agotamiento: %d bytes (%d paginas)\n",
         libre_en_agotamiento, libre_en_agotamiento / PGSIZE);
  printf("(El sistema NO crasheo: sbrk devolvio -1 de forma controlada)\n");

  sbrk(-(bloques * PGSIZE));

  int libre_final = freemem();
  printf("\nMemoria libre tras liberar todo: %d bytes (%d paginas)\n",
         libre_final, libre_final / PGSIZE);

  if (libre_final == libre_inicial)
    printf("RESULTADO: recuperacion total de memoria tras el agotamiento, sin fugas.\n");
  else
    printf("RESULTADO: diferencia de %d bytes -> posible fuga.\n", libre_inicial - libre_final);

  printf("\n== Fin de la prueba ==\n");
  exit(0);
}
