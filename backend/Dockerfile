FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake \
    libmysqlclient-dev libmysqlcppconn-dev \
    libssl-dev nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . /src
WORKDIR /src/build
RUN cmake .. && make -j$(nproc)

FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y \
    libmysqlclient21 libmysqlcppconn8t64 libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/output/oj_backend /app/oj_backend

WORKDIR /app
EXPOSE 8080

CMD ["./oj_backend"]
