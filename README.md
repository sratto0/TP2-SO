# TP2 - Operating System Kernel

Este proyecto implementa un kernel educativo en C y asm x86_64 desarrollado para el Trabajo Práctico 2 de SO. El objetivo es mostrar cómo se integran un scheduler con prioridades, sincronización entre procesos, un userland completo y distintas estrategias de memoria dentro de una misma base de código.

## Capacidades principales

- **Scheduler con prioridades**: round-robin ponderado con múltiples colas; las prioridades altas reciben más CPU sin hambrear a las bajas.
- **Dos asignadores de memoria**: allocator propio y versión con buddy system opcional.
- **Semáforos con nombre** basados en primitivas atómicas y soporte para bloqueo/desbloqueo de procesos.
- **Pipes unidireccionales y bloqueo en I/O** para sincronizar procesos en userland.
- **Shell enriquecida** con jobs en background (`&`), pipelines (`|`), histórico de comandos y páginas de manual (`man`).
- **Tests integrados** (memoria, scheduler, sincronización) para validar los subsistemas desde userland.

## Requisitos

- [Docker](https://www.docker.com/) instalado.
- Sistema operativo host con soporte para virtualización (Linux/macOS/WSL2).

## Compilación

Podés compilar el proyecto de dos maneras: manual (una única sesión) o usando el script `compile.sh` para iteraciones repetidas.

---

### ▶️ Ejecución manual

1. Cloná el repositorio y entrá al directorio raíz:
   ```bash
   git clone <url-del-repo>
   cd TP2-SO
   ```
2. Bajá e iniciá el contenedor desde el root del repo:
   ```bash
   docker pull agodio/itba-so-multi-platform:3.0
   docker run -v ${PWD}:/root --security-opt seccomp:unconfined -ti agodio/itba-so-multi-platform:3.0
   ```
3. Compilá la toolchain y el kernel dentro del contenedor:
   ```bash
   cd root/Toolchain
   make all
   cd ..
   make all
   ```
4. (Opcional) Para usar el buddy allocator:
   ```bash
   cd root/Toolchain
   make MM="USE_BUDDY"
   cd ..
   make MM="USE_BUDDY"
   ```

---

### 🔁 Flujo recurrente (`compile.sh`)

1. Creá un contenedor nombrado (desde el root del repo):
   ```bash
   docker pull agodio/itba-so-multi-platform:3.0
   docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name SO agodio/itba-so-multi-platform:3.0
   ```
   > ⚠️ Ajustá `SO` si querés otro nombre; recordá usar el mismo en `compile.sh`.

2. Ejecutá el script:
   ```bash
   ./compile.sh       # build normal
   ./compile.sh buddy # build con buddy allocator
   ```
   El script limpia y recompila `Toolchain` y `Kernel`, muestra el resultado y detiene el contenedor.

## Ejecución del kernel

Después de compilar:
```bash
./run.sh
```

> ℹ️ Si no usás macOS, eliminá las flags de audio específicas (`-audiodev coreaudio...`) del `run.sh`.

## Programas de userland destacados

- `sh`: shell interactiva con soporte para background, pipes y señales.
- `help` / `man`: documentación de los comandos.
- `ps`, `mem`, `loop`, `kill`, `nice`, `block`, `unblock`: utilidades de procesos y memoria.
- `cat`, `wc`, `filter`, `pipeTest`: ejemplos de I/O y pipes.
- `mvar`, `phylo`, `test-mm`, `test-prio`, `test-processes`, `test-sync`: suites de prueba para concurrencia, scheduler y memoria.

## Pruebas de memoria (opcional)

El repositorio incluye una batería adicional en `MemoryTests`. Para correrla dentro del contenedor:
```bash
docker start SO
docker exec -it SO bash
cd root/MemoryTests
make all
./mmTest 1000000
```

## Créditos

Proyecto desarrollado como TP2 de Sistemas Operativos (ITBA). Basado en x64BareBones, extendido con nuevos subsistemas de planificación, sincronización y userland.
