FROM python:3.7.2-alpine3.8

LABEL  maintainer="bukhtuev.ga@phystech.edu"

ENV ADMIN="NewThunder"

ENV Path=$(pwd)


RUN apk update && apk upgrade && apk add bash && apk add gcc

RUN apk add bash

ADD https://github.com/NThunder/SimpClientServer ./app

CMD ["gcc", "-o", "client", "./app/client.c"]

RUN ["gcc", "-o", "server", "./app/server.c"]

RUN ["./server", "80", Path]

RUN ["./client", "127.0.0.1", "80", "GET", "file"]

RUN ["./client"]

EXPOSE 80