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
COPY --from=builder /src/examples /opt/cagen/examples
COPY --from=builder /src/artifact_evaluation /opt/cagen/artifact_evaluation
COPY --from=builder /src/README.md /opt/cagen/README.md

ENTRYPOINT ["java","-jar","/opt/cagen/build/libs/cagen-all.jar"]

