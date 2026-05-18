# RPC Squid Game: The Glass Bridge

Proyecto completo en C para VSCode. Implementa una simulación cliente-servidor con estilo RPC sobre TCP sockets. El cliente invoca procedimientos remotos y el servidor mantiene el estado autoritativo del juego.

## Qué incluye

- Servidor RPC en C.
- Cliente interactivo en C.
- Demo automática.
- Demo concurrente con varios clientes al mismo tiempo.
- Protocolo RPC propio con encabezado, número de programa, versión, número de procedimiento y XID.
- Serialización en orden de red, similar al propósito de XDR.
- Archivo `rpc_interface/squid_game.x` como definición de servicios inspirada en `rpcgen`.
- Scripts `.bat` para Windows.
- Makefile para Linux, WSL o MSYS2.
- Reporte editable en `report/report_draft.md`.
- Evidencia de ejecución en `evidence/execution_demo.txt` y `screenshots/execution_demo.png`.

## Requisitos

### Windows

Instala GCC con MSYS2 MinGW o cualquier distribución que deje `gcc` disponible en `PATH`.

Comando recomendado para MSYS2 MinGW:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
```

En Windows puedes correr todo con doble clic:

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

## Modo manual

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

## Procedimientos remotos implementados

- `JOIN_PLAYER`: registra un jugador.
- `CHOOSE_BRIDGE`: selecciona puente 1, 2 o 3.
- `MOVE_PLAYER`: mueve 1 o 2 pasos.
- `GET_STATE`: obtiene el estado visible.
- `RESET_GAME`: reinicia el juego para demos.
- `SHUTDOWN`: apaga el servidor local de demo.

## Reglas implementadas

- Hay 3 puentes paralelos.
- Cada puente tiene N pasos.
- Cada paso es fuerte o débil.
- Los pasos son ocultos hasta que se prueban.
- El jugador inicia en Side A.
- El jugador escoge un puente y no puede cambiar.
- Solo puede avanzar hacia adelante.
- Solo puede saltar 1 o 2 pasos.
- Si pisa un paso débil, el paso se rompe y el jugador muere.
- Si llega a Side B, vive y aumenta el contador global.
- Si el reloj global se agota, los jugadores restantes mueren.
- Si otro jugador está adelante en el mismo puente, el jugador debe esperar.

## Diseño de concurrencia

El servidor acepta conexiones concurrentes. Cada solicitud se procesa en un hilo trabajador, pero las modificaciones al estado global se protegen con un mutex. Por eso el servidor puede recibir solicitudes simultáneas sin permitir que dos movimientos cambien el estado al mismo tiempo.

## Nota sobre RPC

El proyecto no depende de `rpcgen` para ser más fácil de ejecutar en Windows con VSCode. Aun así, el diseño sigue el modelo de las guías: definición de servicios, stubs de cliente, dispatcher de servidor, runtime de comunicación, XID, serialización y llamadas remotas orientadas a procedimientos.
