FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ python3 \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /tmp/judge

COPY judge_entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/judge_entrypoint.sh

ENTRYPOINT ["/usr/local/bin/judge_entrypoint.sh"]
