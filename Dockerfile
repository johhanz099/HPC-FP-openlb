# Para contruir y correr con Docker
#   docker build -t name-image .
#   docker run -it --rm -v $(pwd):/workspace name-image


# Dockerfile básico para compilar y usar OpenLB
FROM ubuntu:22.04

# Evita interacciones en la instalación
ENV DEBIAN_FRONTEND=noninteractive

# Instala paquetes básicos
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    unzip \
    libopenmpi-dev \
    openmpi-bin \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Nuevas herramientas
RUN apt-get update && apt-get install -y \
    gnuplot \
    && rm -rf /var/lib/apt/lists/*

# Crear usuario no-root
RUN useradd -m -s /bin/bash user
USER user

# Usa una ruta limpia y fija dentro del contenedor
WORKDIR /workspace

# Por defecto inicia con bash
CMD ["/bin/bash"]