// ============================================================================
// prioridad.c  -  Programa de prueba de la PLANIFICACION POR PRIORIDADES
// ----------------------------------------------------------------------------
// Crea varios procesos hijos, a cada uno le asigna una prioridad distinta con
// la syscall setpriority(pid, prioridad) y los pone a hacer trabajo intensivo
// de CPU. Con el planificador por prioridades modificado, el hijo con MAYOR
// prioridad deberia terminar primero.
//
// IMPORTANTE: para ver el efecto con claridad, ejecute xv6 con UN SOLO nucleo:
//     make qemu CPUS=1
// (con varios nucleos los hijos corren en paralelo y el orden no es tan visible)
//
// Uso dentro de xv6:  prioridad
// ============================================================================

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NHIJOS 4                 // cantidad de procesos hijos
#define TRABAJO 800000000L       // iteraciones del bucle de CPU (ajustable)

int
main(int argc, char *argv[])
{
  // Prioridades que asignaremos a cada hijo (recuerde: MAYOR valor = MAYOR
  // prioridad). Las ponemos "al reves" a proposito para comprobar que el orden
  // de terminacion NO depende del orden de creacion, sino de la prioridad.
  int prioridades[NHIJOS] = {5, 10, 15, 18};

  printf("== Prueba de planificacion por prioridades ==\n");
  printf("Recuerde ejecutar con: make qemu CPUS=1\n\n");

  for (int i = 0; i < NHIJOS; i++) {
    int pid = fork();

    if (pid < 0) {
      printf("fork fallo\n");
      exit(1);
    }

    if (pid == 0) {
      // ---- Codigo del hijo ----
      // El hijo se queda con la prioridad que el padre le asigno.
      volatile long x = 0;
      for (long j = 0; j < TRABAJO; j++)
        x += j;                 // trabajo puro de CPU

      printf("HIJO pid=%d con prioridad=%d TERMINO\n", getpid(), prioridades[i]);
      exit(0);
    } else {
      // ---- Codigo del padre ----
      // Asignamos la prioridad al hijo recien creado.
      setpriority(pid, prioridades[i]);
    }
  }

  // El padre espera a que terminen todos los hijos.
  for (int i = 0; i < NHIJOS; i++)
    wait(0);

  printf("\nOrden esperado (mayor a menor prioridad): 18, 15, 10, 5\n");
  printf("== Fin de la prueba ==\n");
  exit(0);
}
