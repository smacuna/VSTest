# Proyecto de prueba de VST

Este es un proyecto en el que tengo VSTs de instrumentos de prueba.

## Hacer build

Para obtener el .vst3 y el Standalone del proyecto, se debe correr el siguiente código

```bash
cmake --build build
```

## Generar archivo `compile_commands.json`

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build
```