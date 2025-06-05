# 🏹 Javelin Telemetry System

## Prototipo de Jabalina Sensorizada de Bajo Costo

> Proyecto de Fin de Máster – Ingeniería de Sistemas y Control  
> Universidad Nacional de Educación a Distancia (UNED) + Universidad Complutense de Madrid (UCM)  
> Autor: **Andrei Braic**  
> Fecha: Junio 2025  

---

![Prototipo de Jabalina Sensorizada](docs/infographics/desarrollo/hardware/planoExplosionado.png)

---

## 📌 Descripción General

Este proyecto propone una solución práctica y económica para la monitorización biomecánica del lanzamiento de jabalina. El sistema está basado en una unidad de medida inercial (IMU) integrada directamente en la jabalina, gestionada por un microcontrolador **ESP32S3**, y dotado de una interfaz web accesible desde cualquier smartphone con Wi-Fi.

Permite:
- Descargar datos en formato CSV.
- Generar informes en PDF con gráficas biomecánicas clave.

---

## 🎯 Objetivos Principales

- 🔌 Integrar sensores IMU de bajo costo (ICM20649 + LIS3MDL) y su circuito.
- 🧠 Implementar algoritmos de fusión de datos (filtro Madgwick).
- 📡 Transmitir datos vía Wi-Fi mediante servidor web embebido.
- 📱 Proporcionar una interfaz accesible desde móviles.
- 🛠️ Diseñar y fabricar soporte físico mediante impresión 3D.
- 📈 Validar el sistema con registros de lanzamiento reales.

---

## 🧰 Tecnologías y Herramientas

| Componente            | Descripción                                 |
|-----------------------|---------------------------------------------|
| 🧠 Microcontrolador    | ESP32S3 con Wi-Fi integrado                  |
| 🎯 IMU                 | ICM20649 (acelerómetro + giroscopio)        |
| 🧭 Magnetómetro        | LIS3MDL                                     |
| ⚙️ Software            | VSCode + PlatformIO (C++)                     |
| 🌐 Interfaz Web        | HTML, CSS, JavaScript + ESP Async WebServer      |
| 🧱 Diseño físico       | Soporte PCB y carcasa impresa en 3D         |

---

## 📷 Vista del Sistema

<!-- Puedes añadir imágenes reales o renders -->
![PCB](docs/infographics/desarrollo/hardware/pcb.jpg)
![Sistema ensamblado](docs/infographics/desarrollo/hardware/montaje_1.jpg)
![Jabalina Completa](docs/infographics/desarrollo/hardware/jabalinaCompleta.jpg)
![Captura Web UI](docs/infographics/desarrollo/software/capturaWeb.png)  


---

## 📚 Estructua del código

![Diagrama de clases](docs/infographics/desarrollo/software/diagramaClases.png)


