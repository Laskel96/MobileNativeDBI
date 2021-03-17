FROM ubuntu:18.04

MAINTAINER Laskel <tls09611@gmail.com>

SHELL ["/bin/bash", "-c"]

# Install basic packages
RUN \
    apt update && \
    apt install -y wget unzip openjdk-8-jdk qemu-kvm python

# Gradle Settings
WORKDIR /
RUN \
    wget https://services.gradle.org/distributions/gradle-5.4.1-bin.zip -P /tmp && \
    unzip -d /opt/gradle /tmp/gradle-5.4.1-bin.zip

RUN \
    mkdir -p /opt/gradlew && \
    /opt/gradle/gradle-5.4.1/bin/gradle wrapper --gradle-version 5.4.1 --distribution-type all -p /opt/gradlew && \
    /opt/gradle/gradle-5.4.1/bin/gradle wrapper -p /opt/gradlew

# Android SDK Installation
# This Image targets Android API 27, x86 arch
RUN \
    wget 'https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip' -P /tmp && \
    unzip -d /opt/android /tmp/sdk-tools-linux-4333796.zip

RUN \
    yes y | /opt/android/tools/bin/sdkmanager --install "platform-tools" "system-images;android-22;google_apis;x86" "platforms;android-22" "build-tools;27.0.3" "emulator" && \
    yes y | /opt/android/tools/bin/sdkmanager --licenses

RUN \
    echo "no" | /opt/android/tools/bin/avdmanager --verbose create avd --force --name "test" --device "pixel" --package "system-images;android-22;google_apis;x86" \
    --tag "google_apis" --abi "x86"

# Setting Path
ENV GRADLE_HOME=/opt/gradle/gradle-5.4.1
ENV ANDROID_HOME=/opt/android
ENV PATH "$PATH:$GRADLE_HOME/bin:/opt/gradlew:$ANDROID_HOME/emulator:$ANDROID_HOME/tools/bin:$ANDROID_HOME/platform-tools"
ENV LD_LIBRARY_PATH "$ANDROID_HOME/emulator/lib64:$ANDROID_HOME/emulator/lib64/qt/lib"

# Install Native DBI
ADD valgrind /valgrind


# Make Sdcard for Emulator for future use
RUN mksdcard -l mySdcard 2048M /mySdcardFile.img
