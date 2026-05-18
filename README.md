# RPC Squid Game: The Glass Bridge

Proyecto en C que implementa una simulación cliente-servidor con estilo RPC sobre sockets TCP. El cliente solicita procedimientos remotos y el servidor mantiene el estado principal del juego.

## Contenido

- Servidor RPC.
- Cliente interactivo.
- Demo automática.
- Demo concurrente con varios clientes.
- Protocolo con encabezado, número de programa, versión, procedimiento y XID.
- Serlización en orden de red.
- Archivo `rpc_interface/squid_game.x` como definición de servicios.
- Scripts para compilar y ejecutar en Windows.
- Makefile para Linux, WSL o MSYS2.
- Evidenc de ejecución en `evidence/`.
- Captura de ejecución en `screenshots/`.

## Requisitos

### Windows

Instalar un compilador compatible con GCC, por ejemplo MSYS2 MinGW, y asegurarse de que `gcc` esté disponible en el `PATH`.

En MSYS2 MinGW:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
```

Para ejecutar el menú principal:

```bat
RUN_PROJECT.bat
```

Opciones directas:

```bat
RUN_AUTOMATIC_DEMO.bat
RUN_CONCURRENT_DEMO.bat
RUN_MANUAL_GAME.bat
```

### Linux, WSL o MSYS2

```bash
make all
./build/demo
./build/concurrent_demo
```

## Ejecución manual

Terminal 1:

```bash
./build/server 5050 10 90
```

Terminal 2:

```bash
./build/client 127.0.0.1 5050
```

Terminal 3:

```bash
./build/client 127.0.0.1 5050
```

## Procedimientos remotos

- `JOIN_PLAYER`: registra un jugador.
- `CHOOSE_BRIDGE`: selecciona puente 1, 2 o 3.
- `MOVE_PLAYER`: mueve 1 o 2 pasos.
- `GET_STATE`: obtiene el estado visible.
- `RESET_GAME`: reinic el juego para demos.
- `SHUTDOWN`: apaga el servidor local de demo.

## Reglas implementadas

- Hay 3 puentes paralelos.
- Cada puente tiene N pasos.
- Cada paso es fuerte o débil.
- Los pasos son ocultos hasta que se prueban.
- El jugador inic en Side A.
- El jugador escoge un puente y no puede cambr.
- Solo puede avanzar hac adelante.
- Solo puede saltar 1 o 2 pasos.
- Si pisa un paso débil, el paso se rompe y el jugador muere.
- Si llega a Side B, vive y aumenta el contador global.
- Si el reloj global se agota, los jugadores restantes mueren.
- Si otro jugador está adelante en el mismo puente, el jugador debe esperar.

## Concurrenc

El servidor acepta conexiones concurrentes. Cada solicitud se atiende en un hilo trabajador y las modificaciones al estado global se protegen con un mutex. De esta forma, varios clientes pueden envr solicitudes al mismo tiempo sin modificar el estado de forma inconsistente.

## Nota sobre RPC

El proyecto usa una implementación RPC prop sobre sockets TCP. La estructura mantiene la idea de servicios remotos, stubs de cliente, dispatcher del servidor, XID y serlización de datos.
