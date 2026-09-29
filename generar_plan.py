#!/usr/bin/env python3
"""Genera un plan.txt compatible con parser.c."""

import argparse
import random


def generar_plan(
    ruta_salida: str,
    cantidad: int,
    semilla: int,
    max_dependencias: int,
) -> None:
    generador = random.Random(semilla)

    with open(ruta_salida, "w", encoding="utf-8") as archivo:
        for numero in range(1, cantidad + 1):
            nombre = f"actividad_{numero:05d}"
            duracion = generador.randint(100, 5000)

            if numero == 1 or max_dependencias == 0:
                dependencias = []
            else:
                cantidad_dependencias = generador.randint(
                    0,
                    min(max_dependencias, numero - 1),
                )
                dependencias = sorted(
                    generador.sample(
                        range(1, numero),
                        cantidad_dependencias,
                    )
                )

            linea = f"{numero} : {nombre} : {duracion} :"

            if dependencias:
                linea += " " + ", ".join(
                    str(dependencia) for dependencia in dependencias
                )

            archivo.write(linea + "\n")


def main() -> None:
    argumentos = argparse.ArgumentParser(
        description="Genera un plan.txt compatible con el planificador."
    )
    argumentos.add_argument(
        "-o",
        "--salida",
        default="plan.txt",
        help="archivo de salida (por defecto: plan.txt)",
    )
    argumentos.add_argument(
        "-n",
        "--cantidad",
        type=int,
        default=10000,
        help="cantidad de actividades (por defecto: 10000)",
    )
    argumentos.add_argument(
        "-s",
        "--semilla",
        type=int,
        default=20260929,
        help="semilla aleatoria para repetir el mismo plan",
    )
    argumentos.add_argument(
        "-d",
        "--max-dependencias",
        type=int,
        default=3,
        help="máximo de dependencias por actividad (por defecto: 3)",
    )

    opciones = argumentos.parse_args()

    if opciones.cantidad <= 0:
        argumentos.error("la cantidad debe ser mayor que cero")

    if opciones.max_dependencias < 0:
        argumentos.error("max-dependencias no puede ser negativo")

    generar_plan(
        opciones.salida,
        opciones.cantidad,
        opciones.semilla,
        opciones.max_dependencias,
    )

    print(
        f"Se generaron {opciones.cantidad} actividades en "
        f"{opciones.salida}."
    )


if __name__ == "__main__":
    main()
