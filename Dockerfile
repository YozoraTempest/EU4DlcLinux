FROM registry.gitlab.steamos.cloud/steamrt/sniper/sdk:3.0.20261007.263965
USER root
WORKDIR /usr/src/creamlinux
CMD ["sh", "./build.sh"]
