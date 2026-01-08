# Stage 1: build fat jar with Java 21
FROM docker.io/library/eclipse-temurin:21-jdk AS builder
WORKDIR /src
COPY . .
RUN chmod +x ./gradlew && ./gradlew shadowJar --no-daemon

# Stage 2: final Fedora image
FROM docker.io/library/fedora:42
WORKDIR /opt/cagen

RUN dnf -y update && \
    dnf -y install java-21-openjdk-headless clang gcc-c++ make which && \
    dnf clean all && rm -rf /var/cache/dnf

COPY --from=builder /src/build/libs/ /opt/cagen/build/libs/
COPY --from=builder /src/examples/gasburner /opt/cagen/examples/gasburner
COPY --from=builder /src/examples/mine_pump /opt/cagen/examples/mine_pump
COPY --from=builder /src/examples/ecs /opt/cagen/examples/ecs

ENTRYPOINT ["java","-jar","/opt/cagen/build/libs/cagen-all.jar"]

