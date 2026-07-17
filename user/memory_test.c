// ============================================================================
// memory_test.c  -  Prueba de memoria con syscalls ORIGINALES de xv6
// ----------------------------------------------------------------------------
// No usa freemem() ni ninguna syscall nueva. Solo sbrk() y uptime(), que ya
// existen en el xv6 original.
//
// Como no podemos preguntarle directamente al kernel "cuanta memoria libre
// hay", medimos el comportamiento del asignador de forma INDIRECTA:
//
//   1) Reservamos memoria en bloques (por ejemplo de 1 pagina = 4096 bytes)
//      uno tras otro con sbrk(), hasta que sbrk() falla (devuelve -1).
//      La cantidad de bloques que logramos reservar antes de fallar es una
//      medida indirecta de la memoria disponible para el proceso.
//   2) Medimos con uptime() cuanto tarda ese proceso de reserva completo.
//      Si tu asignador de memoria es mas eficiente/rapido, deberia tardar
//      menos ticks en reservar la misma cantidad de memoria.
//   3) Liberamos todo con sbrk(-total) y confirmamos que el proceso puede
//      volver a reservar la MISMA cantidad de bloques que la primera vez
//      (evidencia de que no hay fuga: si hubiera fuga, la segunda vuelta
//      reservaria MENOS bloques antes de fallar).
//
// Uso dentro de xv6:  memory_test
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE     4096
#define MAX_BLOQUES 20000   // limite de seguridad para el bucle de reserva

int
reservar_hasta_fallar(void)
{
  int bloques = 0;
  while (bloques < MAX_BLOQUES) {
    char *p = sbrk(PGSIZE);
    if (p == (char *)-1)
      break;
    p[0] = 1;              // tocar la pagina para forzar su uso real
    bloques++;
  }
  return bloques;
}

int
main(int argc, char *argv[])
{
  printf("== Prueba de memoria (solo syscalls originales: sbrk, uptime) ==\n\n");

  // ---- Primera vuelta: reservar hasta fallar ----
  int t0 = uptime();
  int bloques1 = reservar_hasta_fallar();
  int t1 = uptime();

  printf("1) Primera reserva: %d bloques de %d bytes (%d KB total)\n",
         bloques1, PGSIZE, (bloques1 * PGSIZE) / 1024);
  printf("   Tiempo empleado: %d ticks\n\n", t1 - t0);

  // ---- Liberar todo lo reservado ----
  sbrk(-(bloques1 * PGSIZE));

  // ---- Segunda vuelta: volver a reservar hasta fallar ----
  int t2 = uptime();
  int bloques2 = reservar_hasta_fallar();
  int t3 = uptime();

  printf("2) Segunda reserva (tras liberar todo): %d bloques (%d KB total)\n",
         bloques2, (bloques2 * PGSIZE) / 1024);
  printf("   Tiempo empleado: %d ticks\n\n", t3 - t2);

  // Liberamos de nuevo para dejar el proceso limpio
  sbrk(-(bloques2 * PGSIZE));

  // ---- Verificacion ----
  int diferencia = bloques1 - bloques2;
  if (diferencia < 0) diferencia = -diferencia;

  printf("== Verificacion ==\n");
  printf("Bloques 1ra vuelta: %d | Bloques 2da vuelta: %d | diferencia: %d\n",
         bloques1, bloques2, diferencia);

  if (diferencia <= 1)
    printf("RESULTADO: memoria totalmente recuperada, no hay indicios de fuga.\n");
  else
    printf("RESULTADO: la segunda vuelta reservo menos memoria -> posible fuga.\n");

  printf("\n== Fin de la prueba ==\n");
  exit(0);
}
