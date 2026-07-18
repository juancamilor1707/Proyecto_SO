// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  // MODIFICACION (memoria): contador de paginas fisicas libres.
  // Nos permite instrumentar el asignador y medir cuantitativamente el uso
  // de memoria del sistema (ver la syscall freemem()).
  uint64 free_pages;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  // MODIFICACION (memoria): iniciamos el contador en 0; freerange() -> kfree()
  // lo ira incrementando por cada pagina que se agregue a la lista libre.
  kmem.free_pages = 0;
  freerange(end, (void *)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.free_pages++; // MODIFICACION (memoria): una pagina mas disponible
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r) {
    kmem.freelist = r->next;
    kmem.free_pages--; // MODIFICACION (memoria): una pagina menos disponible
  }
  release(&kmem.lock);

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}

// ============================================================================
// MODIFICACION (memoria): freemem() devuelve la cantidad de BYTES de memoria
// fisica libre (paginas libres * tamano de pagina). Es la base de la syscall
// freemem(), que usamos para comparar el consumo de memoria antes y despues
// de ejecutar programas.
// ============================================================================
uint64
freemem(void)
{
  uint64 pages;

  acquire(&kmem.lock);
  pages = kmem.free_pages;
  release(&kmem.lock);

  return pages * PGSIZE;
}

// ============================================================================
// MODIFICACION (memoria): kalloc_batch() - politica de asignacion POR LOTES.
// ----------------------------------------------------------------------------
// kalloc() original: para reservar N paginas hay que llamarla N veces, y
// cada llamada hace su propio acquire()/release() del lock de kmem. Con
// N grande (ej. sbrk de varios MB) eso es N adquisiciones de lock separadas,
// compitiendo cada vez con otros CPUs/procesos que tambien usan kalloc/kfree.
//
// kalloc_batch(n, out[]) saca las n paginas de la freelist en UNA SOLA
// seccion critica (un solo acquire/release), y llena out[] con los punteros.
// Reduce la contencion del lock de O(n) adquisiciones a O(1) por cada
// reserva multi-pagina. El algoritmo de fondo sigue siendo el mismo (pop de
// una lista enlazada); lo que cambia es la GRANULARIDAD del lock.
//
// Devuelve la cantidad de paginas realmente conseguidas (puede ser menor a
// n si no habia suficiente memoria libre; el llamador debe revisar el
// valor de retorno y liberar lo obtenido con kfree() si no le alcanza).
// ============================================================================
int
kalloc_batch(int n, void *out[])
{
  struct run *r;
  int conseguidas = 0;

  if (n <= 0)
    return 0;

  acquire(&kmem.lock);
  while (conseguidas < n && kmem.freelist != 0) {
    r = kmem.freelist;
    kmem.freelist = r->next;
    kmem.free_pages--;
    out[conseguidas] = (void *)r;
    conseguidas++;
  }
  release(&kmem.lock);

  // El llenado con "junk" se hace FUERA del lock, igual que en kalloc(),
  // porque no requiere tocar la estructura compartida kmem.
  for (int i = 0; i < conseguidas; i++)
    memset(out[i], 5, PGSIZE);

  return conseguidas;
}