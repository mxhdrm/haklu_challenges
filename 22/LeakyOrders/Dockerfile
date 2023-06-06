FROM ubuntu:22.04
ENV user ctf

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && \
    apt upgrade -y && \
    apt install python3 -y

RUN useradd ${user} -m -s /bin/bash
WORKDIR /chal
COPY main /chal/sigs
COPY solve.py /solve.py
ADD test.flag /flag
ADD solve /chal/solve
RUN chown root /flag
RUN chmod 400 /flag
RUN chown root /chal/sigs
RUN chmod u+s /chal/sigs

USER ${user}
CMD ["/bin/bash"]
