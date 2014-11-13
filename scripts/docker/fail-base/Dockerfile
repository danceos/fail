# Set the base image to Ubuntu Utopic (14.10)
FROM ubuntu:utopic

MAINTAINER Christian Dietrich <stettberger@dokucode.de>

# Install Packages required to build FAIL*
RUN apt-get update
RUN apt-get install -y  \
    binutils-dev        \
    build-essential     \
    cmake               \
    git                 \
    libboost-regex-dev  \
    libboost-system-dev \
    libboost-thread-dev \
    libdwarf-dev        \
    libelf-dev          \
    libfontconfig1-dev  \
    libiberty-dev       \
    libmysqlclient-dev  \
    libpcl1-dev         \
    libprotobuf-dev     \
    libsvga1-dev        \
    llvm-3.4-dev        \
    screen              \
    protobuf-compiler   \
    wget                \
    openssh-server      \
    vim                 \
    zlib1g-dev

# Add a user for compiling FAIL*
RUN useradd fail; mkdir /home/fail; chown fail /home/fail
RUN echo 'fail:fail' | chpasswd; chsh fail --shell /bin/bash
RUN adduser fail sudo

# SSH login fix. Otherwise user is kicked off after login
RUN mkdir /var/run/sshd
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

USER fail
ENV HOME /home/fail
WORKDIR /home/fail

# Get AspectC++ v1.2 for 64 Bit
RUN wget http://www.aspectc.org/releases/1.2/ac-bin-linux-x86-64bit-1.2.tar.gz
RUN tar xvzf ac-bin-linux-x86-64bit-1.2.tar.gz; mkdir bin; mv aspectc++/ac++ aspectc++/ag++ bin/; rm -rf aspectc++
ENV PATH /home/fail/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# Clone FAIL*
RUN git clone https://github.com/danceos/fail.git
WORKDIR fail

USER root
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
