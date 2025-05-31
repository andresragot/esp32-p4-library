import re
import sys

def calcular_promedios(texto):
    """
    Dada una cadena de texto con líneas que contienen:
      - "Uso de RAM: <número> bytes"
      - "FPS: <número>"
    esta función extrae todos los valores de RAM y FPS y devuelve sus promedios.
    """
    # Expresiones regulares para extraer números de RAM y FPS
    regex_ram = re.compile(r"Uso de RAM:\s*(\d+)\s*bytes")
    regex_fps = re.compile(r"FPS:\s*([\d.]+)")

    suma_ram = 0
    cuenta_ram = 0
    suma_fps = 0.0
    cuenta_fps = 0

    for linea in texto.splitlines():
        # Buscar RAM
        match_ram = regex_ram.search(linea)
        if match_ram:
            valor_ram = int(match_ram.group(1))
            suma_ram += valor_ram
            cuenta_ram += 1

        # Buscar FPS
        match_fps = regex_fps.search(linea)
        if match_fps:
            valor_fps = float(match_fps.group(1))
            suma_fps += valor_fps
            cuenta_fps += 1

    promedio_ram = (suma_ram / cuenta_ram) if cuenta_ram > 0 else 0
    promedio_fps = (suma_fps / cuenta_fps) if cuenta_fps > 0 else 0.0

    return promedio_ram, promedio_fps

def main():
    """
    Lee todo el texto desde stdin (hasta EOF) y calcula los promedios de RAM y FPS.
    Ejemplo de uso:
      python calcular_promedios.py < mi_archivo_de_logs.txt
    """
    texto_entrada = sys.stdin.read()
    promedio_ram, promedio_fps = calcular_promedios(texto_entrada)

    print(f"Promedio de uso de RAM: {promedio_ram:.2f} bytes")
    print(f"Promedio de FPS: {promedio_fps:.2f}")

if __name__ == "__main__":
    main()
