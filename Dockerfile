FROM ubuntu:12.04
MAINTAINER Michal Papierski <michal@papierski.net
ADD simpledeamon-0.1.1-Linux.deb /tmp/
RUN dpkg -i /tmp/simpledeamon-0.1.1-Linux.deb
RUN mkdir -p /opt/simpledaemon/
WORKDIR /opt/simpledaemon/
VOLUME ["/opt/simpledaemon"]
EXPOSE 9876
ENTRYPOINT ["/usr/bin/simpledaemon"]