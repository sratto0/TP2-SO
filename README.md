# TP2 - Operating System Kernel

Este proyecto implementa un kernel educativo en C y asm x86_64 desarrollado para el Trabajo Práctico 2 de SO. El objetivo es mostrar cómo se integran un scheduler con prioridades, sincronización entre procesos, un userland completo y distintas estrategias de memoria dentro de una misma base de código.

## Capacidades principales

- **Scheduler con prioridades**: round-robin ponderado con múltiples colas; las prioridades altas reciben más CPU sin hambrear a las bajas.
- **Dos asignadores de memoria**: allocator propio y versión con buddy system opcional.
- **Semáforos con nombre** basados en primitivas atómicas y soporte para bloqueo/desbloqueo de procesos.
- **Pipes unidireccionales y bloqueo en I/O** para sincronizar procesos en userland.
- **Shell enriquecida** con jobs en background (`&`), pipelines (`|`) y páginas de manual (`man`).
- **Tests integrados** (memoria, scheduler, sincronización) para validar los subsistemas desde userland.

## Requisitos

- [Docker](https://www.docker.com/) instalado.
- Sistema operativo host con soporte para virtualización (Linux/macOS/WSL2).

## Compilación

### Ejecución manual

1. Cloná el repositorio y entrá al directorio raíz:
   ```bash
   git clone <url-del-repo>
   cd TP2-SO
   ```
2. Bajá e iniciá el contenedor desde el root del repo:
   ```bash
   docker pull agodio/itba-so-multi-platform:3.0
   docker run -v "${PWD}:/root" --privileged -ti --add-host=host.docker.internal:host-gateway agodio/itba-so-multi-platform:3.0
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
   make all
   cd ..
   make buddy
   ```

---

## Ejecución del kernel

Luego de compliar, ejecutar fuera del docker:
```bash
./run.sh
```

## Análisis estático con PVS-Studio

PVS-Studio es una herramienta de análisis estático que detecta posibles bugs, vulnerabilidades y code smells en el código C/C++.

### Ejecución del análisis

**Requisitos previos:**
- Ejecutar **fuera del contenedor Docker** (desde el host).
- Tener PVS-Studio instalado y configurado en el sistema.

**Pasos:**

1. **Generar el reporte en formato log:**
   ```bash
   make pvs
   ```
   Esto analiza el código y crea un archivo `.log` con los warnings detectados.

2. **Generar reporte HTML (opcional):**
   ```bash
   make pvs-html
   ```
   Convierte el log en un reporte navegable en formato HTML para revisión más cómoda.

3. **Limpiar archivos generados:**
   ```bash
   make pvs-clean
   ```
   Elimina todos los archivos temporales y reportes del análisis.


## Instrucciones de replicación

### Nombre preciso y breve descripción de cada comando/test y parámetros que admiten

Una vez que el kernel esté corriendo, la shell interactiva permite ejecutar los siguientes comandos:

#### **[General]**

- **`help`**  
  Lista todos los comandos disponibles agrupados por categoría.  
  *Parámetros*: Ninguno.

- **`man <comando>`**  
  Describe un comando en detalle, mostrando uso, descripción, parámetros, notas y ejemplos.  
  *Parámetros*: `<comando>` - Nombre del comando tal como aparece en `help`.

- **`clear`**  
  Limpia la pantalla y resetea el cursor a la esquina superior izquierda.  
  *Parámetros*: Ninguno.

- **`font-size <1|2|3>`**  
  Cambia el tamaño de la fuente de la terminal.  
  *Parámetros*: `<1|2|3>` - Nivel de zoom (1=pequeño, 2=mediano, 3=grande).

- **`div <dividendo> <divisor>`**  
  Divide dos números naturales e imprime el cociente entero.  
  *Parámetros*:  
  - `<dividendo>` - Entero natural.  
  - `<divisor>` - Entero natural distinto de cero.

- **`kaboom`**  
  Dispara una excepción de opcode inválido para probar el manejador de excepciones.  
  *Parámetros*: Ninguno.

#### **[Información]**

- **`inforeg`**  
  Muestra el último snapshot de registros capturado (presionando Ctrl+S).  
  *Parámetros*: Ninguno.

- **`time`**  
  Despliega la hora actual en formato UTC-3 (hh:mm:ss).  
  *Parámetros*: Ninguno.

- **`printmem <direccion_hex>`**  
  Vuelca 32 bytes consecutivos de memoria a partir de la dirección física indicada.  
  *Parámetros*: `<direccion_hex>` - Dirección en hexadecimal (sin prefijo 0x).

- **`ps`**  
  Lista todos los procesos mostrando PID, nombre, estado, prioridad, stack y si están en foreground/background.  
  *Parámetros*: Ninguno.

- **`mem`**  
  Muestra el uso de memoria (total, usado y libre).  
  *Parámetros*: Ninguno.

#### **[Tests]**

- **`test-mm <max_mem>`**  
  Ejecuta el test del memory manager, reservando y liberando bloques aleatorios hasta el límite indicado.  
  *Parámetros*: `<max_mem>` - Cantidad máxima de bytes a reservar antes de liberar.  
  *Nota*: Usar Ctrl+C para detener el test.

- **`test-processes <cantidad>`**  
  Estresa al scheduler creando múltiples procesos, matándolos, bloqueándolos y desbloqueándolos al azar.  
  *Parámetros*: `<cantidad>` - Cantidad de procesos creados por ronda (1 hasta MAX_PROCESSES-1).  
  *Nota*: Corre en bucle infinito; detener con Ctrl+C.

- **`test-prio <vueltas>`**  
  Muestra cómo impactan las prioridades en el scheduler ejecutando tres escenarios distintos.  
  *Parámetros*: `<vueltas>` - Iteraciones que cada proceso ejecuta antes de finalizar.  
  *Nota*: Usar valores >= 100000000 para observar claramente el efecto de las prioridades.

- **`test-sync <n> <usar_sem>`**  
  Testea sincronización con y sin semáforos lanzando pares de procesos que modifican una variable compartida.  
  *Parámetros*:  
  - `<n>` - Iteraciones por proceso.  
  - `<usar_sem>` - 1 para usar semáforos, 0 para condición de carrera.

- **`mvar <writers> <readers>`**  
  Simula productores y consumidores sincronizados sobre una variable compartida (MVar).  
  *Parámetros*:  
  - `<writers>` - Cantidad de escritores (1-6).  
  - `<readers>` - Cantidad de lectores (1-6).  
  *Nota*: Detener con Ctrl+C.

#### **[Procesos]**

- **`loop <segundos>`**  
  Imprime su PID y duerme en un bucle infinito el intervalo indicado.  
  *Parámetros*: `<segundos>` - Intervalo de sleep en segundos (entero positivo).  
  *Nota*: Detener con Ctrl+C.

- **`kill <pid>`**  
  Mata el proceso indicado por PID.  
  *Parámetros*: `<pid>` - Identificador del proceso (mayor a 1).  
  *Nota*: No se puede matar init (0) ni la shell (1).

- **`nice <pid> <prioridad>`**  
  Cambia la prioridad de un proceso.  
  *Parámetros*:  
  - `<pid>` - Identificador del proceso.  
  - `<prioridad>` - Nuevo valor de prioridad (0-5).

- **`block <pid>`**  
  Detiene temporalmente un proceso, marcándolo como BLOQUEADO.  
  *Parámetros*: `<pid>` - Identificador del proceso (mayor a 1).
  *Nota*: No se puede bloquear init (0) ni shell (1).

- **`unblock <pid>`**  
  Devuelve un proceso bloqueado al estado READY.  
  *Parámetros*: `<pid>` - Identificador del proceso (debe estar bloqueado, y debe ser un numero mayor a 1).
  *Nota*: No se puede desbloquear init (0) ni shell (1).

#### **[Entrada/Salida]**

- **`cat`**  
  Reproduce stdin en pantalla sin modificaciones hasta recibir EOF (Ctrl+D).  
  *Parámetros*: Ninguno.  
  *Nota*: Útil para testear redirecciones y pipes.

- **`wc`**  
  Cuenta líneas del texto ingresado desde stdin.  
  *Parámetros*: Ninguno.  
  *Nota*: Termina con Ctrl+D.

- **`filter`**  
  Filtra vocales del texto ingresado, reimprimiendo solo consonantes y otros caracteres.  
  *Parámetros*: Ninguno.  
  *Nota*: Termina con Ctrl+D.

>**Nota general:** ps y mem estan en la seccion de informacion porque brindan informacion, pero son procesos.

### Caracteres especiales para pipes y comandos en background

La shell soporta los siguientes operadores especiales para combinar comandos y ejecutar procesos en segundo plano:

#### **Pipe (`|`)**
Conecta la salida estándar (stdout) de un comando con la entrada estándar (stdin) del siguiente.

**Sintaxis:** <comando1> | <comando2>

**Ejemplo:** cat | wc
En este caso, `cat` lee desde el teclado y envía cada línea a `wc`, que las cuenta y muestra el total al presionar Ctrl+D.

**Notas:**
- El pipe se crea automáticamente y se destruye cuando ambos procesos terminan.
- Ambos procesos se ejecutan en foreground por defecto (la shell espera a que terminen).
- Si el primer proceso (escritor) termina antes que el segundo (lector), el lector recibirá EOF y finalizará normalmente.

---

#### **Background (`&`)**
Ejecuta un comando en segundo plano, permitiendo que la shell continúe aceptando nuevos comandos sin esperar a que termine.

**Sintaxis:** <comando> <parametros> &

**Ejemplo:** loop 5 &
Esto inicia `loop` en background; el proceso imprimirá su PID cada 5 segundos mientras la shell permanece disponible.

**Notas:**
- Los procesos en background no pueden leer desde stdin (se les asigna `/dev/null` como entrada).
- La salida estándar (stdout) sigue siendo la terminal, por lo que sus mensajes se mezclarán con el prompt de la shell.
- Para detener un proceso en background, usar `kill <pid>` o `block <pid>`.
- Se pueden combinar pipes y background: `cat | wc &` ejecuta ambos comandos en segundo plano.

---

#### **Combinación de operadores**

**Pipe + Background:** cat | filter &
Ambos procesos (`cat` y `filter`) se ejecutan en segundo plano conectados por un pipe.

**Restricciones:**
- Solo se soporta **un único pipe** por comando (máximo 2 procesos encadenados).
- El operador `&` debe aparecer **al final** de la línea de comandos.
- No se pueden anidar pipes ni usar redirecciones adicionales.

### Atajos de teclado para interrumpir la ejecución y enviar EOF

La shell reconoce las siguientes combinaciones de teclas especiales durante la ejecución de procesos:

#### **Ctrl+C - Interrupción de proceso**
Envía una señal de terminación al proceso en foreground que esté corriendo actualmente.

**Uso:**
- Presionar `Ctrl+C` durante la ejecución de cualquier comando en primer plano.
- El proceso actual se detiene inmediatamente y retorna el control a la shell.
- Si no hay ningún proceso en foreground (solo el prompt de la shell), `Ctrl+C` no tiene efecto.

**Ejemplos:**
- loop 2 # Presionar Ctrl+C para detener el bucle infinito
- test-mm 1000000 # Presionar Ctrl+C para abortar el test de memoria
- cat | wc # Presionar Ctrl+C para terminar ambos procesos del pipe


**Notas:**
- Los procesos en background (`&`) **no** se ven afectados por `Ctrl+C`.
- Para terminar un proceso en background, usar `kill <pid>`.
- `Ctrl+C` no afecta a la shell misma (PID 1), solo a sus procesos hijos en foreground.

---

#### **Ctrl+D - End of File (EOF)**
Envía una señal de fin de archivo (EOF) al proceso que está leyendo desde stdin.

**Uso:**
- Presionar `Ctrl+D` cuando un proceso esté esperando entrada del usuario.
- El proceso interpreta EOF y finaliza su lectura, usualmente terminando su ejecución.
- En algunos casos (como `cat`), el proceso puede seguir ejecutándose pero deja de leer nueva entrada.

**Ejemplos:**
- cat # Escribir varias líneas, luego Ctrl+D para terminar
- wc # Ingresar texto, Ctrl+D muestra el conteo de líneas
- filter # Escribir texto con vocales, Ctrl+D finaliza el filtrado
- cat | wc # Ctrl+D cierra cat y permite que wc muestre el total


**Notas:**
- `Ctrl+D` solo funciona cuando un proceso está bloqueado esperando entrada de teclado.
- No tiene efecto sobre procesos que no leen de stdin.
- Los procesos en background reciben `/dev/null` como stdin, por lo que `Ctrl+D` no los afecta.

#### **Resumen de atajos**

| Atajo    | Función                          | Afecta a                    |
|----------|----------------------------------|-----------------------------|
| `Ctrl+C` | Termina proceso en foreground    | Proceso actual (foreground) |
| `Ctrl+D` | Envía EOF (fin de archivo)       | Proceso leyendo de stdin    |

> **Importante**: Estos atajos son manejados por el kernel a nivel de interrupciones de teclado, no por la shell misma.

## Ejemplos (fuera de los tests)

A continuación se muestran comandos de uso directo en la shell del kernel para demostrar cada requerimiento sin ejecutar los tests automáticos.

- Scheduler con prioridades
  - Comandos:
    - Iniciar 3 procesos en background:  
      loop 1 &  
      loop 1 &  
      loop 1 &
    - Obtener PIDs con: ps
    - Ajustar prioridades: nice <pid1> 5 ; nice <pid2> 2 ; nice <pid3> 0
    - Observar comportamiento: mirar las impresiones en pantalla (el proceso con mayor prioridad imprime más frecuentemente) o usar ps para ver prioridades.
  - Resultado esperado: procesos con mayor prioridad reciben más tiempo CPU; procesos de baja prioridad no quedan hambrientos.

- Dos asignadores de memoria (allocator propio / buddy)
  - Pasos:
    - Compilar con buddy: seguir instrucciones en "Compilación".
    - Arrancar kernel: ./run.sh
    - Ver uso de memoria: mem
    - Crear múltiples procesos que consuman memoria (por ejemplo, varios módulos o procesos que alojen buffers) y volver a consultar: mem
  - Resultado esperado: comparar uso/devoción de bloques entre builds normales y build con buddy; mem muestra total/used/free.

- Semáforos con nombre y sincronización
  - Comando de demostración (userland):  
    mvar 2 2
  - Resultado esperado: varios escritores y lectores sincronizados correctamente sin condiciones de carrera (mvar usa semáforos internamente para coordinar).

- Pipes unidireccionales y bloqueo en I/O
  - Ejemplos:
    - Pipe foreground: cat | wc  (escribir texto, Ctrl+D para enviar EOF)
    - Pipe en background: cat | filter &
  - Resultado esperado: la salida de la etapa izquierda fluye a la derecha a través del pipe; procesos background no pueden leer stdin (se les asigna /dev/null).

- Shell enriquecida (jobs, pipelines, man, help)
  - Ejemplos:
    - Background job: loop 5 &
    - Pipeline: cat | wc
    - Ver ayuda: help
    - Ver manual de un comando: man kill
  - Resultado esperado: la shell acepta &, |; man muestra uso detallado; help lista comandos.

- Pruebas manuales de I/O y señales
  - Ctrl+C: interrumpe el proceso en foreground (no afecta procesos en background ni a la shell).
  - Ctrl+D: envía EOF al proceso que lee de stdin (p. ej. cat o wc).
  - Resultado esperado: atajos manejados por el kernel tal como está documentado en la sección de atajos.

### Requerimientos faltantes o parcialmente implementados

- **Máximo un pipe por comando**: La shell no soporta encadenar múltiples pipes (ej: `cmd1 | cmd2 | cmd3`). Solo se permite conectar dos procesos mediante un único `|`.
- **Sin redirecciones de archivos**: No se implementaron operadores como `>`, `>>`, `<` ya que no existe un sistema de archivos persistente.
- **Background jobs sin control**: Los procesos en background no tienen un sistema de jobs completo (no hay comandos `jobs`, `fg`, `bg` como en bash).
- **Semáforos sin persistencia**: Los semáforos se destruyen al finalizar todos los procesos que los usan; no hay persistencia entre reinicios.

## Limitaciones conocidas

### Análisis estático con PVS-Studio

Durante la compilación con PVS-Studio aparecen varios warnings que **no** han sido suprimidos mediante comentarios especiales (`//-V:código`). Estos warnings no representan errores reales en el contexto de desarrollo de kernel bare-metal:

#### **kernel.c**
- **V566**: Warnings sobre conversión de direcciones fijas a punteros.  
  *Justificación*: En kernel development es común y necesario mapear direcciones de memoria específicas. Estas conversiones son intencionales y válidas para acceder a regiones de hardware.

#### **video.c**
- **V566**: Conversión de `0x5C00` a puntero.  
  *Justificación*: Es código válido en bare-metal para mapeo de memoria VBE (Video BIOS Extensions). La dirección física es parte de la especificación del hardware.

#### **scheduler.c**
- **V522**: Warning sobre dereferencia potencial de NULL.  
  *Justificación*: El scheduler se mapea a una dirección física fija (`SCHEDULER_ADDRESS`) conocida en tiempo de compilación; no puede ser NULL en este contexto de kernel.

#### **shell.c**
- **V576** (línea 220): Falta de especificador de ancho en `scanf` (ej: `%255s`).  
  *Justificación*: Nuestra implementación custom de `scanf` ya maneja el límite del buffer internamente (lee hasta `MAX_CHARS-1`). El código está protegido contra overflow.
- **V576** (línea 220): PVS detecta `%S` y asume el formato estándar de C (wide string).  
  *Justificación*: En nuestra implementación, `%S` se usa para leer una línea completa con propósitos específicos del kernel. El warning es un falso positivo.

#### **shell_commands.c**
- **V576**: Conversión de valores de 64 bits (`uint64_t`) en contextos que esperan 32 bits.  
  *Justificación*: Nuestro `printf` personalizado solo soporta `%d` (32 bits). Los valores se convierten explícitamente a 32 bits o se formatean de manera alternativa según las necesidades del kernel.

#### **stdio.c**
- **V576** (líneas 187-188): Warnings sobre `printf` con formato `%x` recibiendo argumentos `uint64_t`.  
  *Justificación*: Aunque `printf` estándar espera 32 bits para `%x`, nuestra implementación custom maneja correctamente valores de 64 bits. PVS asume el comportamiento estándar de C, generando falsos positivos.

> **Nota general**: Los warnings de nivel **Low** de PVS han sido evaluados individualmente. No se agregaron supresiones en el código ya que estos warnings indican comportamiento intencional y válido en el contexto de sistemas operativos bare-metal, donde se opera directamente con hardware y direcciones de memoria específicas.

### Cantidades máximas de recursos

El kernel impone límites estáticos en la cantidad de recursos concurrentes para simplificar la gestión de memoria y evitar fragmentación excesiva:

- **Procesos**: Máximo de 64 procesos simultáneos (definido por `MAX_PROCESSES`).
- **Pipes**: Hasta 64 pipes activos a la vez (definido por `MAX_PIPES`).
- **Semáforos**: Máximo de 64 semáforos con nombre en el sistema (definido por `MAX_SEMAPHORES`).

Estos límites están configurados en tiempo de compilación y pueden ajustarse modificando las constantes en los headers correspondientes (`scheduler.h`, `pipes.h`, `semaphore.h`).

## Citas de fragmentos de codigo / uso de IA
Durante el desarrollo de este trabajo práctico se utilizaron herramientas de IA (GitHub Copilot, ChatGPT, Claude) como recursos de consulta y aprendizaje. El propósito fue profundizar en los conceptos teóricos del TP y entender correctamente cómo implementar los distintos subsistemas (scheduler, semáforos, pipes, memory management) antes de escribir el código final por nuestra cuenta.

## Créditos

Proyecto desarrollado como TP2 de Sistemas Operativos (ITBA). Basado en x64BareBones, extendido con nuevos subsistemas de planificación, sincronización y userland.